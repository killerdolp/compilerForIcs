#ifndef HEADER_H
#define HEADER_H

char *fsm_function(char *input);

typedef struct{
    char * schedule;
    char * location;
    char * description;
    char * dateStart;
    char * dateEnd;
} Event;

#endif /* HEADER_H */