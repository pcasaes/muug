muug~.pd_linux: muug~.c
	gcc -O3 -std=c99 -Wall -pedantic -shared -o muug~.pd_linux muug~.c
