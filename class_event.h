/*This file defines the struct of the class_event,
  that is generated from the parser*/

/*
Struct fields/attributes:
DTSTART
DTEND
SUMMARY (name of prof.)
LOCATION
DESCRIPTION (name of course)
*/

/* NOTE: need to get exact struct structure from task 2*/

typedef struct {
  char dtstart[32];
  char dtend[32];
  char summary[256];
  char location[256];
  char description[256];
} C;