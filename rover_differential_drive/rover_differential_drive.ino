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

// ================== MOTOR SETTINGS ==================
#define MAX_SPEED 1023

// ================== HTML PAGE ==================
String webpage = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>ESP8266 Differential Drive Rover</title>

<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body{
    margin:0;
    background:#111;
    color:white;
    font-family:Arial;
    text-align:center;
}

h1{
    margin-top:20px;
}

#joystickContainer{
    width:300px;
    height:300px;
    background:#222;
    border-radius:50%;
    margin:40px auto;
    position:relative;
    touch-action:none;
}

#joystick{
    width:100px;
    height:100px;
    background:#3498db;
    border-radius:50%;
    position:absolute;
    left:100px;
    top:100px;
}

.info{
    font-size:20px;
    margin-top:20px;
}

</style>
</head>

<body>

<h1>ESP8266 Differential Rover</h1>

<div id="joystickContainer">
    <div id="joystick"></div>
</div>

<div class="info">
    Drag joystick to move rover
</div>

<script>

const joy = document.getElementById("joystick");
const container = document.getElementById("joystickContainer");

let dragging = false;

const centerX = 150;
const centerY = 150;
const maxRadius = 100;

function sendMotor(left, right)
{
    fetch(`/move?left=${left}&right=${right}`);
}

function moveJoystick(x, y)
{
    joy.style.left = (x - 50) + "px";
    joy.style.top  = (y - 50) + "px";
}

function processJoystick(clientX, clientY)
{
    const rect = container.getBoundingClientRect();

    let x = clientX - rect.left;
    let y = clientY - rect.top;

    let dx = x - centerX;
    let dy = centerY - y;

    // Limit joystick movement
    let distance = Math.sqrt(dx*dx + dy*dy);

    if(distance > maxRadius)
    {
        dx = dx * maxRadius / distance;
        dy = dy * maxRadius / distance;
    }

    moveJoystick(centerX + dx, centerY - dy);

    // Normalize
    let forward = dy / maxRadius;
    let turn = dx / maxRadius;

    // Differential drive mixing
    let leftMotor  = forward + turn;
    let rightMotor = forward - turn;

    // Clamp
    leftMotor = Math.max(-1, Math.min(1, leftMotor));
    rightMotor = Math.max(-1, Math.min(1, rightMotor));

    // Convert to PWM
    leftMotor = parseInt(leftMotor * 1023);
    rightMotor = parseInt(rightMotor * 1023);

    sendMotor(leftMotor, rightMotor);
}

// ================= MOUSE =================

container.addEventListener("mousedown", (e)=>{
    dragging = true;
    processJoystick(e.clientX, e.clientY);
});

document.addEventListener("mousemove", (e)=>{
    if(dragging)
    {
        processJoystick(e.clientX, e.clientY);
    }
});

document.addEventListener("mouseup", ()=>{
    dragging = false;

    moveJoystick(centerX, centerY);

    sendMotor(0,0);
});

// ================= TOUCH =================

container.addEventListener("touchstart", (e)=>{
    dragging = true;

    const touch = e.touches[0];

    processJoystick(touch.clientX, touch.clientY);
});

container.addEventListener("touchmove", (e)=>{
    e.preventDefault();

    if(dragging)
    {
        const touch = e.touches[0];

        processJoystick(touch.clientX, touch.clientY);
    }
});

container.addEventListener("touchend", ()=>{
    dragging = false;

    moveJoystick(centerX, centerY);

    sendMotor(0,0);
});

</script>

</body>
</html>
)=====";


// ================== MOTOR CONTROL ==================

void setMotor(
    int in1,
    int in2,
    int enablePin,
    int speedValue
)
{
    speedValue = constrain(speedValue, -1023, 1023);

    if(speedValue > 0)
    {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);

        analogWrite(enablePin, speedValue);
    }
    else if(speedValue < 0)
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);

        analogWrite(enablePin, -speedValue);
    }
    else
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);

        analogWrite(enablePin, 0);
    }
}

void moveRover(int leftSpeed, int rightSpeed)
{
    // Left Motor
    setMotor(IN1, IN2, ENA, leftSpeed);

    // Right Motor
    setMotor(IN3, IN4, ENB, rightSpeed);
}

void stopMotors()
{
    moveRover(0,0);
}

// ================== WEB HANDLERS ==================

void handleRoot()
{
    server.send(200, "text/html", webpage);
}

void handleMove()
{
    if(server.hasArg("left") && server.hasArg("right"))
    {
        int leftSpeed = server.arg("left").toInt();
        int rightSpeed = server.arg("right").toInt();

        moveRover(leftSpeed, rightSpeed);

        server.send(200, "text/plain", "OK");
    }
    else
    {
        server.send(400, "text/plain", "Bad Request");
    }
}

// ================== SETUP ==================

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    stopMotors();

    // ================== START ACCESS POINT ==================
    WiFi.softAP(ssid, password);

    Serial.println();
    Serial.println("Access Point Started");

    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // ================== WEB ROUTES ==================
    server.on("/", handleRoot);

    server.on("/move", handleMove);

    server.begin();

    Serial.println("Web Server Started");
}

// ================== LOOP ==================

void loop()
{
    server.handleClient();
}