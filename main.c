#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>

/////////////
// Configs //
/////////////

#define BLOCK_SIZE 32
#define DEBUG 1

#define TAG_CONFIG 1
#define TAG_DATA 2

////////////
// Macros //
////////////

#ifdef DEBUG
#define dbg_print(fmt, ...) do { \
if (DEBUG) { \
fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
__FILE__, __LINE__, __func__, ##__VA_ARGS__); \
} \
} while (0)
#else
#define dbg_print(fmt, ...) do {} while (0)
#endif

#ifdef DEBUG
#define dbg_print_clean(fmt, ...) do { \
if (DEBUG) { \
fprintf(stderr, fmt, ##__VA_ARGS__); \
} \
} while (0)
#else
#define dbg_print_clean(fmt, ...) do {} while (0)
#endif

/////////////////////
// Types & Structs //
/////////////////////

// Since the value ranges from 0 to 99 an 8 bit int will be just enough. Using a signed to avoid accidental overflows
// while doing calculations, even though casts will be needed anyways.
typedef struct Point {
    int8_t x, y, z;
} Point;

typedef Point data_t;

typedef struct PartitionConfig {
    uint32_t n;
    uint32_t n_owned_cols;
    uint32_t* owned_cols;
} PartitionConfig;

typedef struct DatasetPartition {
    PartitionConfig config;
    // The data is represented in column major fashion, that is, an entire column in sequence
    // as this will be more useful for handling cache coherence and even work distribution
    data_t* data;
} DatasetPartition;

typedef struct PartitionChunk {
    uint32_t from_row;
    uint32_t to_row;
    data_t* data;
} PartitionChunk;

//////////////////
// Declarations //
//////////////////

void init_mpi_data();
void parse_args(int argc, char* argv[], uint32_t* n, uint32_t* seed, uint32_t* thread_count);
DatasetPartition* generate_distributed_matrix(uint32_t n, uint32_t seed, int cluster_size, int my_rank);

//////////
// Impl //
//////////

int main(int argc, char* argv[]) {
    // MPI Initialization
    MPI_Init(&argc, &argv);
    init_mpi_data();

    // Cli argument parsing
    uint32_t n;
    uint32_t seed;
    uint32_t thread_count;
    parse_args(argc, argv, &n, &seed, &thread_count);

    // Load MPI process information
    int cluster_size;
    int my_rank;

    // Load cluster size
    if (MPI_Comm_size(MPI_COMM_WORLD, &cluster_size) != MPI_SUCCESS) {
        fprintf(stderr, "Failed to retrieve MPI cluster size\n");
        MPI_Finalize();
        exit(1);
    }

    // Load rank
    if (MPI_Comm_rank(MPI_COMM_WORLD, &my_rank) != MPI_SUCCESS) {
        fprintf(stderr, "Failed to retrieve instance rank on MPI cluster\n");
        MPI_Finalize();
        exit(1);
    }

    // Debug process info
    dbg_print("Initialized process on MPI Cluster! Cluster size: %d; My rank: %d\n", cluster_size, my_rank);

    // Initialize data across cluster
    DatasetPartition* my_data = generate_distributed_matrix(n, seed, cluster_size, my_rank);

    if (DEBUG) {
        dbg_print("Partition %u data:\n", my_rank);
        for (uint32_t i = 0; i < my_data->config.n_owned_cols; i++) {
            dbg_print_clean("Column %u: [", my_data->config.owned_cols[i]);
            for (uint32_t j = 0; j < my_data->config.n; j++) {
                if (j != 0) {
                    dbg_print_clean(", ");
                }
                const Point p = my_data->data[i * my_data->config.n + j];
                dbg_print_clean("(%u, %u, %u)", p.x, p.y, p.z);
            }
            dbg_print_clean("]\n");
        }
    }

    // MPI Finishing & Cleanup
    MPI_Finalize();
    free(my_data->data);
    free(my_data);
    return 0;
}

/**
 * \brief Initiate global data associated to MPI, such as data types
 */
void init_mpi_data() {

}

/**
 * \brief Load the program arguments from the CLI args, providing reasonable defaults
 * if not available.
 * \param argc standard argc
 * \param argv standard argv
 * \param n pointer to the matrix rank variable
 * \param seed pointer to the seed variable
 * \param thread_count pointer to the thread count variable
 */
void parse_args(const int argc, char* argv[], uint32_t* n, uint32_t* seed, uint32_t* thread_count) {
    // Load matrix rank
    if (argc >= 2) {
        *n = strtoul(argv[1], NULL, 10);
        dbg_print("Received CLI arg n=%u", *n);
    }
    else {
        *n = 100;
        dbg_print("Using fallback CLI arg n=%u", *n);
    }

    // Load RNG seed
    if (argc >= 3) {
        *seed = strtoul(argv[2], NULL, 10);
        dbg_print("Received CLI arg seed=%u", *seed);
    }
    else {
        *seed = 1;
        dbg_print("Using fallback CLI arg seed=%u", *seed);
    }

    // Load local thread count to be used
    if (argc >= 4) {
        *thread_count = strtoul(argv[3], NULL, 10);
        dbg_print("Received CLI arg thread_count=%u", *thread_count);
    }
    else {
        *thread_count = omp_get_num_procs();
        dbg_print("Using fallback CLI arg thread_count=%u", *thread_count);
    }
}

