
#include <stdio.h>
#include <string.h>

static char tempString[256];
static char actString[256];
static int indexTemp = 0;

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
static void clear_temp_string(){
    tempString[0] = '\0'; /* Clear tempString */
    indexTemp = 0;
}

/* save to tempString */
static void save_temp_string(FSM *fsm, char input)
{
    tempString[indexTemp++] = input;
    tempString[indexTemp] = '\0'; /* Null-terminate the string */
}

/* save tempString to actString */
static void save_act_string(FSM *fsm)
{
    /* concatenate tempstring to actString */
    strcat(actString, tempString);
}

/* perform the actions associated with each state of the FSM */
static void perform_action(FSM *fsm, char current_input){
    if (fsm->current_state == START)
    {
        clear_temp_string();
    }
    else if (fsm->current_state == SAVE_TEMP)
    {
        save_temp_string(fsm, current_input);
    }
    else if (fsm->current_state == SAVE_ACT)
    {
        remove_nbsp(tempString);
        save_act_string(fsm);
        clear_temp_string();
    }
}

char *fsm_function(char *input)
{
    FSM fsm;
    size_t i;
    int guard;
    fsm.current_state = START;
    actString[0] = '\0'; /* Initialize actString to empty */
    for (i = 0; i < strlen(input); i++)
    {
        transition(&fsm, input[i]);
        perform_action(&fsm, input[i]);
        
        /* handles epsilon transitions, max 8 in a row */
        guard = 0;
        while (epsilon_transition(&fsm) && guard++ < 8) {
            perform_action(&fsm, input[i]);
        }
    }

    return actString;
}
