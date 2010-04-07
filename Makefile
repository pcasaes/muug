CFLAGS = -std=c99 -DPD -O3 -Wall -W -Wshadow -Wstrict-prototypes -Wno-unused -Wno-parentheses -Wno-switch -DMUUG_INTERPOLATE=1 -DMUUG_TILDE_TABLE_SIZE=512

UNIVERSAL=-arch i386 -arch ppc
DARWINCFLAGS = $(CFLAGS) -DDARWIN $(UNIVERSAL)
DARWIN_LIBS=$(UNIVERSAL)
NTCFLAGS = $(CFLAGS) -DNT
WIN_CC=gcc.exe
WIN_STRIP=strip.exe

linux: muug~.c
	gcc $(CFLAGS) -o muug~.o -c muug~.c
	ld -export_dynamics -shared -o muug~.pd_linux muug~.o
	strip --strip-unneeded muug~.pd_linux

darwin: muug~.c
	  cc  $(DARWINCFLAGS) -pedantic -o muug~.o -c muug~.c
		cc -bundle -undefined suppress -flat_namespace $(DARWIN_LIBS) -o muug~.pd_darwin muug~.o

win32: muug~.c
	${WIN_CC} $(NTCFLAGS) -o muug~.o  -c  muug~.c   
	${WIN_CC} $(NTCFLAGS) -Lc:/Arquivos\ de\ programas/pd/bin/ -lpd  -shared -o muug~.dll   muug~.o -W1  
 	#${WIN_STRIP} --strip-unneeded muug~.dll

clean:
	rm *.o
	rm muug~.pd*
