#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/* Prints prompt, reads one line (without newline) into out. Exits on EOF. */
void readLine(const char *prompt, char *out, size_t size);

/* Re-asks until the line is non-empty. */
void readRequired(const char *prompt, char *out, size_t size);

/* Returns a whole number >= 0, or -1 if the input is not a valid number. */
int readIntOrInvalid(const char *prompt);

/* Re-asks until a whole number in [min, max] is entered. */
int readIntRange(const char *prompt, int min, int max);

/* Re-asks until a non-negative amount is entered. */
double readAmount(const char *prompt);

#endif
