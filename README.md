# HMS-CSD-project

## Build and run

From the project root, compile all modules:

```powershell
gcc -std=c99 -Wall -Wextra -pedantic -I. -Iutility -Ilogin main.c admin\admin.c admin\admin_login.c docter\doctor.c docter\doctor_login.c patient\patient.c patient\patient_login.c nurse\nurse.c nurse\nurse_login.c utility\hospital_state.c utility\hospital_common.c utility\ui.c utility\util.c -o hospital.exe
```

Run the program on Windows with:

```powershell
.\hospital.exe
```

## Folder layout

- `admin/` - administrator functions and administrator login
- `docter/` - doctor functions and doctor login
- `patient/` - patient functions and patient registration/login
- `nurse/` - nurse management, login, attendance, and duty scheduling
- `login/` - shared login declarations
- `utility/` - input helpers, shared hospital data, date/time helpers, and console formatting
- `main.c` - starts the application and routes users to their role menus
- `hospital.h` - shared data structures and module interfaces

Administrators create nurse logins when adding nurses and can record attendance
for scheduled shifts, including the assigned work area or ward. Nurses can view
their weekly shift/team assignments and their own attendance percentage. The
percentage is present shifts divided by shifts explicitly marked Present or
Absent. The administrator demo password is `admin123`.

This educational prototype keeps accounts and hospital data in memory, so they
are lost when the program exits. Passwords are not stored securely. Do not use
it for real patients or sensitive medical information.
