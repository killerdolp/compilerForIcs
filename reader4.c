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
#include <dirent.h>

#include "header.h"
#include "ics_writer.h"


static void split_date_range(const char *range, char **start_date, char **end_date) {
    const char *sep;
    size_t start_len;
    size_t end_len;

    *start_date = NULL;
    *end_date = NULL;

    if (!range) return;

    sep = strstr(range, " - ");
    if (!sep) {
        *start_date = malloc(strlen(range) + 1);
        if (*start_date) {
            strcpy(*start_date, range);
        }
        return;
    }

    start_len = (size_t)(sep - range);
    end_len = strlen(sep + 3);

    *start_date = malloc(start_len + 1);
    *end_date = malloc(end_len + 1);

    if (*start_date) {
        memcpy(*start_date, range, start_len);
        (*start_date)[start_len] = '\0';
    }

    if (*end_date) {
        memcpy(*end_date, sep + 3, end_len);
        (*end_date)[end_len] = '\0';
    }
}

/* Quoted-Printable decoder */
static char *qp_decode(const char *src, size_t src_len, size_t *out_len) {
    size_t r, w;
    char *dst = malloc(src_len + 1);
    if (!dst) return NULL;
    r = w = 0;

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

/* main */
int main(int argc, char *argv[]) {
    const char *output_ics_filename = "Test Class Schedule.ics";
    long sz;
    char *raw, *schedule, *location, *description, *date, *html, buf[512];
    char *filename;
    char html_files[100][256];

    Event *eventList;
    size_t html_len, schedule_len, loc_len, desc_len, date_len;
    const char *ROW_OPEN, *ROW_CLOSE, *count_pos, *start, *end, *pos, *sched_pos, *loc_pos, *desc_pos, *date_pos;
    int block, i, k, rc;
    FILE *fp;
    int count_files = 0;
    int total_blocks_all_files = 0;
    int global_block_index = 0;


    struct dirent *entry;
    DIR *dir = opendir("."); /*  current directory */

    if (dir == NULL) {
        printf("Could not open directory\n");
        return 1;
    }

    /* Array to store file names (simple version) */

    while ((entry = readdir(dir)) != NULL) {
        char *name = entry->d_name;

        /* Check if file ends with ".html" */
        int len = strlen(name);
        if (len > 5 && strcmp(name + len - 5, ".html") == 0) {
            strcpy(html_files[count_files], name);
            printf("Found HTML file: %s\n", name);
            count_files++;
        }
    }

    closedir(dir);


    /* Error handling for HTML files   */
    if (count_files == 0) { fprintf(stderr, "No HTML files found.\n"); return 1; }
    if (count_files > 1) { fprintf(stderr, "Multiple HTML files found. Processing Both.\n"); }
   /*  Print results */

    /* First pass: count total blocks across all files */
    
    for (k = 0; k < count_files; k++) {
        filename = html_files[k];
        printf("Scanning file: %s\n", filename);

        /* load */
        fp = fopen(filename, "rb");
        if (!fp) { perror(filename); continue; }
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        rewind(fp);

        raw = malloc((size_t)sz + 1);
        fread(raw, 1, (size_t)sz, fp);
        raw[sz] = '\0';
        fclose(fp);

        html = qp_decode(raw, (size_t)sz, &html_len);
        free(raw);

        if (!html) continue;

        /* Count blocks in this file */
        ROW_OPEN = "<tr id=\"trCLASS_MTG_VW";
        ROW_CLOSE = "</tr>";
        count_pos = html;
        while (1) {
            start = strstr(count_pos, ROW_OPEN);
            if (!start) break;
            end = strstr(start, ROW_CLOSE);
            if (!end) break;
            end += strlen(ROW_CLOSE);
            total_blocks_all_files++;
            count_pos = end;
        }

        free(html);
    }

    printf("Total blocks found in all files: %d\n\n", total_blocks_all_files);

    /* Allocate eventList once for all files */
    eventList = calloc((size_t)total_blocks_all_files, sizeof(Event));

    /* Second pass: process all files and fill eventList */

    for (k = 0; k < count_files; k++) {
        filename = html_files[k];

        printf("========== Processing file: %s ==========\n", filename);

        /* load */
        fp = fopen(filename, "rb");
        if (!fp) { perror(filename); continue; }
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        rewind(fp);

        raw = malloc((size_t)sz + 1);
        fread(raw, 1, (size_t)sz, fp);
        raw[sz] = '\0';
        fclose(fp);

        /* QP decode */
        schedule = NULL;
        location = NULL;
        description = NULL;
        date = NULL;
        html = qp_decode(raw, (size_t)sz, &html_len);
        free(raw);

        if (!html) continue;

        /* scan for <tr id="trCLASS_MTG_VW ... </tr> timetable blocks */
        ROW_OPEN = "<tr id=\"trCLASS_MTG_VW";
        ROW_CLOSE = "</tr>";

        pos = html;
        block = 0;

    while (1) {
        /* find next opening tag */
        start = strstr(pos, ROW_OPEN);
        if (!start) break;

        /* find the matching closing tag */
        end = strstr(start, ROW_CLOSE);
        if (!end) break;
        end += strlen(ROW_CLOSE);   /* include </tr> itself */

        /* extract and print sub-blocks */
        /* printf("=== BLOCK %d ===\n", block++); */
        
        /* Schedule */
        sched_pos = strstr(start, "MTG_SCHED$");
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
        /* Location */
        loc_pos = strstr(start, "MTG_LOC$");
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
        
        /* Description (instructors) */
        desc_pos = strstr(start, "DERIVED_CLS_DTL_SSR_INSTR_LONG$");
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

        /* Date */
        date_pos = strstr(start, "MTG_DATES$");
        if (date_pos && date_pos < end) {
            const char *date_td = date_pos;
            while (date_td > start && strncmp(date_td, "<td", 3) != 0) date_td--;
            if (date_td >= start && strncmp(date_td, "<td", 3) == 0) {
                const char *date_end = strstr(date_td, "</td>");
                if (date_end) {
                    date_end += 5;
                    date_len = date_end - date_td;
                    date = malloc(date_len + 1);
                    if (date) {
                        memcpy(date, date_td, date_len);
                        date[date_len] = '\0';
                    }
                }
            }
        }
        

        
        
        /* printf("\n"); */
        /* free schedule */


        rc = fsm_function(schedule, buf, sizeof(buf));
        if (rc != FSM_ACC) {
            fprintf(stderr, "fsm error %d on block %d\n", rc, block);
            eventList[global_block_index].schedule = malloc(6);
            eventList[global_block_index].schedule = "Error";
        } else {
            eventList[global_block_index].schedule = malloc(strlen(buf) + 1);
            strcpy(eventList[global_block_index].schedule, buf);
            /* printf("Schedule:%s\n", schedule_text); */
            free(schedule);
            schedule = NULL;
        }
        rc = fsm_function(location, buf, sizeof(buf));
        if (rc != FSM_ACC) {
            fprintf(stderr, "fsm error %d on block %d\n", rc, block);
            eventList[global_block_index].location = malloc(6);
            eventList[global_block_index].location = "Error";
        } else {
            eventList[global_block_index].location = malloc(strlen(buf) + 1);
            strcpy(eventList[global_block_index].location, buf);
            /* printf("Schedule:%s\n", schedule_text); */
            free(schedule);
            schedule = NULL;
        }
        rc = fsm_function(description, buf, sizeof(buf));
        if (rc != FSM_ACC) {
            fprintf(stderr, "fsm error %d on block %d\n", rc, block);
            eventList[global_block_index].description = malloc(6);
            eventList[global_block_index].description = "Error";
        } else {
            eventList[global_block_index].description = malloc(strlen(buf) + 1);
            strcpy(eventList[global_block_index].description, buf);
            /* printf("Schedule:%s\n", schedule_text); */
            free(schedule);
            schedule = NULL;
        }
        rc = fsm_function(date, buf, sizeof(buf));
        if (rc != FSM_ACC) {
            fprintf(stderr, "fsm error %d on block %d\n", rc, block);
            eventList[global_block_index].dateStart = malloc(6);
            eventList[global_block_index].dateStart = "Error";
            eventList[global_block_index].dateEnd = malloc(6);
            eventList[global_block_index].dateEnd = "Error";
        } else {
            split_date_range(buf, &eventList[global_block_index].dateStart, &eventList[global_block_index].dateEnd);
            /* printf("Date:%s\n", date_text); */
            free(date);
            date = NULL;
        }
        
        block++;
        global_block_index++;

        pos = end;
    }

    printf("Blocks found in this file: %d\n", block);
    
    free(html);
    }  /* End of file processing loop */

    if (convert_events_to_ics(output_ics_filename, eventList, (size_t)global_block_index) != 0) {
        fprintf(stderr, "Failed to convert parsed events into ICS output.\n");
    } else {
        printf("Generated %s with parsed class events.\n", output_ics_filename);
    }

    /* printf("\n========== All events from all files ==========\n"); */
    /* Run this if output needs to be checked */
    /*    for(j = 0; j < total_blocks_all_files; j++) {
        printf("=== EVENT %d ===\n", j);
        printf("Schedule: %s ", eventList[j].schedule);
        printf("Location: %s ", eventList[j].location);
        printf("Description: %s ", eventList[j].description);
        printf("Date Start: %s ", eventList[j].dateStart ? eventList[j].dateStart : ""); 
        printf("Date End: %s\n", eventList[j].dateEnd ? eventList[j].dateEnd : "");
    } */

    print_errors();

    for(i = 0; i < total_blocks_all_files; i++) {
        free(eventList[i].schedule);
        free(eventList[i].location);
        free(eventList[i].description);
        free(eventList[i].dateStart);
        free(eventList[i].dateEnd);
    }
    free(eventList);

    /* printf("\n========== Processing complete ==========\n"); */
    return 0;
}
