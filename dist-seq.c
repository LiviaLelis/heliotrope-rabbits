// Versão Sequencial do cálculo das Distâncias Euclidianas e de Manhathan.
// Encontra as menores e maiores distâncias de cada medida.
// Encontra os somatórios das menores e maiores distâncias de cada medida e de cada ponto.
//
// O padrão de entrada dos dados deste algoritmo sequencial difere do padrão solicitado no trabalho.
// Siga a especificação dada para o trabalho.
//
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

// Ordem das matrizes quadradas. 
// O valor de N está fixo nesta versão sequencial. 
// Para a versão paralela, neste e nos demais casos, siga todas as especificações passadas para o trabalho.
#define N 4 

// Calcula a distância de Manhattan entre dois pontos
int manhattan_distance(int x1, int y1, int z1, int x2, int y2, int z2) 
{
    return abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2);
}

// Calcular a distância Euclidiana entre dois pontos
double euclidean_distance(int x1, int y1, int z1, int x2, int y2, int z2) 
{
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}

int main() 
{
    // Criação das matrizes para as coordenadas X, Y e Z
	// A maneira de criar as matrizes e inserir os dados nelas foi simplificada nesta versão sequencial. 
	// Para a versão paralela, neste e nos demais casos, siga todas as especificações passadas para o trabalho.
	int x[] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
	int y[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	int z[] = {1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16};

	int ij, k;
	int manhattan_dist;
	double euclidean_dist;
    
	int min_manhattan_per_point, min_manhattan = INT_MAX, sum_min_manhattan = 0;
    int max_manhattan_per_point, max_manhattan = 0, sum_max_manhattan = 0;
    double min_euclidean_per_point, min_euclidean = DBL_MAX, sum_min_euclidean = 0.0;
    double max_euclidean_per_point, max_euclidean = 0.0, sum_max_euclidean = 0.0;

    // Calcule as distâncias de Manhattan e Euclidiana e atualize os valores mínimo e máximo
    for (ij = 0; ij < N*N; ij++) 
	{
		min_manhattan_per_point = INT_MAX;
		max_manhattan_per_point = 0;
		min_euclidean_per_point = DBL_MAX;
		max_euclidean_per_point = 0;
				
		for (k = ij+1; k < N*N; k++) 
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
            
        } // fim for k

		// acerta os mínimos e os máximos globais
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
			// terminou um ponto de origem (um i,j). Agora soma as distancias min e max de cada métrica, deste ponto de origem
			sum_min_manhattan += min_manhattan_per_point;
			sum_max_manhattan += max_manhattan_per_point;
			sum_min_euclidean += min_euclidean_per_point;
			sum_max_euclidean += max_euclidean_per_point;
		}
	
		//printf("Dist Manhattan ponto {%d,%d,%d}: min: %d e máx: %d.\n", x[ij], y[ij], z[ij], min_manhattan_per_point, max_manhattan_per_point);
		//printf("Dist Euclidiean ponto {%d,%d,%d}: min: %.2lf e máx: %.2lf.\n", x[ij], y[ij], z[ij], min_euclidean_per_point, max_euclidean_per_point);  
        
    } // fim for ij

    printf("Distância de Manhattan mínima: %d (soma min: %d) e máxima: %d (soma max: %d).\n", min_manhattan, sum_min_manhattan, max_manhattan, sum_max_manhattan);
    printf("Distância Euclidiana mínima: %.2lf (soma min: %.2lf) e máxima: %.2lf (soma max: %.2lf).\n", min_euclidean, sum_min_euclidean, max_euclidean, sum_max_euclidean);
  
    return 0;
} // fim da main() 
