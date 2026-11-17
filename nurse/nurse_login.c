#include "hospital.h"
#include "login.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define MAX_PASSWORD 64

typedef struct {
    int nurseId;
    char password[MAX_PASSWORD];
} NurseAccount;

static NurseAccount nurseAccounts[MAX_NURSES];
static int nurseAccountCount;

void registerNurseAccount(int nurseId)
{
    char password[MAX_PASSWORD];

    if (nurseAccountCount >= MAX_NURSES) {
        puts("Nurse login list is full.");
        return;
    }

    readRequired("Set nurse login password: ", password, sizeof(password));
    nurseAccounts[nurseAccountCount].nurseId = nurseId;
    snprintf(nurseAccounts[nurseAccountCount].password,
             sizeof(nurseAccounts[nurseAccountCount].password), "%s", password);
    ++nurseAccountCount;
    puts("Nurse login created.");
}

int loginNurse(int *nurseId)
{
    int i, id;
    char password[MAX_PASSWORD];

    id = readIntOrInvalid("Nurse ID: ");
    if (id < 1) {
        puts("Invalid nurse ID.");
        return 0;
    }
    readRequired("Password: ", password, sizeof(password));

    for (i = 0; i < nurseAccountCount; ++i) {
        if (nurseAccounts[i].nurseId == id &&
            strcmp(nurseAccounts[i].password, password) == 0) {
            *nurseId = id;
            return 1;
        }
    }

    puts("Invalid nurse ID or password.");
    return 0;
}
