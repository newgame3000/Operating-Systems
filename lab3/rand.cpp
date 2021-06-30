#include<iostream>

using namespace std;



int main(){

	int k,m,n,wi,wj;


	cin >> n >> m >> wi >> wj >> k;

	cout << n << " " << m << endl;

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < m; ++j) {
			double a;
			a = rand();
			cout << a << " ";
		}
		cout << endl;
	}

	cout << endl;
	cout << wi<< " " << wj << endl;

	cout << k << endl;
}