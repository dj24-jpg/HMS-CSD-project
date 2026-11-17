/*
 * Hospital bookkeeping prototype in C.
 *
 * Compile: cc -std=c99 -Wall -Wextra hospital_bookkeeping.c -o hospital
 *
 * This prototype keeps data in memory, so records are lost when the program
 * exits. Payment confirmation is simulated; real online payments, secure
 * patient records, authentication, and regulatory safeguards require
 * additional systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DOCTORS 100
#define MAX_NURSES 100
#define MAX_ROSTER 2000
#define MAX_APPOINTMENTS 5000
#define MAX_CABIN_BOOKINGS 1000
#define MAX_PAYROLL_RUNS 100
#define MAX_TEXT 100
#define CABIN_COUNT 20

typedef struct {
    int id;
    int active;
    int visiting;              /* 0 = regular, 1 = visiting consultant */
    char name[MAX_TEXT];
    char qualification[MAX_TEXT];
    char expertise[MAX_TEXT];
    double basicPay;
} Doctor;

typedef struct {
    int id;
    int active;
    char name[MAX_TEXT];
    double salary;
} Nurse;

typedef struct {
    int doctorId;
    int day;                   /* 0 = Monday ... 6 = Sunday */
    int startMinute;
    int endMinute;
    int room;
} RosterEntry;

typedef struct {
    int doctorId;
    char patient[MAX_TEXT];
    char date[11];             /* YYYY-MM-DD */
    int timeMinute;
    int paid;
    int visited;
    char diagnosis[256];
    char prescription[256];
} Appointment;

typedef struct {
    int cabin;
    char patient[MAX_TEXT];
    char startDate[11];        /* YYYY-MM-DD */
    int days;
    int paid;
} CabinBooking;

typedef struct {
    char month[8];             /* YYYY-MM */
    double total;
} PayrollRun;

static Doctor doctors[MAX_DOCTORS];
static Nurse nurses[MAX_NURSES];
static RosterEntry roster[MAX_ROSTER];
static Appointment appointments[MAX_APPOINTMENTS];
static CabinBooking cabinBookings[MAX_CABIN_BOOKINGS];
static PayrollRun payrollRuns[MAX_PAYROLL_RUNS];

static int doctorCount, nurseCount, rosterCount, appointmentCount;
static int cabinBookingCount, payrollRunCount;
static int nextDoctorId = 1, nextNurseId = 1;
static double totalIncome, totalSalaryOutgo;

static const char *dayNames[] = {
    "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday", "Sunday"
};

static void readLine(const char *prompt, char *out, size_t size)
{
    printf("%s", prompt);
    if (fgets(out, (int)size, stdin) == NULL) {
        out[0] = '\0';
        return;
    }
    out[strcspn(out, "\n")] = '\0';
}

static int readInt(const char *prompt)
{
    char line[80];
    char *end;
    long value;

    for (;;) {
        readLine(prompt, line, sizeof(line));
        value = strtol(line, &end, 10);
        if (end != line && *end == '\0')
            return (int)value;
        puts("Please enter a valid whole number.");
    }
}

static double readDouble(const char *prompt)
{
    char line[80];
    char *end;
    double value;

    for (;;) {
        readLine(prompt, line, sizeof(line));
        value = strtod(line, &end);
        if (end != line && *end == '\0' && value >= 0)
            return value;
        puts("Please enter a valid non-negative amount.");
    }
}

static int readDay(void)
{
    int day;
    do {
        day = readInt("Day (1=Monday ... 7=Sunday): ");
    } while (day < 1 || day > 7);
    return day - 1;
}

static int parseTime(const char *text)
{
    int hour, minute;
    char extra;

    if (sscanf(text, "%d:%d%c", &hour, &minute, &extra) != 2)
        return -1;
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
        return -1;
    return hour * 60 + minute;
}

static void formatTime(int minute, char *out, size_t size)
{
    snprintf(out, size, "%02d:%02d", minute / 60, minute % 60);
}

static Doctor *findDoctor(int id)
{
    int i;
    for (i = 0; i < doctorCount; ++i)
        if (doctors[i].id == id && doctors[i].active)
            return &doctors[i];
    return NULL;
}

static Nurse *findNurse(int id)
{
    int i;
    for (i = 0; i < nurseCount; ++i)
        if (nurses[i].id == id && nurses[i].active)
            return &nurses[i];
    return NULL;
}

