#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <stddef.h>

#define MAX_DOCTORS 100
#define MAX_NURSES 100
#define MAX_ROSTER 2000
#define MAX_APPOINTMENTS 5000
#define MAX_CABIN_BOOKINGS 1000
#define MAX_PAYROLL_RUNS 100
#define MAX_DUTIES 700
#define MAX_NURSE_ATTENDANCE 5000
#define MAX_TEXT 100
#define DATE_LEN 11
#define MONTH_LEN 8
#define CABIN_COUNT 20

#define SLOT_MINUTES 30
#define APPOINTMENT_FEE 500.0
#define CABIN_RATE_PER_DAY 3000.0
#define VISITING_RATE_PER_PATIENT 200.0
#define REGULAR_RATE_PER_PATIENT 100.0
#define MAX_REGULAR_MINUTES_PER_DAY 360
#define VISITING_SLOT_MINUTES 120
#define VISITING_SLOTS_PER_WEEK 2
#define DAY_SHIFT_START 480
#define DAY_SHIFT_END 1200
#define MIN_DAY_DOCTORS 3
#define MIN_NIGHT_DOCTORS 1

typedef struct {
    int id;
    int active;
    int visiting;
    char name[MAX_TEXT];
    char qualification[MAX_TEXT];
    char expertise[MAX_TEXT];
    double basicPay;
} Doctor;

typedef struct {
    int id;
    int active;
    int isHead;
    char name[MAX_TEXT];
    double salary;
} Nurse;

typedef struct {
    int nurseId;
    int day;
    int shift;
    char location[MAX_TEXT];
} NurseDuty;

typedef struct {
    int nurseId;
    char date[DATE_LEN];
    long dayNumber;
    int shift;
    int present;
} NurseAttendance;

typedef struct {
    int doctorId;
    int day;
    int startMinute;
    int endMinute;
    int room;
} RosterEntry;

typedef struct {
    int doctorId;
    char patient[MAX_TEXT];
    char patientUsername[MAX_TEXT];
    char date[DATE_LEN];
    long dayNumber;
    int timeMinute;
    int visited;
    char diagnosis[256];
    char prescription[256];
} Appointment;

typedef struct {
    int cabin;
    char patient[MAX_TEXT];
    char patientUsername[MAX_TEXT];
    char startDate[DATE_LEN];
    long startDay;
    int days;
} CabinBooking;

typedef struct {
    char month[MONTH_LEN];
    double total;
} PayrollRun;

extern Doctor doctors[MAX_DOCTORS];
extern Nurse nurses[MAX_NURSES];
extern NurseDuty duties[MAX_DUTIES];
extern NurseAttendance nurseAttendanceRecords[MAX_NURSE_ATTENDANCE];
extern RosterEntry roster[MAX_ROSTER];
extern Appointment appointments[MAX_APPOINTMENTS];
extern CabinBooking cabinBookings[MAX_CABIN_BOOKINGS];
extern PayrollRun payrollRuns[MAX_PAYROLL_RUNS];

extern int doctorCount;
extern int nurseCount;
extern int dutyCount;
extern int nurseAttendanceCount;
extern int rosterCount;
extern int appointmentCount;
extern int cabinBookingCount;
extern int payrollRunCount;
extern int nextDoctorId;
extern int nextNurseId;

extern const char *dayNames[7];
extern const char *shiftNames[3];

Doctor *findDoctor(int id);
Nurse *findNurse(int id);
long readDate(const char *prompt, char *text);
void readMonth(const char *prompt, char *month);
int readTime(const char *prompt);
int readDay(void);
int weekdayOf(long dayNumber);
long todayDayNumber(void);
int rangesOverlap(long aStart, long aEnd, long bStart, long bEnd);
void formatTime(int minute, char *out, size_t size);
void printPanelTitle(const char *title);
void printMenuOption(int option, const char *label);
void printRosterDay(int day, int onlyDoctorId, int detailed);
void printWeeklyRoster(int onlyDoctorId, int detailed);

void runHospital(void);
void adminMenu(void);
void doctorMenu(int doctorId);
void patientMenu(const char *patientUsername, const char *patientName);
void nurseManagementMenu(void);
void nurseDashboard(int nurseId);

#endif
