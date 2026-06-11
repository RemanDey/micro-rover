import requests
import argparse
import socket
import time
import sys

# --- Configuration ---
ROVER_HOST = "micro-rover.local"
DEFAULT_SPEED = 800
TURN_DURATION = 0.5  # Approximate time for a 90-degree turn at DEFAULT_SPEED

def send_move(session, url, left, right, label="Moving"):
    """
    Sends the motor speed values to the rover API.
    """
    params = {'left': left, 'right': right}
    try:
        response = session.get(url, params=params, timeout=0.5)
        if response.status_code == 200:
            print(f"\r[SHAPE] {label:15} | L: {left:4} | R: {right:4}", end="")
    except requests.exceptions.RequestException as e:
        print(f"\n[!] Connection Error: {e}")

def execute_path(session, url, steps):
    """Loops through a list of (left, right, duration, label) tuples."""
    for left, right, duration, label in steps:
        send_move(session, url, left, right, label)
        time.sleep(duration)
    # Always stop after a sequence
    send_move(session, url, 0, 0, "Stop")

def main():
    parser = argparse.ArgumentParser(
        description="Autonomous geometric path tracing using timed open-loop control.",
        epilog="Note: Accuracy depends on battery levels and surface friction. "
               "Adjust TURN_DURATION in the script to calibrate 90-degree turns."
    )
    parser.add_argument('--shape', type=str, required=True, 
                        choices=['square', 'circle', 'heart', 'triangle'],
                        help="The geometric figure for the rover to execute.")
    parser.add_argument('--speed', type=int, default=DEFAULT_SPEED, 
                        help=f"Base speed (0-1023). Default: {DEFAULT_SPEED}")
    parser.add_argument('--size', type=float, default=1.0,
                        help="Multiplies the duration of travel segments to scale the "
                             "overall size of the shape. Default: 1.0")
    args = parser.parse_args()

    # Resolve IP once
    try:
        rover_ip = socket.gethostbyname(ROVER_HOST)
        target = f"http://{rover_ip}/move"
    except socket.gaierror:
        target = f"http://{ROVER_HOST}/move"

    session = requests.Session()
    speed = max(0, min(1023, args.speed))
    s = args.size # Shorthand for scaling

    print(f"Tracing {args.shape.upper()} at speed {speed}...")

    path = []

    if args.shape == 'square':
        for i in range(4):
            path.append((speed, speed, 1.0 * s, f"Side {i+1}"))
            path.append((0, 0, 0.2, "Pause"))
            path.append((speed, -speed, TURN_DURATION, "Turn"))
            path.append((0, 0, 0.2, "Pause"))

    elif args.shape == 'triangle':
        for i in range(3):
            path.append((speed, speed, 1.2 * s, f"Side {i+1}"))
            path.append((0, 0, 0.2, "Pause"))
            # Approximately 120 degree turn
            path.append((speed, -speed, TURN_DURATION * 1.33, "Turn"))
            path.append((0, 0, 0.2, "Pause"))

    elif args.shape == 'circle':
        # One continuous arc: outer wheel faster than inner wheel
        path.append((speed, int(speed * 0.5), 6.0 * s, "Circular Arc"))

    elif args.shape == 'heart':
        # A heart is essentially two arcs and two diagonals
        # 1. Diagonal Up-Right
        path.append((speed, speed, 0.8 * s, "Diagonal Up"))
        path.append((0, 0, 0.1, "P"))
        # 2. Right Hump (Arc)
        path.append((speed, int(speed * 0.2), 1.5 * s, "Right Hump"))
        path.append((0, 0, 0.1, "P"))
        # 3. Quick rotation to transition humps
        path.append((speed, -speed, TURN_DURATION * 0.5, "Center Turn"))
        # 4. Left Hump (Arc)
        path.append((int(speed * 0.2), speed, 1.5 * s, "Left Hump"))
        path.append((0, 0, 0.1, "P"))
        # 5. Diagonal Down-Right back to tip
        path.append((speed, speed, 0.8 * s, "Diagonal Down"))

    try:
        execute_path(session, target, path)
        print("\n\nShape complete!")
    except KeyboardInterrupt:
        send_move(session, target, 0, 0, "Emergency Stop")
        print("\nInterrupted.")

if __name__ == "__main__":
    main()