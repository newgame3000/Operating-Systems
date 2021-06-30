
int GCD(int a, int b) {
	if (b == 0) {
		return 0;
	} 
	int c;
	while (b > 0) {
		c = a % b;
		a = b;
		b = c;
	}
	return a;
}

float Pi(int k) {
	float pi = 0;
	float one;
	for (int i = 0; i < k; ++i) {
		if (i % 2 == 0) {
			one = 1;
		} else {
			one = -1;
		}
		pi += one/(2*i + 1);
	}
	return 4*pi;
}