#include "login.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define MAX_PATIENT_ACCOUNTS 500
#define MAX_USERNAME 64
#define MAX_PASSWORD 64
#define MAX_NAME 100

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char fullName[MAX_NAME];
} PatientAccount;

static PatientAccount patients[MAX_PATIENT_ACCOUNTS];
static int patientCount;

static PatientAccount *findPatient(const char *username)
{
    int i;

    for (i = 0; i < patientCount; ++i)
        if (strcmp(patients[i].username, username) == 0)
            return &patients[i];
    return NULL;
}

static void registerPatient(void)
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char fullName[MAX_NAME];

    if (patientCount >= MAX_PATIENT_ACCOUNTS) {
        puts("Patient account list is full.");
        return;
    }

    readRequired("Choose username: ", username, sizeof(username));
    if (findPatient(username)) {
        puts("That username is already registered.");
        return;
    }
    readRequired("Choose password: ", password, sizeof(password));
    readRequired("Full name: ", fullName, sizeof(fullName));

    snprintf(patients[patientCount].username,
             sizeof(patients[patientCount].username), "%s", username);
    snprintf(patients[patientCount].password,
             sizeof(patients[patientCount].password), "%s", password);
    snprintf(patients[patientCount].fullName,
             sizeof(patients[patientCount].fullName), "%s", fullName);
    ++patientCount;
    puts("Patient account created. You can now log in.");
}

static int authenticatePatient(char *username, size_t usernameSize,
                               char *fullName, size_t fullNameSize)
{
    char enteredUsername[MAX_USERNAME];
    char password[MAX_PASSWORD];
    PatientAccount *patient;

    readRequired("Username: ", enteredUsername, sizeof(enteredUsername));
    readRequired("Password: ", password, sizeof(password));
    patient = findPatient(enteredUsername);
    if (!patient || strcmp(patient->password, password) != 0) {
        puts("Invalid username or password.");
        return 0;
    }

    snprintf(username, usernameSize, "%s", patient->username);
    snprintf(fullName, fullNameSize, "%s", patient->fullName);
    return 1;
}

int loginPatient(char *username, size_t usernameSize,
                 char *fullName, size_t fullNameSize)
{
    int choice;

    for (;;) {
        puts("\nPATIENT ACCOUNT");
        puts("1 Register  2 Log in  0 Back");
        choice = readIntOrInvalid("Choice: ");
        switch (choice) {
        case 1:
            registerPatient();
            break;
        case 2:
            return authenticatePatient(username, usernameSize,
                                       fullName, fullNameSize);
        case 0:
            return 0;
        default:
            puts("Invalid choice.");
        }
    }
}
