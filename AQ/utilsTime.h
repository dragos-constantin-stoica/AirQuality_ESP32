#ifndef UTILSTIME_H /* include guards */
#define UTILSTIME_H

// Utility print function
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime();
String getTimeStamp();
void RTC_Update();
bool RTC_Valid();

void printLocalTime();
void init_RTC();
void init_NTP();

#endif /* UTILSTIME_H */
