#include<stdio.h>
#include <stdlib.h>
#include "time.h"
#include <pthread.h>

double ** matrix_nar;
double ** matrix_er;
int winI;
int winJ;
int m;
int n;
int c = 0;

struct TPointers {

	int i;
	int j;
	double max;

};

int max(int lhs, int rhs){
    if(lhs > rhs){
        return lhs;
    }
    return rhs;
}

int min(int lhs, int rhs){
    if(lhs < rhs){
        return lhs;
    }
    return rhs;
}



void* ThreadFunction_nar (void * T) {

	struct TPointers *points = (struct TPointers*) T;

	points->max = matrix_nar[points->i][points->j];

	for (int i = max(0, points->i - winI); i <= min(n - 1, points->i + winI); ++i) {
 	    for (int j = max(0, points->j - winJ); j <= min(m - 1, points->j + winJ); ++j) {

 	    	if (matrix_nar[i][j] > points->max) {
 	    		points->max = matrix_nar[i][j];
 	    	}

  		}
	}	
}

void* ThreadFunction_er (void* T) {

	struct TPointers *points = (struct TPointers*) T;  

	points->max = matrix_er[points->i][points->j];

	for (int i = max(0, points->i - winI); i <= min(n - 1, points->i + winI); ++i) {
 	    for (int j = max(0, points->j - winJ); j <= min(m - 1, points->j + winJ); ++j) {

 	    	if (matrix_er[i][j] < points->max) {
 	    		points->max = matrix_er[i][j];
 	    	}

  		}
	}

}


int main(int argc, char** argv){

	int ThreadsNumber = 0;
	for (int i = 0; argv[1][i] > 0; ++i) {
		if (argv[1][i] >= '0' && argv[1][i] <= '9') {
			ThreadsNumber = ThreadsNumber * 10 + argv[1][i] - '0';
		}
	}

	printf("Количество потоков: %d\n", ThreadsNumber);

	printf("Введите размеры матрицы\n");
	scanf("%d%d",&n,&m);
	if(n<=0 || m<=0) {
		printf("Введены неверные размеры\n");
		return 3;
	}

	matrix_nar = (double**)malloc(sizeof(double*)*n);
	for (int i=0; i<n; i++) {
		matrix_nar[i] = (double*) malloc(sizeof(double)*m);
	}

	matrix_er = (double**)malloc(sizeof(double*)*n);
	for (int i=0; i<n; i++) {
		matrix_er[i] = (double*) malloc(sizeof(double)*m);
	}

	printf("Введите элементы матрицы\n");
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < m; ++j) {
			scanf("%lf", &matrix_nar[i][j]);
			matrix_er[i][j] = matrix_nar[i][j];
		}
	}
	printf("Введите размеры окна\n");
	scanf("%d %d", &winI, &winJ);

	if (winI == 0 || winJ == 0 || winI > n || winJ > m) {
		printf("Введены неверные данные окна\n");
		return -1;
	}

	printf("Введите K\n");
	int k;
	scanf("%d",&k);

	double start,end;

	pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t)*ThreadsNumber);
	struct TPointers* points_nar = (struct TPointers*) malloc(sizeof(struct TPointers)*n*m);
	struct TPointers* points_er = (struct TPointers*) malloc(sizeof(struct TPointers)*n*m);

	for (int i = 0; i < n; ++i) {

		for (int j = 0; j < m; ++j) {

			points_nar[c].i = i;
			points_nar[c].j = j;

			points_er[c].i = i;
			points_er[c].j = j;
			c++;
		}

	}

	start = clock();

	for (int p = 0 ; p < k; ++p) {

		for (int i = 0; i < n * m; i += ThreadsNumber) {

			for (int j = 0; j < ThreadsNumber; ++j) {

				if(i+j >= n*m){
	                break;
	            }

	            //printf("Поток номер %d (наращивание)\n",j);

	            if(pthread_create(&threads[j], NULL, ThreadFunction_nar,(void*) &points_nar[i+j]) != 0){
	                perror("Ошибка в создании потока\n");
	                return 1;
	            }
			}

			for (int j = 0; j < ThreadsNumber; ++j) {
				if(i+j >= n*m){
	                break;
	            }

	            if(pthread_join(threads[j], NULL)){
	                perror("Поток не завершился\n");
	                return 2;
	            }
			}

		}

		for (int i = 0; i < c; ++i) {
			matrix_nar[points_nar[i].i][points_nar[i].j] = points_nar[i].max;
		}		

	}


	for (int p = 0 ; p < k; ++p) {

		for (int i = 0; i < n * m; i += ThreadsNumber) {
			for (int j = 0; j < ThreadsNumber; ++j) {

				if(i+j >= n*m){
	                break;
	            }

	            //printf("Поток номер %d (эррозия)\n",j);

	            if(pthread_create(&threads[j],NULL, ThreadFunction_er,(void*) &points_er[i+j]) != 0){
	                perror("Ошибка в создании потока\n");
	                return 1;
	            }

			}

			for (int j = 0; j < ThreadsNumber; ++j) {

				if(i+j >= n*m){
	                break;
	            }

	            if(pthread_join(threads[j], NULL)){
	                perror("Поток не завершился\n");
	                return 2;
	            }
			}
		}

		for (int i = 0; i < c; ++i) {
			matrix_er[points_er[i].i][points_er[i].j] = points_er[i].max;
		}
	}

	end = clock();

	// printf("Матрица с фильтром наращивания\n");
	// for (int i = 0; i < n; ++i) {
 //  		for (int j = 0; j < m; ++j) {
 //  			printf("%lf ", matrix_nar[i][j]);
 //  		}
 //  		printf("\n");
 //  	}

	// printf("Матрица с фильтром эррозии\n");
 //  	for (int i = 0; i < n; ++i) {
 //  		for (int j = 0; j < m; ++j) {
 //  			printf("%lf ", matrix_er[i][j]);
 //  		}
 //  		printf("\n");
 //  	}

  	printf("Время работы: %fms\n",(end-start)/1000);

}