#include "stdio.h"
#include "dlfcn.h"

extern int GCD(int x, int y);
extern float Pi(int k);

int main() {
	char* libnames[] = {"./library_1.so", "./library_2.so"};
	void *handle;
	int j = 0;
	handle = dlopen(libnames[j], RTLD_LAZY);
	if (handle == NULL) {
		printf("Ошибка в зaгрузке библиотек\n");
		return -1;
	}
	int (*GCD)(int, int);
	float (*Pi)(int);
	*(void**) (&GCD) = dlsym(handle, "GCD");
	*(void**) (&Pi) = dlsym(handle, "Pi");
	int input;
	int f;
	while (scanf("%d",&input) > 0) { 
		if (input == 0) {
			
			if (dlclose(handle) != 0) {
				printf("Ошибка в зaгрузке библиотек\n");
				return -2;
			}

			if (j == 0) {
				j = 1;
				handle = dlopen(libnames[j], RTLD_LAZY);
			} else {
				j = 0;
				handle = dlopen(libnames[j], RTLD_LAZY);
			}

			*(void**) (&GCD) = dlsym(handle, "GCD");
			*(void**) (&Pi) = dlsym(handle, "Pi");

			printf("Библиотека сменена\n");
		}

		if (input == 1) {
			int a,b;
			f = scanf("%d%d", &a,&b);
			a = GCD(a,b);
			printf("Наибольший общий делитель: %d\n",a);
		} 

		if (input == 2) {
			int k;
			f = scanf("%d",&k);
			float pi;
			pi = Pi(k);
			printf("Число Пи: %f\n",pi);
		}
	}
	if (dlclose(handle) != 0) {
		printf("Ошибка в зaгрузке библиотек\n");
		return -2;
	}
	(void) f;
}