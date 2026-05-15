import requests
import time

# ==========================
# PHYPhox URL
# ==========================
PHY_URL = "http://10.37.109.106:8080/get?accX&accY"

# ==========================
# ESP8266 URL
# ==========================
ESP_IP = "192.168.4.1"

# ==========================
# Send command to rover
# ==========================
def send_command(cmd):
    try:
        requests.get(f"http://{ESP_IP}/{cmd}", timeout=0.2)
        print("CMD:", cmd)
    except:
        print("ESP not reachable")

# ==========================
# Main loop
# ==========================
last_command = ""

while True:

    try:
        # Read IMU
        r = requests.get(PHY_URL)
        data = r.json()

        ax = data["buffer"]["accX"]["buffer"][0]
        ay = data["buffer"]["accY"]["buffer"][0]

        print("AX:", ax, " AY:", ay)

        # ==========================
        # Motion logic
        # ==========================

        command = "stop"

        # Phone tilted forward
        if ay > 2:
            command = "forward"

        # Phone tilted backward
        elif ay < -2:
            command = "backward"

        # Tilt left
        elif ax > 2:
            command = "left"

        # Tilt right
        elif ax < -2:
            command = "right"

        # Send only if changed
        if command != last_command:
            send_command(command)
            last_command = command

    except Exception as e:
        print(e)

    time.sleep(0.1)