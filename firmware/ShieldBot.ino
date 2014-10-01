#include <SoftwareSerial.h>
#include <Shieldbot.h>

//Software Serial Ports
#define RxD 2
#define TxD 3

#define DEBUG_ENABLED  1

SoftwareSerial BLE(RxD,TxD);
Shieldbot shieldbot = Shieldbot();

int speed = 0;
int yaw = 0;
int transmission = 0;
int direction = 0;

void setup(){
  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBleConnection();
  shieldbot.setMaxSpeed(255);//255 is max
  Serial.println("BLE Shieldbot");
  runCommand(0x00);
  runCommand(0x80);
}

void loop(){
  char recvChar;
  if (BLE.available()) {
    recvChar = BLE.read();
    Serial.print(recvChar);
    runCommand(recvChar);
  }
  if(Serial.available()) {
    recvChar  = (char)Serial.parseInt();
    //Serial.println(recvChar);
    //BLE.print(recvChar);
    runCommand(recvChar);
  }
  delay(50);
}

// Single byte commands format:
// 0-6bits: value (0 - 63)
// 7bit: direction (1 - forward/right, 0 - backward/left)
// 8bit: 0 - yaw, 1 - speed
void runCommand(char command) {
  if (command & 0x80) {
    // update speed & transmission
    speed = command & 0x3F;
    transmission = (command & 0x40) >> 6;
  } else {
    // update yaw & direction
    yaw = command & 0x3F;
    direction = (command & 0x40) >> 6;
  }
  updateCourse();
}

void updateCourse() {
  int leftPower;
  int rightPower;
  
  Serial.print("speed = ");
  Serial.print(speed);
  Serial.print(", yaw= ");
  Serial.println(yaw);
  Serial.print("transmission = ");
  Serial.print(transmission);
  Serial.print(", direction = ");
  Serial.println(direction);
  
  int power = speed * (transmission*2-1);
  if (speed == 0 && yaw != 0) {
    power = 63;
    if (direction == 0) {
      // left yaw
      leftPower = 0;
      rightPower = map(yaw, 0, 64, 0, power);
    } else {
      // right yaw
      rightPower = 0;
      leftPower = map(yaw, 0, 64, 0, power);
    }    
  } else {
    if (direction == 0) {
      // left yaw
      leftPower = map(power, -64, 64, yaw-64, 64-yaw);
      rightPower = power;
    } else {
      // right yaw
      rightPower = map(power, -64, 64, yaw-64, 64-yaw);
      leftPower = power;
    }
  }
  
  // motor protection
  leftPower = map(leftPower, -64, 64, -128, 127);
  rightPower = map(rightPower, -64, 64, -128, 127);

  shieldbot.leftMotor(leftPower);
  shieldbot.rightMotor(rightPower);
  
  Serial.print("power = ");
  Serial.print(power);
  Serial.print("leftPower = ");
  Serial.print(leftPower);
  Serial.print(", rightPower = ");
  Serial.println(rightPower);
}

void setupBleConnection()
{
  BLE.begin(9600); //Set BLE BaudRate to default baud rate 9600
  BLE.print("AT+CLEAR"); //clear all previous setting
  BLE.print("AT+ROLE1"); //set the bluetooth name as a master
  BLE.print("AT+SAVE1");  //don't save the connect information
}