static void listDoctors(void)
{
    int i;
    puts("\nID  Type       Name                         Qualification / Expertise");
    puts("--------------------------------------------------------------------------");
    for (i = 0; i < doctorCount; ++i) {
        if (doctors[i].active) {
            printf("%-3d %-10s %-28s %s / %s\n",
                   doctors[i].id,
                   doctors[i].visiting ? "Visiting" : "Regular",
                   doctors[i].name,
                   doctors[i].qualification,
                   doctors[i].expertise);
        }
    }
}

static void addDoctor(void)
{
    Doctor *d;
    int visiting;

    if (doctorCount >= MAX_DOCTORS) {
        puts("Doctor list is full.");
        return;
    }

    visiting = readInt("Doctor type (0=Regular, 1=Visiting): ");
    if (visiting != 0 && visiting != 1) {
        puts("Invalid doctor type.");
        return;
    }

    d = &doctors[doctorCount++];
    memset(d, 0, sizeof(*d));
    d->id = nextDoctorId++;
    d->active = 1;
    d->visiting = visiting;
    readLine("Name: ", d->name, sizeof(d->name));
    readLine("Qualification: ", d->qualification, sizeof(d->qualification));
    readLine("Expertise: ", d->expertise, sizeof(d->expertise));
    d->basicPay = visiting ? 0 : readDouble("Monthly basic pay (Rs.): ");

    printf("Doctor added with ID %d.\n", d->id);
}

static void deleteDoctor(void)
{
    int id = readInt("Doctor ID to delete: ");
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
    int id = readInt("Regular doctor ID: ");
    Doctor *d = findDoctor(id);

    if (!d || d->visiting) {
        puts("Active regular doctor not found.");
        return;
    }
    d->basicPay = readDouble("New monthly basic pay (Rs.): ");
    puts("Basic pay updated.");
}

static void addRosterEntry(void)
{
    RosterEntry *r;
    int id, day, start, end, room;
    char timeText[32];
    Doctor *d;

    if (rosterCount >= MAX_ROSTER) {
        puts("Roster is full.");
        return;
    }

    id = readInt("Doctor ID: ");
    d = findDoctor(id);
    if (!d) {
        puts("Active doctor not found.");
        return;
    }

    day = readDay();
    readLine("Start time (HH:MM): ", timeText, sizeof(timeText));
    start = parseTime(timeText);
    readLine("End time (HH:MM): ", timeText, sizeof(timeText));
    end = parseTime(timeText);

    if (start < 0 || end < 0 || end <= start) {
        puts("Invalid time range. Overnight shifts must be entered as separate entries.");
        return;
    }
    if (!d->visiting && end - start > 360) {
        puts("A regular doctor's roster entry cannot exceed six hours.");
        return;
    }

    room = readInt("Consultation room number: ");
    if (room < 1) {
        puts("Room number must be positive.");
        return;
    }

    r = &roster[rosterCount++];
    r->doctorId = id;
    r->day = day;
    r->startMinute = start;
    r->endMinute = end;
    r->room = room;
    puts("Roster entry added.");
}

static void showRosterForDay(int day)
{
    int i, found = 0;
    char start[8], end[8];

    printf("\n%s roster:\n", dayNames[day]);
    for (i = 0; i < rosterCount; ++i) {
        Doctor *d;
        if (roster[i].day != day)
            continue;
        d = findDoctor(roster[i].doctorId);
        if (!d)
            continue;

        formatTime(roster[i].startMinute, start, sizeof(start));
        formatTime(roster[i].endMinute, end, sizeof(end));
        printf("%s (ID %d, %s)  %s-%s  Room %d\n",
               d->name, d->id, d->visiting ? "Visiting" : "Regular",
               start, end, roster[i].room);
        found = 1;
    }
    if (!found)
        puts("No roster entries.");
}

static void showFullRoster(void)
{
    int day;
    for (day = 0; day < 7; ++day)
        showRosterForDay(day);
}

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
    readLine("Nurse name: ", n->name, sizeof(n->name));
    n->salary = readDouble("Monthly salary (Rs.): ");
    printf("Nurse added with ID %d.\n", n->id);
}

