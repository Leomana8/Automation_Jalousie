#include <Servo.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <IRremote.h>

#define IR A3
#define CLK_SDA A4
#define CLK_SCL A5


enum eKeys : unsigned long
{
  KEY_CH_DOWN =    16753245,
  KEY_CH_UP =      16736925,
  KEY_PREV_DOWN =  16720605,
  KEY_NEXT_UP =    16712445,
  KEY_VOL_DOWN =   16769055,
  KEY_VOL_UP =     16754775,
  KEY_0_DOWN =     16738455,
  KEY_100_UP =     16750695,

  KEY_1_DOWN =     16724175,
  KEY_2_UP =       16718055,
  KEY_4_CLOSE =    16716015,
  KEY_5_OPEN =     16726215,

  KEY_CH_SWITCH =   16769565,
  KEY_PLAY_SWITCH = 16761405,
  KEY_EQ_SWITCH =   16748655,
  KEY_200_SWITCH =  16756815,

  STICK = 4294967295
};

enum eDirection
{
  CONNECT_LEFT,
  CONNECT_RIGHT
};

int const ServoCount = 4;
int const StepServo = 8;
int SERVO_PINS[ServoCount] = {3, 5, 6, 9};
int SERVO_TYPE_CONNECT[ServoCount] = {CONNECT_LEFT, CONNECT_RIGHT, CONNECT_LEFT, CONNECT_RIGHT};
Servo servos[ServoCount];

IRrecv irrecv(IR);
decode_results results;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  setSyncProvider(RTC.get);

  irrecv.enableIRIn();
  Alarm.alarmRepeat( 6, 20, 0, OpenAllJalousieByAlarm);
  Alarm.alarmRepeat( 16, 0, 0, CloseAllJalousieByAlarm);

  Serial.println("Setup end");
  digitalClockDisplay();
}

void loop() {
  ManageServoByIR();
  Alarm.delay(100);
}

void RotateServoToEnd( int servoNum,  int pos )
{
  if ( pos >= 180)
    pos = 180;
  for (int i = servos[servoNum].read(); i <= pos; i++)
  {
    servos[servoNum].write(i);
    Alarm.delay(15);
  }
}

void RotateServoToStart( int servoNum,  int pos )
{
  if ( pos <= 0)
    pos = 0;
  for (int i = servos[servoNum].read(); i >= pos; i--)
  {
    servos[servoNum].write(i);
    Alarm.delay(15);
  }
}

void UpServo( int servoNum,  int step )
{
  servos[servoNum].attach(SERVO_PINS[servoNum]);
  int currentPos = servos[servoNum].read();
  Serial.print( "UP. Servo " + String(servoNum) );
  Serial.println( " Current pos " + String(currentPos) );
  if ( SERVO_TYPE_CONNECT[servoNum] ==  CONNECT_RIGHT )
  {
    int pos = currentPos - step;
    RotateServoToStart( servoNum, pos );
  }
  else
  {
    int pos = currentPos + step;
    RotateServoToEnd( servoNum, pos );
  }
  Serial.println(String(servos[servoNum].read()));
  servos[servoNum].detach();
}

void DownServo( int servoNum,  int step )
{
  servos[servoNum].attach(SERVO_PINS[servoNum]);
  int currentPos = servos[servoNum].read();
  Serial.print( "DOWN. Servo " + String(servoNum) );
  Serial.println( " Current pos " + String(currentPos) );
  if ( SERVO_TYPE_CONNECT[servoNum] == CONNECT_LEFT )
  {
    int pos = currentPos - step;
    RotateServoToStart( servoNum, pos );
  }
  else
  {
    int pos = currentPos + step;
    RotateServoToEnd( servoNum, pos );
  }
  Serial.println(String(servos[servoNum].read()));
  servos[servoNum].detach();
}

void UpJalousie( int servoNum )
{
  UpServo(servoNum, StepServo);
}

void DownJalousie( int servoNum )
{
  DownServo(servoNum, StepServo);
}

void OpenAllJalousie()
{
  Serial.println("OpenAllJalousie");
  for (byte i = 0; i < ServoCount; i++)
    UpServo(i, 180);
}

void CloseAllJalousie()
{
  Serial.println("CloseAllJalousie");
  for (byte i = 0; i < ServoCount; i++)
    DownServo(i, 180);

}

