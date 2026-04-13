all:
	gcc -Wall -Wextra -o reader4 reader4.c

clean:
	rm -f reader4 *.o