static void manageNurses(void)
{
    int choice, i, id;
    Nurse *n;

    do {
        puts("\nNURSE MENU: 1 Add  2 List  3 Change salary  4 Delete  5 Weekly duty  0 Back");
        choice = readInt("Choice: ");
        switch (choice) {
        case 1:
            addNurse();
            break;
        case 2:
            for (i = 0; i < nurseCount; ++i)
                if (nurses[i].active)
                    printf("ID %d: %s, Rs. %.2f/month\n",
                           nurses[i].id, nurses[i].name, nurses[i].salary);
            break;
        case 3:
            id = readInt("Nurse ID: ");
            n = findNurse(id);
            if (n) {
                n->salary = readDouble("New monthly salary (Rs.): ");
                puts("Salary updated.");
            } else {
                puts("Active nurse not found.");
            }
            break;
        case 4:
            id = readInt("Nurse ID to delete: ");
            n = findNurse(id);
            if (n) {
                n->active = 0;
                puts("Nurse removed.");
            } else {
                puts("Active nurse not found.");
            }
            break;
        case 5: {
            int day = readDay();
            id = readInt("Nurse ID assigned for this duty: ");
            n = findNurse(id);
            if (n)
                printf("%s assigned for %s duty (weekly roster entry).\n",
                       n->name, dayNames[day]);
            else
                puts("Active nurse not found.");
            break;
        }
        case 0:
            break;
        default:
            puts("Invalid choice.");
        }
    } while (choice != 0);
}

static int appointmentSlotTaken(int doctorId, const char *date, int time)
{
    int i;
    for (i = 0; i < appointmentCount; ++i)
        if (appointments[i].doctorId == doctorId &&
            strcmp(appointments[i].date, date) == 0 &&
            appointments[i].timeMinute == time)
            return 1;
    return 0;
}

static void listAvailableDoctors(void)
{
    int day = readDay();
    int i, found = 0;
    char start[8], end[8];

    printf("\nDoctors available on %s:\n", dayNames[day]);
    for (i = 0; i < rosterCount; ++i) {
        Doctor *d;
        if (roster[i].day != day)
            continue;
        d = findDoctor(roster[i].doctorId);
        if (!d)
            continue;

        formatTime(roster[i].startMinute, start, sizeof(start));
        formatTime(roster[i].endMinute, end, sizeof(end));
        printf("ID %d: %s (%s; %s; %s), %s-%s, Room %d\n",
               d->id, d->name, d->visiting ? "Visiting" : "Regular",
               d->qualification, d->expertise, start, end, roster[i].room);
        found = 1;
    }
    if (!found)
        puts("No doctors are rostered for that day.");
}

static void bookAppointment(void)
{
    char date[11], timeText[32], patient[MAX_TEXT];
    int id, time, i, available = 0;
    Doctor *d;

    if (appointmentCount >= MAX_APPOINTMENTS) {
        puts("Appointment list is full.");
        return;
    }

    id = readInt("Doctor ID: ");
    d = findDoctor(id);
    if (!d) {
        puts("Active doctor not found.");
        return;
    }
    readLine("Appointment date (YYYY-MM-DD): ", date, sizeof(date));
    readLine("Appointment time (HH:MM): ", timeText, sizeof(timeText));
    time = parseTime(timeText);
    if (time < 0) {
        puts("Invalid time.");
        return;
    }

    for (i = 0; i < rosterCount; ++i) {
        if (roster[i].doctorId == id &&
            time >= roster[i].startMinute && time < roster[i].endMinute) {
            available = 1;
            break;
        }
    }
    if (!available) {
        puts("That time is outside the doctor's roster.");
        return;
    }
    if (appointmentSlotTaken(id, date, time)) {
        puts("That doctor slot is already booked.");
        return;
    }

    readLine("Patient name: ", patient, sizeof(patient));
    printf("Appointment slot fee: Rs. 500. Simulate successful online payment? (1=yes): ");
    if (readInt("") != 1) {
        puts("Appointment not confirmed.");
        return;
    }

    {
        Appointment *a = &appointments[appointmentCount++];
        memset(a, 0, sizeof(*a));
        a->doctorId = id;
        snprintf(a->patient, sizeof(a->patient), "%s", patient);
        snprintf(a->date, sizeof(a->date), "%s", date);
        a->timeMinute = time;
        a->paid = 1;
    }
    totalIncome += 500.0;
    puts("Appointment confirmed.");
}

