
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fsm.c"

/* =========================================================
 * Quoted-Printable decoder
 * ========================================================*/
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
            if (r + 2 < src_len && isxdigit(n1)) {
                unsigned char n2 = (unsigned char)src[r + 2];
                if (isxdigit(n2)) {
                    int hi = isdigit(n1) ? n1-'0' : toupper(n1)-'A'+10;
                    int lo = isdigit(n2) ? n2-'0' : toupper(n2)-'A'+10;
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

/* =========================================================
 * main
 * ========================================================*/
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

    /* QP decode */
    size_t html_len = 0;
    char *html = qp_decode(raw, (size_t)sz, &html_len);
    free(raw);

    /* scan for <tr id="trCLASS_MTG_VW ... </tr> timetable blocks */
    const char *ROW_OPEN  = "<tr id=\"trCLASS_MTG_VW";
    const char *ROW_CLOSE = "</tr>";

    const char *pos = html;
    int block = 0;
/* declare your target function */

char *main_function(char *input);

while (1) {
    const char *start = strstr(pos, ROW_OPEN);
    if (!start) break;

    const char *end = strstr(start, ROW_CLOSE);
    if (!end) break;
    end += strlen(ROW_CLOSE);

    size_t block_len = (size_t)(end - start);

    /* single variable for this iteration */
    char *block_html = malloc(block_len + 1);
    if (!block_html) {
        perror("malloc");
        break;
    }

    memcpy(block_html, start, block_len);
    block_html[block_len] = '\0';

    /* pass to another function */
    char *result = main_function(block_html);
    printf("Block %d:\n%s\n\n", ++block, result);

    free(block_html);   /* free after function call */
    pos = end;
}
    fprintf(stderr, "Total blocks: %d\n", block);

    free(html);
    return 0;
}