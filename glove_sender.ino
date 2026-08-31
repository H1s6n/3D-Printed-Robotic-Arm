#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <Stepper.h>
#include <ezButton.h>

const int EN_motor_1 = 23; // Enable pin for Motor 1
const int EN_motor_2 = 5;  // Enable pin for Motor 2
const int EN_motor_3 = 15; // Enable pin for Motor 3

ezButton limitSwitch_1(12); // Right

#define DIRECTION_1_CCW  1
#define DIRECTION_1_ACW -1
#define DIRECTION_2_CCW  1
#define DIRECTION_2_ACW -1
#define DIRECTION_3_CCW  1
#define DIRECTION_3_ACW -1

int motor_1_direction = DIRECTION_1_CCW;
int motor_2_direction = DIRECTION_2_CCW;
int motor_3_direction = DIRECTION_3_CCW;

static const int servoPin = 4;

const int steps_per_rev = 220; // Set to 220 for NEMA 17

// Motor 1, right
#define IN1_0 22
#define IN2_0 21
#define IN3_0 32
#define IN4_0 33

// Motor 2, base motor
#define IN1_1 26
#define IN2_1 25
#define IN3_1 27
#define IN4_1 14

// Motor 3, left
#define IN1_2 19
#define IN2_2 18
#define IN3_2 17
#define IN4_2 16

Servo servo1;

Stepper motor_1(steps_per_rev, IN1_0, IN2_0, IN3_0, IN4_0);
Stepper motor_2(steps_per_rev, IN1_1, IN2_1, IN3_1, IN4_1);
Stepper motor_3(steps_per_rev, IN1_2, IN2_2, IN3_2, IN4_2);

int rotation_val = steps_per_rev / 10;

float ax, ay, az = 0;
float gx, gy, gz = 0;

// ESP-NOW Data Structure
typedef struct struct_message {
  char a[100];
  int b;
  bool button;
  float ax_val; // Linear acceleration X
  float ay_val; // Linear acceleration Y
  float az_val; // Linear acceleration Z
  float gx_val;
  float gy_val;
  float gz_val;
} struct_message;

// Variables to hold received data
struct_message myData;

// Callback function executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("Button Value: ");
  Serial.println(myData.button);
  Serial.print("Linear Acceleration X: ");
  Serial.println(myData.ax_val);
  Serial.print("Linear Acceleration Y: ");
  Serial.println(myData.ay_val);
  Serial.print("Linear Acceleration Z: ");
  Serial.println(myData.az_val);
}

int button_saved = 0;

void setup(void) {
  Serial.begin(115200);

  pinMode(EN_motor_1, OUTPUT);
  pinMode(EN_motor_2, OUTPUT);
  pinMode(EN_motor_3, OUTPUT);

  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback function
esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  Serial.println("SETUP COMPLETE");

  servo1.attach(servoPin);
  motor_1.setSpeed(30);
  motor_2.setSpeed(40);
  motor_3.setSpeed(30);
  delay(10);
}

void loop() {
  // Update sensor values from received data
  servo1.write(myData.b);
  gx = myData.gx_val;
  gy = myData.gy_val;
  gz = myData.gz_val;

  ax = myData.ax_val;
  ay = myData.ay_val;
  az = myData.az_val;

  button_saved = myData.button; // Update button state

  if (button_saved == 1) {
    // Control Motor 2 with linear acceleration in Y-axis
    if (ay > 6) {
      digitalWrite(EN_motor_2, HIGH);
      digitalWrite(EN_motor_3, LOW);
      digitalWrite(EN_motor_1, LOW);
      motor_2.step(-rotation_val); // Move clockwise
      motor_2_direction = DIRECTION_2_CCW;
      Serial.println("Motor 2: Moving clockwise");
    } else if (ay < -6) {
      digitalWrite(EN_motor_2, HIGH);
      digitalWrite(EN_motor_3, LOW);
      digitalWrite(EN_motor_1, LOW);
      motor_2.step(rotation_val); // Move counterclockwise
      motor_2_direction = DIRECTION_2_ACW;
      Serial.println("Motor 2: Moving counterclockwise");
    } else {
      digitalWrite(EN_motor_2, LOW); // Disable Motor 2
      digitalWrite(EN_motor_3, LOW);
      digitalWrite(EN_motor_1, LOW);
    }
    
    // Motor 3 ON moves the forearm enable for
    if (ax > 5.5) {
      digitalWrite(EN_motor_3, HIGH);
      digitalWrite(EN_motor_1, LOW);
      digitalWrite(EN_motor_2, LOW);
      motor_3.step(rotation_val); // Move clockwise
      motor_3_direction = DIRECTION_3_CCW;
      Serial.println("Motor 3: Moving clockwise");
    } else if (ax < -5.5) {
      digitalWrite(EN_motor_3, HIGH);
      digitalWrite(EN_motor_1, LOW);
      digitalWrite(EN_motor_2, LOW);
      motor_3.step(-rotation_val); // Move counterclockwise
      motor_3_direction = DIRECTION_3_ACW;
      Serial.println("Motor 3: Moving counterclockwise");
    } else {
      digitalWrite(EN_motor_3, LOW); // Disable Motor 3
      digitalWrite(EN_motor_1, LOW);
      digitalWrite(EN_motor_2, LOW);
    }
  } else {
    // Motor 1 with linear acceleration in Z-axis extend the arm
    if (ax > 5.5) {
      digitalWrite(EN_motor_1, HIGH);
      digitalWrite(EN_motor_3, LOW);
      digitalWrite(EN_motor_2, LOW);
      motor_1.step(-rotation_val); // Move clockwise
      motor_1_direction = DIRECTION_1_CCW;
      Serial.println("Motor 1: Moving clockwise");
    } else if (ax < -5.5) {
      digitalWrite(EN_motor_1, HIGH);
      digitalWrite(EN_motor_3, LOW);
      digitalWrite(EN_motor_2, LOW);
      motor_1.step(rotation_val); // Move counterclockwise
      motor_1_direction = DIRECTION_1_ACW;
      Serial.println("Motor 1: Moving counterclockwise");
    } else {
      digitalWrite(EN_motor_1, LOW); // Disable Motor 1
      digitalWrite(EN_motor_3, LOW); // Disable Motor 3
      digitalWrite(EN_motor_2, LOW);
    }
  }

  // Disable all motor drivers briefly to avoid overdriving
  digitalWrite(EN_motor_1, LOW);
  digitalWrite(EN_motor_2, LOW);
  digitalWrite(EN_motor_3, LOW);

  delay(1); // Avoid flooding with commands
}

