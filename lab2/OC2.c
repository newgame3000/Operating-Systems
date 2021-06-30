#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

 int main(){
 	char fl[100];
 	scanf("%s",fl);
 	FILE *f=fopen(fl,"w");
 	if (f == NULL) {
 		printf("Ошибка в открытии файла\n");
 		return -6;
 	}
 	
 	int pipe1[2];

 	if(pipe(pipe1)<0){
 		printf("Ошибка создания трубы\n");
 		return  -1;
 	}

 	float a=0,b=0,d;
 	char** j=NULL;
 	char c;
 	int minus=0;
 	int tochka=0;
 	int one=0;

 	int id=fork();

    if(id==-1){
    	printf("Ошибка создания процессa\n");
    	return -2;
    }

	if(id!=0){

		while(scanf("%c",&c)>0){

 		if(c=='-'){
 			minus=1;
 			continue;
 		}

 		if(c=='\n'){

 			if(one==0){
 				one=1;
 				continue;
 			}

 			while(b>1){
				b/=10;
			}

			d=a+b;
			
			if(minus==1){
				d*=-1;
				minus=0;
			}
			tochka=0;
			b=0;
			a=0;
			write(pipe1[1],&d,sizeof(float));

	 		float nan=NAN;
		    write(pipe1[1],&nan,sizeof(float));
		    continue;
		}

		if(c==' '){

			while(b>1){
				b/=10;
			}

			d=a+b;
			if(minus==1){
				d*=-1;
				minus=0;
			}
			tochka=0;
			b=0;
			a=0;
			write(pipe1[1],&d,sizeof(float));
			continue;
		}

		if(c=='.'){
			tochka=1;
			continue;
		}

		if(tochka==0){
			a=c+a*10-'0';
		}

		if(tochka==1){
			b=c+b*10-'0';
		}
	 }

	 close(pipe1[1]);
	 close(pipe1[0]);

	}else{

		close(pipe1[1]);
	    if(dup2(pipe1[0],fileno(stdin)) == -1) {
	    	printf("Ошибка перенапраления ввода\n");
	    	return -3;
	    }
	    if(dup2(fileno(f),fileno(stdout)) == -1) {
	    	printf("Ошибка перенапраления вывода\n");
	    	return -4;
	    }
	    if (execv("OC2_child.out",j) == -1) {
			printf("Ошибка выполнения дочернего процесса\n");
	    	return -5;
	    }	

	}

	fclose(f);
	fflush(stdout);
 }