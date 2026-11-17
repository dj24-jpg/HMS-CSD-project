#include "hospital.h"
#include "login.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define MAX_PASSWORD 64

typedef struct {
    int doctorId;
    char password[MAX_PASSWORD];
} DoctorAccount;

static DoctorAccount doctorAccounts[MAX_DOCTORS];
static int doctorAccountCount;

void registerDoctorAccount(int doctorId)
{
    char password[MAX_PASSWORD];

    if (doctorAccountCount >= MAX_DOCTORS) {
        puts("Doctor login list is full.");
        return;
    }

    readRequired("Set doctor login password: ", password, sizeof(password));
    doctorAccounts[doctorAccountCount].doctorId = doctorId;
    snprintf(doctorAccounts[doctorAccountCount].password,
             sizeof(doctorAccounts[doctorAccountCount].password), "%s", password);
    ++doctorAccountCount;
    puts("Doctor login created.");
}

int loginDoctor(int *doctorId)
{
    int i, id;
    char password[MAX_PASSWORD];

    id = readIntOrInvalid("Doctor ID: ");
    if (id < 1) {
        puts("Invalid doctor ID.");
        return 0;
    }
    readRequired("Password: ", password, sizeof(password));

    for (i = 0; i < doctorAccountCount; ++i) {
        if (doctorAccounts[i].doctorId == id &&
            strcmp(doctorAccounts[i].password, password) == 0) {
            *doctorId = id;
            return 1;
        }
    }

    puts("Invalid doctor ID or password.");
    return 0;
}
