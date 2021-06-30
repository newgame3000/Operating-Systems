
#include <stdio.h>

int GCD(int a, int b) {
	if (b == 0) {
		return 0;
	} 
	int c;
	if (a < b) {
		c = a;
	} else {
		c = b;
	}
	while ((a % c != 0) || (b % c != 0)) {
		c -= 1;
	}
	return c;
}

float Pi(int k) {
	float pi = 1;
	for (float i = 1; i <= k; ++i) {
		pi *= (2 * i * 2 * i) / ((2 * i - 1) * (2 * i + 1));
	}
	return 2*pi;
}