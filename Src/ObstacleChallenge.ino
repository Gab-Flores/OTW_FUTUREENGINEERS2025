#include <Servo.h>
#include <SoftwareSerial.h>

// Motor Driver Pins
#define L_EN 2
#define R_EN 4
#define LPWM 5
#define RPWM 6

// Ultrasonic Pins
#define TRIG_LEFT 13
#define ECHO_LEFT 12
#define TRIG_FRONT 11
#define ECHO_FRONT 10
#define TRIG_RIGHT 9
#define ECHO_RIGHT 8

// Servo Pins
#define STEERING_SERVO_PIN A1
#define SECOND_SERVO_PIN A0

// Button Pin
#define BUTTON_PIN 7

// HuskyLens Serial
SoftwareSerial huskySerial(0, 1); // TX=0, RX=1

Servo steeringServo;
Servo secondServo;

bool started = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

// Constants
const int STRAIGHT_ANGLE = 30;
const int LEFT_ANGLE = 0;
const int RIGHT_ANGLE = 60;

// ========================
// === Helper Functions ===
// ========================

long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH, 30000) * 0.034 / 2;
}

void stopMotors() {
  digitalWrite(L_EN, LOW);
  digitalWrite(R_EN, LOW);
  analogWrite(LPWM, 0);
  analogWrite(RPWM, 0);
}

void moveForward(int speed = 150) {
  digitalWrite(L_EN, HIGH);
  digitalWrite(R_EN, HIGH);
  analogWrite(LPWM, speed);
  analogWrite(RPWM, speed);
}

void turnLeft(int speed = 100) {
  digitalWrite(L_EN, HIGH);
  digitalWrite(R_EN, HIGH);
  analogWrite(LPWM, speed);
  analogWrite(RPWM, 0);
}

void turnRight(int speed = 100) {
  digitalWrite(L_EN, HIGH);
  digitalWrite(R_EN, HIGH);
  analogWrite(LPWM, 0);
  analogWrite(RPWM, speed);
}

void steer(int angle) {
  steeringServo.write(angle);
}

int readColorID() {
  if (huskySerial.available()) {
    if (huskySerial.read() == 0x55 && huskySerial.read() == 0xAA) {
      for (int i = 0; i < 4; i++) huskySerial.read(); // skip 4 bytes
      int id = huskySerial.read(); // color ID
      return id;
    }
  }
  return 0;
}

// ========================
// === Setup & Loop =======
// ========================

void setup() {
  Serial.begin(9600);
  huskySerial.begin(9600);

  // Motor driver setup
  pinMode(L_EN, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(RPWM, OUTPUT);

  // Ultrasonic sensors
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Servos
  steeringServo.attach(STEERING_SERVO_PIN);
  secondServo.attach(SECOND_SERVO_PIN);
  steer(STRAIGHT_ANGLE);

  stopMotors();
  Serial.println("Waiting for button press...");
}

void loop() {
  // === Start on Button Press ===
  if (!started && digitalRead(BUTTON_PIN) == LOW) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      started = true;
      Serial.println("Started!");
    }
    lastDebounceTime = millis();
  }

  if (!started) return;

  // === Read Sensors ===
  long leftDist = readDistanceCM(TRIG_LEFT, ECHO_LEFT);
  long frontDist = readDistanceCM(TRIG_FRONT, ECHO_FRONT);
  long rightDist = readDistanceCM(TRIG_RIGHT, ECHO_RIGHT);
  int colorID = readColorID();

  Serial.print("Left: "); Serial.print(leftDist);
  Serial.print(" | Front: "); Serial.print(frontDist);
  Serial.print(" | Right: "); Serial.print(rightDist);
  Serial.print(" | Color ID: "); Serial.println(colorID);

  // === Color Obstacle Avoidance ===
  if (colorID == 1) {  // Red → left
    steer(LEFT_ANGLE);
    turnLeft();
    delay(500);
    steer(STRAIGHT_ANGLE);
  } else if (colorID == 2) {  // Green → right
    steer(RIGHT_ANGLE);
    turnRight();
    delay(500);
    steer(STRAIGHT_ANGLE);
  }

  // === Wall Following Logic ===
  else if (leftDist > 200 && rightDist < 200) {
    steer(LEFT_ANGLE);
    turnLeft();
    delay(500);
    steer(STRAIGHT_ANGLE);
  } else if (rightDist > 200 && leftDist < 200) {
    steer(RIGHT_ANGLE);
    turnRight();
    delay(500);
    steer(STRAIGHT_ANGLE);
  } else {
    steer(STRAIGHT_ANGLE);
    moveForward();
  }

  delay(100); // short loop delay
}
