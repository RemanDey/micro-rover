import requests
import argparse
import socket
import time
import sys

# --- Configuration ---
ROVER_HOST = "micro-rover.local"
DEFAULT_SPEED = 800

def send_move(session, url, left, right, label="Moving"):
    """
    Sends the motor speed values to the rover API.
    """
    params = {'left': left, 'right': right}
    try:
        response = session.get(url, params=params, timeout=0.5)
        if response.status_code == 200:
            print(f"\r[ACTION] {label:15} | Left: {left:4} | Right: {right:4}", end="")
    except requests.exceptions.RequestException as e:
        print(f"\n[!] Connection Error during '{label}': {e}")

def main():
    parser = argparse.ArgumentParser(
        description="Executes a pre-defined autonomous dance routine on the Micro-Rover.",
        epilog="The routine includes: Forward Surge, Backward Retreat, Spin Right/Left, "
               "and Wide Arcs followed by a Final Bow."
    )
    parser.add_argument('--speed', type=int, default=DEFAULT_SPEED, 
                        help=f"Base speed (0-1023). Default: {DEFAULT_SPEED}")
    parser.add_argument('--repeat', type=int, default=1,
                        help="Number of times to repeat the dance. Default: 1")
    args = parser.parse_args()

    print("========================================")
    print("   Micro-Rover Autonomous Dance Bot    ")
    print("========================================")

    # Resolve IP
    try:
        rover_ip = socket.gethostbyname(ROVER_HOST)
        print(f"Target: http://{rover_ip} ({ROVER_HOST})")
    except socket.gaierror:
        rover_ip = ROVER_HOST
        print(f"Target: http://{ROVER_HOST} (mDNS)")

    base_url = f"http://{rover_ip}/move"
    session = requests.Session()
    speed = max(0, min(1023, args.speed))

    # Define the dance steps: (left_speed, right_speed, duration_seconds, label)
    dance_steps = [
        (speed, speed, 1.0, "Forward Surge"),
        (0, 0, 0.2, "Pause"),
        (-speed, -speed, 1.0, "Backward Retreat"),
        (0, 0, 0.2, "Pause"),
        (speed, -speed, 0.8, "Spin Right"),
        (-speed, speed, 0.8, "Spin Left"),
        (speed, speed // 2, 1.5, "Wide Right Arc"),
        (speed // 2, speed, 1.5, "Wide Left Arc"),
        (0, 0, 0.5, "Final Bow")
    ]

    try:
        for i in range(args.repeat):
            print(f"\n--- Starting Iteration {i+1}/{args.repeat} ---")
            for left, right, duration, label in dance_steps:
                send_move(session, base_url, left, right, label)
                time.sleep(duration)
        
        # Ensure full stop at the end
        send_move(session, base_url, 0, 0, "Finished")
        print("\n\nDance sequence completed successfully.")

    except KeyboardInterrupt:
        print("\n\n[!] Sequence interrupted. Sending STOP command...")
        send_move(session, base_url, 0, 0, "Emergency Stop")
        sys.exit(0)

if __name__ == "__main__":
    main()