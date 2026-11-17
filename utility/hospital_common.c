#include "hospital.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long daysFromCivil(int y, int m, int d)
{
    long era;
    unsigned yoe, doy, doe;

    y -= (m <= 2);
    era = (y >= 0 ? y : y - 399) / 400;
    yoe = (unsigned)(y - era * 400);
    doy = (unsigned)((153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1);
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long)doe - 719468;
}

static long parseDate(const char *text)
{
    static const int monthDays[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    int y, m, d, i, leap, maxDay;

    if (strlen(text) != 10 || text[4] != '-' || text[7] != '-')
        return -1;
    for (i = 0; i < 10; ++i)
        if (i != 4 && i != 7 && (text[i] < '0' || text[i] > '9'))
            return -1;
    if (sscanf(text, "%d-%d-%d", &y, &m, &d) != 3)
        return -1;
    if (y < 2000 || y > 2100 || m < 1 || m > 12)
        return -1;

    leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    maxDay = monthDays[m - 1] + (m == 2 && leap);
    if (d < 1 || d > maxDay)
        return -1;
    return daysFromCivil(y, m, d);
}

long todayDayNumber(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    return daysFromCivil(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

int weekdayOf(long dayNumber)
{
    return (int)((dayNumber + 3) % 7);
}

long readDate(const char *prompt, char *text)
{
    char line[32];
    long dayNumber;

    for (;;) {
        readLine(prompt, line, sizeof(line));
        dayNumber = parseDate(line);
        if (dayNumber >= 0) {
            memcpy(text, line, DATE_LEN);
            return dayNumber;
        }
        puts("Invalid date. Use YYYY-MM-DD (years 2000-2100).");
    }
}

void readMonth(const char *prompt, char *month)
{
    char line[32], full[16];

    for (;;) {
        readLine(prompt, line, sizeof(line));
        if (strlen(line) == 7) {
            memcpy(full, line, 8);
            strcpy(full + 7, "-01");
            if (parseDate(full) >= 0) {
                memcpy(month, line, MONTH_LEN);
                return;
            }
        }
        puts("Invalid month. Use YYYY-MM.");
    }
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

int readTime(const char *prompt)
{
    char line[32];
    int minutes;

    for (;;) {
        readLine(prompt, line, sizeof(line));
        minutes = parseTime(line);
        if (minutes >= 0)
            return minutes;
        puts("Invalid time. Use HH:MM (24-hour).");
    }
}

void formatTime(int minute, char *out, size_t size)
{
    snprintf(out, size, "%02d:%02d", minute / 60, minute % 60);
}

int readDay(void)
{
    return readIntRange("Day (1=Monday ... 7=Sunday): ", 1, 7) - 1;
}

int rangesOverlap(long aStart, long aEnd, long bStart, long bEnd)
{
    return aStart < bEnd && bStart < aEnd;
}

Doctor *findDoctor(int id)
{
    int i;

    for (i = 0; i < doctorCount; ++i)
        if (doctors[i].id == id && doctors[i].active)
            return &doctors[i];
    return NULL;
}

Nurse *findNurse(int id)
{
    int i;

    for (i = 0; i < nurseCount; ++i)
        if (nurses[i].id == id && nurses[i].active)
            return &nurses[i];
    return NULL;
}
