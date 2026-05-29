#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// --- Configuration ---
const char* WIFI_AP_SSID = "ESP_Rover";
const char* WIFI_AP_PASS = "12345678";
const char* WIFI_STA_SSID = "YOUR_HOUSE_WIFI_SSID";
const char* WIFI_STA_PASS = "YOUR_HOUSE_WIFI_PASSWORD";
const char* MDNS_NAME     = "micro-rover";

// --- Hardware Pins ---
const int PIN_IN1 = D1;
const int PIN_IN2 = D2;
const int PIN_IN3 = D6;
const int PIN_IN4 = D7;
const int PIN_ENA = D0;
const int PIN_ENB = D5;

const int MAX_PWM = 1023;

ESP8266WebServer server(80);
extern const char INDEX_HTML[] PROGMEM;

// --- Arduino Lifecycle ---

void setup() {
    Serial.begin(115200);

    // Pin Initialization
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_IN1, OUTPUT); pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT); pinMode(PIN_IN4, OUTPUT);
    pinMode(PIN_ENA, OUTPUT); pinMode(PIN_ENB, OUTPUT);

    stopMotors();

    // WiFi Setup
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);

    Serial.print("\nConnecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.printf("\nConnected! STA IP: %s, AP IP: %s\n", 
                  WiFi.localIP().toString().c_str(), 
                  WiFi.softAPIP().toString().c_str());

    if (MDNS.begin(MDNS_NAME)) {
        Serial.printf("mDNS: http://%s.local\n", MDNS_NAME);
    }

    server.on("/", handleRoot);
    server.on("/move", handleMove);
    server.begin();

    Serial.println("Web Server Started");
}

void loop() {
    server.handleClient();
    MDNS.update();
}

// --- Motor Control Helpers ---

void setMotor(int in1, int in2, int enablePin, int speedValue) {
    speedValue = constrain(speedValue, -MAX_PWM, MAX_PWM);

    if (speedValue > 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(enablePin, speedValue);
    } else if (speedValue < 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(enablePin, -speedValue);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(enablePin, 0);
    }
}

void moveRover(int leftSpeed, int rightSpeed) {
    setMotor(PIN_IN1, PIN_IN2, PIN_ENA, leftSpeed);
    setMotor(PIN_IN3, PIN_IN4, PIN_ENB, rightSpeed);
}

void stopMotors() {
    moveRover(0, 0);
}

// --- Web Server Handlers ---

