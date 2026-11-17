#include "login.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define ADMIN_PASSWORD "admin123"
#define MAX_PASSWORD 64

int loginAdmin(void)
{
    char password[MAX_PASSWORD];
    int attempt;

    for (attempt = 0; attempt < 3; ++attempt) {
        readRequired("Administrator password: ", password, sizeof(password));
        if (strcmp(password, ADMIN_PASSWORD) == 0)
            return 1;
        puts("Wrong password.");
    }
    puts("Too many failed attempts.");
    return 0;
}
