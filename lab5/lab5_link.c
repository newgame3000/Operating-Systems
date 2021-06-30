#include "stdio.h"

extern int GCD(int x, int y);
extern float Pi(int k);

int main() {
	int input;
	int f;
	while (scanf("%d",&input) > 0) { 
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
	(void) f;
}