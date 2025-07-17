#include <Servo.h>

// === Steering Servos ===
Servo steeringServo;
Servo steeringServo2;
const int servoPin = A1;
const int servoPin2 = A0;

// === Motor Driver Pins ===
const int L_EN  = 2;
const int R_EN  = 4;
const int LPWM  = 5;
const int RPWM  = 6;

// === Ultrasonic Sensors ===
const int trigFront = 11;
const int echoFront = 10;
const int trigLeft  = 13;
const int echoLeft  = 12;
const int trigRight = 9;
const int echoRight = 8;

// === Button Pin (Pull-down) ===
const int buttonPin = 7;

// === Motion Constants ===
const int motorSpeed = 200;
const int minWallDistance = 20;
const int baseSteer = 0;

// === Control Flags ===
bool activationDone = false;
int lastSteeringAngle = baseSteer;

// === Sensor Readings ===
long leftDist = 0, rightDist = 0;

void setup() {
  Serial.begin(9600);

  // Servo setup
  steeringServo.attach(servoPin);
  steeringServo2.attach(servoPin2);
  setSteeringAngle(0);

  // Motor setup
  pinMode(L_EN, OUTPUT); digitalWrite(L_EN, HIGH);
  pinMode(R_EN, OUTPUT); digitalWrite(R_EN, HIGH);
  pinMode(LPWM, OUTPUT); pinMode(RPWM, OUTPUT);

  // Ultrasonic sensor setup
  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft, OUTPUT);  pinMode(echoLeft, INPUT);
  pinMode(trigRight, OUTPUT); pinMode(echoRight, INPUT);

  // Pull-down button setup
  pinMode(buttonPin, INPUT); // External pull-down resistor used

  Serial.println("Waiting for button press...");
}

void loop() {
  // === Wait for activation via button press (HIGH when pressed) ===
  if (!activationDone) {
    if (digitalRead(buttonPin) == HIGH) {
      activationDone = true;
      Serial.println("Bot Activated");
      delay(300); // debounce delay
    }
    return;
  }

  // === Update side distances ===
  leftDist = getDistance(trigLeft, echoLeft);
  rightDist = getDistance(trigRight, echoRight);

  // === Check for open space to left or right
  if (leftDist > 105 || rightDist > 105) {
    if (leftDist > rightDist) {
      Serial.println("Turning LEFT to open space");
      turnLeft();
    } else {
      Serial.println("Turning RIGHT to open space");
      turnRight();
    }
    return;
  }

  // === Wall-following alignment logic ===
  int steerAdjust = baseSteer;
  if (leftDist < 30) steerAdjust = 40;
  else if (rightDist < minWallDistance) steerAdjust = -30;
  else if (leftDist > 25 && rightDist > 25) steerAdjust = -20;

  lastSteeringAngle = steerAdjust;
  setSteeringAngle(steerAdjust);

  // Move forward
  analogWrite(RPWM, motorSpeed);
  analogWrite(LPWM, 0);


}

// === Set Steering Angle Function ===
void setSteeringAngle(int angle) {
  angle = constrain(angle, -30, 30);
  int servoAngle = map(angle, -30, 30, 0, 60);
  int servoAngle2 = 60 - servoAngle;
  steeringServo.write(servoAngle);
  steeringServo2.write(servoAngle2);
}

// === Ultrasonic Distance Calculation ===
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  return duration * 0.034 / 2;
}

// === Turn Functions ===
void turnLeft() {
  setSteeringAngle(30);
  analogWrite(RPWM, motorSpeed);
  analogWrite(LPWM, 0);
  delay(1000);
  setSteeringAngle(0);
}

void turnRight() {
  setSteeringAngle(-30);
  analogWrite(RPWM, motorSpeed);
  analogWrite(LPWM, 0);
  delay(1000);
  setSteeringAngle(0);
}
