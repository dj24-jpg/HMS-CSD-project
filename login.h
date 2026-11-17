#ifndef LOGIN_H
#define LOGIN_H

#include <stddef.h>

void registerDoctorAccount(int doctorId);
int loginDoctor(int *doctorId);
int loginPatient(char *username, size_t usernameSize,
                 char *fullName, size_t fullNameSize);

#endif
