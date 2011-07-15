



CC = gcc

#CFLAGS = \
#	-Xcpluscomm

#CFLAGS = -g
CFLAGS = -O2

#INCLUDES = -I/usr/X11R6/include -I/usr/local/include -I. -I/usr/freeware/include
#LIBRARIES =  -L/usr/X11R6/lib  -L/usr/freeware/lib -L/usr/local/lib -lmalloc -lX11 -lXext -ljpeg -lpng -lz -lm -lImlib 
INCLUDES = -I/usr/X11R6/include -I/usr/local/include -I.
LIBRARIES =  -L/usr/X11R6/lib  -L/usr/local/lib  -lX11 -lXext -ljpeg -lpng -lz -lm -lImlib 


all: streamAnim


streamAnim: streamAnim.o getopt.o getopt1.o Makefile
	${CC} ${CFLAGS} streamAnim.o getopt.o getopt1.o -o streamAnim ${LIBRARIES}


streamAnim.o: streamAnim.c Makefile
	${CC} ${CFLAGS} -c streamAnim.c  ${INCLUDES}


getopt.o: getopt.c Makefile
	${CC} ${CFLAGS} -c getopt.c ${INCLUDES}

getopt1.o: getopt1.c Makefile
	${CC} ${CFLAGS} -c getopt1.c ${INCLUDES}

install:
	install streamAnim /usr/local/bin
 

clean:
	rm -f ./*.o ./*~


.PHONY: all install uninstall clean distclean depend dummy





