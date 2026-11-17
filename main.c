#include "hospital.h"
#include "login.h"
#include "util.h"

#include <stdio.h>

void runHospital(void)
{
    int choice;
    int doctorId, nurseId;
    char patientUsername[MAX_TEXT], patientName[MAX_TEXT];

    do {
        printPanelTitle("HOSPITAL MANAGEMENT SYSTEM");
        printMenuOption(1, "Administrator login");
        printMenuOption(2, "Patient login");
        printMenuOption(3, "Doctor login");
        printMenuOption(4, "Nurse login");
        printMenuOption(0, "Exit");
        choice = readIntOrInvalid("Choice: ");

        switch (choice) {
        case 1:
            if (loginAdmin())
                adminMenu();
            break;
        case 2:
            if (loginPatient(patientUsername, sizeof(patientUsername),
                             patientName, sizeof(patientName)))
                patientMenu(patientUsername, patientName);
            break;
        case 3:
            if (loginDoctor(&doctorId)) {
                if (findDoctor(doctorId))
                    doctorMenu(doctorId);
                else
                    puts("This doctor account is no longer active.");
            }
            break;
        case 4:
            if (loginNurse(&nurseId)) {
                if (findNurse(nurseId))
                    nurseDashboard(nurseId);
                else
                    puts("This nurse account is no longer active.");
            }
            break;
        case 0:
            puts("Goodbye.");
            break;
        default:
            puts("Invalid choice.");
        }
    } while (choice != 0);
}

int main(void)
{
    runHospital();
    return 0;
}
