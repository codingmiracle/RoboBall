#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


// --- CLASSES ---
int lastchannel = 0;
class Motor {
  private:
    int pin1;
    int pin2;
    int ch1;
    int ch2;
    int Speed;

  public:
    Motor() {}
    Motor(int, int, int, int);
    void drive();
    void setspeed(int);
    void backgroundActivity();
};


Motor::Motor(int p1, int p2, int freq, int res) {
  int pin1 = p1;
  int pin2 = p2;
  int ch1 = lastchannel++;
  int ch2 = lastchannel++;
  int Speed = 0;

  ledcSetup(ch1, freq, res);
  ledcSetup(ch2, freq, res);
  ledcAttachPin(pin1, ch1);
  ledcAttachPin(pin2, ch2);
}

void Motor::drive() {
  if ( Speed == 0 ) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  } else if (Speed > 0) {
    digitalWrite(pin2, LOW);
    ledcWrite(ch1, Speed);
  } else if (Speed < 0) {
    digitalWrite(pin1, LOW);
    ledcWrite(ch2, Speed * -1);
  }

  /*
  ledcwrite(Speed)
  */
}

void Motor::setspeed(int s)
{
  Speed = s;

}

void Motor::backgroundActivity()
{
  this->drive();
  this->Speed -= 1;
}

// --- GLOBAL VARIABLES ---
const int frequency = 5000;
const int resolution = 8;
int testchannel = 1;
const int testpin = 14;
int devchannel = 2;
const int devpin = 27;
BluetoothSerial SerialBT;

//Motor Right(35, 34, frequency, resolution);
//Motor Left(13, 12, frequency, resolution);

char command[30] = "";
int cmdp = 0;
int cmdavailable = false;


void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(35, OUTPUT);

  ledcSetup(testchannel, frequency, resolution);
  ledcAttachPin(testpin, testchannel);
  ledcSetup(devchannel, frequency, resolution);
  testchannel += 1;
  ledcAttachPin(devpin, testchannel);
  testchannel -= 1;


  Serial.begin(115200);
  SerialBT.begin("RoboBall"); //Bluetooth device name
  Serial.println("The Roboter started, now you can pair it with bluetooth!");
}


void loop() {
  int cmd;
  int speed;
  if (SerialBT.available() && !cmdavailable ) {
    cmd = SerialBT.read();
    if (cmd == 10) {
      command[cmdp-1] = '\0';
      cmdp = 0;
      cmdavailable = 1;
    } else {
      command[cmdp++] = (char)lowByte(cmd);
    }
  }
  if (cmdavailable) {
    cmdavailable = 0;
    Serial.println(command);
    if (String(command) == "left") {
      Serial.println("left");
    }
    speed = String(command).toInt();
    Serial.println(speed);
    if(speed >= 0) {
      ledcWrite(testchannel, speed);
      ledcWrite(devchannel, 0);
    } else if(speed < 0) {
      ledcWrite(testchannel, 0);
      ledcWrite(devchannel, speed*-1);
    } else {
      ledcWrite(testchannel, 0);
      ledcWrite(devchannel, 0);
    }
  }

}
