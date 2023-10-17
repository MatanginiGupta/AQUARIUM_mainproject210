#include <LiquidCrystal.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// LCD
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Sensors
OneWire oneWire(7); // DS18B20 Temperature sensor on pin D7
DallasTemperature sensors(&oneWire);
Servo servo;

float temperature; // Store temperature value
int tdsSensorPin = A0;
int ldrSensorPin = A1;
int servoPin = 3;


// LED
int ledPin1 = 5; // PWM-capable LED pin
int ledPin2 = 6; // PWM-capable LED pin

// Thresholds and motor delay
int sensorThresholds[3];
int motorDelay;
int ldrThreshold;
int Flag=1;
long int t1,t2;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // LCD setup
  lcd.begin(16, 2);

  // Sensors
  sensors.begin();
  pinMode(tdsSensorPin, INPUT);
  pinMode(ldrSensorPin, INPUT);
  
  servo.attach(servoPin);
  servo.write(90);
  // LED
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  analogWrite(ledPin1, 10); // Low intensity LED using PWM
  digitalWrite( ledPin2,LOW );

  // Initial LCD message
  lcd.setCursor(0, 0);
  lcd.print("Aquarium Health");
  lcd.setCursor(0, 1);
  lcd.print("Tracking Systm");

  delay(2000);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting....");
  delay(1000);
}

void loop() {
  if (Flag==1){
    serialRec();
    Flag=2;
    t1 = millis();  
  }
  // Read sensors
  temperature = readTemperature();
  int tdsValue = analogRead(tdsSensorPin);
  int ldrValue = analogRead(ldrSensorPin);
  
  // Display sensor data and messages on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:" + String(temperature) + "C");
  lcd.print(" Td:" + String(tdsValue));
  lcd.setCursor(0, 1);
  lcd.print("Lux:" + String(ldrValue));

  if (temperature > sensorThresholds[0]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp Rise");
  } 

  if (tdsValue > sensorThresholds[1]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bad Quality");
    lcd.setCursor(0, 1);
    lcd.print("Water");
  } else if (tdsValue > 50 && tdsValue<sensorThresholds[1]-10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Good Quality");
    lcd.setCursor(0, 1);
    lcd.print("Water");
  }


  if (ldrValue < ldrThreshold) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Light is Low ");
    analogWrite(ledPin1, 255); // Full intensity LED
  } else {
    analogWrite(ledPin1, 10); // Low intensity LED using PWM
  }

  // Perform servo action based on motor delay
  t2 = millis();
  if ((t2-t1) >= motorDelay) {
    servo.write(40); // Open servo to 90 degrees
    delay(3000); // Keep it open for 3 seconds
    servo.write(90); // Close servo to 0 degrees
    delay(3000); // Keep it closed for 3 seconds
    t1 = millis();
  }

  // Send sensor data via serial
  String sensorData = "*";
  sensorData += String(temperature) + ",";
  sensorData += String(ldrValue) + ",";
  sensorData += String(tdsValue) + "#";
  Serial.println(sensorData);
  delay(1000);
  
}

float readTemperature() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  return sensors.getTempCByIndex(0); // Index 0 is the only sensor
}

void serialRec() {
  String packet = ""; // Store the received packet
  while (true) {
  if (Serial.available()>0) {
    packet = Serial.readStringUntil('#');
      processPacket(packet);     
      packet = ""; // Clear the packet for the next reception
      break;
    } 
  }

  }

void processPacket(String packet) {
  // Parse the packet and extract values
  int values[4]; // Assuming there are 4 values in the packet
  int currentIndex = 0;
  String temp = "";
  packet+=",";
  for (int i = 1; i < packet.length(); i++) {
    char c = packet[i];
    if (c == ',') {
      int val=temp.toInt();
      values[currentIndex] =val; 
      currentIndex++;
      temp = "";
    } else {
      temp += c;
    }
  }

  // Check if the received values match your expected format
  if (currentIndex == 4) {
    sensorThresholds[0] = values[0]; // Temperature threshold
    sensorThresholds[1] = values[1]; // TDS threshold
    ldrThreshold = values[2]; // LDR threshold
    motorDelay = values[3]*1000; // Motor delay
  }
}
