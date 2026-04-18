/*This file contains the logic required, that takes class event structs as input
  and writes them into a .ics file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "ics_writer.h"

#define INITIAL_CAPACITY 64

static int to_lower_ascii(int c)
{
  if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  }
  return c;
}

static int is_space_ascii(int c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static int is_digit_ascii(int c)
{
  return c >= '0' && c <= '9';
}

static int is_leap_year(int year)
{
  if (year % 400 == 0) return 1;
  if (year % 100 == 0) return 0;
  return (year % 4 == 0);
}

static int days_in_month(int year, int month)
{
  static const int dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (month < 1 || month > 12) {
    return 0;
  }
  if (month == 2 && is_leap_year(year)) {
    return 29;
  }
  return dim[month - 1];
}

/* Returns 0=Sun ... 6=Sat */
static int weekday_index(int year, int month, int day)
{
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  int y = year;

  if (month < 3) {
    y -= 1;
  }

  return (y + y / 4 - y / 100 + y / 400 + t[month - 1] + day) % 7;
}

static int compare_dates(int y1, int m1, int d1, int y2, int m2, int d2)
{
  if (y1 != y2) return (y1 < y2) ? -1 : 1;
  if (m1 != m2) return (m1 < m2) ? -1 : 1;
  if (d1 != d2) return (d1 < d2) ? -1 : 1;
  return 0;
}

static void add_days(int *year, int *month, int *day, int delta)
{
  int y = *year;
  int m = *month;
  int d = *day;

  while (delta > 0) {
    int dim = days_in_month(y, m);
    d++;
    if (d > dim) {
      d = 1;
      m++;
      if (m > 12) {
        m = 1;
        y++;
      }
    }
    delta--;
  }

  *year = y;
  *month = m;
  *day = d;
}

