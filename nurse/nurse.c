#include "hospital.h"
#include "login.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static void addNurse(void)
{
    Nurse *n;

    if (nurseCount >= MAX_NURSES) {
        puts("Nurse list is full.");
        return;
    }

    n = &nurses[nurseCount++];
    memset(n, 0, sizeof(*n));
    n->id = nextNurseId++;
    n->active = 1;
    readRequired("Nurse name: ", n->name, sizeof(n->name));
    n->salary = readAmount("Monthly salary (Rs.): ");
    registerNurseAccount(n->id);
    printf("Nurse added with ID %d.\n", n->id);
}

static void listNurses(void)
{
    int i, found = 0;

    printPanelTitle("NURSE DIRECTORY");
    puts("+-----+----------------------+-----------------+------------------+");
    puts("| ID  | NAME                 | ROLE            | MONTHLY SALARY  |");
    puts("+-----+----------------------+-----------------+------------------+");
    for (i = 0; i < nurseCount; ++i) {
        if (nurses[i].active) {
            printf("| %3d | %-20.20s | %-15.15s | Rs. %10.2f  |\n",
                   nurses[i].id, nurses[i].name,
                   nurses[i].isHead ? "Head nurse" : "Nurse",
                   nurses[i].salary);
            found = 1;
        }
    }
    puts("+-----+----------------------+-----------------+------------------+");
    if (!found)
        puts("No active nurses found.");
}

static void setHeadNurse(void)
{
    int i;
    Nurse *n = findNurse(readIntOrInvalid("Nurse ID to make head nurse: "));

    if (!n) {
        puts("Active nurse not found.");
        return;
    }
    for (i = 0; i < nurseCount; ++i)
        nurses[i].isHead = 0;
    n->isHead = 1;
    printf("%s is now the head nurse.\n", n->name);
}

static void assignNurseDuty(void)
{
    Nurse *head, *n;
    int day, shift, i;
    char location[MAX_TEXT];

    head = findNurse(readIntOrInvalid("Head nurse ID: "));
    if (!head || !head->isHead) {
        puts("Only the head nurse can allocate duty.");
        return;
    }
    if (dutyCount >= MAX_DUTIES) {
        puts("Duty list is full.");
        return;
    }

    n = findNurse(readIntOrInvalid("Nurse ID to assign: "));
    if (!n) {
        puts("Active nurse not found.");
        return;
    }
    day = readDay();
    shift = readIntRange("Shift (1=Morning 2=Evening 3=Night): ", 1, 3) - 1;
    readRequired("Work area / ward: ", location, sizeof(location));

    for (i = 0; i < dutyCount; ++i) {
        if (duties[i].nurseId == n->id && duties[i].day == day) {
            puts("That nurse already has duty on that day.");
            return;
        }
    }
    duties[dutyCount].nurseId = n->id;
    duties[dutyCount].day = day;
    duties[dutyCount].shift = shift;
    snprintf(duties[dutyCount].location,
             sizeof(duties[dutyCount].location), "%s", location);
    ++dutyCount;
    printf("%s assigned: %s, %s.\n", n->name, dayNames[day], shiftNames[shift]);
}

static void showNurseDuty(void)
{
    int day, shift, i, found;

    printPanelTitle("WEEKLY NURSE DUTY");
    puts("+-----------+--------------------------+----------------------+--------------------------+");
    puts("| DAY       | SHIFT                    | WORK AREA            | NURSE                    |");
    puts("+-----------+--------------------------+----------------------+--------------------------+");
    for (day = 0; day < 7; ++day) {
        for (shift = 0; shift < 3; ++shift) {
            found = 0;
            for (i = 0; i < dutyCount; ++i) {
                Nurse *n;
                if (duties[i].day != day || duties[i].shift != shift)
                    continue;
                n = findNurse(duties[i].nurseId);
                if (n) {
                    printf("| %-9.9s | %-24.24s | %-20.20s | %-24.24s |\n",
                           dayNames[day], shiftNames[shift],
                           duties[i].location, n->name);
                    found = 1;
                }
            }
            if (!found)
                printf("| %-9.9s | %-24.24s | %-20s | %-24s |\n",
                       dayNames[day], shiftNames[shift], "-", "-");
        }
    }
    puts("+-----------+--------------------------+----------------------+--------------------------+");
}

static void recordNurseAttendance(void)
{
    NurseAttendance *record = NULL;
    Nurse *nurse;
    char date[DATE_LEN];
    long dayNumber;
    int i, shift = -1, present;

    nurse = findNurse(readIntOrInvalid("Nurse ID: "));
    if (!nurse) {
        puts("Active nurse not found.");
        return;
    }

    dayNumber = readDate("Shift date (YYYY-MM-DD): ", date);
    if (dayNumber > todayDayNumber()) {
        puts("Attendance can only be recorded for today or an earlier date.");
        return;
    }

    for (i = 0; i < dutyCount; ++i) {
        if (duties[i].nurseId == nurse->id &&
            duties[i].day == weekdayOf(dayNumber)) {
            shift = duties[i].shift;
            break;
        }
    }
    if (shift < 0) {
        puts("This nurse has no assigned shift on that date.");
        return;
    }

    for (i = 0; i < nurseAttendanceCount; ++i) {
        NurseAttendance *existing = &nurseAttendanceRecords[i];
        if (existing->nurseId == nurse->id &&
            existing->dayNumber == dayNumber &&
            existing->shift == shift) {
            record = existing;
            break;
        }
    }

    if (!record && nurseAttendanceCount >= MAX_NURSE_ATTENDANCE) {
        puts("Attendance record list is full.");
        return;
    }

    present = readIntRange("Attendance (1=Present, 0=Absent): ", 0, 1);
    if (!record) {
        record = &nurseAttendanceRecords[nurseAttendanceCount++];
        record->nurseId = nurse->id;
        record->dayNumber = dayNumber;
        record->shift = shift;
        snprintf(record->date, sizeof(record->date), "%s", date);
    }
    record->present = present;
    printf("%s marked %s for %s, %s.\n", nurse->name,
           present ? "Present" : "Absent", date, shiftNames[shift]);
}

