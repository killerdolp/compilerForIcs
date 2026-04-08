#include <stdio.h>
#include <string.h>

char tempString[256];
char actString[256];

// Possible states
typedef enum
{
    START,
    DO_NOTHING,
    SAVE_TEMP,
    CHECKER,
    SAVE_ACT,
    STOP
} State;

// Define our FSM object, it has only one
// attribute of interest, its current state.
typedef struct
{
    State current_state;
} FSM;

// Define transition function updating the FSM state
// Following the logic of the FSM.
void transition(FSM *fsm, char input)
{
    if (input == '>' && fsm->current_state == START)
    {
        fsm->current_state = DO_NOTHING;
        printf("Transitioned to DO_NOTHING state. Char input: %c\n", input);
    }
    else if (fsm->current_state == DO_NOTHING)
    {
        if (input == '\0')
        {
            fsm->current_state = STOP;
            printf("Transitioned to STOP state. Char input: %c\n", input);
        }
        else if (input == '<')
        {
            fsm->current_state = CHECKER;
            printf("Transitioned to CHECKER state. Char input: %c\n", input);
        }
        else if (input != '\n')
        {
            fsm->current_state = SAVE_TEMP;
            printf("Transitioned to SAVE_TEMP state. Char input: %c\n", input);
        }
    }
    else if (fsm->current_state == SAVE_TEMP)
    {
        if (input == '\n')
        {
            fsm->current_state = DO_NOTHING;
            printf("Transitioned to DO_NOTHING state. Char input: %c\n", input);
        }
        else if (input == '\0')
        {
            fsm->current_state = STOP;
            printf("Transitioned to STOP state. Char input: %c\n", input);
        }
        else if (input == '<')
        {
            fsm->current_state = CHECKER;
            printf("Transitioned to CHECKER state. Char input: %c\n", input);
        }
    }

    if (fsm->current_state == CHECKER)
    {
        printf("In CHECKER state. Char input:-%lli-\n", strlen(tempString));
        if (strlen(tempString) == 1)
        {
            fsm->current_state = START;
            printf("Transitioned to START state. Char input: %c\n", input);
        }
        else
        {
            fsm->current_state = SAVE_ACT;

            printf("Transitioned to SAVE_ACT state. Char input: %c\n", input);
        }

        printf("Transitioned to SAVE_ACT state. Char input: %c\n", input);
    }
    printf("Current state: %d. Char input: %c\n", fsm->current_state, input);
}
// Repeat for every binary character, start in EVEN state.
// Return 0 if odd number of zeroes and 1 otherwise.

void save_temp_string(FSM *fsm, char input)
{
    static int indexTemp = 0;
    if (fsm->current_state == SAVE_TEMP)
    {
        {
            tempString[indexTemp++] = input;
            tempString[indexTemp] = '\0'; // Null-terminate the string
        }
    }
}

void save_act_string(FSM *fsm, char input)
{
    static int index = 0;
    if (fsm->current_state == SAVE_ACT)
    {
        {
            actString[index++] = input;
            actString[index] = '\0'; // Null-terminate the string
        }
    }
}

char *main_function(char *input)
{
    FSM fsm;
    fsm.current_state = START;
    for (size_t i = 0; i < strlen(input); i++)
    {
        transition(&fsm, input[i]);

        if (fsm.current_state == SAVE_TEMP)
        {
            save_temp_string(&fsm, input[i]);
        }
        else if (fsm.current_state == SAVE_ACT)
        {
            save_act_string(&fsm, input[i]);
        }
        else if (fsm.current_state == STOP)
        {
            break;
        }
    }

    return actString;
}

int main()
{
    char input[] = "<td class=3D\"PSLEVEL2GRIDODDROW\" align=3D\"left \"> <div id=3D\"win0divMTG_LOC$101\"><span class=3D\"PSEDITBOX_DISPONLY\" id=3D\"MTG=_LOC$101\">Think Tank 8 (1.410)</span></div></td>";
main_function(input);
    printf("Extracted string: %s\n", actString);
    return 0;
}
