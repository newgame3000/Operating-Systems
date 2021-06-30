library_1: implementation_1.c
	gcc -g -O2 -pedantic -std=c11 -Wall -Werror -shared -o library_1.so -fPIC implementation_1.c
library_2: implementation_2.c
	gcc -g -O2 -pedantic -std=c11 -Wall -Werror -shared -o library_2.so -fPIC implementation_2.c
on_link_1: library_1
	gcc -g -O2 -pedantic -std=c11 -Wall -Werror lab5_link.c -o on_link_1.out -L. -Wl,-rpath=./ -lrary_1
on_link_2: library_2
	gcc -g -O2 -pedantic -std=c11 -Wall -Werror lab5_link.c -o on_link_2.out -L. -Wl,-rpath=./ -lrary_2
on_dynamic: library_1 library_2
	gcc -g -O2 -pedantic -std=c11 -Wall -Werror lab5_dynamic.c -o on_dynamic.out -ldl