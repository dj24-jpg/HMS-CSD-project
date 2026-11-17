#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readLine(const char *prompt, char *out, size_t size)
{
    size_t length;

    printf("%s", prompt);
    fflush(stdout);
    if (fgets(out, (int)size, stdin) == NULL) {
        puts("\nInput closed. Exiting.");
        exit(0);
    }

    length = strcspn(out, "\r\n");
    if (out[length] != '\0') {
        out[length] = '\0';
    } else {
        int ch;                 /* line was longer than the buffer: discard rest */
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
}

void readRequired(const char *prompt, char *out, size_t size)
{
    do {
        readLine(prompt, out, size);
        if (out[0] == '\0')
            puts("This field cannot be empty.");
    } while (out[0] == '\0');
}

int readIntOrInvalid(const char *prompt)
{
    char line[32];
    char *end;
    long value;

    readLine(prompt, line, sizeof(line));
    value = strtol(line, &end, 10);
    if (end == line || *end != '\0' || value < 0 || value > 2147483647L)
        return -1;
    return (int)value;
}

int readIntRange(const char *prompt, int min, int max)
{
    int value;

    for (;;) {
        value = readIntOrInvalid(prompt);
        if (value >= min && value <= max)
            return value;
        printf("Please enter a whole number from %d to %d.\n", min, max);
    }
}

double readAmount(const char *prompt)
{
    char line[32];
    char *end;
    double value;

    for (;;) {
        readLine(prompt, line, sizeof(line));
        value = strtod(line, &end);
        if (end != line && *end == '\0' && value >= 0 && value < 1e9)
            return value;
        puts("Please enter a valid non-negative amount.");
    }
}
