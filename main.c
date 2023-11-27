#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>

// TODO: check for success on every MPI call and malloc

/////////////
// Configs //
/////////////

#define BLOCK_SIZE 32
#define DEBUG 1

#define TAG_CONFIG 1
#define TAG_DATA 2
#define TAG_COMPUTE_REQUEST 3
#define TAG_COMPUTE_RESULT 4
#define TAG_FINAL_RESULT 5

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

#define errprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define INFINITY (1.0 / 0.0)

/////////////////////
// Types & Structs //
/////////////////////

// Since the value ranges from 0 to 99 an 8 bit int will be just enough. Using a signed to avoid accidental overflows
// while doing calculations, even though casts will be needed anyways.
typedef int8_t data_t;

typedef struct PartitionConfig {
    uint32_t n;
    uint32_t n_owned_cols;
    uint32_t* owned_cols;
} PartitionConfig;

typedef struct DatasetPartition {
    PartitionConfig config;
    // The data is represented in column major fashion, that is, an entire column in sequence
    // as this will be more useful for handling cache coherence and even work distribution
    // Use SoA for better SIMD usage
    data_t* x;
    data_t* y;
    data_t* z;
} DatasetPartition;

typedef struct PartitionChunk {
    uint32_t from_row;
    uint32_t to_row;
    data_t* data;
} PartitionChunk;

typedef struct ComputeTargetRequest {
    uint32_t idx;
    data_t x;
    data_t y;
    data_t z;
} ComputeTargetRequest;

// Keep all the euclideans squared, as we can calculate the SQRT only once at the end
typedef struct ComputeTargetResult {
    uint32_t idx;
    data_t x;
    data_t y;
    data_t z;

    uint32_t min_euclidean;
    uint32_t max_euclidean;
    uint32_t min_manhattan;
    uint32_t max_manhattan;
} ComputeTargetResult;

typedef struct ComputeTargetResultLocal {
    uint32_t min_euclidean;
    uint32_t max_euclidean;
    uint32_t min_manhattan;
    uint32_t max_manhattan;
} ComputeTargetResultLocal;

typedef struct ComputeResult {
    uint32_t min_euclidean;
    uint32_t max_euclidean;
    uint32_t min_manhattan;
    uint32_t max_manhattan;

    uint64_t sum_min_euclidean;
    uint64_t sum_max_euclidean;
    uint64_t sum_min_manhattan;
    uint64_t sum_max_manhattan;
} ComputeResult;

//////////////////
// Declarations //
//////////////////

void parse_args(int argc, char* argv[], uint32_t* n, uint32_t* seed, uint32_t* thread_count);
DatasetPartition* generate_distributed_matrix(uint32_t n, uint32_t seed, int cluster_size, int my_rank);
ComputeResult compute(const DatasetPartition* data, int my_rank, int cluster_size);
void compute_point(const DatasetPartition* data, ComputeTargetResult* target_result);

// Fast, branch-less, ABS calculation for integers
// Src: https://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs
static uint8_t fast_abs(const int8_t v) {
    const uint8_t mask = v >> 7;
    return v + mask ^ mask;
}

//////////
// Impl //
//////////