static void recordConsultation(void)
{
    int i, found = 0;
    char date[11], timeText[32];
    int id, time;

    id = readInt("Doctor ID: ");
    readLine("Appointment date (YYYY-MM-DD): ", date, sizeof(date));
    readLine("Appointment time (HH:MM): ", timeText, sizeof(timeText));
    time = parseTime(timeText);
    if (time < 0) {
        puts("Invalid time.");
        return;
    }

    for (i = 0; i < appointmentCount; ++i) {
        Appointment *a = &appointments[i];
        Doctor *d;
        if (a->doctorId != id || strcmp(a->date, date) != 0 ||
            a->timeMinute != time)
            continue;

        d = findDoctor(id);
        if (!d) {
            puts("Doctor is no longer active.");
            return;
        }

        a->visited = 1;
        readLine("Diagnosis: ", a->diagnosis, sizeof(a->diagnosis));
        readLine("Prescription: ", a->prescription, sizeof(a->prescription));
        if (d->visiting)
            totalIncome += 200.0; /* Consultant charge recorded per patient. */
        puts("Consultation record saved.");
        found = 1;
        break;
    }
    if (!found)
        puts("Appointment not found.");
}

static int cabinIsFree(int cabin, const char *startDate, int days)
{
    int i;
    /* ISO YYYY-MM-DD strings sort chronologically for same-format dates. */
    for (i = 0; i < cabinBookingCount; ++i) {
        CabinBooking *b = &cabinBookings[i];
        if (b->cabin == cabin && strcmp(b->startDate, startDate) == 0)
            return 0;
    }
    (void)days;
    return 1;
}

static void bookCabin(void)
{
    int cabin, days;
    char date[11], patient[MAX_TEXT];

    if (cabinBookingCount >= MAX_CABIN_BOOKINGS) {
        puts("Cabin booking list is full.");
        return;
    }

    readLine("Patient name: ", patient, sizeof(patient));
    readLine("Start date (YYYY-MM-DD): ", date, sizeof(date));
    days = readInt("Number of days: ");
    if (days < 1) {
        puts("Number of days must be positive.");
        return;
    }

    for (cabin = 1; cabin <= CABIN_COUNT; ++cabin)
        if (cabinIsFree(cabin, date, days))
            break;
    if (cabin > CABIN_COUNT) {
        puts("No cabin is available for that start date.");
        return;
    }

    printf("Cabin %d available. Charge is Rs. %d. Simulate successful online payment? (1=yes): ",
           cabin, days * 3000);
    if (readInt("") != 1) {
        puts("Cabin booking not confirmed.");
        return;
    }

    cabinBookings[cabinBookingCount].cabin = cabin;
    snprintf(cabinBookings[cabinBookingCount].patient,
             sizeof(cabinBookings[cabinBookingCount].patient), "%s", patient);
    snprintf(cabinBookings[cabinBookingCount].startDate,
             sizeof(cabinBookings[cabinBookingCount].startDate), "%s", date);
    cabinBookings[cabinBookingCount].days = days;
    cabinBookings[cabinBookingCount].paid = 1;
    ++cabinBookingCount;
    totalIncome += days * 3000.0;
    printf("Cabin %d booked.\n", cabin);
}

static void viewPatientRecords(void)
{
    char patient[MAX_TEXT];
    int i, found = 0;

    readLine("Patient name: ", patient, sizeof(patient));
    for (i = 0; i < appointmentCount; ++i) {
        Appointment *a = &appointments[i];
        Doctor *d;
        char time[8];

        if (!a->visited || strcmp(a->patient, patient) != 0)
            continue;

        d = findDoctor(a->doctorId);
        formatTime(a->timeMinute, time, sizeof(time));
        printf("\nDate: %s, Time: %s, Doctor: %s\n",
               a->date, time, d ? d->name : "Unavailable");
        printf("Diagnosis: %s\nPrescription: %s\n",
               a->diagnosis, a->prescription);
        found = 1;
    }
    if (!found)
        puts("No completed consultation records found.");
}

