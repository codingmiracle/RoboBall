#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const int frequency = 1000;
const int resolution = 16;
int lastchannel = 0;
BluetoothSerial SerialBT;

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
  pin1 = p1;
  pin2 = p2;
  ch1 = lastchannel + 1;
  ch2 = lastchannel + 2;
  lastchannel += 2;

  ledcSetup(ch1, freq, res);
  ledcSetup(ch2, freq, res);
  ledcAttachPin(pin1, ch1);
  ledcAttachPin(pin2, ch2);
}

void Motor::drive() {
  if( Speed == 0 ) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  } else if(Speed > 0) {
    digitalWrite(pin2, LOW);
    ledcWrite(ch1, Speed);
  } else if(Speed < 0) {
    digitalWrite(pin1, LOW);
    ledcWrite(ch2, Speed * -1);
  }
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

Motor Right(13, 12, frequency, resolution);
Motor Left(35, 32, frequency, resolution);
String cmd;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(32, OUTPUT);

  Serial.begin(115200);
  SerialBT.begin("ESPDavid"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  
  if (SerialBT.available()) {
    cmd = SerialBT.read();
      if(cmd == "go") {
        Right.setspeed(50);
        Left.setspeed(50);
        delay(1);
      }
      /*if(cmd == "stop") {
        Right.Speed(0);
        Left.Speed(0);
      }*/
    }
    Right.backgroundActivity();
    Left.backgroundActivity();
}
