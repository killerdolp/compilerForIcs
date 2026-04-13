CFLAGS = -ansi -pedantic -Wall -Werror 
OFILES = fsm.o reader4.o
EXEFILES = reader.exe

reader4: $(OFILES)
	gcc $(OFILES) -o reader $(CFLAGS)
fsm.o: fsm.c
	gcc -c fsm.c -o fsm.o $(CFLAGS)
reader4.o: reader4.c
	gcc -c reader4.c -o reader4.o $(CFLAGS)
clean:
	rm $(OFILES) $(EXEFILES)

# all:
# 	gcc -Wall -Wextra -o reader4 reader4.c

# clean:
# 	rm -f reader4 *.o
