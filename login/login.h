#ifndef LOGIN_H
#define LOGIN_H

#include <stddef.h>

void registerDoctorAccount(int doctorId);
int loginDoctor(int *doctorId);
void registerNurseAccount(int nurseId);
int loginNurse(int *nurseId);
int loginPatient(char *username, size_t usernameSize,
                 char *fullName, size_t fullNameSize);
int loginAdmin(void);

#endif
