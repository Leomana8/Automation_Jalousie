#include <IRremote.h>
#include <Servo.h>

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
  STICK = 4294967295
};

int const ServoCount = 4;
int const StepServo = 6;
int SERVO_PINS[ServoCount] = {3, 5, 6, 9};
int SERVO_MinImps[ServoCount] = {590, 590, 590, 590};
int SERVO_MaxImps[ServoCount] = {2200, 2200, 2200, 2400};
Servo servos[ServoCount];

IRrecv irrecv(IR);
decode_results results;


void setup() {
  Serial.begin(9600);
  for (int i = 0; i < ServoCount; i++)
  {
    servos[i].attach(SERVO_PINS[i]);
    UpServo( i, SERVO_MaxImps[i] );
  }
  irrecv.enableIRIn();
  Serial.println("Setup end");
}

void loop() {
  ManageServoByIR();
  delay(100);
}

void UpServo( int servoNum )
{
  int currentPos = servos[servoNum].readMicroseconds();
  Serial.print("UP. Current ");
  Serial.println(currentPos);
  UpServo(servoNum, currentPos + StepServo*10);
}

void UpServo( int servoNum,  int pos )
{
  if (pos >= SERVO_MaxImps[servoNum])
    pos = SERVO_MaxImps[servoNum];
  for (int i = servos[servoNum].readMicroseconds(); i <= pos; i = i + StepServo)
  {
    Serial.print("UP. Current " + servoNum);
    Serial.println(i);
    servos[servoNum].writeMicroseconds(i);
    //delay(15);
  }
}

void DownServo( int servoNum )
{
  int currentPos = servos[servoNum].readMicroseconds();
  Serial.print("DOWN. Current ");
  Serial.println(currentPos);
  DownServo(servoNum, currentPos - StepServo*10);
}

void DownServo( int servoNum,  int pos )
{
  if (pos <= SERVO_MinImps[servoNum])
    pos = SERVO_MinImps[servoNum];
  for (int i = servos[servoNum].readMicroseconds(); i >= pos; i = i - StepServo)
  {
    Serial.print("DOWN. Current ");
    Serial.println(i);
    servos[servoNum].writeMicroseconds(i);
    //delay(15);
  }
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
    case KEY_CH_DOWN: {
        lastButton = IRSignal;
        DownServo(0);
        break;
      }
    case KEY_CH_UP: {
        lastButton = IRSignal;
        UpServo(0);
        break;
      }
  }
  //delay(20);
}