int main(int argc, char* argv[]) {
    // MPI Initialization
    MPI_Init(&argc, &argv);

    // Cli argument parsing
    uint32_t n;
    uint32_t seed;
    uint32_t thread_count;
    parse_args(argc, argv, &n, &seed, &thread_count);

    // OpenMP configs
    omp_set_nested(1);
    omp_set_num_threads(thread_count);

    // Load MPI process information
    int cluster_size;
    int my_rank;

    // Load cluster size
    if (MPI_Comm_size(MPI_COMM_WORLD, &cluster_size) != MPI_SUCCESS) {
        errprintf("Failed to retrieve MPI cluster size\n");
        MPI_Finalize();
        exit(1);
    }

    // Load rank
    if (MPI_Comm_rank(MPI_COMM_WORLD, &my_rank) != MPI_SUCCESS) {
        errprintf("Failed to retrieve instance rank on MPI cluster\n");
        MPI_Finalize();
        exit(1);
    }

    // Debug process info
    dbg_print("Initialized process on MPI Cluster! Cluster size: %d; My rank: %d\n", cluster_size, my_rank);

    // Initialize data across cluster
    DatasetPartition* my_data = generate_distributed_matrix(n, seed, cluster_size, my_rank);

    // Print the current node partition data (when DEBUG is enabled)
    if (DEBUG == 2) {
        dbg_print("Partition %u data:\n", my_rank);
        for (uint32_t i = 0; i < my_data->config.n_owned_cols; i++) {
            dbg_print_clean("[Node %d] Column %u: [", my_rank, my_data->config.owned_cols[i]);
            for (uint32_t j = 0; j < my_data->config.n; j++) {
                if (j != 0) {
                    dbg_print_clean(", ");
                }
                const size_t idx = i * my_data->config.n + j;
                dbg_print_clean("(%u, %u, %u)", my_data->x[idx], my_data->y[idx], my_data->z[idx]);
            }
            dbg_print_clean("]\n");
        }
    }

    compute(my_data, my_rank, cluster_size);

    // MPI Finishing & Cleanup
    MPI_Finalize();
    free(my_data->x);
    free(my_data->y);
    free(my_data->z);
    free(my_data);
    return 0;
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
        dbg_print("Received CLI arg n=%u\n", *n);
    }
    else {
        *n = 10;
        dbg_print("Using fallback CLI arg n=%u\n", *n);
    }

    // Load RNG seed
    if (argc >= 3) {
        *seed = strtoul(argv[2], NULL, 10);
        dbg_print("Received CLI arg seed=%u\n", *seed);
    }
    else {
        *seed = 1;
        dbg_print("Using fallback CLI arg seed=%u\n", *seed);
    }

    // Load local thread count to be used
    if (argc >= 4) {
        *thread_count = strtoul(argv[3], NULL, 10);
        dbg_print("Received CLI arg thread_count=%u\n", *thread_count);
    }
    else {
        *thread_count = omp_get_num_procs();
        dbg_print("Using fallback CLI arg thread_count=%u\n", *thread_count);
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
    const size_t buffer_prefix_offset = 8;

    DatasetPartition* my_data = malloc(sizeof(DatasetPartition));
    if (my_data == NULL) {
        errprintf("Failed to alloc local partition data");
        exit(1);
    }

    // The generator node
    if (my_rank == 0) {
        // Seed RNG
        srand(seed);

        dbg_print("Creating partition configurations\n");

        // Currently generating block (since, theoretically, not all data can be held by a single node)
        // Represented in row major fashion to suit the problem requirements
        data_t* work_data = malloc(n * BLOCK_SIZE * sizeof(data_t));
        if (work_data == NULL) {
            errprintf("Failed to alloc work_data buffer (matrix generation)");
            exit(1);
        }

        // Configuration for each node
        PartitionConfig* configs = malloc(cluster_size * sizeof(PartitionConfig));
        if (configs == NULL) {
            errprintf("Failed to alloc configs temp array (matrix generation)");
            exit(1);
        }

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
            if (configs[i].owned_cols == NULL) {
                errprintf("Failed to alloc configs[%u] owned collumns array (matrix generation)", i);
                exit(1);
            }
        }

        // Assign the columns to each node
        int32_t node = 0;
        for (uint32_t i = 0; i < n; i++) {
            const uint32_t intra_idx = configs[node].n_owned_cols;
            if (DEBUG == 2) {
                dbg_print("Assigned column (%u @ %u) to node %u\n", i, intra_idx, node);
            }
            configs[node].n_owned_cols++;
            configs[node].owned_cols[intra_idx] = i;

            // Alternate iteration order
            if (cluster_size != 1) {
                if (i / cluster_size % 2 == 0) {
                    if (node != cluster_size - 1) {
                        node++;
                    }
                } else {
                    if (node != 0) {
                        node--;
                    }
                }
            }
        }

        // Load local partition config (this operation could be avoided by conditionally writing in the lines above,
        // but the readability impact is not worth it, might change it still, tho).
        memcpy(&my_data->config, &configs[0], sizeof(PartitionConfig));
        my_data->config.owned_cols = malloc(my_data->config.n_owned_cols * sizeof(uint32_t));
        memcpy(my_data->config.owned_cols, configs[0].owned_cols, my_data->config.n_owned_cols * sizeof(uint32_t));

        // Transfer data as an array
        uint32_t* config_buffer = malloc((base_size + 2) * sizeof(uint32_t));
        if (config_buffer == NULL) {
            errprintf("Failed to alloc config buffer for MPI transfer (matrix generation)");
            exit(1);
        }

        // Send the partition configs to the other nodes
        for (int i = 1; i < cluster_size; i++) {
            // Initialize transfer data buffer
            config_buffer[0] = configs[i].n;
            config_buffer[1] = configs[i].n_owned_cols;
            memcpy(&config_buffer[2], configs[i].owned_cols, sizeof(uint32_t) * configs[i].n_owned_cols);

            // Send the configs in a single shot
            MPI_Send(config_buffer, configs[i].n_owned_cols + 2, MPI_UNSIGNED, i, TAG_CONFIG, MPI_COMM_WORLD);
        }

        // Allocate my x partition
        my_data->x = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));
        if (my_data->x == NULL) {
            errprintf("Failed to alloc local x partition (matrix generation)");
            exit(1);
        }

        // Allocate my y partition
        my_data->y = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));
        if (my_data->y == NULL) {
            errprintf("Failed to alloc local y partition (matrix generation)");
            exit(1);
        }

        // Allocate my z partition
        my_data->z = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));
        if (my_data->z == NULL) {
            errprintf("Failed to alloc local z partition (matrix generation)");
            exit(1);
        }

        // Number of blocks to be sent
        const uint32_t block_count = (n - 1) / BLOCK_SIZE + 1;

        // Allocate buffer for data sending
        uint8_t* data_buffer = malloc(base_size * n * sizeof(data_t) + 2  * sizeof(uint32_t));
        if (data_buffer == NULL) {
            errprintf("Failed to alloc data buffer for MPI transfer (matrix generation)");
            exit(1);
        }

        data_t* my_dests[] = {
            my_data->x,
            my_data->y,
            my_data->z
        };

        // Generate & send blocks
        for (uint8_t o = 0; o < 3; o++) {
            for (uint32_t i = 0; i < block_count; i++) {
                const uint32_t block_start = i * BLOCK_SIZE;
                const uint32_t block_end =  (n < block_start + BLOCK_SIZE ? n : block_start + BLOCK_SIZE) - 1;
                const uint32_t actual_block_size = block_end - block_start + 1;

                for (uint32_t j = 0; j < actual_block_size; j++) {
                    // Generate full row
                    for (uint32_t k = 0; k < n; k++) {
                        // This RNG won't really be uniform due to rand's range being between 0 to 2147483647
                        work_data[k * n + j] = rand() % 100;
                    }
                }

                // Handle root
                dbg_print("Writing root rows from %u to %u\n", block_start, block_end);
                for (uint32_t j = 0; j < my_data->config.n_owned_cols; j++) {
                    const uint32_t cur_col = my_data->config.owned_cols[j];
                    // Iterate block rows, copying the elements to each column
                    const size_t dest_offset = j * n + block_start;
                    const size_t src_offset= n * cur_col;
                    memcpy(my_dests[o] + dest_offset, work_data + src_offset, actual_block_size * sizeof(data_t));
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

                    // Build node-specific column buffer for current rows
                    for (uint32_t k = 0; k < configs[j].n_owned_cols; k++) {
                        // Memory offset calculation
                        const uint32_t cur_col = configs[j].owned_cols[k];
                        const size_t dest_offset = buffer_prefix_offset + k * actual_block_size * sizeof(data_t);
                        const size_t src_offset = n * cur_col;
                        // Copy all the rows of column at a time
                        memcpy(data_buffer + dest_offset, work_data + src_offset, actual_block_size * sizeof(data_t));
                    }
                    // Send the current block data to it's node
                    MPI_Send(data_buffer, buffer_prefix_offset + configs[j].n_owned_cols * actual_block_size * sizeof(data_t), MPI_BYTE, j, TAG_DATA, MPI_COMM_WORLD);
                }
            }
        }

        // Cleanup of temp data
        for (uint32_t i = 0; i < cluster_size; i++) {
            free(configs[i].owned_cols);
        }
        free(configs);
        free(work_data);
        free(data_buffer);
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
    if (config_buffer == NULL) {
        errprintf("Failed to alloc config buffer for MPI transfer (matrix generation)");
        exit(1);
    }

    MPI_Recv(config_buffer, config_len, MPI_UNSIGNED, 0, TAG_CONFIG, MPI_COMM_WORLD, &config_status);

    // Load config data
    my_data->config.n = config_buffer[0];
    my_data->config.n_owned_cols = config_buffer[1];
    my_data->config.owned_cols = malloc(my_data->config.n_owned_cols * sizeof(uint32_t));
    if (my_data->config.owned_cols == NULL) {
        errprintf("Failed to alloc owned cols array (matrix generation)");
        exit(1);
    }

    memcpy(my_data->config.owned_cols, &config_buffer[2], sizeof(uint32_t) * my_data->config.n_owned_cols);

    if (DEBUG) {
        dbg_print("I'm node %d and received the configs: n=%u n_owned_cols=%u\n", my_rank, my_data->config.n, my_data->config.n_owned_cols);
        dbg_print_clean("owned_cols=[");
        for (uint32_t i = 0; i < my_data->config.n_owned_cols; i++) {
            if (i != 0) {
                dbg_print_clean(", ");
            }
            dbg_print_clean("%u", my_data->config.owned_cols[i]);
        }
        dbg_print_clean("]\n");
    }

    // Allocate my x partition
    my_data->x = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));
    if (my_data->x == NULL) {
        errprintf("Failed to alloc local x partition (matrix generation)");
        exit(1);
    }

    // Allocate my y partition
    my_data->y = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));
    if (my_data->y == NULL) {
        errprintf("Failed to alloc local y partition (matrix generation)");
        exit(1);
    }

    // Allocate my z partition
    my_data->z = malloc(my_data->config.n * my_data->config.n_owned_cols * sizeof(data_t));
    if (my_data->z == NULL) {
        errprintf("Failed to alloc local z partition (matrix generation)");
        exit(1);
    }

    // Utility for looping
    data_t* my_dests[] = {
        my_data->x,
        my_data->y,
        my_data->z
    };

    // Receive this partition's data
    MPI_Status data_status;
    int32_t data_size;
    uint8_t* data_buffer = malloc(sizeof(uint32_t) * 2 + BLOCK_SIZE * my_data->config.n_owned_cols * sizeof(data_t));
    if (data_buffer == NULL) {
        errprintf("Failed to alloc data buffer for MPI transfer (matrix generation)");
        exit(1);
    }

    for (uint8_t o = 0; o < 3; o++) {
        uint32_t received_rows = 0;
        while (received_rows < n) {
            // Wait for message data
            MPI_Probe(0, TAG_DATA, MPI_COMM_WORLD, &data_status);
            MPI_Get_count(&data_status, MPI_BYTE, &data_size);

            // Receive it
            MPI_Recv(data_buffer, data_size, MPI_BYTE, 0, TAG_DATA, MPI_COMM_WORLD, &data_status);

            // Parse block boundaries
            const uint32_t block_start = ((uint32_t) data_buffer[0] << 24)
                                    + ((uint32_t) data_buffer[1] << 16)
                                    + ((uint32_t) data_buffer[2] << 8)
                                    + ((uint32_t) data_buffer[3] << 0);
            const uint32_t block_end = ((uint32_t) data_buffer[4] << 24)
                                    + ((uint32_t) data_buffer[5] << 16)
                                    + ((uint32_t) data_buffer[6] << 8)
                                    + ((uint32_t) data_buffer[7] << 0);
            const uint32_t actual_block_size = block_end - block_start + 1;
            received_rows += actual_block_size;

            dbg_print("[Node %d] Receiving rows[%u] from %u to %u\n", my_rank, (uint32_t) o, block_start, block_end);

            // Load block data from buffer
            for (uint32_t i = 0; i < my_data->config.n_owned_cols; i++) {
                const size_t data_offset = i * my_data->config.n + block_start;
                const size_t buffer_offset = buffer_prefix_offset + i * actual_block_size * sizeof(data_t);
                memcpy(my_dests[o] + data_offset, data_buffer + buffer_offset, actual_block_size * sizeof(data_t));
            }
        }
    }

    free(data_buffer);
    free(config_buffer);
    return my_data;
}

