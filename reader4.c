/*
 * reader3.c  --  Extract raw schedule HTML blocks from MHTML timetable
 *
 * Build:
 *   gcc -Wall -Wextra -o reader4 reader4.c -lgnurx
 * Run:
 *   ./reader4 schedule.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fsm.c"
#include "header.h"

// place this in the header file later
char *main_function(char *input);

// Quoted-Printable decoder
static char *qp_decode(const char *src, size_t src_len, size_t *out_len) {
    char *dst = malloc(src_len + 1);
    if (!dst) return NULL;

    size_t r = 0, w = 0;
    while (r < src_len) {
        unsigned char c = (unsigned char)src[r];

        if (c == '=' && r + 1 < src_len) {
            unsigned char n1 = (unsigned char)src[r + 1];

            /* soft line break: =\r\n or =\n */
            if (n1 == '\r' && r + 2 < src_len &&
                (unsigned char)src[r + 2] == '\n') { r += 3; continue; }
            if (n1 == '\n') { r += 2; continue; }

            /* hex escape: =XX */
            if (r + 2 < src_len && n1 >= '0' && n1 <= '9') {
                unsigned char n2 = (unsigned char)src[r + 2];
                if (((n2) >= '0' && (n2) <= '9') || \
                      ((n2) >= 'a' && (n2) <= 'f') || \
                      ((n2) >= 'A' && (n2) <= 'F')) {
                    int hi = (n1 >= '0' && n1 <= '9') ? n1-'0' : (((n1 >= 'a' && n1 <= 'z') ? n1 - 32 : n1))-'A'+10;
                    int lo = (n2 >= '0' && n2 <= '9') ? n2-'0' : (((n2 >= 'a' && n2 <= 'z') ? n2 - 32 : n2))-'A'+10;
                    dst[w++] = (char)((hi << 4) | lo);
                    r += 3; continue;
                }
            }
        }

        dst[w++] = (char)c;
        r++;
    }
    dst[w] = '\0';
    *out_len = w;
    return dst;
}

// main
int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "My Class Schedule.html";

    /* load */
    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror(filename); return 1; }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);

    char *raw = malloc((size_t)sz + 1);
    fread(raw, 1, (size_t)sz, fp);
    raw[sz] = '\0';
    fclose(fp);


    Event *eventList = NULL;
    /* QP decode */
    size_t html_len = 0;
    size_t schedule_len = 0;
    size_t loc_len = 0;
    size_t desc_len = 0;
    char *schedule = NULL;
    char *location = NULL;
    char *description = NULL;
    char *html = qp_decode(raw, (size_t)sz, &html_len);
    free(raw);

    /* scan for <tr id="trCLASS_MTG_VW ... </tr> timetable blocks */
    const char *ROW_OPEN  = "<tr id=\"trCLASS_MTG_VW";
    const char *ROW_CLOSE = "</tr>";

    // Sub Blocks

    //use this to get the total size need for the arraay to store the STRUCT that can be passed to the ics converter
    int total_blocks = 0;
    const char *count_pos = html;
    while (1) {
        const char *start = strstr(count_pos, ROW_OPEN);
        if (!start) break;

        const char *end = strstr(start, ROW_CLOSE);
        if (!end) break;
        end += strlen(ROW_CLOSE);

        total_blocks++;
        count_pos = end;
    }

    //printf("Total blocks found: %d\n", total_blocks);

    eventList = malloc(sizeof(Event) * total_blocks);

    const char *pos = html;
    int block = 0;

    while (1) {
        /* find next opening tag */
        const char *start = strstr(pos, ROW_OPEN);
        if (!start) break;

        /* find the matching closing tag */
        const char *end = strstr(start, ROW_CLOSE);
        if (!end) break;
        end += strlen(ROW_CLOSE);   /* include </tr> itself */

        /* extract and print sub-blocks */
        //printf("=== BLOCK %d ===\n", block++);
        
        // Schedule
        const char *sched_pos = strstr(start, "MTG_SCHED$");
        if (sched_pos && sched_pos < end) {
            const char *sched_td = sched_pos;
            while (sched_td > start && strncmp(sched_td, "<td", 3) != 0) sched_td--;
            if (sched_td >= start && strncmp(sched_td, "<td", 3) == 0) {
                const char *sched_end = strstr(sched_td, "</td>");
                if (sched_end) {
                    sched_end += 5;
                    schedule_len = sched_end - sched_td;
                    schedule = malloc(schedule_len + 1);
                    if (schedule) {
                        memcpy(schedule, sched_td, schedule_len);
                        schedule[schedule_len] = '\0';
                    }
                }
            }
        }
        // Location
        const char *loc_pos = strstr(start, "MTG_LOC$");
        if (loc_pos && loc_pos < end) {
            const char *loc_td = loc_pos;
            while (loc_td > start && strncmp(loc_td, "<td", 3) != 0) loc_td--;
            if (loc_td >= start && strncmp(loc_td, "<td", 3) == 0) {
                const char *loc_end = strstr(loc_td, "</td>");
                if (loc_end) {
                    loc_end += 5;
                    loc_len = loc_end - loc_td;
                    location = malloc(loc_len + 1);
                    if (location) {
                        memcpy(location, loc_td, loc_len);
                        location[loc_len] = '\0';
                    }
                }
            }
        }
        
        // Description (instructors)
        const char *desc_pos = strstr(start, "DERIVED_CLS_DTL_SSR_INSTR_LONG$");
        if (desc_pos && desc_pos < end) {
            const char *desc_td = desc_pos;
            while (desc_td > start && strncmp(desc_td, "<td", 3) != 0) desc_td--;
            if (desc_td >= start && strncmp(desc_td, "<td", 3) == 0) {
                const char *desc_end = strstr(desc_td, "</td>");
                if (desc_end) {
                    desc_end += 5;
                    desc_len = desc_end - desc_td;
                    description = malloc(desc_len + 1);
                    if (description) {
                        memcpy(description, desc_td, desc_len);
                        description[desc_len] = '\0';
                    }
                }
            }
        }

        
        
        //printf("\n");
        //free schedule



        if (schedule) {
            char* schedule_text = main_function(schedule);
            eventList[block].schedule = malloc(strlen(schedule_text) + 1);
            strcpy(eventList[block].schedule, schedule_text);
            //printf("Schedule:%s\n", schedule_text);
            free(schedule);
            schedule = NULL;
        }
        if (location) {
            char* location_text = main_function(location);
            eventList[block].location = malloc(strlen(location_text) + 1);
            strcpy(eventList[block].location, location_text);
            //printf("Location:%s\n", location_text);
            free(location);
            location = NULL;
        }
        if (description) {
            char *description_text = main_function(description);
            eventList[block].description = malloc(strlen(description_text) + 1);
            strcpy(eventList[block].description, description_text);
            //printf("Description:%s\n", description_text);
            free(description);
            description = NULL;
        }
        block ++;

        pos = end;
    }

    //printf(stderr, "Total blocks: %d\n", block);

    for(int j = 0; j < block; j++) {
        printf("=== EVENT %d ===\n", j);
        printf("Schedule: %s\n", eventList[j].schedule);
        printf("Location: %s\n", eventList[j].location);
        printf("Description: %s\n", eventList[j].description);
    }

    for(int i = 0; i < total_blocks; i++) {
        free(eventList[i].schedule);
        free(eventList[i].location);
        free(eventList[i].description);
    }
    free(eventList);
    free(html);
    return 0;
}