import requests
import keyboard
import time

# =========================
# ESP8266 IP ADDRESS
# =========================
ESP_IP = "192.168.4.1"

# =========================
# Send Command Function
# =========================
def send_command(cmd):

    try:
        requests.get(f"http://{ESP_IP}/{cmd}", timeout=0.1)
        print("Sent:", cmd)

    except:
        print("ESP8266 not reachable")


# =========================
# Instructions
# =========================
print("\n===== TELEOP CONTROLS =====")
print("W / ↑  -> Forward")
print("S / ↓  -> Backward")
print("A / ←  -> Left")
print("D / →  -> Right")
print("SPACE  -> Stop")
print("Q      -> Quit\n")


# =========================
# Main Loop
# =========================
last_command = ""

while True:

    command = "stop"

    # =========================
    # FORWARD
    # =========================
    if (keyboard.is_pressed('w') or
        keyboard.is_pressed('up')):

        command = "forward"

    # =========================
    # BACKWARD
    # =========================
    elif (keyboard.is_pressed('s') or
          keyboard.is_pressed('down')):

        command = "backward"

    # =========================
    # LEFT
    # =========================
    elif (keyboard.is_pressed('a') or
          keyboard.is_pressed('left')):

        command = "left"

    # =========================
    # RIGHT
    # =========================
    elif (keyboard.is_pressed('d') or
          keyboard.is_pressed('right')):

        command = "right"

    # =========================
    # STOP
    # =========================
    elif keyboard.is_pressed('space'):

        command = "stop"

    # =========================
    # QUIT
    # =========================
    elif keyboard.is_pressed('q'):

        send_command("stop")
        print("Exiting...")
        break

    # =========================
    # Send only if changed
    # =========================
    if command != last_command:

        send_command(command)
        last_command = command

    time.sleep(0.05)