#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>

// Declarations //

int manhattan_distance(int x1, int y1, int z1, int x2, int y2, int z2);
double euclidean_distance(int x1, int y1, int z1, int x2, int y2, int z2);
void parse_args(int argc, char* argv[], unsigned* n, unsigned* seed);

int main(int argc, char* argv[])
{
    unsigned n, seed;
    parse_args(argc, argv, &n, &seed);

    int *x = malloc(n * n * sizeof(int));
    int *y = malloc(n * n * sizeof(int));
    int *z = malloc(n * n * sizeof(int));

    int* targets[3] = {
        x,
        y,
        z
    };

    srand(seed);
    for(int o = 0; o < 3; o++) {
        for(int i = 0; i < n * n; i++) {
            targets[o][i] = rand() % 100;
        }
    }

    int ij, k;
    int manhattan_dist;
    double euclidean_dist;
    
    int min_manhattan_per_point, min_manhattan = INT_MAX, sum_min_manhattan = 0;
    int max_manhattan_per_point, max_manhattan = 0, sum_max_manhattan = 0;
    double min_euclidean_per_point, min_euclidean = DBL_MAX, sum_min_euclidean = 0.0;
    double max_euclidean_per_point, max_euclidean = 0.0, sum_max_euclidean = 0.0;

    // Compute the solution for each point, then reduce
    for (ij = 0; ij < n*n; ij++)
    {
        min_manhattan_per_point = INT_MAX;
        max_manhattan_per_point = 0;
        min_euclidean_per_point = DBL_MAX;
        max_euclidean_per_point = 0;

        // Compute sub-solutions to each point
        for (k = ij+1; k < n*n; k++)
        {
            manhattan_dist = manhattan_distance(x[ij], y[ij], z[ij], x[k], y[k], z[k]);
            euclidean_dist = euclidean_distance(x[ij], y[ij], z[ij], x[k], y[k], z[k]);

            // acerta os mínimos e os máximos locais ao ponto de origem (um i,j)
            if (manhattan_dist < min_manhattan_per_point)
            {
                min_manhattan_per_point = manhattan_dist;
            }

            if (manhattan_dist > max_manhattan_per_point) 
            {
                max_manhattan_per_point = manhattan_dist;
            }
                    
            if (euclidean_dist < min_euclidean_per_point) 
            {
                min_euclidean_per_point = euclidean_dist;
            }
                    
            if (euclidean_dist > max_euclidean_per_point) 
            {
                max_euclidean_per_point = euclidean_dist;
            }
            
        }

        // Reduce
        if (min_manhattan_per_point < min_manhattan)
            min_manhattan = min_manhattan_per_point;

        if (max_manhattan_per_point > max_manhattan) 
            max_manhattan = max_manhattan_per_point;
                
        if (min_euclidean_per_point < min_euclidean) 
            min_euclidean = min_euclidean_per_point;
            
        if (max_euclidean_per_point > max_euclidean) 
            max_euclidean = max_euclidean_per_point;
            
        if (min_manhattan_per_point != INT_MAX && min_euclidean_per_point != DBL_MAX)
        {
            sum_min_manhattan += min_manhattan_per_point;
            sum_max_manhattan += max_manhattan_per_point;
            sum_min_euclidean += min_euclidean_per_point;
            sum_max_euclidean += max_euclidean_per_point;
        }
    }

    printf("Distância de Manhattan mínima: %d (soma min: %d) e máxima: %d (soma max: %d).\n", min_manhattan, sum_min_manhattan, max_manhattan, sum_max_manhattan);
    printf("Distância Euclidiana mínima: %.2lf (soma min: %.2lf) e máxima: %.2lf (soma max: %.2lf).\n", min_euclidean, sum_min_euclidean, max_euclidean, sum_max_euclidean);
  
    return 0;
}

int manhattan_distance(int x1, int y1, int z1, int x2, int y2, int z2)
{
    return abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2);
}

double euclidean_distance(int x1, int y1, int z1, int x2, int y2, int z2)
{
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}

void parse_args(int argc, char* argv[], unsigned* n, unsigned* seed) {
    // Load matrix rank
    if (argc >= 2) {
        *n = strtoul(argv[1], NULL, 10);
    }
    else {
        *n = 10;
    }

    // Load RNG seed
    if (argc >= 3) {
        *seed = strtoul(argv[2], NULL, 10);
    }
    else {
        *seed = 1;
    }
}
