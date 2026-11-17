#include "hospital.h"
#include "login.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static void listDoctors(void)
{
    int i, found = 0;

    printPanelTitle("DOCTOR DIRECTORY");
    puts("+-----+----------+--------------------+--------------------+--------------------+");
    puts("| ID  | TYPE     | NAME               | QUALIFICATION      | EXPERTISE          |");
    puts("+-----+----------+--------------------+--------------------+--------------------+");
    for (i = 0; i < doctorCount; ++i) {
        if (doctors[i].active) {
            printf("| %3d | %-8.8s | %-18.18s | %-18.18s | %-18.18s |\n",
                   doctors[i].id,
                   doctors[i].visiting ? "Visiting" : "Regular",
                   doctors[i].name,
                   doctors[i].qualification,
                   doctors[i].expertise);
            found = 1;
        }
    }
    puts("+-----+----------+--------------------+--------------------+--------------------+");
    if (!found)
        puts("No active doctors found.");
}

static void addDoctor(void)
{
    Doctor *d;
    int visiting;

    if (doctorCount >= MAX_DOCTORS) {
        puts("Doctor list is full.");
        return;
    }

    visiting = readIntRange("Doctor type (0=Regular, 1=Visiting): ", 0, 1);

    d = &doctors[doctorCount++];
    memset(d, 0, sizeof(*d));
    d->id = nextDoctorId++;
    d->active = 1;
    d->visiting = visiting;
    readRequired("Name: ", d->name, sizeof(d->name));
    readRequired("Qualification: ", d->qualification, sizeof(d->qualification));
    readRequired("Expertise: ", d->expertise, sizeof(d->expertise));
    d->basicPay = visiting ? 0 : readAmount("Monthly basic pay (Rs.): ");
    registerDoctorAccount(d->id);

    printf("Doctor added with ID %d.\n", d->id);
}

static void deleteDoctor(void)
{
    int id = readIntOrInvalid("Doctor ID to delete: ");
    Doctor *d = findDoctor(id);

    if (!d) {
        puts("Active doctor not found.");
        return;
    }
    d->active = 0;
    puts("Doctor removed.");
}

static void changeDoctorPay(void)
{
    int id = readIntOrInvalid("Regular doctor ID: ");
    Doctor *d = findDoctor(id);

    if (!d || d->visiting) {
        puts("Active regular doctor not found.");
        return;
    }
    d->basicPay = readAmount("New monthly basic pay (Rs.): ");
    puts("Basic pay updated.");
}

static int rosteredMinutes(int doctorId, int day)
{
    int i, total = 0;
    for (i = 0; i < rosterCount; ++i)
        if (roster[i].doctorId == doctorId && roster[i].day == day)
            total += roster[i].endMinute - roster[i].startMinute;
    return total;
}

static int weeklyEntries(int doctorId)
{
    int i, count = 0;
    for (i = 0; i < rosterCount; ++i)
        if (roster[i].doctorId == doctorId)
            ++count;
    return count;
}

static void addRosterEntry(void)
{
    int id, day, start, end, room, i;
    Doctor *d;

    if (rosterCount >= MAX_ROSTER) {
        puts("Roster is full.");
        return;
    }

    id = readIntOrInvalid("Doctor ID: ");
    d = findDoctor(id);
    if (!d) {
        puts("Active doctor not found.");
        return;
    }

    day = readDay();
    start = readTime("Start time (HH:MM): ");
    end = readTime("End time (HH:MM): ");
    if (end <= start) {
        puts("End must be after start. Enter overnight shifts as two entries.");
        return;
    }

    if (d->visiting) {
        if (end - start != VISITING_SLOT_MINUTES) {
            puts("Visiting consultants come for two-hour slots.");
            return;
        }
        if (weeklyEntries(id) >= VISITING_SLOTS_PER_WEEK) {
            puts("Visiting consultants already have two slots this week.");
            return;
        }
    } else if (rosteredMinutes(id, day) + (end - start) >
               MAX_REGULAR_MINUTES_PER_DAY) {
        puts("A regular doctor sits for at most six hours a day.");
        return;
    }

    room = readIntRange("Consultation room number: ", 1, 999);
    for (i = 0; i < rosterCount; ++i) {
        if (roster[i].day != day ||
            !rangesOverlap(start, end, roster[i].startMinute, roster[i].endMinute))
            continue;
        if (roster[i].doctorId == id) {
            puts("This doctor is already rostered during that time.");
            return;
        }
        if (roster[i].room == room) {
            puts("That room is already in use during that time.");
            return;
        }
    }

    roster[rosterCount].doctorId = id;
    roster[rosterCount].day = day;
    roster[rosterCount].startMinute = start;
    roster[rosterCount].endMinute = end;
    roster[rosterCount].room = room;
    ++rosterCount;
    puts("Roster entry added.");
}

