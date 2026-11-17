#include "hospital.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static void recordConsultation(int doctorId)
{
    int i, minute;
    char date[DATE_LEN];
    long dayNumber;

    dayNumber = readDate("Appointment date (YYYY-MM-DD): ", date);
    minute = readTime("Appointment time (HH:MM): ");

    for (i = 0; i < appointmentCount; ++i) {
        Appointment *a = &appointments[i];
        if (a->doctorId != doctorId || a->dayNumber != dayNumber ||
            a->timeMinute != minute)
            continue;

        if (a->visited) {
            puts("A consultation record already exists for this appointment.");
            return;
        }
        printf("Patient: %s\n", a->patient);
        readRequired("Diagnosis: ", a->diagnosis, sizeof(a->diagnosis));
        readRequired("Prescription: ", a->prescription, sizeof(a->prescription));
        a->visited = 1;
        puts("Consultation record saved.");
        return;
    }
    puts("Appointment not found.");
}

static void showDoctorAppointments(int doctorId)
{
    int i, found = 0;
    char text[8];

    printPanelTitle("MY APPOINTMENTS");
    puts("+------------+-------+----------------------+------------+");
    puts("| DATE       | TIME  | PATIENT              | STATUS     |");
    puts("+------------+-------+----------------------+------------+");
    for (i = 0; i < appointmentCount; ++i) {
        Appointment *a = &appointments[i];
        if (a->doctorId != doctorId)
            continue;
        formatTime(a->timeMinute, text, sizeof(text));
        printf("| %-10.10s | %-5.5s | %-20.20s | %-10.10s |\n",
               a->date, text, a->patient,
               a->visited ? "Completed" : "Pending");
        found = 1;
    }
    puts("+------------+-------+----------------------+------------+");
    if (!found)
        puts("No appointments.");
}

void doctorMenu(int doctorId)
{
    int choice;

    do {
        printPanelTitle("DOCTOR MENU");
        printMenuOption(1, "View my weekly roster");
        printMenuOption(2, "View my appointments");
        printMenuOption(3, "Record a completed consultation");
        printMenuOption(0, "Back");
        choice = readIntOrInvalid("Choice: ");

        switch (choice) {
        case 1: printWeeklyRoster(doctorId, 0); break;
        case 2: showDoctorAppointments(doctorId); break;
        case 3: recordConsultation(doctorId); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}
