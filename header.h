#ifndef HEADER_H
#define HEADER_H

typedef enum {
    FSM_ACC = 0,
    FSM_NONACC = 1,
    FSM_BUF_OVERFLOW = 2 
} FSMResult;

int fsm_function(const char *input, char *out_buf, size_t out_buf_size);

typedef struct{
    char * schedule;
    char * location;
    char * description;
    char * dateStart;
    char * dateEnd;
} Event;

#endif /* HEADER_H */