void printRosterDay(int day, int onlyDoctorId, int detailed)
{
    int i, found = 0;
    char start[8], end[8];

    printPanelTitle(dayNames[day]);
    puts("+-----+----------------------+------------+-------------+------+");
    puts("| ID  | DOCTOR               | TYPE       | TIME        | ROOM |");
    puts("+-----+----------------------+------------+-------------+------+");
    for (i = 0; i < rosterCount; ++i) {
        Doctor *d;
        if (roster[i].day != day)
            continue;
        if (onlyDoctorId && roster[i].doctorId != onlyDoctorId)
            continue;
        d = findDoctor(roster[i].doctorId);
        if (!d)
            continue;

        formatTime(roster[i].startMinute, start, sizeof(start));
        formatTime(roster[i].endMinute, end, sizeof(end));
        printf("| %3d | %-20.20s | %-10.10s | %s-%s | %4d |\n",
               d->id, d->name, d->visiting ? "Visiting" : "Regular",
               start, end, roster[i].room);
        if (detailed)
            printf("     Qualification: %-20.20s  Expertise: %-20.20s\n",
                   d->qualification, d->expertise);
        found = 1;
    }
    puts("+-----+----------------------+------------+-------------+------+");
    if (!found)
        puts("No roster entries.");
}

void printWeeklyRoster(int onlyDoctorId, int detailed)
{
    int day;
    for (day = 0; day < 7; ++day)
        printRosterDay(day, onlyDoctorId, detailed);
}

static void checkRosterCoverage(void)
{
    int day, minute, i, problems = 0;

    puts("\nRule: at least 3 regular doctors 08:00-20:00, at least 1 at other times.");
    for (day = 0; day < 7; ++day) {
        int runStart = -1, runNeed = 0;

        for (minute = 0; minute <= 1440; ++minute) {
            int count = 0, need = 0, short_ = 0;

            if (minute < 1440) {
                need = (minute >= DAY_SHIFT_START && minute < DAY_SHIFT_END)
                           ? MIN_DAY_DOCTORS : MIN_NIGHT_DOCTORS;
                for (i = 0; i < rosterCount; ++i) {
                    Doctor *d;
                    if (roster[i].day != day || minute < roster[i].startMinute ||
                        minute >= roster[i].endMinute)
                        continue;
                    d = findDoctor(roster[i].doctorId);
                    if (d && !d->visiting)
                        ++count;
                }
                short_ = count < need;
            }

            if (runStart >= 0 && (!short_ || need != runNeed)) {
                char from[8], to[8];
                formatTime(runStart, from, sizeof(from));
                formatTime(minute % 1440, to, sizeof(to));
                printf("%s %s-%s: needs %d regular doctor(s)\n",
                       dayNames[day], from, to, runNeed);
                runStart = -1;
                problems = 1;
            }
            if (short_ && runStart < 0) {
                runStart = minute;
                runNeed = need;
            }
        }
    }
    if (!problems)
        puts("Roster meets the coverage rule for every day.");
}

static void generatePayroll(void)
{
    char month[MONTH_LEN];
    int i, j;
    double runTotal = 0.0;

    if (payrollRunCount >= MAX_PAYROLL_RUNS) {
        puts("Payroll run list is full.");
        return;
    }

    readMonth("Payroll month (YYYY-MM): ", month);
    for (i = 0; i < payrollRunCount; ++i) {
        if (strcmp(payrollRuns[i].month, month) == 0) {
            puts("Payroll has already been generated for that month.");
            return;
        }
    }

    {
        char title[40];
        snprintf(title, sizeof(title), "PAY SLIPS - %s", month);
        printPanelTitle(title);
    }
    puts("+--------------------------+----------+----------+--------------+--------------+--------------+");
    puts("| STAFF                    | TYPE     | PATIENTS | BASIC PAY    | VARIABLE PAY | TOTAL PAY    |");
    puts("+--------------------------+----------+----------+--------------+--------------+--------------+");
    for (i = 0; i < doctorCount; ++i) {
        Doctor *d = &doctors[i];
        int patients = 0;
        double variablePay, totalPay;

        if (!d->active)
            continue;
        for (j = 0; j < appointmentCount; ++j)
            if (appointments[j].visited && appointments[j].doctorId == d->id &&
                strncmp(appointments[j].date, month, 7) == 0)
                ++patients;

        variablePay = patients * (d->visiting ? VISITING_RATE_PER_PATIENT
                                              : REGULAR_RATE_PER_PATIENT);
        totalPay = d->basicPay + variablePay;
        printf("| %-24.24s | %-8.8s | %8d | %12.2f | %12.2f | %12.2f |\n",
               d->name, d->visiting ? "Visiting" : "Regular", patients,
               d->basicPay, variablePay, totalPay);
        runTotal += totalPay;
    }
    for (i = 0; i < nurseCount; ++i) {
        if (nurses[i].active) {
            printf("| %-24.24s | %-8s | %8d | %12.2f | %12.2f | %12.2f |\n",
                   nurses[i].name, "Nurse", 0, nurses[i].salary, 0.0,
                   nurses[i].salary);
            runTotal += nurses[i].salary;
        }
    }
    puts("+--------------------------+----------+----------+--------------+--------------+--------------+");

    snprintf(payrollRuns[payrollRunCount].month, MONTH_LEN, "%s", month);
    payrollRuns[payrollRunCount].total = runTotal;
    ++payrollRunCount;
    printf("Payroll total: Rs. %.2f\n", runTotal);
}

