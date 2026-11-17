#include "hospital.h"

Doctor doctors[MAX_DOCTORS];
Nurse nurses[MAX_NURSES];
NurseDuty duties[MAX_DUTIES];
NurseAttendance nurseAttendanceRecords[MAX_NURSE_ATTENDANCE];
RosterEntry roster[MAX_ROSTER];
Appointment appointments[MAX_APPOINTMENTS];
CabinBooking cabinBookings[MAX_CABIN_BOOKINGS];
PayrollRun payrollRuns[MAX_PAYROLL_RUNS];

int doctorCount, nurseCount, dutyCount, rosterCount;
int nurseAttendanceCount;
int appointmentCount, cabinBookingCount, payrollRunCount;
int nextDoctorId = 1, nextNurseId = 1;

const char *dayNames[7] = {
    "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday", "Sunday"
};

const char *shiftNames[3] = {
    "Morning (08:00-14:00)", "Evening (14:00-20:00)", "Night (20:00-08:00)"
};
