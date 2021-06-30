#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

 int main(int argc, char *argv[]){


 	FILE *f = fopen(argv[0],"w");

 	if (f == NULL) {
 		printf("Ошибка в открытии файла\n");
 		return -6;
 	}

 	int area = shm_open(argv[2], O_RDWR | O_CREAT, S_IRWXU);
 	int mutex1 = shm_open(argv[1], O_RDWR | O_CREAT, S_IRWXU);
 	int size_data = shm_open(argv[3], O_RDWR | O_CREAT, S_IRWXU);


 	if (area == -1 || mutex1 == -1) {
 		printf("Ошибка в доступе к памяти в shm_open\n");
 		return -1;
 	}

 	float d=0;


 	float* Data = (float*) mmap(NULL, getpagesize(),  PROT_READ | PROT_WRITE, MAP_SHARED, area, 0);
    int* Data_s = (int*) mmap(NULL, sizeof(int),  PROT_READ | PROT_WRITE, MAP_SHARED, size_data, 0);
    pthread_mutex_t *mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex1, 0);
 
    if (Data == MAP_FAILED || mutex == MAP_FAILED || Data_s == MAP_FAILED) {
        printf("Ошибка в отображении файла на память\n");
        return -3;
    }


    for (int i = 0; i < *Data_s; ++i) {
    	if (isnan(Data[i])) {
    		fprintf(f, "%g\n", d);
    		d = 0;
    	} else {
    		d += Data[i];
    	}
    }

    if (shm_unlink(argv[1]) != 0) {
        perror("Ошибка в удалении памяти в shm_open\n");
        return -10;
    }

    if (shm_unlink(argv[2]) != 0) {
        perror("Ошибка в удалении памяти в shm_open\n");
        return -10;
    }

    if (shm_unlink(argv[3]) != 0) {
        perror("Ошибка в удалении памяти в shm_open\n");
        return -10;
    }
}