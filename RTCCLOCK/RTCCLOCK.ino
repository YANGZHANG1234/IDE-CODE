#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

void setup () {
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // 获取当前编译时间
  DateTime compileTime(F(__DATE__), F(__TIME__));
  // 获取RTC当前时间
  DateTime rtcTime = rtc.now();

  // 检查RTC时间是否失效或与编译时间不一致
  if (!rtc.isrunning() || !isTimeEqual(rtcTime, compileTime)) {
    Serial.println("RTC lost power or time mismatch, setting the time!");
    rtc.adjust(compileTime);
  }
}

void loop () {
  String currentTime = getCurrentTime();
  Serial.println(currentTime);
  delay(1000);
}

// 获取当前时间的函数
String getCurrentTime() {
  DateTime now = rtc.now();
  char buffer[20];
  sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", 
          now.year(), now.month(), now.day(), 
          now.hour(), now.minute(), now.second());
  return String(buffer);
}

// 比较两个时间是否相等的函数
bool isTimeEqual(DateTime t1, DateTime t2) {
  return (t1.year() == t2.year() &&
          t1.month() == t2.month() &&
          t1.day() == t2.day() &&
          t1.hour() == t2.hour() &&
          t1.minute() == t2.minute() &&
          t1.second() == t2.second());
}
