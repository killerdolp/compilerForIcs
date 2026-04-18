#ifndef ICS_WRITER_H
#define ICS_WRITER_H

#include <stdlib.h>
#include "class_event.h"
#include "header.h"

int write_ics_file(const char *output_path, const ClassEvent *events, size_t event_count);
int convert_events_to_ics(const char *output_path, const Event *events, size_t event_count);

#endif
