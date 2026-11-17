# HMS-CSD-project

## Build and run

Compile the prototype with both source files:

```sh
gcc -std=c99 -Wall -Wextra -pedantic admin.c login.c -o hospital
```

On Windows, run the generated program with `.\hospital.exe`.

The administrator creates each doctor's login when adding the doctor. Patients
can register and then log in from the patient option. Patient bookings and
consultation records are associated with the logged-in account; doctors can
only view their own roster and record consultations for their own appointments.

This is an educational, in-memory prototype: all accounts and hospital data are
lost when the program exits, and passwords are not stored securely. Do not use
it for real patients or sensitive medical information. The administrator option
also remains unprotected.