/**
 * \brief Generates the point matrix dataset for the problem, distributing it alongside
 * all the nodes during execution. Returning this node's partition.
 * \param n the matrix rank
 * \param seed the RNG seed
 * \param cluster_size the number of nodes in the cluster
 * \param my_rank my rank in the cluster
 * \return this node's partition of the data
 */
DatasetPartition* generate_distributed_matrix(
    const uint32_t n,
    const uint32_t seed,
    const int cluster_size,
    const int my_rank
) {
    DatasetPartition* my_data = malloc(sizeof(DatasetPartition));

    // The generator node
    if (my_rank == 0) {
        // Seed RNG
        srand(seed);

        dbg_print("Creating partition configurations");

        // Currently generating block (since, theoretically, not all data can be held by a single node)
        // Represented in row major fashion to suit the problem requirements
        data_t* work_data = malloc(n * BLOCK_SIZE * sizeof(data_t));

        // Configuration for each node
        PartitionConfig* configs = malloc(cluster_size * sizeof(PartitionConfig));

        /**
         * Partiotining strategy: in order to ensure a more even workload distribution accross all members,
         * we are distributing the data in columns. One important aspect to notice is that the later columns
         * need to do less work overall. As a mitigation, we are employing two strategies: firstly we round-robin
         * each column across the nodes, instead of allocating sequential ones. Secondly, we alternate the order,
         * so that in the first node in batch will be the last one in the next.
         */

        // Over allocate one column for some nodes, since it shouldn't matter much and predicting
        // which nodes will own one extra column would result in more complex code.
        const uint32_t base_size = n / cluster_size + 1;

        // Base initialization of configs
        for (uint32_t i = 0; i < cluster_size; i++) {
            configs[i].n = n;
            configs[i].n_owned_cols = 0;
            configs[i].owned_cols = malloc(base_size * sizeof(uint32_t));
        }

        // Assign the columns to each node
        uint32_t node = 0;
        for (uint32_t i = 0; i < n; i++) {
            const uint32_t intra_idx = i / cluster_size;
            configs[node].n_owned_cols++;
            configs[node].owned_cols[intra_idx] = i;

            dbg_print("Assigned column (%u) to node %u\n", i, node);

            // Alternate iteration order
            if (i % cluster_size == 0) {
            } else if (i / cluster_size % 2 == 0) {
                node++;
            } else {
                node--;
            }
        }

        // Load local partition config (this operation could be avoided by conditionally writing in the lines above,
        // but the readability impact is not worth it, might change it still, tho).
        memcpy(&my_data->config, &configs[0], sizeof(PartitionConfig));

        // Transfer data as an array
        uint32_t* config_buffer = malloc((base_size + 2) * sizeof(uint32_t));

        // Send the partition configs to the other nodes
        for (int i = 1; i < cluster_size; i++) {
            // Initialize transfer data buffer
            config_buffer[0] = configs[i].n;
            config_buffer[1] = configs[i].n_owned_cols;
            memcpy(&config_buffer[2], configs[i].owned_cols, sizeof(uint32_t) * configs[i].n_owned_cols);

            // Send the configs in a single shot
            MPI_Send(config_buffer, configs[i].n_owned_cols + 2, MPI_UNSIGNED, i, TAG_CONFIG, MPI_COMM_WORLD);
        }

        // Allocate my data partition
        my_data->data = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));

        // Number of blocks to be sent
        const uint32_t block_count = (n - 1) / BLOCK_SIZE + 1;

        // Allocate buffer for data sending
        uint8_t* data_buffer = malloc(base_size * n * sizeof(data_t) + 2  * sizeof(uint32_t));

        // Generate & send blocks
        for (uint32_t i = 0; i < block_count; i++) {
            const uint32_t block_start = i * BLOCK_SIZE;
            const uint32_t block_end =  (n < block_start + BLOCK_SIZE ? n : block_start + BLOCK_SIZE) - 1;
            const uint32_t actual_block_size = block_end - block_start + 1;

            for (uint32_t j = 0; j < actual_block_size; j++) {
                // Generate full row
                for (uint32_t k = 0; k < n; k++) {
                    // This RNG won't really be uniform due to rand's range being between 0 to 2147483647
                    work_data[j * n + k] = (Point){
                        .x = rand() % 100,
                        .y = rand() % 100,
                        .z = rand() % 100
                    };
                }
            }

            // Handle root
            dbg_print("Writing root rows from %u to %u\n", block_start, block_end);
            for (uint32_t j = 0; j < my_data->config.n_owned_cols; j++) {
                const uint32_t cur_col = my_data->config.owned_cols[j];
                const size_t col_offset = n * j * sizeof(data_t);
                // Iterate block rows, copying the elements to each column
                for (uint32_t k = 0; k < actual_block_size; k++) {
                    const data_t target = work_data[n * k + cur_col];
                    const size_t cur_offset = col_offset + (block_start + k) * sizeof(data_t);
                    memcpy(my_data->data + cur_offset, &target, sizeof(data_t));
                }
            }

            // Write start info in byte form
            data_buffer[0] = (uint8_t) (block_start >> 24);
            data_buffer[1] = (uint8_t) (block_start >> 16);
            data_buffer[2] = (uint8_t) (block_start >> 8);
            data_buffer[3] = (uint8_t) (block_start >> 0);
            // Write end info in byte form
            data_buffer[4] = (uint8_t) (block_end >> 24);
            data_buffer[5] = (uint8_t) (block_end >> 16);
            data_buffer[6] = (uint8_t) (block_end >> 8);
            data_buffer[7] = (uint8_t) (block_end >> 0);
            // Send data to each node
            for (uint32_t j = 1; j < cluster_size; j++) {
                dbg_print("Sending cluster %u rows from %u to %u\n", j, block_start, block_end);

                const size_t data_buffer_offset = 8;
                // Build node-specific column buffer for current rows
                for (uint32_t k = 0; k < configs[j].n_owned_cols; k++) {
                    const uint32_t cur_col = configs[j].owned_cols[k];
                    // Iterate block rows
                    for (uint32_t l = 0; l < actual_block_size; l++) {
                        // Memory offset calculation
                        const size_t cur_offset = data_buffer_offset + (k * actual_block_size + l) * sizeof(data_t);
                        const data_t target = work_data[n * l + cur_col];
                        // Copy a point at a time (can be improved by changing the generation to write inside the block
                        // in column major fashion, avoided initially to make it a bit less confusing to the reader, by
                        // doing such it would be possible to copy a whole column block at a time)
                        memcpy(data_buffer + cur_offset, &target, sizeof(data_t));
                    }
                }
                // Send the current block data to it's node
                MPI_Send(data_buffer, data_buffer_offset + configs[j].n_owned_cols * actual_block_size * sizeof(data_t), MPI_BYTE, j, TAG_DATA, MPI_COMM_WORLD);
            }
        }

        // Cleanup of temp data
        for (uint32_t i = 0; i < cluster_size; i++) {
            free(configs[i].owned_cols);
        }
        free(configs);
        free(work_data);
        // free(data_buffer);
        free(config_buffer);
        return my_data;
    }

    /////////////////////////
    // The Receiving Nodes //
    /////////////////////////

    // Wait for incoming configs
    int32_t config_len;
    MPI_Status config_status;
    MPI_Probe(0, TAG_CONFIG, MPI_COMM_WORLD, &config_status);
    MPI_Get_count(&config_status, MPI_UNSIGNED, &config_len);

    // Receive configs
    uint32_t* config_buffer = malloc(config_len * sizeof(uint32_t));
    MPI_Recv(config_buffer, config_len, MPI_UNSIGNED, 0, TAG_CONFIG, MPI_COMM_WORLD, &config_status);

    // Load config data
    my_data->config.n = config_buffer[0];
    my_data->config.n_owned_cols = config_buffer[1];
    my_data->config.owned_cols = malloc(my_data->config.n_owned_cols * sizeof(uint32_t));
    memcpy(my_data->config.owned_cols, &config_buffer[2], sizeof(uint32_t) * my_data->config.n_owned_cols);

    // Allocate my data partition
    my_data->data = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));

    // Receive this partition's data
    MPI_Status data_status;
    int32_t data_size;
    uint8_t* data_buffer = malloc(sizeof(uint32_t) * 2 + BLOCK_SIZE * my_data->config.n_owned_cols * sizeof(data_t));
    uint32_t received_rows = 0;

    while (received_rows < n) {
        // Wait for message data
        MPI_Probe(0, TAG_DATA, MPI_COMM_WORLD, &data_status);
        MPI_Get_count(&data_status, MPI_BYTE, &data_size);

        // Receive it
        MPI_Recv(data_buffer, data_size, MPI_BYTE, 0, TAG_DATA, MPI_COMM_WORLD, &data_status);

        // Parse block boundaries
        const uint32_t block_start = (uint32_t) data_buffer[0] << 24
                                + (uint32_t) data_buffer[1] << 16
                                + (uint32_t) data_buffer[2] << 8
                                + (uint32_t) data_buffer[3] << 0;
        const uint32_t block_end = (uint32_t) data_buffer[4] << 24
                                + (uint32_t) data_buffer[5] << 16
                                + (uint32_t) data_buffer[6] << 8
                                + (uint32_t) data_buffer[7] << 0;
        const uint32_t actual_block_size = block_end - block_start + 1;
        received_rows += actual_block_size;

        dbg_print("Receiving rows from %u to %u\n", block_start, block_end);

        // Load block data from buffer
        for (uint32_t i = 0; i < my_data->config.n_owned_cols; i++) {
            const size_t cur_offset = (i * my_data->config.n + block_start) * sizeof(size_t);
            memcpy(my_data->data + cur_offset, &data_buffer[8], actual_block_size * sizeof(data_t));
        }
    }

    free(data_buffer);
    free(config_buffer);
    return my_data;
}