static void showFinancialReport(void)
{
    char fromText[DATE_LEN], toText[DATE_LEN];
    char period[32];
    long from, to;
    int i, visited = 0, booked = 0, cabinDays = 0;
    double income, outgo = 0.0;

    from = readDate("Report start date (YYYY-MM-DD): ", fromText);
    to = readDate("Report end date (YYYY-MM-DD): ", toText);
    if (to < from) {
        puts("End date is before start date.");
        return;
    }

    for (i = 0; i < appointmentCount; ++i) {
        if (appointments[i].dayNumber < from || appointments[i].dayNumber > to)
            continue;
        ++booked;
        if (appointments[i].visited)
            ++visited;
    }
    for (i = 0; i < cabinBookingCount; ++i)
        if (cabinBookings[i].startDay >= from && cabinBookings[i].startDay <= to)
            cabinDays += cabinBookings[i].days;

    /* Salary outgo: payroll runs for months from the start month to the end month. */
    for (i = 0; i < payrollRunCount; ++i)
        if (strncmp(payrollRuns[i].month, fromText, 7) >= 0 &&
            strncmp(payrollRuns[i].month, toText, 7) <= 0)
            outgo += payrollRuns[i].total;

    income = booked * APPOINTMENT_FEE + cabinDays * CABIN_RATE_PER_DAY;
    snprintf(period, sizeof(period), "%s to %s", fromText, toText);
    printPanelTitle("FINANCIAL REPORT");
    puts("+------------------------------+--------------------------+");
    puts("| ITEM                         | VALUE                    |");
    puts("+------------------------------+--------------------------+");
    printf("| %-28.28s | %-24.24s |\n", "Report period", period);
    printf("| %-28.28s | %24d |\n", "Patients who visited", visited);
    printf("| %-28.28s | %24d |\n", "Appointments booked", booked);
    printf("| %-28.28s | Rs. %20.2f |\n", "Appointment fees",
           booked * APPOINTMENT_FEE);
    printf("| %-28.28s | Rs. %20.2f |\n", "Cabin income",
           cabinDays * CABIN_RATE_PER_DAY);
    printf("| %-28.28s | Rs. %20.2f |\n", "Total income", income);
    printf("| %-28.28s | Rs. %20.2f |\n", "Total salary outgo", outgo);
    puts("+------------------------------+--------------------------+");
}

void adminMenu(void)
{
    int choice;

    do {
        printPanelTitle("ADMINISTRATOR MENU");
        printMenuOption(1, "Add doctor");
        printMenuOption(2, "Delete doctor");
        printMenuOption(3, "Change regular doctor's basic pay");
        printMenuOption(4, "List doctors");
        printMenuOption(5, "Add roster entry");
        printMenuOption(6, "View weekly roster");
        printMenuOption(7, "Check roster coverage");
        printMenuOption(8, "Manage nurses");
        printMenuOption(9, "Generate monthly pay slips");
        printMenuOption(10, "Income / patients / salary report");
        printMenuOption(0, "Back");
        choice = readIntOrInvalid("Choice: ");

        switch (choice) {
        case 1: addDoctor(); break;
        case 2: deleteDoctor(); break;
        case 3: changeDoctorPay(); break;
        case 4: listDoctors(); break;
        case 5: addRosterEntry(); break;
        case 6: printWeeklyRoster(0, 0); break;
        case 7: checkRosterCoverage(); break;
        case 8: nurseManagementMenu(); break;
        case 9: generatePayroll(); break;
        case 10: showFinancialReport(); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}
