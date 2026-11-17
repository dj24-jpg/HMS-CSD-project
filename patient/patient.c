#include "hospital.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static int appointmentSlotTaken(int doctorId, long dayNumber, int minute)
{
    int i;
    for (i = 0; i < appointmentCount; ++i)
        if (appointments[i].doctorId == doctorId &&
            appointments[i].dayNumber == dayNumber &&
            appointments[i].timeMinute == minute)
            return 1;
    return 0;
}

static int slotInRoster(int doctorId, int day, int minute)
{
    int i;
    for (i = 0; i < rosterCount; ++i) {
        if (roster[i].doctorId == doctorId && roster[i].day == day &&
            minute >= roster[i].startMinute &&
            minute + SLOT_MINUTES <= roster[i].endMinute &&
            (minute - roster[i].startMinute) % SLOT_MINUTES == 0)
            return 1;
    }
    return 0;
}

static int showFreeSlots(int doctorId, long dayNumber)
{
    int i, t, count = 0, day = weekdayOf(dayNumber);
    char text[8];

    for (i = 0; i < rosterCount; ++i) {
        if (roster[i].doctorId != doctorId || roster[i].day != day)
            continue;
        for (t = roster[i].startMinute; t + SLOT_MINUTES <= roster[i].endMinute;
             t += SLOT_MINUTES) {
            if (!appointmentSlotTaken(doctorId, dayNumber, t)) {
                formatTime(t, text, sizeof(text));
                printf("  %s", text);
                ++count;
            }
        }
    }
    if (count)
        putchar('\n');
    return count;
}

static void listAvailableDoctors(void)
{
    printRosterDay(readDay(), 0, 1);
}

static void viewFreeSlots(void)
{
    char date[DATE_LEN];
    long dayNumber;
    int id;
    Doctor *d;

    id = readIntOrInvalid("Doctor ID: ");
    d = findDoctor(id);
    if (!d) {
        puts("Active doctor not found.");
        return;
    }
    dayNumber = readDate("Date (YYYY-MM-DD): ", date);
    printf("Free %d-minute slots on %s (%s):\n", SLOT_MINUTES, date,
           dayNames[weekdayOf(dayNumber)]);
    if (!showFreeSlots(id, dayNumber))
        puts("  None.");
}

static void bookAppointment(const char *patientUsername, const char *patientName)
{
    char date[DATE_LEN];
    long dayNumber;
    int id, minute;
    Doctor *d;
    Appointment *a;

    if (appointmentCount >= MAX_APPOINTMENTS) {
        puts("Appointment list is full.");
        return;
    }

    d = findDoctor(readIntOrInvalid("Doctor ID: "));
    if (!d) {
        puts("Active doctor not found.");
        return;
    }
    id = d->id;

    dayNumber = readDate("Appointment date (YYYY-MM-DD): ", date);
    if (dayNumber < todayDayNumber()) {
        puts("That date is in the past.");
        return;
    }

    printf("Free slots with %s on %s:\n", d->name, date);
    if (!showFreeSlots(id, dayNumber)) {
        puts("  None. The doctor is not rostered that day, or is fully booked.");
        return;
    }

    minute = readTime("Choose a slot (HH:MM): ");
    if (!slotInRoster(id, weekdayOf(dayNumber), minute)) {
        puts("That is not a valid slot for this doctor.");
        return;
    }
    if (appointmentSlotTaken(id, dayNumber, minute)) {
        puts("That slot is already booked.");
        return;
    }

    printf("Slot fee is Rs. %.0f. ", APPOINTMENT_FEE);
    if (readIntRange("Simulate successful online payment? (1=yes, 0=no): ", 0, 1) != 1) {
        puts("Appointment not confirmed.");
        return;
    }

    a = &appointments[appointmentCount++];
    memset(a, 0, sizeof(*a));
    a->doctorId = id;
    snprintf(a->patient, sizeof(a->patient), "%s", patientName);
    snprintf(a->patientUsername, sizeof(a->patientUsername), "%s", patientUsername);
    snprintf(a->date, sizeof(a->date), "%s", date);
    a->dayNumber = dayNumber;
    a->timeMinute = minute;
    puts("Appointment confirmed.");
}

static int cabinIsFree(int cabin, long startDay, int days)
{
    int i;
    for (i = 0; i < cabinBookingCount; ++i) {
        CabinBooking *b = &cabinBookings[i];
        if (b->cabin == cabin &&
            rangesOverlap(startDay, startDay + days, b->startDay,
                          b->startDay + b->days))
            return 0;
    }
    return 1;
}