ComputeResult compute(
    const DatasetPartition* data,
    int my_rank,
    int cluster_size
) {
    ComputeResult compute_result = {
        .min_euclidean = UINT32_MAX,
        .max_euclidean = 0,
        .min_manhattan = UINT32_MAX,
        .max_manhattan = 0,

        .sum_min_euclidean = 0,
        .sum_max_euclidean = 0,
        .sum_min_manhattan = 0,
        .sum_max_manhattan = 0
    };

    // Request/Response data
    const size_t full_size = data->config.n * data->config.n_owned_cols;

    // Represent every request as two integers: the index + the x,y,z coords packed in a single 32 bit integer
    uint32_t* requests_buffer = malloc(full_size * 2 * sizeof(uint32_t));

    ComputeTargetResultLocal* local_results = malloc(full_size * sizeof(ComputeTargetResultLocal));
    ComputeTargetResultLocal* external_results = malloc(full_size * sizeof(ComputeTargetResultLocal));
    uint32_t received = 0;

    // Initialize the data
    // TODO: OMP here (SIMD + parallel for)
    for (size_t i = 0; i < full_size; i++) {
        const uint32_t row = i % data->config.n;
        const uint32_t col = data->config.owned_cols[i / data->config.n];
        const uint32_t global_idx = col * data->config.n + row;

        requests_buffer[2*i] = global_idx;
        requests_buffer[2*i + 1] = (uint32_t) data->x[i] << 16 | (uint32_t) data->x[i] << 8 | (uint32_t) data->x[i] << 0;


        local_results[i] = external_results[i] = (ComputeTargetResultLocal){
            .min_euclidean = UINT32_MAX,
            .max_euclidean = 0,
            .min_manhattan = UINT32_MAX,
            .max_manhattan =  0
        };
    }

    // Total number of broadcasts needed to send
    uint32_t block_sends = (full_size - 1) / BLOCK_SIZE + 1;
    MPI_Request sends[block_sends];

    #pragma omp parallel
    {
        #pragma omp for nowait
        for (uint32_t i = 0; i < full_size; i++) {
            // dbg_print("[Node %d - Thread %d] Computing the results for point %u\n", my_rank, omp_get_thread_num(), i);
            const uint32_t row = i % data->config.n;
            const uint32_t col = data->config.owned_cols[i / data->config.n];
            const uint32_t global_idx = col * data->config.n + row;

            ComputeTargetResult result = (ComputeTargetResult){
                .idx = global_idx,
                .x = data->x[i],
                .y = data->y[i],
                .z = data->z[i],
                .min_manhattan = UINT32_MAX,
                .max_manhattan = 0,
                .min_euclidean = UINT32_MAX,
                .max_euclidean = UINT32_MAX
            };

            compute_point(data, &result);

            local_results[i].min_euclidean = result.min_euclidean;
            local_results[i].max_euclidean = result.max_euclidean;
            local_results[i].min_manhattan = result.min_manhattan;
            local_results[i].max_manhattan = result.max_manhattan;
        }

        #pragma omp single
        {
            // Publish points to be computed
            #pragma omp task
            {
                for (uint32_t i = 0; i < full_size; i += BLOCK_SIZE) {
                    const uint32_t start_block = i * 2;
                    const uint32_t end_block = (i + BLOCK_SIZE > full_size ?  full_size : i + BLOCK_SIZE) - 1;
                    const uint32_t actual_size = end_block - start_block + 1;
                    MPI_Ibcast(requests_buffer, actual_size * 2, MPI_UNSIGNED, my_rank, MPI_COMM_WORLD, &sends[i / BLOCK_SIZE]);
                }
            }

            // Receive remotes

        }
    }
    // Task 2: Send remote computes
    // Task 3: Receive remote computes

    // Reduce everything
    // Send to primary


    free(requests_buffer);
    free(local_results);
    return compute_result;
}

