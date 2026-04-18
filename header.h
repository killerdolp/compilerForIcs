#ifndef HEADER_H
#define HEADER_H

typedef enum {
    FSM_ACC = 0,
    FSM_NONACC = 1,
    FSM_BUF_OVERFLOW = 2 
} FSMResult;

int fsm_function(const char *input, char *out_buf, size_t out_buf_size);
#define MAX_ERROR_COUNT 128
#define MAX_ERROR_LENGTH 256

extern char error_messages[MAX_ERROR_COUNT][MAX_ERROR_LENGTH];
extern char error_contexts[MAX_ERROR_COUNT][MAX_ERROR_LENGTH];
extern int error_message_counts[MAX_ERROR_COUNT];
extern int error_count;

void add_error(const char *message, const char *context);
void clear_errors(void);
void print_errors(void);

typedef struct{
    char * schedule;
    char * location;
    char * description;
    char * dateStart;
    char * dateEnd;
    char * classTitle;
} Event;

#endif /* HEADER_H */
