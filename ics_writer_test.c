/*This file contains test cases, used to test sample class_event structs against ics_writer.c */

#include <stdio.h>
#include <string.h>

#include "ics_writer.h"
#include "ics_writer_test.h"

int run_ics_writer_smoke_test(void)
{
    Event e = {0};

    e.schedule = "50.001 - Intro to Info Sys & Program Lecture\nMon 09:00AM - 10:30AM";
    e.location = "Lecture Theatre 2 (1.203)";
    e.description = "Fredy Tantri";
    e.dateStart = "01/27/2025";
    e.dateEnd = "02/10/2025";

    return convert_events_to_ics("ics_writer_test_output.ics", &e, 1);
}

int main()
{
    if (run_ics_writer_smoke_test() != 0) {
        fprintf(stderr, "ics_writer smoke test failed\n");
        return 1;
    }

    printf("ics_writer smoke test passed\n");
    return 0;
}
