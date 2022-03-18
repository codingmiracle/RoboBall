#include <BluetoothSerial.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// --- MOTOR CLASS ---
int lastchannel = 0;
class Motor {
private:
  int pin1;
  int pin2;
  int ch1;
  int ch2;
  int currentSpeed; //slow-follows the controlSpeed

public:
  int controlSpeed; //controls the speed
  static const int updatedelay = 2000; //time in micros (* 256 = ramptime = time to go from 0 to max)

  Motor() {}
  Motor(int, int, int, int);
  void setSpeed(int);  
  void drive();
  void adjustCurrentSpeed();
};

Motor::Motor(int p1, int p2, int freq, int res)
{
  int pin1 = p1;
  int pin2 = p2;
  int ch1 = lastchannel++;
  int ch2 = lastchannel++;
  int controlSpeed = 0; 
  int currentSpeed = 0; 
 
  

  ledcSetup(ch1, freq, res);
  ledcSetup(ch2, freq, res);
  ledcAttachPin(pin1, ch1);
  ledcAttachPin(pin2, ch2);

}

void Motor::adjustCurrentSpeed()
{ 
  //update currentSpeed
  if(controlSpeed > currentSpeed) {
    currentSpeed += 1;
  } else if(controlSpeed < currentSpeed) {
    currentSpeed -= 1;
  }
  //set new currentSpeed
  if (currentSpeed == 0)
  {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }
  else if (currentSpeed > 0 && currentSpeed <= 256)
  {
    digitalWrite(pin2, LOW);
    ledcWrite(ch1, currentSpeed);
  }
  else if (currentSpeed < 0 && currentSpeed >= -256)
  {
    digitalWrite(pin1, LOW);
    ledcWrite(ch2, currentSpeed * -1);
  }
}

void Motor::setSpeed(int s)
{
  controlSpeed = s;
}


void Motor::drive()
{
  if (controlSpeed == 0)
  {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }
  else if (controlSpeed > 0 && controlSpeed <= 256)
  {
    digitalWrite(pin2, LOW);
    ledcWrite(ch1, currentSpeed);
  }
  else if (controlSpeed < 0 && controlSpeed >= -256)
  {
    digitalWrite(pin1, LOW);
    ledcWrite(ch2, controlSpeed * -1);
  }
}




// --- GLOBAL VARIABLES ---
const int motorpins[] = {12,13,26,27};
const int frequency = 5000;
const int resolution = 8;
int testchannel = 1;
const int testpin = 26;
int devchannel = 2;
const int devpin = 27;


BluetoothSerial SerialBT;
hw_timer_t * timer; // our timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

Motor motor;
Motor mRight(motorpins[0], motorpins[1], frequency, resolution);
Motor mLeft(motorpins[2], motorpins[3], frequency, resolution);

char command[30] = "";
int cmdp = 0;
int cmdavailable = false;
int spd;

// --- onInterrupt ---
void onIntervallUpdate() {
  mRight.adjustCurrentSpeed();
  mLeft.adjustCurrentSpeed();
}

// --- SETUP ---
void setup()
{
  int i;
  for(i = 0; i < 4; i++) {
    pinMode(motorpins[i], OUTPUT);
  }

  pinMode(testpin, OUTPUT);
  pinMode(devpin, OUTPUT);
  ledcSetup(testchannel, frequency, resolution);
  ledcAttachPin(testpin, testchannel);
  ledcSetup(devchannel, frequency, resolution);
  ledcAttachPin(devpin, devchannel);
  
  timer = timerBegin(0, 240, true);
  timerAttachInterrupt(timer, onIntervallUpdate, true);
  timerAlarmWrite(timer, motor.updatedelay, true);
  timerAlarmEnable(timer);

  Serial.begin(115200);
  SerialBT.begin("RoboBall"); // Bluetooth device name
  Serial.println("The Roboter started, now you can pair it with bluetooth!");


}

// --- LOOP ---
void loop()
{
  int cmd;
  // BT Handling -> Interrupt
  if (SerialBT.available() && !cmdavailable)
  {
    cmd = SerialBT.read();
    if (cmd == 10)
    {
      command[cmdp - 1] = '\0';
      cmdp = 0;
      cmdavailable = 1;
    }
    else
    {
      command[cmdp++] = (char)lowByte(cmd);
    }
  }

  // Command Handling
  if (cmdavailable)
  {
    cmdavailable = 0;
    Serial.println(command);
    
    spd = String(command).toInt();
    Serial.println(spd);

    mRight.setSpeed(spd);
    // motor action
    if (spd > 0)
    {
      ledcWrite(testchannel, spd);
      ledcWrite(devchannel, 0);
    }
    else if (spd < 0)
    {
      ledcWrite(testchannel, 0);
      ledcWrite(devchannel, spd * -1);
    }
    else
    {
      ledcWrite(testchannel, 0);
      ledcWrite(devchannel, 0);
    }
  }
}
