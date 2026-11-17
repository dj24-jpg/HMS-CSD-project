#include "hospital.h"

#include <stdio.h>

static void printRule(void)
{
    int i;

    putchar('+');
    for (i = 0; i < 70; ++i)
        putchar('-');
    puts("+");
}

void printPanelTitle(const char *title)
{
    printRule();
    printf("| %-68.68s |\n", title);
    printRule();
}

void printMenuOption(int option, const char *label)
{
    printf("  %2d. %s\n", option, label);
}