void OpenAllJalousieByAlarm()
{
  Serial.println("OpenAllJalousieByAlarm");
  int currentWeekday = weekday();
  if ( currentWeekday == dowSaturday || currentWeekday == dowSunday )
    Alarm.alarmOnce( 9, 20, 0, OpenAllJalousie);
  else
  {
    float currentDate = GetCurrentDay();
    float offsetTime = ( -sin(0.018 * currentDate - 1.608) + 1 ) * 0.8;
    if (offsetTime < 1)
      offsetTime = 0.35; // 6:20
    int offsetHour = (int)offsetTime;
    int offsetMinute = (offsetTime - (float)offsetHour) * 58;
    Alarm.alarmOnce( 6 + offsetHour, offsetMinute + 1, 0, OpenAllJalousie);
  }
}

void CloseAllJalousieByAlarm()
{
  Serial.println("CloseAllJalousieByAlarm");
  float currentDate = GetCurrentDay();
  float offsetTime = ( sin(0.018 * currentDate - 1.608) + 1 ) * 2.5 + 0.6;
  int offsetHour = (int)offsetTime;
  int offsetMinute = (offsetTime - (float)offsetHour) * 58;
  Alarm.alarmOnce( 16 + offsetHour, offsetMinute + 1, 0, CloseAllJalousie);
}

int GetCurrentDay()
{
  int currentMonth = month();
  float currentDay = day();
  return (currentMonth - 1) * (float)30.4 + currentDay;
}
void SwitchJalousie( int servoNum )
{
  Serial.print("SwitchJalousie. Number ");
  Serial.println(servoNum);
  if ( ( (int)(servos[servoNum].read() < 90) + (int)(SERVO_TYPE_CONNECT[servoNum] == CONNECT_LEFT) ) & 1 )
    DownServo(servoNum, 180);
  else
    UpServo(servoNum, 180);
}

void UpAllJalousie()
{
  Serial.println("UpAllJalousie");
  for (byte i = 0; i < ServoCount; i++)
    UpJalousie(i);
}

void DownAllJalousie()
{
  Serial.println("DownAllJalousie");
  for (byte i = 0; i < ServoCount; i++)
    DownJalousie(i);
}

unsigned long ReciveKeys()
{
  decode_results results;
  if (irrecv.decode(&results))
  {
    Serial.print("SIG ");
    Serial.println(results.value);
    irrecv.resume();
    return results.value;
  }
}

unsigned long lastButton = 0;
void ManageServoByIR()
{
  unsigned long IRSignal = ReciveKeys();
  if (IRSignal == STICK)
  {
    IRSignal = lastButton;
  }
  switch (IRSignal)
  {
    // НУЛЕВАЯ
    case KEY_CH_DOWN: {
        lastButton = IRSignal;
        DownJalousie(0);
        break;
      }
    case KEY_CH_UP: {
        lastButton = IRSignal;
        UpJalousie(0);
        break;
      }
    case KEY_CH_SWITCH: {
        SwitchJalousie(0);
        break;
      }
    // ПЕРВАЯ
    case KEY_PREV_DOWN: {
        lastButton = IRSignal;
        DownJalousie(1);
        break;
      }
    case KEY_NEXT_UP: {
        lastButton = IRSignal;
        UpJalousie(1);
        break;
      }
    case KEY_PLAY_SWITCH: {
        SwitchJalousie(1);
        break;
      }
    // ВТОРАЯ
    case KEY_VOL_DOWN: {
        lastButton = IRSignal;
        DownJalousie(2);
        break;
      }
    case KEY_VOL_UP: {
        lastButton = IRSignal;
        UpJalousie(2);
        break;
      }
    case KEY_EQ_SWITCH: {
        SwitchJalousie(2);
        break;
      }
    // ТРЕТЬЯ
    case KEY_0_DOWN: {
        lastButton = IRSignal;
        DownJalousie(3);
        break;
      }
    case KEY_100_UP: {
        lastButton = IRSignal;
        UpJalousie(3);
        break;
      }
    case KEY_200_SWITCH: {
        SwitchJalousie(3);
        break;
      }
    // ВСЕ
    case KEY_1_DOWN: {
        lastButton = IRSignal;
        DownAllJalousie();
        break;
      }
    case KEY_2_UP: {
        lastButton = IRSignal;
        UpAllJalousie();
        break;
      }
    case KEY_4_CLOSE: {
        CloseAllJalousie();
        break;
      }
    case KEY_5_OPEN: {
        OpenAllJalousie();
        break;
      }
  }
}

void digitalClockDisplay() {
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.print(" Weekday ");
  Serial.print(weekday());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
