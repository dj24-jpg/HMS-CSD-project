#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATIENT_ACCOUNTS 500
#define MAX_USERNAME 64
#define MAX_PASSWORD 64
#define MAX_NAME 100
#define MAX_DOCTORS 100

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char fullName[MAX_NAME];
} PatientAccount;

typedef struct {
    int doctorId;
    char password[MAX_PASSWORD];
} DoctorAccount;

static PatientAccount patients[MAX_PATIENT_ACCOUNTS];
static DoctorAccount doctors[MAX_DOCTORS];
static int patientCount;
static int doctorCount;

static void readLine(const char *prompt, char *out, size_t size)
{
    size_t length;

    printf("%s", prompt);
    if (fgets(out, (int)size, stdin) == NULL) {
        out[0] = '\0';
        return;
    }

    length = strcspn(out, "\n");
    if (out[length] == '\n') {
        out[length] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
}

static int readInt(const char *prompt)
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

static void readRequired(const char *prompt, char *out, size_t size)
{
    do {
        readLine(prompt, out, size);
        if (out[0] == '\0')
            puts("This field cannot be empty.");
    } while (out[0] == '\0');
}

void registerDoctorAccount(int doctorId)
{
    char password[MAX_PASSWORD];

    if (doctorCount >= MAX_DOCTORS) {
        puts("Doctor login list is full.");
        return;
    }

    readRequired("Set doctor login password: ", password, sizeof(password));
    doctors[doctorCount].doctorId = doctorId;
    snprintf(doctors[doctorCount].password,
             sizeof(doctors[doctorCount].password), "%s", password);
    ++doctorCount;
    puts("Doctor login created.");
}

int loginDoctor(int *doctorId)
{
    int i, id;
    char password[MAX_PASSWORD];

    id = readInt("Doctor ID: ");
    if (id < 1) {
        puts("Invalid doctor ID.");
        return 0;
    }
    readRequired("Password: ", password, sizeof(password));

    for (i = 0; i < doctorCount; ++i) {
        if (doctors[i].doctorId == id &&
            strcmp(doctors[i].password, password) == 0) {
            *doctorId = id;
            return 1;
        }
    }

    puts("Invalid doctor ID or password.");
    return 0;
}

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
        choice = readInt("Choice: ");
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