static void showMyWeeklySchedule(int nurseId)
{
    int i, j, found = 0;
    char displayName[MAX_TEXT + 16];

    printPanelTitle("MY WEEKLY SHIFTS AND TEAM");
    puts("+-----------+--------------------------+----------------------+--------------------------+");
    puts("| DAY       | SHIFT                    | WORK AREA            | ASSIGNED NURSE           |");
    puts("+-----------+--------------------------+----------------------+--------------------------+");
    for (i = 0; i < dutyCount; ++i) {
        NurseDuty *mine = &duties[i];
        if (mine->nurseId != nurseId)
            continue;

        for (j = 0; j < dutyCount; ++j) {
            Nurse *assigned;
            if (duties[j].day != mine->day || duties[j].shift != mine->shift)
                continue;
            assigned = findNurse(duties[j].nurseId);
            if (!assigned)
                continue;

            snprintf(displayName, sizeof(displayName), "%s%s", assigned->name,
                     assigned->id == nurseId ? " (You)" : "");
            printf("| %-9.9s | %-24.24s | %-20.20s | %-24.24s |\n",
                   dayNames[mine->day], shiftNames[mine->shift],
                   duties[j].location, displayName);
            found = 1;
        }
    }
    puts("+-----------+--------------------------+----------------------+--------------------------+");
    if (!found)
        puts("No weekly shifts have been assigned.");
}

static void showMyAttendance(int nurseId)
{
    int i, recorded = 0, present = 0;
    char status[10];

    printPanelTitle("MY ATTENDANCE");
    puts("+------------+--------------------------+------------+");
    puts("| DATE       | SHIFT                    | STATUS     |");
    puts("+------------+--------------------------+------------+");
    for (i = 0; i < nurseAttendanceCount; ++i) {
        NurseAttendance *record = &nurseAttendanceRecords[i];
        if (record->nurseId != nurseId)
            continue;

        snprintf(status, sizeof(status), "%s",
                 record->present ? "Present" : "Absent");
        printf("| %-10.10s | %-24.24s | %-10.10s |\n",
               record->date, shiftNames[record->shift], status);
        ++recorded;
        if (record->present)
            ++present;
    }
    puts("+------------+--------------------------+------------+");
    if (recorded == 0) {
        puts("No attendance has been recorded yet.");
        return;
    }

    printf("Attendance: %d of %d recorded shifts (%.1f%%)\n",
           present, recorded, 100.0 * present / recorded);
    puts("Percentage uses shifts explicitly marked Present or Absent.");
}

void nurseDashboard(int nurseId)
{
    int choice;

    do {
        printPanelTitle("NURSE DASHBOARD");
        printMenuOption(1, "View my weekly shifts and assigned team");
        printMenuOption(2, "View my attendance percentage");
        printMenuOption(0, "Log out");
        choice = readIntOrInvalid("Choice: ");

        switch (choice) {
        case 1: showMyWeeklySchedule(nurseId); break;
        case 2: showMyAttendance(nurseId); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}

void nurseManagementMenu(void)
{
    int choice;
    Nurse *n;

    do {
        printPanelTitle("NURSE MANAGEMENT");
        printMenuOption(1, "Add nurse");
        printMenuOption(2, "List nurses");
        printMenuOption(3, "Change salary");
        printMenuOption(4, "Delete nurse");
        printMenuOption(5, "Set head nurse");
        printMenuOption(6, "Allocate duty (head nurse)");
        printMenuOption(7, "View weekly duty");
        printMenuOption(8, "Record nurse attendance");
        printMenuOption(0, "Back");
        choice = readIntOrInvalid("Choice: ");
        switch (choice) {
        case 1: addNurse(); break;
        case 2: listNurses(); break;
        case 3:
            n = findNurse(readIntOrInvalid("Nurse ID: "));
            if (n) {
                n->salary = readAmount("New monthly salary (Rs.): ");
                puts("Salary updated.");
            } else {
                puts("Active nurse not found.");
            }
            break;
        case 4:
            n = findNurse(readIntOrInvalid("Nurse ID to delete: "));
            if (n) {
                n->active = 0;
                n->isHead = 0;
                puts("Nurse removed.");
            } else {
                puts("Active nurse not found.");
            }
            break;
        case 5: setHeadNurse(); break;
        case 6: assignNurseDuty(); break;
        case 7: showNurseDuty(); break;
        case 8: recordNurseAttendance(); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}
