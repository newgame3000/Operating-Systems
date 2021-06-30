#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

 int main(){

    float a,d=0;
	while(read(fileno(stdin),&a,sizeof(float))>0){

		if(isnan(a)){
			
		printf("%f\n",d);
		d=0;

		}
		else{
		d=d+a;
	}

	}
    fflush(stdout);
}