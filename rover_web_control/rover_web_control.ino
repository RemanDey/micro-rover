#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ================== WIFI AP ==================
const char* ssid = "ESP_Rover";
const char* password = "12345678";

ESP8266WebServer server(80);

// ================== MOTOR PINS ==================
#define IN1 D1
#define IN2 D2
#define IN3 D6
#define IN4 D7

#define ENA D0
#define ENB D5

// ================== HTML PAGE ==================
String webpage = R"=====(

<!DOCTYPE html>
<html>
<head>
<title>ESP8266 Rover</title>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>
body{
    font-family: Arial;
    text-align:center;
    background:#111;
    color:white;
}

button{
    width:120px;
    height:60px;
    font-size:20px;
    margin:10px;
    border:none;
    border-radius:10px;
}

.forward{background:#2ecc71;}
.backward{background:#e74c3c;}
.left{background:#3498db;}
.right{background:#3498db;}
.stop{background:#f1c40f;}
</style>
</head>

<body>

<h1>ESP8266 2WD Rover</h1>

<div>
    <button class="forward"
    ontouchstart="send('forward')"
    ontouchend="send('stop')"
    onmousedown="send('forward')"
    onmouseup="send('stop')">
    Forward
    </button>
</div>

<div>
    <button class="left"
    ontouchstart="send('left')"
    ontouchend="send('stop')"
    onmousedown="send('left')"
    onmouseup="send('stop')">
    Left
    </button>

    <button class="stop"
    onclick="send('stop')">
    Stop
    </button>

    <button class="right"
    ontouchstart="send('right')"
    ontouchend="send('stop')"
    onmousedown="send('right')"
    onmouseup="send('stop')">
    Right
    </button>
</div>

<div>
    <button class="backward"
    ontouchstart="send('backward')"
    ontouchend="send('stop')"
    onmousedown="send('backward')"
    onmouseup="send('stop')">
    Backward
    </button>
</div>

<script>
function send(cmd)
{
    fetch("/" + cmd);
}
</script>

</body>
</html>

)=====";

// ================== MOTOR FUNCTIONS ==================

void stopMotors()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

void moveForward()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    analogWrite(ENA, 1023);
    analogWrite(ENB, 1023);
}

void moveBackward()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    analogWrite(ENA, 1023);
    analogWrite(ENB, 1023);
}

void turnLeft()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    analogWrite(ENA, 200);
    analogWrite(ENB, 200);
}

void turnRight()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    analogWrite(ENA, 200);
    analogWrite(ENB, 200);
}

// ================== HANDLERS ==================

void handleRoot()
{
    server.send(200, "text/html", webpage);
}

void setup()
{
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    stopMotors();

    // Start Access Point
    WiFi.softAP(ssid, password);


    // Web Routes
    server.on("/", handleRoot);

    server.on("/forward", [](){
        moveForward();
        server.send(200, "text/plain", "Forward");
    });

    server.on("/backward", [](){
        moveBackward();
        server.send(200, "text/plain", "Backward");
    });

    server.on("/left", [](){
        turnLeft();
        server.send(200, "text/plain", "Left");
    });

    server.on("/right", [](){
        turnRight();
        server.send(200, "text/plain", "Right");
    });

    server.on("/stop", [](){
        stopMotors();
        server.send(200, "text/plain", "Stop");
    });

    server.begin();

 
}

void loop()
{
    server.handleClient();
}