void compute_point(
    const DatasetPartition* data,
    ComputeTargetResult* target_result
) {
    const uint32_t target_row = target_result->idx % data->config.n;
    const uint32_t target_col = target_result->idx / data->config.n;

    // Run a loop for each column. Could be paralelized, but there is not much benefit into it, since it's better to
    // paralelize the call to this entire function for each point.
    for (uint32_t i = 0; i < data->config.n_owned_cols; i++) {
        const uint32_t col = data->config.owned_cols[i];
        const size_t col_offset = data->config.n * i;
        const uint32_t st_row = target_row + (col < target_col ? 1 : 0);

        // All those loop breaks might look overkill or dumb, but it really does matter for the optimizer to apply SIMD
        // optimizations better. (Trust me, I disassembled the code and checked, luls)

        // Use local buffers for better SIMD
        int8_t xd[data->config.n];
        int8_t yd[data->config.n];
        int8_t zd[data->config.n];

        // Calculate differences
        #pragma omp simd
        for (uint32_t j = st_row; j < data->config.n; j++) {
            xd[j] = target_result->x - data->x[col_offset + j];
            yd[j] = target_result->y - data->y[col_offset + j];
            zd[j] = target_result->z - data->z[col_offset + j];
        }

        // Calculate differences abs
        #pragma omp simd
        for (uint32_t j = st_row; j < data->config.n; j++) {
            xd[j] = fast_abs(xd[j]);
            yd[j] = fast_abs(yd[j]);
            zd[j] = fast_abs(zd[j]);
        }

        // Distance Reduction
        #pragma omp simd
        for (uint32_t j = st_row; j < data->config.n; j++) {
            // Manhattan
            const uint32_t manhattan = (uint32_t) xd[j] + yd[j] + zd[j];
            target_result->min_manhattan = manhattan < target_result->min_manhattan ? manhattan : target_result->min_manhattan;
            target_result->max_manhattan = manhattan < target_result->max_manhattan ? manhattan : target_result->max_manhattan;

            // Euclidean
            const uint32_t euclidean = (uint32_t) xd[j] * xd[j] + (uint32_t) yd[j] * yd[j] + (uint32_t) zd[j] * zd[j];
            target_result->min_euclidean = euclidean < target_result->min_euclidean ? euclidean : target_result->min_euclidean;
            target_result->max_euclidean = euclidean < target_result->max_euclidean ? euclidean : target_result->max_euclidean;
        }
    }
}

// ComputeResult compute_local(
//     DatasetPartition* data,
//     int my_rank,
//     int cluster_size
// ) {
//
// }
//
// ComputeResult compute_remote(
//     DatasetPartition* data,
//     int my_rank,
//     int cluster_size
// ) {
//
// }
