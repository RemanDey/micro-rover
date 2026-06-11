import requests
import argparse
import socket
import sys
import tty
import termios

# --- Configuration ---
# Use "micro-rover.local" or the specific IP address found in the Serial Monitor
ROVER_HOST = "micro-rover.local"
INITIAL_SPEED = 800  # Default speed between 0 and 1023
SPEED_STEP = 50      # Amount to increase/decrease speed per keypress

def get_key():
    """
    Captures a single keypress from the terminal in raw mode.
    """
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def send_move(session, url, left, right):
    """
    Sends the motor speed values to the rover API.
    """
    params = {'left': left, 'right': right}
    try:
        # Short timeout to ensure the script remains responsive
        response = session.get(url, params=params, timeout=0.5)
        if response.status_code == 200:
            print(f"\rCommand Sent: Left={left:4} | Right={right:4}", end="")
    except requests.exceptions.RequestException as e:
        print(f"\n[!] Connection Error: {e}")

def main():
    parser = argparse.ArgumentParser(
        description="Interactive keyboard teleoperation for the Micro-Rover using persistent HTTP sessions.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
CONTROLS:
  W / Arrow Up    : Forward
  S / Arrow Down  : Backward
  A / Arrow Left  : Spin Left
  D / Arrow Right : Spin Right
  [ / ]           : Decrease / Increase Speed Step
  Space           : Emergency STOP
  Q               : Quit Program""")
    parser.add_argument('--speed', type=int, default=INITIAL_SPEED, 
                        help=f"Initial speed (0-1023). Default: {INITIAL_SPEED}")
    parser.add_argument('--step', type=int, default=SPEED_STEP,
                        help=f"Speed increment/decrement step. Default: {SPEED_STEP}")
    args = parser.parse_args()

    print("========================================")
    print("   Micro-Rover Keyboard Teleoperation   ")
    print("========================================")

    # Resolve hostname to IP once to optimize repetitive calling
    try:
        rover_ip = socket.gethostbyname(ROVER_HOST)
        target_display = f"http://{rover_ip} (resolved from {ROVER_HOST})"
    except socket.gaierror:
        rover_ip = ROVER_HOST
        target_display = f"http://{ROVER_HOST}"

    print(f"Target: {target_display}")
    print("\nCONTROLS:")
    print("  W / ↑ : Forward")
    print("  S / ↓ : Backward")
    print("  A / ← : Turn Left")
    print("  D / → : Turn Right")
    print("  [ / ] : Decrease / Increase Speed")
    print("  Space : STOP")
    print("  Q     : Quit")
    print("----------------------------------------")

    base_url = f"http://{rover_ip}/move"
    session = requests.Session()
    current_speed = max(0, min(1023, args.speed))

    try:
        while True:
            key = get_key().lower()

            if key == '[':
                current_speed = max(0, current_speed - args.step)
                print(f"\r[!] Speed decreased to: {current_speed:4}                  ", end="")
                continue
            elif key == ']':
                current_speed = min(1023, current_speed + args.step)
                print(f"\r[!] Speed increased to: {current_speed:4}                  ", end="")
                continue

            if key == 'w':
                send_move(session, base_url, current_speed, current_speed)
            elif key == 's':
                send_move(session, base_url, -current_speed, -current_speed)
            elif key == 'a':
                send_move(session, base_url, -current_speed, current_speed)
            elif key == 'd':
                send_move(session, base_url, current_speed, -current_speed)
            elif key == ' ':
                send_move(session, base_url, 0, 0)
            elif key == 'q':
                print("\n\nExiting and stopping rover...")
                send_move(session, base_url, 0, 0)
                break
            
    except KeyboardInterrupt:
        send_move(session, base_url, 0, 0)
        print("\nInterrupted by user.")

if __name__ == "__main__":
    # Ensure requests is installed: pip install requests
    main()