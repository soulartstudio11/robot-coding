#include <SoftwareSerial.h>
#include <Servo.h>

// Bluetooth TX -> Arduino D10; Bluetooth RX -> Arduino D11
SoftwareSerial bluetooth(10, 11);

// L298N direction pins; leave ENA/ENB jumpers installed.
const byte IN1=2, IN2=3, IN3=4, IN4=7;
const byte TRIG_PIN=5, ECHO_PIN=6, MOISTURE_PIN=A0, SERVO_PIN=9, PUMP_PIN=8;

// Calibrate these for your probe mechanism and soil sensor.
const int SERVO_UP=20, SERVO_DOWN=100;
const int DRY_RAW=1023, WET_RAW=300;
const int OBSTACLE_CM=20;
const unsigned long AUTO_INTERVAL=1500;

Servo probeServo;
String serialBuffer="", bleBuffer="";
bool autoMode=false, pumpOnState=false;
int moistureMin=30, moistureMax=60;
unsigned long lastAuto=0;

void reply(const String &s) { Serial.println(s); bluetooth.println(s); }

void stopMotors() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW); digitalWrite(IN4,LOW);
}
void forward() {
  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);
}
void backward() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH);
}
void left() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);
}
void right() {
  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH);
}
// Assumes active-HIGH relay/driver input. Verify before connecting the pump.
void pumpOff() { digitalWrite(PUMP_PIN,LOW); pumpOnState=false; }
void pumpOn() { digitalWrite(PUMP_PIN,HIGH); pumpOnState=true; }

long readDistance() {
  digitalWrite(TRIG_PIN,LOW); delayMicroseconds(3);
  digitalWrite(TRIG_PIN,HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);
  unsigned long d=pulseIn(ECHO_PIN,HIGH,30000UL);
  return d==0 ? -1 : (long)(d/58);
}
int readMoistureRaw() {
  long sum=0;
  for(byte i=0;i<5;i++){ sum+=analogRead(MOISTURE_PIN); delay(8); }
  return (int)(sum/5);
}
int moisturePercent(int raw) {
  return constrain(map(raw,DRY_RAW,WET_RAW,0,100),0,100);
}
void moistureCheck() {
  autoMode=false; stopMotors(); pumpOff();
  reply("MOISTURE_CHECK: lowering probe");
  probeServo.write(SERVO_DOWN); delay(1000);
  int raw=readMoistureRaw();
  reply("MOISTURE_RAW="+String(raw));
  reply("MOISTURE_PERCENT="+String(moisturePercent(raw)));
  probeServo.write(SERVO_UP); delay(700);
  reply("MOISTURE_CHECK: probe raised");
}
void statusReport() {
  int raw=readMoistureRaw();
  reply("STATUS_BEGIN");
  reply("MODE="+String(autoMode ? "AUTO":"MANUAL"));
  reply("DISTANCE_CM="+String(readDistance()));
  reply("MOISTURE_RAW="+String(raw));
  reply("MOISTURE_PERCENT="+String(moisturePercent(raw)));
  reply("PUMP="+String(pumpOnState ? "ON":"OFF"));
  reply("STATUS_END");
}
void command(String c) {
  c.trim(); c.toUpperCase();
  if(!c.length()) return;
  reply("CMD="+c);

  if(c=="FORWARD"||c=="UP"||c=="W") {
    autoMode=false; pumpOff(); forward(); reply("OK: moving forward");
  } else if(c=="BACKWARD"||c=="DOWN"||c=="S") {
    autoMode=false; pumpOff(); backward(); reply("OK: moving backward");
  } else if(c=="LEFT"||c=="A") {
    autoMode=false; pumpOff(); left(); reply("OK: turning left");
  } else if(c=="RIGHT"||c=="D") {
    autoMode=false; pumpOff(); right(); reply("OK: turning right");
  } else if(c=="STOP"||c=="RELEASE") {
    autoMode=false; stopMotors(); pumpOff(); reply("OK: stopped");
  } else if(c=="WATER_ON"||c=="PUMP_ON") {
    pumpOn(); reply("OK: pump on");
  } else if(c=="WATER_OFF"||c=="PUMP_OFF") {
    pumpOff(); reply("OK: pump off");
  } else if(c=="MOISTURE_CHECK"||c=="CHECK_MOISTURE") {
    moistureCheck();
  } else if(c=="STATUS") {
    statusReport();
  } else if(c=="AUTO_START") {
    stopMotors(); pumpOff(); autoMode=true; lastAuto=0;
    reply("OK: basic auto started");
  } else if(c=="AUTO_STOP"||c=="STOP_AUTO") {
    autoMode=false; stopMotors(); pumpOff(); reply("OK: auto stopped");
  } else if(c.startsWith("MIN:")) {
    moistureMin=constrain(c.substring(4).toInt(),0,100);
    reply("OK: MIN="+String(moistureMin));
  } else if(c.startsWith("MAX:")) {
    moistureMax=constrain(c.substring(4).toInt(),0,100);
    reply("OK: MAX="+String(moistureMax));
  } else {
    reply("ERROR: unknown command");
  }
}
void readPort(Stream &port, String &buf) {
  while(port.available()) {
    char ch=(char)port.read();
    if(ch=='\r') continue;
    if(ch=='\n') { command(buf); buf=""; }
    else if(buf.length()<80) buf+=ch;
    else { buf=""; reply("ERROR: command too long"); }
  }
}
void automaticControl() {
  long cm=readDistance();
  if(cm>0 && cm<=OBSTACLE_CM) {
    stopMotors(); pumpOff(); reply("AUTO: obstacle detected; stopped"); return;
  }
  int pct=moisturePercent(readMoistureRaw());
  reply("AUTO: moisture="+String(pct)+"%");
  // Demonstration only; not route navigation or field-ready autonomous driving.
  if(pct<moistureMin) {
    stopMotors(); pumpOn(); delay(1200); pumpOff();
    reply("AUTO: short watering cycle complete");
  } else {
    pumpOff(); forward();
  }
}
void setup() {
  Serial.begin(9600); bluetooth.begin(9600);
  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT); pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
  pinMode(TRIG_PIN,OUTPUT); pinMode(ECHO_PIN,INPUT); pinMode(PUMP_PIN,OUTPUT);
  stopMotors(); pumpOff();
  probeServo.attach(SERVO_PIN); probeServo.write(SERVO_UP);
  delay(300);
  reply("Farming Robot ready");
  reply("Commands: STATUS, FORWARD, BACKWARD, LEFT, RIGHT, STOP");
}
void loop() {
  readPort(Serial,serialBuffer);
  readPort(bluetooth,bleBuffer);
  if(autoMode && millis()-lastAuto>=AUTO_INTERVAL) {
    lastAuto=millis(); automaticControl();
  }
}