static int equals_ignore_case(const char *a, const char *b)
{
  while (*a && *b) {
    if (to_lower_ascii((unsigned char)*a) != to_lower_ascii((unsigned char)*b)) {
      return 0;
    }
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

static int starts_with_ignore_case(const char *text, const char *prefix)
{
  while (*prefix) {
    if (*text == '\0') {
      return 0;
    }
    if (to_lower_ascii((unsigned char)*text) != to_lower_ascii((unsigned char)*prefix)) {
      return 0;
    }
    text++;
    prefix++;
  }
  return 1;
}

static void trim_in_place(char *s)
{
  size_t len;
  size_t start = 0;

  if (!s) {
    return;
  }

  len = strlen(s);
  while (start < len && is_space_ascii((unsigned char)s[start])) {
    start++;
  }

  if (start > 0) {
    memmove(s, s + start, len - start + 1);
    len = strlen(s);
  }

  while (len > 0 && is_space_ascii((unsigned char)s[len - 1])) {
    s[len - 1] = '\0';
    len--;
  }
}

static int parse_date_flexible(const char *src, int *year, int *month, int *day)
{
  int a;
  int b;
  int y;

  if (!src || !year || !month || !day) {
    return 0;
  }

  if (sscanf(src, " %d/%d/%d", &a, &b, &y) != 3) {
    return 0;
  }

  /* Accept both dd/mm/yyyy and mm/dd/yyyy; prefer dd/mm when ambiguous. */
  if (a > 12) {
    *day = a;
    *month = b;
  } else if (b > 12) {
    *month = a;
    *day = b;
  } else {
    *day = a;
    *month = b;
  }

  if (*month < 1 || *month > 12 || *day < 1 || *day > 31 || y < 1900) {
    return 0;
  }

  *year = y;
  return 1;
}

static int parse_time_at(const char *s, size_t offset, int *consumed, int *hour, int *minute)
{
  int h;
  int m;
  int n = 0;
  char marker[3] = {0};
  const char *p = s + offset;

  if (sscanf(p, "%d:%d%2[AaPpMm]%n", &h, &m, marker, &n) >= 2 && n > 0) {
    if (m < 0 || m > 59 || h < 0 || h > 23) {
      return 0;
    }

    if (marker[0] != '\0') {
      int is_pm = (to_lower_ascii((unsigned char)marker[0]) == 'p');
      if (h < 1 || h > 12) {
        return 0;
      }
      if (h == 12) {
        h = is_pm ? 12 : 0;
      } else if (is_pm) {
        h += 12;
      }
    }

    *hour = h;
    *minute = m;
    *consumed = n;
    return 1;
  }

  n = 0;
  marker[0] = '\0';
  if (sscanf(p, "%d:%d %2[AaPpMm]%n", &h, &m, marker, &n) >= 2 && n > 0) {
    if (m < 0 || m > 59 || h < 0 || h > 23) {
      return 0;
    }

    if (marker[0] != '\0') {
      int is_pm = (to_lower_ascii((unsigned char)marker[0]) == 'p');
      if (h < 1 || h > 12) {
        return 0;
      }
      if (h == 12) {
        h = is_pm ? 12 : 0;
      } else if (is_pm) {
        h += 12;
      }
    }

    *hour = h;
    *minute = m;
    *consumed = n;
    return 1;
  }

  return 0;
}

static int parse_time_range(const char *text, int *start_hour, int *start_min, int *end_hour, int *end_min)
{
  size_t i;
  int found = 0;
  int consumed = 0;
  int h = 0;
  int m = 0;

  if (!text) {
    return 0;
  }

  for (i = 0; text[i] != '\0'; i++) {
    if (!is_digit_ascii((unsigned char)text[i])) {
      continue;
    }

    if (parse_time_at(text, i, &consumed, &h, &m)) {
      if (found == 0) {
        *start_hour = h;
        *start_min = m;
        found = 1;
      } else {
        *end_hour = h;
        *end_min = m;
        return 1;
      }
      i += (size_t)consumed;
    }
  }

  return 0;
}

static int weekday_to_tm_index(const char *schedule)
{
  char copy[512];
  char *token;
  const char *delims = " \t\r\n-:,()";

  if (!schedule) {
    return -1;
  }

  strncpy(copy, schedule, sizeof(copy) - 1);
  copy[sizeof(copy) - 1] = '\0';

  token = strtok(copy, delims);
  while (token) {
    if (starts_with_ignore_case(token, "Mon") || equals_ignore_case(token, "Mo")) return 1;
    if (starts_with_ignore_case(token, "Tue") || equals_ignore_case(token, "Tu")) return 2;
    if (starts_with_ignore_case(token, "Wed") || equals_ignore_case(token, "We")) return 3;
    if (starts_with_ignore_case(token, "Thu") || equals_ignore_case(token, "Th")) return 4;
    if (starts_with_ignore_case(token, "Fri") || equals_ignore_case(token, "Fr")) return 5;
    if (starts_with_ignore_case(token, "Sat") || equals_ignore_case(token, "Sa")) return 6;
    if (starts_with_ignore_case(token, "Sun") || equals_ignore_case(token, "Su")) return 0;
    token = strtok(NULL, delims);
  }

  return -1;
}

static void pick_summary(const char *schedule, char *summary, size_t summary_size)
{
  char copy[512];
  char *line;

  summary[0] = '\0';
  if (!schedule) {
    return;
  }

  strncpy(copy, schedule, sizeof(copy) - 1);
  copy[sizeof(copy) - 1] = '\0';

  line = strtok(copy, "\r\n");
  while (line) {
    trim_in_place(line);
    if (line[0] != '\0' && !parse_time_range(line, &(int){0}, &(int){0}, &(int){0}, &(int){0})) {
      strncpy(summary, line, summary_size - 1);
      summary[summary_size - 1] = '\0';
      return;
    }
    line = strtok(NULL, "\r\n");
  }

  strncpy(summary, schedule, summary_size - 1);
  summary[summary_size - 1] = '\0';
  trim_in_place(summary);
}

static void format_dt(char *buffer, size_t size, int year, int month, int day, int hour, int minute)
{
  snprintf(buffer, size, "%04d%02d%02dT%02d%02d00", year, month, day, hour, minute);
}

static void append_escaped_text(FILE *fp, const char *text)
{
  size_t i;
  if (!text) {
    return;
  }

  for (i = 0; text[i] != '\0'; i++) {
    char c = text[i];
    if (c == '\\' || c == ';' || c == ',') {
      fputc('\\', fp);
      fputc(c, fp);
    } else if (c == '\n' || c == '\r') {
      fputs("\\n", fp);
    } else {
      fputc(c, fp);
    }
  }
}

int write_ics_file(const char *output_path, const ClassEvent *events, size_t event_count)
{
  size_t i;
  FILE *fp;
  char stamp[32];

  if (!output_path || (!events && event_count > 0)) {
    return -1;
  }

  fp = fopen(output_path, "w");
  if (!fp) {
    return -1;
  }

  strncpy(stamp, "19700101T000000Z", sizeof(stamp) - 1);
  stamp[sizeof(stamp) - 1] = '\0';

  fprintf(fp, "BEGIN:VCALENDAR\r\n");
  fprintf(fp, "PRODID:-//compilerForIcs//ICS Writer//EN\r\n");
  fprintf(fp, "VERSION:2.0\r\n");
  fprintf(fp, "CALSCALE:GREGORIAN\r\n");

  for (i = 0; i < event_count; i++) {
    fprintf(fp, "BEGIN:VEVENT\r\n");
    fprintf(fp, "UID:%zu@compilerForIcs\r\n", i + 1);
    fprintf(fp, "DTSTAMP:%s\r\n", stamp);
    fprintf(fp, "DTSTART:%s\r\n", events[i].dtstart);
    fprintf(fp, "DTEND:%s\r\n", events[i].dtend);

    fprintf(fp, "SUMMARY:");
    append_escaped_text(fp, events[i].summary);
    fprintf(fp, "\r\n");

    fprintf(fp, "LOCATION:");
    append_escaped_text(fp, events[i].location);
    fprintf(fp, "\r\n");

    fprintf(fp, "DESCRIPTION:");
    append_escaped_text(fp, events[i].description);
    fprintf(fp, "\r\n");

    fprintf(fp, "END:VEVENT\r\n");
  }

  fprintf(fp, "END:VCALENDAR\r\n");
  fclose(fp);
  return 0;
}

int convert_events_to_ics(const char *output_path, const Event *events, size_t event_count)
{
  size_t i;
  int rc;
  size_t out_count = 0;
  size_t out_cap = INITIAL_CAPACITY;
  ClassEvent *out_events;

  if (!events || event_count == 0 || !output_path) {
    return -1;
  }

  out_events = malloc(out_cap * sizeof(ClassEvent));
  if (!out_events) {
    return -1;
  }

  for (i = 0; i < event_count; i++) {
    int start_year;
    int start_month;
    int start_day;
    int end_year;
    int end_month;
    int end_day;
    int start_hour = 9;
    int start_min = 0;
    int end_hour = 10;
    int end_min = 0;
    int weekday;
    int current_year;
    int current_month;
    int current_day;

    if (!parse_date_flexible(events[i].dateStart, &start_year, &start_month, &start_day)) {
      continue;
    }

    if (!parse_date_flexible(events[i].dateEnd, &end_year, &end_month, &end_day)) {
      end_year = start_year;
      end_month = start_month;
      end_day = start_day;
    }

    if (events[i].schedule) {
      parse_time_range(events[i].schedule, &start_hour, &start_min, &end_hour, &end_min);
    }

    if (compare_dates(start_year, start_month, start_day, end_year, end_month, end_day) > 0) {
      continue;
    }

    current_year = start_year;
    current_month = start_month;
    current_day = start_day;

    weekday = weekday_to_tm_index(events[i].schedule);
    if (weekday >= 0) {
      while (weekday_index(current_year, current_month, current_day) != weekday) {
        add_days(&current_year, &current_month, &current_day, 1);
        if (compare_dates(current_year, current_month, current_day, end_year, end_month, end_day) > 0) {
          break;
        }
      }
    }

    while (compare_dates(current_year, current_month, current_day, end_year, end_month, end_day) <= 0) {
      ClassEvent ce;

      if (out_count == out_cap) {
        ClassEvent *next;
        out_cap *= 2;
        next = realloc(out_events, out_cap * sizeof(ClassEvent));
        if (!next) {
          free(out_events);
          return -1;
        }
        out_events = next;
      }

      memset(&ce, 0, sizeof(ce));
      format_dt(ce.dtstart, sizeof(ce.dtstart), current_year, current_month, current_day, start_hour, start_min);
      format_dt(ce.dtend, sizeof(ce.dtend), current_year, current_month, current_day, end_hour, end_min);

      pick_summary(events[i].schedule ? events[i].schedule : "Class", ce.summary, sizeof(ce.summary));
      strncpy(ce.location, events[i].location ? events[i].location : "TBA", sizeof(ce.location) - 1);
      ce.location[sizeof(ce.location) - 1] = '\0';
      strncpy(ce.description, events[i].description ? events[i].description : "", sizeof(ce.description) - 1);
      ce.description[sizeof(ce.description) - 1] = '\0';

      out_events[out_count++] = ce;

      if (weekday >= 0) {
        add_days(&current_year, &current_month, &current_day, 7);
      } else {
        break;
      }
    }
  }

  if (out_count == 0) {
    free(out_events);
    return -1;
  }

  rc = write_ics_file(output_path, out_events, out_count);
  free(out_events);
  return rc;
}
