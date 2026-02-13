#include <SoftwareSerial.h>
#define IR_PIN   2
#define LED_PIN  5
#define GSM_RX 8
#define GSM_TX 9
SoftwareSerial gsm(GSM_RX, GSM_TX);
bool ledForcedOff = false;
bool callDone = false;
void setup() {
  pinMode(IR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  gsm.begin(9600);
  delay(8000); 
  Serial.println("Initializing GSM...");
  sendCmd("AT");
  sendCmd("ATE0");
  sendCmd("AT+CMGF=1");
  sendCmd("AT+CLIP=1");
  sendCmd("AT+CVHU=0");
  sendCmd("AT+CSCS=\"GSM\"");
  sendCmd("AT+CNMI=2,2,0,0,0");
  Serial.println("System Ready");
}
void loop() {
  if (digitalRead(IR_PIN) == LOW && !ledForcedOff && !callDone) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("IR detected");
    sendSMS("8754903681", "HEXHive ALERT");
    delay(8000);
    makeCall("+918754903681");
    callDone = true;
  }
  while (gsm.available()) {
    String line = gsm.readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    Serial.println(line);
    if (line == "OFF") {
      digitalWrite(LED_PIN, LOW);
      ledForcedOff = true;
      Serial.println("LED Forced OFF");
      ledForcedOff = false;
      callDone = false;
      Serial.println("Sensor Restored");
    }
    if (line == "ON"){
      digitalWrite(LED_PIN, HIGH);
    }
    if (line.indexOf("RING") != -1) {
      Serial.println("Incoming Call");
      gsm.println("ATA");
      delay(5000);
      gsm.println("ATH");
    }
  }
}
void sendSMS(String number, String text) {
  Serial.println("Sending SMS...");
  gsm.print("AT+CMGS=\"");
  gsm.print(number);
  gsm.println("\"");
  delay(1500);
  gsm.print(text);
  delay(500);
  gsm.write(26);
  delay(4000);
}
void makeCall(String number) {
  Serial.println("Dialing Call...");
  gsm.print("ATD");
  gsm.print(number);
  gsm.println(";");
  unsigned long start = millis();
  while (millis() - start < 20000) {
    if (gsm.available()) {
      String r = gsm.readString();
      Serial.println(r);
      if (r.indexOf("OK") != -1 || r.indexOf("CONNECT") != -1) {
        Serial.println("Call Connected");
        break;
      }
    }
  }
  delay(10000);
  gsm.println("ATH");
  Serial.println("Call Ended");
}
void sendCmd(String cmd) {
  gsm.println(cmd);
  delay(1000);
  while (gsm.available()) {
    Serial.write(gsm.read());
  }
}