void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleMove() {
    if (server.hasArg("left") && server.hasArg("right")) {
        moveRover(server.arg("left").toInt(), server.arg("right").toInt());
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

// --- HTML Content ---

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Zade - Differential Drive Rover</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { margin:0; background:#111; color:white; font-family:Arial; text-align:center; }
        h1 { margin-top:20px; }
        #joystickContainer { width:300px; height:300px; background:#222; border-radius:50%; margin:40px auto; position:relative; touch-action:none; }
        #joystick { width:100px; height:100px; background:#3498db; border-radius:50%; position:absolute; left:100px; top:100px; }
        #trajectoryCanvas { background: #000; border: 1px solid #444; margin-top: 20px; border-radius: 8px; max-width: 90vw; }
        .info { font-size:20px; margin-top:20px; }
        button { background: #3498db; color: white; border: none; padding: 10px 20px; font-size: 16px; border-radius: 5px; cursor: pointer; margin: 10px; transition: background 0.3s; }
        button:hover { background: #2980b9; }
    </style>
</head>
<body>
    <h1>Control Zade</h1>
    <div id="joystickContainer"><div id="joystick"></div></div>
    <div class="info">Drag joystick to move rover</div>
    <canvas id="trajectoryCanvas" width="400" height="400"></canvas>
    <br>
    <button onclick="resetTrajectory()">Reset Trajectory</button>
    <script>
        const joy = document.getElementById("joystick");
        const container = document.getElementById("joystickContainer");
        const canvas = document.getElementById("trajectoryCanvas");
        const ctx = canvas.getContext("2d");

        let dragging = false;
        const centerX = 150, centerY = 150, maxRadius = 100;

        let curL = 0, curR = 0;
        let roverX = canvas.width / 2;
        let roverY = canvas.height / 2;
        let roverHeading = -Math.PI / 2;
        let path = [{x: roverX, y: roverY}];

        function sendMotor(left, right) { fetch(`/move?left=${left}&right=${right}`); }
        function moveJoystick(x, y) { joy.style.left = (x - 50) + "px"; joy.style.top = (y - 50) + "px"; }

        function resetTrajectory() {
            roverX = canvas.width / 2;
            roverY = canvas.height / 2;
            roverHeading = -Math.PI / 2;
            path = [{x: roverX, y: roverY}];
        }

        function processJoystick(clientX, clientY) {
            const rect = container.getBoundingClientRect();
            let dx = clientX - rect.left - centerX;
            let dy = centerY - (clientY - rect.top);
            let distance = Math.sqrt(dx*dx + dy*dy);
            if (distance > maxRadius) { dx *= maxRadius / distance; dy *= maxRadius / distance; }
            moveJoystick(centerX + dx, centerY - dy);
            let forward = dy / maxRadius, turn = dx / maxRadius;
            let leftMotor = Math.max(-1, Math.min(1, forward + turn));
            let rightMotor = Math.max(-1, Math.min(1, forward - turn));

            curL = parseInt(leftMotor * 1023);
            curR = parseInt(rightMotor * 1023);
            sendMotor(curL, curR);
        }

        function updateTrajectory() {
            if (curL !== 0 || curR !== 0) {
                const speedScale = 0.0008;
                const turnScale = 0.000015;
                
                const v = (curL + curR) / 2 * speedScale;
                const w = (curL - curR) * turnScale;
                
                roverHeading += w;
                roverX += v * Math.cos(roverHeading);
                roverY += v * Math.sin(roverHeading);
                
                path.push({x: roverX, y: roverY});
                if (path.length > 3000) path.shift();
            }

            ctx.fillStyle = "black";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            ctx.strokeStyle = "#3498db";
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(path[0].x, path[0].y);
            for (let i = 1; i < path.length; i++) {
                ctx.lineTo(path[i].x, path[i].y);
            }
            ctx.stroke();

            // Draw Rover Pointer (Orientation)
            ctx.save();
            ctx.translate(roverX, roverY);
            ctx.rotate(roverHeading);
            ctx.fillStyle = "#ff4757"; // Red pointer for visibility
            ctx.beginPath();
            ctx.moveTo(12, 0); // Tip
            ctx.lineTo(-8, -8);
            ctx.lineTo(-8, 8);
            ctx.closePath();
            ctx.fill();
            ctx.restore();

            requestAnimationFrame(updateTrajectory);
        }

        const start = (e) => { dragging = true; processJoystick(e.clientX || e.touches[0].clientX, e.clientY || e.touches[0].clientY); };
        const move = (e) => { if (dragging) { if(e.touches) e.preventDefault(); processJoystick(e.clientX || e.touches[0].clientX, e.clientY || e.touches[0].clientY); } };
        const stop = () => { dragging = false; moveJoystick(centerX, centerY); curL = 0; curR = 0; sendMotor(0,0); };

        const keys = {};
        const updateKeyboardControl = () => {
            if (dragging) return; // Joystick takes priority
            let forward = 0, turn = 0;
            if (keys['w'] || keys['arrowup']) forward += 1;
            if (keys['s'] || keys['arrowdown']) forward -= 1;
            if (keys['a'] || keys['arrowleft']) turn -= 1;
            if (keys['d'] || keys['arrowright']) turn += 1;

            // Sync keyboard to visual joystick
            moveJoystick(centerX + turn * maxRadius, centerY - forward * maxRadius);

            let leftMotor = Math.max(-1, Math.min(1, forward + turn));
            let rightMotor = Math.max(-1, Math.min(1, forward - turn));
            
            let newL = parseInt(leftMotor * 1023);
            let newR = parseInt(rightMotor * 1023);
            
            if (newL !== curL || newR !== curR) {
                curL = newL;
                curR = newR;
                sendMotor(curL, curR);
            }
        };

        document.addEventListener("keydown", (e) => { keys[e.key.toLowerCase()] = true; updateKeyboardControl(); });
        document.addEventListener("keyup", (e) => { keys[e.key.toLowerCase()] = false; updateKeyboardControl(); });

        container.addEventListener("mousedown", start);
        document.addEventListener("mousemove", move);
        document.addEventListener("mouseup", stop);
        container.addEventListener("touchstart", start);
        container.addEventListener("touchmove", move);
        container.addEventListener("touchend", stop);

        updateTrajectory();
    </script>
</body>
</html>
)=====";