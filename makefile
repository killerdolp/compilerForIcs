CFLAGS = -ansi -pedantic -Wall -Werror 
OFILES = fsm.o main.o ics_writer.o
EXEFILES = main
# DEFAULT_HTML = MyClassSchedule.html

# all: $(DEFAULT_HTML)

main: $(OFILES)
	gcc $(OFILES) -o main $(CFLAGS)

FORCE:

# %.html: main FORCE
# 	./main "$@"

fsm.o: fsm.c
	gcc -c fsm.c -o fsm.o $(CFLAGS)
main.o: main.c
	gcc -c main.c -o main.o $(CFLAGS)
ics_writer.o: ics_writer.c
	gcc -c ics_writer.c -o ics_writer.o $(CFLAGS)
clean:
	rm -f $(OFILES) $(EXEFILES)

.PHONY: all clean FORCE

# all:
# 	gcc -Wall -Wextra -o main main.c

# clean:
# 	rm -f main *.o
