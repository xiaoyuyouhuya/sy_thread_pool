run:test.o sy_thread_pool.o
	gcc test.o sy_thread_pool.o -o run -lpthread

test.o:test.c
	gcc -c test.c -o test.o -lpthread

sy_thread_pool.o:sy_thread_pool.c
	gcc -c sy_thread_pool.c -o sy_thread_pool.o -lpthread

clean:
	rm -rf *.o
