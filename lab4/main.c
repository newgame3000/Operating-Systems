#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

 int main(){
 	char fl[100];
 	scanf("%s",fl);

 	FILE *f = fopen(fl,"w");

 	 if (f == NULL) {
 		printf("Ошибка в открытии файла\n");
 		return -6;
 	}

 	char area_n[] = "area";
 	char mutex1_n[] = "mutex1";
	char size_data_n[] = "size_data";

 	int area = shm_open(area_n, O_RDWR | O_CREAT, S_IRWXO | S_IRWXU);
 	int mutex1 = shm_open(mutex1_n, O_RDWR | O_CREAT, S_IRWXO | S_IRWXU);
 	int size_data = shm_open(size_data_n, O_RDWR | O_CREAT, S_IRWXO | S_IRWXU);

 	if (area == -1 || mutex1 == -1) {
 		printf("Ошибка в доступе к памяти в shm_open\n");
 		return -1;
 	}

 	if (ftruncate(area, getpagesize()) == -1) {
 		printf("Ошибка в изменении размера файла\n");
 		return -2;
 	}

 	if (ftruncate(size_data, sizeof(int)) == -1) {
 		printf("Ошибка в изменении размера файла\n");
 		return -2;
 	}

 	if (ftruncate(mutex1, sizeof(pthread_mutex_t)) == -1) {
        printf("Ошибка в изменении размера файла\n");
        return -2;
    }

    float* Data = (float*) mmap(NULL, getpagesize(),  PROT_READ | PROT_WRITE, MAP_SHARED, area, 0);
    int* Data_s = (int*) mmap(NULL, sizeof(int),  PROT_READ | PROT_WRITE, MAP_SHARED, size_data, 0);
    pthread_mutex_t *mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex1, 0);

    if (Data == MAP_FAILED || mutex == MAP_FAILED || Data_s == MAP_FAILED) {
        printf("Ошибка в отображении файла на память\n");
        return -3;
    }

    pthread_mutexattr_t mutex_at;

   if (pthread_mutexattr_init(&mutex_at) != 0) {
        printf("Ошибка в инициализации атребутов мьютекса\n");
        return -4;
    }

    if (pthread_mutexattr_setpshared(&mutex_at, PTHREAD_PROCESS_SHARED) != 0) {
        printf("Ошибка в изменении области видимости мьютекса\n");
        return -5;
    }

    if (pthread_mutex_init(mutex, &mutex_at) != 0) {
        printf("Ошибка в инициализации мьютекса\n");
        return -6;
    }

    if (pthread_mutex_lock(mutex) != 0) {
        printf("Ошибка в открытии мьютекса\n");
       	return -7;
	}

 	float a = 0, b = 0, d;
 	char** j = NULL;
 	char c;
 	int minus = 0;
 	int tochka = 0;
 	int one = 0;
 	int count = 0;
 	int id = fork();

    if (id == -1) {
    	printf("Ошибка создания процессa\n");
    	return -2;
    }

	if(id != 0) {

		while(scanf("%c",&c) > 0) {

	 		if(c == '-') {
	 			minus = 1 ;
	 			continue;
	 		}

	 		if(c == '\n'){

	 			if(one == 0){
	 				one = 1;
	 				continue;
	 			}

	 			while(b > 1){
					b /= 10;
				}

				d = a + b;
				if(minus == 1){
					d *= -1;
					minus = 0;
				}

				tochka = 0;
				b = 0;
				a = 0;
				Data[count] = d;
				count += 1;
		 		float nan = NAN;
		 		Data[count] = nan;
		 		count += 1;
			    continue;
			}

			if(c == ' '){

				while(b > 1){
					b /= 10;
				}

				d = a + b;
				if(minus == 1){
					d *= -1;
					minus = 0;
				}
				tochka = 0;
				b = 0;
				a = 0;
				Data[count] = d;
				count++;
				continue;
			}

			if(c == '.'){
				tochka = 1;
				continue;
			}

			if(tochka == 0){
				a = c + a * 10 - '0';
			}

			if(tochka == 1){
				b = c + b * 10 - '0';
			}

		}

 		*Data_s = count;
 		if (pthread_mutex_unlock(mutex) != 0) {
        	printf("Ошибка в открытии мьютекса\n");
       		return -7;
	   	}

	}else{

		if (pthread_mutex_lock(mutex) != 0) {
        	printf("Ошибка в открытии мьютекса\n");
       		return -7;
	   	}
	 
 		char* argv[] = {fl, mutex1_n, area_n, size_data_n, (char *) NULL};
	    if (execv("child.out", argv) == -1) {
			printf("Ошибка выполнения дочернего процесса\n");
			printf("%s\n", strerror(errno));
	    	return -5;
	    }	

	    if (pthread_mutex_unlock(mutex) != 0) {
			printf("Ошибка в открытии мьютекса\n");
			return -7;
		}	
	}
 }