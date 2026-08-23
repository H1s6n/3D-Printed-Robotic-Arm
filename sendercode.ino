#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>

// Flex sensor pin
#define flexsensor  35
int Value;
int servoangle;

#define I2C_SDA 21
#define I2C_SCL 22

// MPU6050 and gravity variables
Adafruit_MPU6050 mpu;
float ax, ay, az;
float gx, gy, gz;

//    Button 
const int buttonPin_0 = 14;

// Receiver's MAC address
uint8_t broadcastAddress[] = {0xE8, 0x68, 0xE7, 0x30, 0x4D, 0x44};

// Define a data structure
typedef struct struct_message {
  char a[100];
  int b;
  bool button;
  float ax_val;
  float ay_val;
  float az_val;
  float gx_val;
  float gy_val;
  float gz_val;
} struct_message;

// Create a structured object
struct_message myData;

// Peer info
esp_now_peer_info_t peerInfo;

// Callback function when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


// Calibration offsets
float offsetaX = 0.0;
float offsetaY = 0.0;
float offsetaZ = 0.0;
float offsetgX = 0.0;
float offsetgY = 0.0;
float offsetgZ = 0.0;

// Calibration function to adjust the MPU6050 accelerometer values
void calibrateMPU() {
  float sumaX = 0, sumaY = 0, sumaZ = 0;
  float sumgX = 0, sumgY = 0, sumgZ = 0;
  int samples = 500;

  // Collect a few samples to get a good average for calibration
  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);  // Get accelerometer event data
    sumaX += a.acceleration.x;
    sumaY += a.acceleration.y;
    sumaZ += a.acceleration.z;
    sumgX += g.gyro.x;
    sumgY += g.gyro.y;
    sumgZ += g.gyro.z;
    delay(5); // Small delay to avoid flooding the sensor
  }

  // Calculate the average (offset) values for calibration
  offsetaX = sumaX / samples;
  offsetaY = sumaY / samples;
  offsetaZ = sumaZ / samples;

  offsetgX = sumgX / samples;
  offsetgY = sumgY / samples;
  offsetgZ = sumgZ / samples;

  Serial.println("Calibration completed!");
}

void setup(void) {
  
  Serial.begin(115200);
  
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin_0, INPUT_PULLUP);
  pinMode(flexsensor, INPUT);
 
  Wire.begin(I2C_SDA, I2C_SCL); 

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 chip found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ );

  delay(3);

    // Perform calibration for the MPU
  calibrateMPU();

  delay(10);
}

void loop() {
  
  myData.button  = digitalRead(buttonPin_0);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  ax = a.acceleration.x - offsetaX;
  ay = a.acceleration.y - offsetaY;
  az = a.acceleration.z - offsetaZ;

  gx = g.gyro.x  - offsetgX;
  gy = g.gyro.y  - offsetgY;
  gz = g.gyro.z  - offsetgZ;

  // Print the results
  Serial.print("Linear Acceleration: X:");
  Serial.print(ax);
  Serial.print(", Y:");
  Serial.print(ay);
  Serial.print(", Z:");
  Serial.print(az);
  Serial.println(" (m/s^2)");

  // Print rotation readings
  Serial.print("Rotation:       X:");
  Serial.print(gx);
  Serial.print(", Y:");
  Serial.print(gy);
  Serial.print(", Z:");
  Serial.print(gz);
  Serial.println(" (rad/s)");

  // Add a delay to avoid flooding the serial monitor
  Serial.println("");

  // Flex Sensor and Servo Logic
  Value = analogRead(flexsensor);
  Serial.print("Sensor Value RAW: ");
  Serial.println(Value);
  delay(1);

  servoangle = map(Value, 900, 2600, 180, 0);
  Serial.print("Sensor Mapped: ");
  Serial.println(servoangle);
  delay(1);

  servoangle = constrain(servoangle, 180, 0);
  Serial.print("Sensor Constrained: ");
  Serial.println(servoangle);
  delay(1);

  // Format structured data
  strcpy(myData.a, "Data from SENDER ESP with MPU6050 (_)");
  myData.b = servoangle;
  Serial.print("Button Value: ");
  Serial.println(myData.button);
  Serial.println("   ");
  myData.ax_val = ax;
  myData.ay_val = ay;
  myData.az_val = az;

  myData.gx_val = gx;
  myData.gy_val = gy;
  myData.gz_val = gz;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  } else {
    Serial.println("Sending error");
  }

  // Add a blank line for readability
  Serial.println("");

  delay(1);
}