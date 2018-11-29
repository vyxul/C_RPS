#Makefile to build and execute lab5.c in this directory

run:	Lab6
	clear
	./Lab6

Lab6:	Lab6.c
	gcc Lab6.c -o Lab6 -lpthread

clean:	
	rm Lab6