static void generatePayroll(void)
{
    char month[8];
    int i, j;
    double runTotal = 0.0;

    if (payrollRunCount >= MAX_PAYROLL_RUNS) {
        puts("Payroll run list is full.");
        return;
    }

    readLine("Payroll month (YYYY-MM): ", month, sizeof(month));
    for (i = 0; i < payrollRunCount; ++i) {
        if (strcmp(payrollRuns[i].month, month) == 0) {
            puts("Payroll has already been generated for that month.");
            return;
        }
    }

    printf("\nPAY SLIPS FOR %s\n", month);
    for (i = 0; i < doctorCount; ++i) {
        Doctor *d = &doctors[i];
        int patients = 0;
        double variablePay, totalPay;

        if (!d->active)
            continue;

        for (j = 0; j < appointmentCount; ++j) {
            Appointment *a = &appointments[j];
            if (a->visited && a->doctorId == d->id &&
                strncmp(a->date, month, 7) == 0)
                ++patients;
        }

        variablePay = patients * (d->visiting ? 200.0 : 100.0);
        totalPay = d->basicPay + variablePay;
        printf("Doctor: %-24s Type: %-8s Patients: %d Basic: Rs. %.2f "
               "Variable: Rs. %.2f Total: Rs. %.2f\n",
               d->name, d->visiting ? "Visiting" : "Regular",
               patients, d->basicPay, variablePay, totalPay);
        runTotal += totalPay;
    }

    for (i = 0; i < nurseCount; ++i) {
        if (nurses[i].active) {
            printf("Nurse:  %-24s Salary: Rs. %.2f\n",
                   nurses[i].name, nurses[i].salary);
            runTotal += nurses[i].salary;
        }
    }

    payrollRuns[payrollRunCount].total = runTotal;
    snprintf(payrollRuns[payrollRunCount].month,
             sizeof(payrollRuns[payrollRunCount].month), "%s", month);
    ++payrollRunCount;
    totalSalaryOutgo += runTotal;
    printf("Payroll total: Rs. %.2f\n", runTotal);
}

static void showFinancialReport(void)
{
    int i, visited = 0;
    char from[11], to[11];

    readLine("Report start date (YYYY-MM-DD): ", from, sizeof(from));
    readLine("Report end date (YYYY-MM-DD): ", to, sizeof(to));

    for (i = 0; i < appointmentCount; ++i)
        if (appointments[i].visited &&
            strcmp(appointments[i].date, from) >= 0 &&
            strcmp(appointments[i].date, to) <= 0)
            ++visited;

    printf("\nCompleted patient visits in period: %d\n", visited);
    printf("Total recorded income so far: Rs. %.2f\n", totalIncome);
    printf("Total generated salary outgo so far: Rs. %.2f\n",
           totalSalaryOutgo);
}

static void adminMenu(void)
{
    int choice;

    do {
        puts("\nADMIN MENU");
        puts("1 Add doctor  2 Delete doctor  3 Change regular doctor's basic pay");
        puts("4 Prepare roster entry  5 View weekly roster  6 Manage nurses");
        puts("7 Generate monthly pay slips  8 Financial report  0 Back");
        choice = readInt("Choice: ");

        switch (choice) {
        case 1: addDoctor(); break;
        case 2: deleteDoctor(); break;
        case 3: changeDoctorPay(); break;
        case 4: addRosterEntry(); break;
        case 5: showFullRoster(); break;
        case 6: manageNurses(); break;
        case 7: generatePayroll(); break;
        case 8: showFinancialReport(); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}

static void patientMenu(void)
{
    int choice;

    do {
        puts("\nPATIENT MENU");
        puts("1 List doctors available by day  2 Book appointment");
        puts("3 Book indoor cabin  4 View consultation records  0 Back");
        choice = readInt("Choice: ");

        switch (choice) {
        case 1: listAvailableDoctors(); break;
        case 2: bookAppointment(); break;
        case 3: bookCabin(); break;
        case 4: viewPatientRecords(); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}

static void doctorMenu(void)
{
    int choice;

    do {
        puts("\nDOCTOR MENU");
        puts("1 View weekly roster  2 Record completed consultation  0 Back");
        choice = readInt("Choice: ");

        switch (choice) {
        case 1: showFullRoster(); break;
        case 2: recordConsultation(); break;
        case 0: break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);
}

int main(void)
{
    int choice;

    puts("Hospital Bookkeeping System (C prototype)");
    do {
        puts("\n1 Administrator  2 Patient  3 Doctor  0 Exit");
        choice = readInt("Choice: ");

        switch (choice) {
        case 1: adminMenu(); break;
        case 2: patientMenu(); break;
        case 3: doctorMenu(); break;
        case 0: puts("Goodbye."); break;
        default: puts("Invalid choice.");
        }
    } while (choice != 0);

    return 0;
}