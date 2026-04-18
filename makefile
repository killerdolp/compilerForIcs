CC = gcc
CFLAGS = -Wall -Wextra
DEFAULT_HTML = My\ Class\ Schedule.html

all: $(DEFAULT_HTML)

reader4: reader4.c ics_writer.c
	$(CC) $(CFLAGS) -o reader4 reader4.c ics_writer.c

FORCE:

%.html: reader4 FORCE
	./reader4 "$@"

test_ics_writer:
	$(CC) $(CFLAGS) -o ics_writer_test ics_writer_test.c ics_writer.c
	./ics_writer_test

clean:
	rm -f reader4 ics_writer_test *.o ics_writer_test_output.ics "Test Class Schedule.ics"

.PHONY: all test_ics_writer clean FORCE
