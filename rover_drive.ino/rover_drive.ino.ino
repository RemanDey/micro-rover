#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

// ======================
// Access Point Settings
// ======================

const char* ssid = "ROS_ROBOT";
const char* password = "12345678";

// ======================
// WebSocket Server
// ======================

WebSocketsServer webSocket = WebSocketsServer(81);

// ======================
// Motor Pins
// ======================

#define IN1 D1
#define IN2 D2
#define IN3 D6
#define IN4 D7

#define ENA D0
#define ENB D5

int motorSpeed = 700;

// ======================
// Motor Functions
// ======================

void stopBot() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void setMotor(int leftSpeed, int rightSpeed) {

  // LEFT MOTOR
  if(leftSpeed >= 0) {

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

  } else {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    leftSpeed = -leftSpeed;
  }

  // RIGHT MOTOR
  if(rightSpeed >= 0) {

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

  } else {

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    rightSpeed = -rightSpeed;
  }

  leftSpeed = constrain(leftSpeed, 0, 1023);
  rightSpeed = constrain(rightSpeed, 0, 1023);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

// ======================
// WebSocket Event
// ======================

void webSocketEvent(uint8_t num,
                    WStype_t type,
                    uint8_t * payload,
                    size_t length) {

  if(type == WStype_TEXT) {

    String data = (char*)payload;

    int commaIndex = data.indexOf(',');

    if(commaIndex > 0) {

      int leftPWM =
          data.substring(0, commaIndex).toInt();

      int rightPWM =
          data.substring(commaIndex + 1).toInt();

      setMotor(leftPWM, rightPWM);
    }
  }
}

// ======================
// Setup
// ======================

void setup() {



  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopBot();

  // ======================
  // Start Access Point
  // ======================

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();

  // Serial.println("");
  // Serial.println("Access Point Started");
  // Serial.print("IP Address: ");
  // Serial.println(IP);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

// ======================
// Main Loop
// ======================

void loop() {

  webSocket.loop();
}