static void bookCabin(const char *patientUsername, const char *patientName)
{
    int cabin, days;
    char date[DATE_LEN];
    long startDay;
    CabinBooking *b;

    if (cabinBookingCount >= MAX_CABIN_BOOKINGS) {
        puts("Cabin booking list is full.");
        return;
    }

    startDay = readDate("Start date (YYYY-MM-DD): ", date);
    if (startDay < todayDayNumber()) {
        puts("That date is in the past.");
        return;
    }
    days = readIntRange("Number of days (1-60): ", 1, 60);

    for (cabin = 1; cabin <= CABIN_COUNT; ++cabin)
        if (cabinIsFree(cabin, startDay, days))
            break;
    if (cabin > CABIN_COUNT) {
        puts("No cabin is free for the whole stay.");
        return;
    }

    printf("Cabin %d is free. Charge is Rs. %.0f. ", cabin,
           days * CABIN_RATE_PER_DAY);
    if (readIntRange("Simulate successful online payment? (1=yes, 0=no): ", 0, 1) != 1) {
        puts("Cabin booking not confirmed.");
        return;
    }

    b = &cabinBookings[cabinBookingCount++];
    memset(b, 0, sizeof(*b));
    b->cabin = cabin;
    snprintf(b->patient, sizeof(b->patient), "%s", patientName);
    snprintf(b->patientUsername, sizeof(b->patientUsername), "%s", patientUsername);
    snprintf(b->startDate, sizeof(b->startDate), "%s", date);
    b->startDay = startDay;
    b->days = days;
    printf("Cabin %d booked.\n", cabin);
}

static void viewMyBookings(const char *patientUsername)
{
    int i, found = 0;
    char text[8];

    printPanelTitle("MY BOOKINGS");
    puts("+------------+-------+----------------------+------------+");
    puts("| DATE       | TIME  | DETAILS              | STATUS     |");
    puts("+------------+-------+----------------------+------------+");
    for (i = 0; i < appointmentCount; ++i) {
        Appointment *a = &appointments[i];
        Doctor *d;
        if (strcmp(a->patientUsername, patientUsername) != 0)
            continue;
        d = findDoctor(a->doctorId);
        formatTime(a->timeMinute, text, sizeof(text));
        printf("| %-10.10s | %-5.5s | %-20.20s | %-10.10s |\n",
               a->date, text, d ? d->name : "Unavailable",
               a->visited ? "Completed" : "Upcoming");
        found = 1;
    }
    for (i = 0; i < cabinBookingCount; ++i) {
        CabinBooking *b = &cabinBookings[i];
        char details[MAX_TEXT];
        if (strcmp(b->patientUsername, patientUsername) != 0)
            continue;
        snprintf(details, sizeof(details), "Cabin %d (%d day(s))",
                 b->cabin, b->days);
        printf("| %-10.10s | %-5s | %-20.20s | %-10s |\n",
               b->startDate, "N/A", details, "Confirmed");
        found = 1;
    }
    puts("+------------+-------+----------------------+------------+");
    if (!found)
        puts("No bookings yet.");
}

static void viewPatientRecords(const char *patientUsername)
{
    int i, found = 0;

    printPanelTitle("MY CONSULTATION RECORDS");
    for (i = 0; i < appointmentCount; ++i) {
        Appointment *a = &appointments[i];
        Doctor *d;
        char text[8];

        if (!a->visited || strcmp(a->patientUsername, patientUsername) != 0)
            continue;

        d = findDoctor(a->doctorId);
        formatTime(a->timeMinute, text, sizeof(text));
        printf("\nDate: %s  |  Time: %s  |  Doctor: %s\n", a->date, text,
               d ? d->name : "Unavailable");
        puts("-----------------------------------------------------------------------");
        printf("Diagnosis:    %s\nPrescription: %s\n", a->diagnosis,
               a->prescription);
        found = 1;
    }
    if (!found)
        puts("No completed consultation records found.");
}

void patientMenu(const char *patientUsername, const char *patientName)
{
    int choice;

    do {
        printPanelTitle("PATIENT MENU");
        printMenuOption(1, "Doctors available on a day");
        printMenuOption(2, "Doctors available this week");
        printMenuOption(3, "View free slots of a doctor");
        printMenuOption(4, "Book an appointment");
        printMenuOption(5, "Book an indoor cabin");
        printMenuOption(6, "View my bookings");
        printMenuOption(7, "View my consultation records");
        printMenuOption(0, "Back");
        choice = readIntOrInvalid("Choice: ");

        switch (choice) {
        case 1: listAvailableDoctors(); break;
        case 2: printWeeklyRoster(0, 1); break;
        case 3: viewFreeSlots(); break;
        case 4: bookAppointment(patientUsername, patientName); break;
        case 5: bookCabin(patientUsername, patientName); break;
        case 6: viewMyBookings(patientUsername); break;
        case 7: viewPatientRecords(patientUsername); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}
