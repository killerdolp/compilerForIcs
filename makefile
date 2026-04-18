CFLAGS = -ansi -pedantic -Wall -Werror 
OFILES = fsm.o reader4.o ics_writer.o
EXEFILES = reader4
DEFAULT_HTML = MyClassSchedule.html

all: $(DEFAULT_HTML)

reader4: $(OFILES)
	gcc $(OFILES) -o reader4 $(CFLAGS)

FORCE:

%.html: reader4 FORCE
	./reader4 "$@"

fsm.o: fsm.c
	gcc -c fsm.c -o fsm.o $(CFLAGS)
reader4.o: reader4.c
	gcc -c reader4.c -o reader4.o $(CFLAGS)
ics_writer.o: ics_writer.c
	gcc -c ics_writer.c -o ics_writer.o $(CFLAGS)
clean:
	rm -f $(OFILES) $(EXEFILES)

.PHONY: all clean FORCE

# all:
# 	gcc -Wall -Wextra -o reader4 reader4.c

# clean:
# 	rm -f reader4 *.o
