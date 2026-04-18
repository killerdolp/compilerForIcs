#ifndef HEADER_H
#define HEADER_H

char *main_function(char *input);

typedef struct {
    char *schedule;
    char *location;
    char *description;
    char *dateEnd;
    char *dateStart;
} Event;

#endif