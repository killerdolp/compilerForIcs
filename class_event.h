/* This file defines the event payload written into an .ics file. */

#ifndef CLASS_EVENT_H
#define CLASS_EVENT_H

typedef struct {
    char dtstart[32];
    char dtend[32];
    char summary[256];
    char location[256];
    char description[256];
    char classTitle[256];
} ClassEvent;

#endif
