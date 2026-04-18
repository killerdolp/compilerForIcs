
#include <stdio.h>
#include <string.h>
#include "header.h"

static char tempString[256];
static char actString[256];
static int indexTemp = 0;


/* Global error array  handling string */
char error_messages[MAX_ERROR_COUNT][MAX_ERROR_LENGTH];
char error_contexts[MAX_ERROR_COUNT][MAX_ERROR_LENGTH];
int error_message_counts[MAX_ERROR_COUNT];
int error_count = 0;

void add_error(const char *message, const char *context)
{
    int i;
    const char *ctx = context ? context : "";

    if (!message || error_count >= MAX_ERROR_COUNT) {
        return;
    }

    for (i = 0; i < error_count; i++) {
        if (strcmp(error_messages[i], message) == 0 &&
            strcmp(error_contexts[i], ctx) == 0) {
            error_message_counts[i]++;
            return;
        }
    }

    strncpy(error_messages[error_count], message, MAX_ERROR_LENGTH - 1);
    error_messages[error_count][MAX_ERROR_LENGTH - 1] = '\0';
    strncpy(error_contexts[error_count], ctx, MAX_ERROR_LENGTH - 1);
    error_contexts[error_count][MAX_ERROR_LENGTH - 1] = '\0';
    error_message_counts[error_count] = 1;
    error_count++;
}

void clear_errors(void)
{
    /* Only the active prefix [0, error_count) is considered valid. */
    error_count = 0;
}

void print_errors(void)
{
    int i;
    for (i = 0; i < error_count; i++) {
        if (error_contexts[i][0] != '\0') {
            printf("%s | context: %s\n",
                   error_messages[i],
                   error_contexts[i]);
        } else {
            printf("%s\n", error_messages[i]);
        }
    }
}


/* Possible states */
typedef enum
{
    START,
    DO_NOTHING,
    SAVE_TEMP,
    CHECKER,
    SAVE_ACT,
    STOP
} State;

/* Define our FSM object, it has only one
attribute of interest, its current state. */
typedef struct
{
    State current_state;
} FSM;

/* Define transition function updating the FSM state */
/* Following the logic of the FSM. */
static void transition(FSM *fsm, char input) {
    switch (fsm->current_state) {
        case START:
            if (input == '>') {
                fsm->current_state = DO_NOTHING;
                /* printf("Transitioned to DO_NOTHING state. Char input: %c\n", input); */
            }
            /* printf("In %d state. Char input: %c\n", fsm->current_state, input); */
            break;
        case DO_NOTHING:
            if (input == '\0') {
                fsm->current_state = STOP;
                /* printf("Transitioned to STOP state. Char input: %c\n", input); */
            }
            else if (input == '<') {
                fsm->current_state = CHECKER;
                /* printf("Transitioned to CHECKER state. Char input: %c\n", input); */
            }
            else if (input != '\n' && input != '\r') {
                fsm->current_state = SAVE_TEMP;
                /* printf("Transitioned to SAVE_TEMP state. Char input: %c\n", input); */
            }
             /* printf("In %d state. Char input: %c\n", fsm->current_state, input); */
            break;
        case SAVE_TEMP:

            if (input == '\n' || input == '\r') {
                fsm->current_state = DO_NOTHING;
                /* printf("Transitioned to DO_NOTHING state. Char input: %c\n", input); */
            }
            else if (input == '\0') {
                fsm->current_state = STOP;
                /* printf("Transitioned to STOP state. Char input: %c\n", input); */
            }
            else if (input == '<') {
                fsm->current_state = CHECKER;
                /* printf("Transitioned to CHECKER state. Char input: %c\n", input); */
            }
            break;
        default:
         /* printf("In %d state. Char input: %c\n", fsm->current_state, input); */
            break;
    }
}

/* Handles epsilon transitions (i.e. no input consumed) */
static int epsilon_transition(FSM *fsm) {
    switch (fsm->current_state) {
        case CHECKER:
            if (indexTemp <= 1) {
                fsm->current_state = START;
                /* printf("Transitioned to START state via epsilon.\n"); */
            } else {
                fsm->current_state = SAVE_ACT;
                /* printf("Transitioned to SAVE_ACT state via epsilon.\n");      */
            }
            return 1;
            break;
        case SAVE_ACT:
            fsm->current_state = START;
            /* printf("Transitioned to START state via epsilon.\n");      */
            break;

        default:
            break;
    }


    return 0;
}

/* removed nbsp characters from str */
static void remove_nbsp(char *str) {
    const char *target = "&nbsp";
    size_t target_len = strlen(target);
    char *pos = str;

    while ((pos = strstr(pos, target)) != NULL) {
        memmove(pos, pos + target_len, strlen(pos + target_len) + 1);
    }
}

/* resets tempString to empty, indexTemp to 0. To be used after the tempString is saved to actString or discarded. */
static void clear_temp_string(void){
    tempString[0] = '\0'; /* Clear tempString */
    indexTemp = 0;
}

/* save to tempString */
static void save_temp_string(char input)
{   
    if (indexTemp < (int)sizeof(tempString) - 1) {
        tempString[indexTemp++] = input;
    } else {
        add_error("Text is too long! " , tempString);
    }
    tempString[indexTemp] = '\0'; /* Null-terminate the string */
}

/* save tempString to actString */
static void save_act_string(void)
{
    size_t act_len;
    size_t temp_len;
    size_t remaining;

    /* concatenate tempstring to actString */
    /* bounded concatenation ensures that actString does not overflow */
    act_len = strlen(actString);
    temp_len = strlen(tempString);
    remaining = sizeof(actString) - act_len - 1;

    if (temp_len > remaining) {
        add_error("Combined text too long: text truncated while appending", actString);
    }

    strncat(actString, tempString, remaining);
}

/* perform the actions associated with each state of the FSM */
void perform_action(FSM *fsm, char current_input){
    if (fsm->current_state == START)
    {
        clear_temp_string();
    }
    else if (fsm->current_state == SAVE_TEMP)
    {
        save_temp_string(current_input);
    }
    else if (fsm->current_state == SAVE_ACT)
    {
        remove_nbsp(tempString);
        save_act_string();
        clear_temp_string();
    }
}

char *fsm_function(char *input)
{
    FSM fsm;
    size_t i, inputLen;
    int guard;
    fsm.current_state = START;
    actString[0] = '\0'; /* Initialize actString to empty */
    inputLen = strlen(input);
    for (i = 0; i < inputLen; i++)
    {
        transition(&fsm, input[i]);
        perform_action(&fsm, input[i]);
        
        /* handles epsilon transitions, max 8 in a row */
        guard = 0;
        while (epsilon_transition(&fsm) && guard++ < 8) {
            perform_action(&fsm, input[i]);
        }
    }

    return actString  ;
}

