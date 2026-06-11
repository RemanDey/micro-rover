import cv2
import numpy as np
import requests
import argparse
import socket
import sys
import time

# --- Configuration ---
ROVER_HOST = "micro-rover.local"
DEFAULT_SPEED = 600

# HSV Color Ranges (Default: Green)
# You may need to tune these based on your lighting and target object
COLOR_LOWER = np.array([35, 100, 100])
COLOR_UPPER = np.array([85, 255, 255])

def send_move(session, url, left, right):
    """Sends motor commands to the rover."""
    params = {'left': int(left), 'right': int(right)}
    try:
        session.get(url, params=params, timeout=0.2)
    except requests.exceptions.RequestException:
        pass

def main():
    parser = argparse.ArgumentParser(
        description="Computer Vision target tracking for the Micro-Rover using OpenCV color detection.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
COLOR CALIBRATION:
  The default tracking color is GREEN. To track a different color, 
  you must modify the COLOR_LOWER and COLOR_UPPER HSV arrays in the script.

CONTROL LOGIC:
  - Steering: Proportional (P) control based on target horizontal offset from center.
  - Distance: Estimated by contour area. 
    * Area < 5000: Rover moves forward to close the gap.
    * Area > 15000: Rover moves backward to maintain safety distance.

  Press 'q' on the video preview window to stop the tracking and exit.
"""
    )
    parser.add_argument('--speed', type=int, default=DEFAULT_SPEED, 
                        help=f"Base speed for tracking maneuvers (0-1023). Default: {DEFAULT_SPEED}")
    parser.add_argument('--cam', type=int, default=0, 
                        help="Index of the camera device to use. Default: 0")
    parser.add_argument('--no-display', action='store_true', 
                        help="Run in headless mode without the video preview window")
    args = parser.parse_args()

    # Resolve IP
    try:
        rover_ip = socket.gethostbyname(ROVER_HOST)
        base_url = f"http://{rover_ip}/move"
    except socket.gaierror:
        base_url = f"http://{ROVER_HOST}/move"

    session = requests.Session()
    cap = cv2.VideoCapture(args.cam)

    if not cap.isOpened():
        print("Error: Could not open video source.")
        sys.exit(1)

    print(f"Targeting: {base_url}")
    print("Starting Vision Tracking... Press 'q' on the video window to stop.")

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            # 1. Pre-process frame
            frame = cv2.flip(frame, 1) # Flip for mirror effect if needed
            h, w, _ = frame.shape
            center_x = w // 2
            
            # 2. Color Detection
            hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
            mask = cv2.inRange(hsv, COLOR_LOWER, COLOR_UPPER)
            mask = cv2.erode(mask, None, iterations=2)
            mask = cv2.dilate(mask, None, iterations=2)

            # 3. Find Contours
            contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            
            left_speed = 0
            right_speed = 0

            if contours:
                # Find the largest contour (the target)
                c = max(contours, key=cv2.contourArea)
                area = cv2.contourArea(c)
                
                if area > 500: # Minimum size threshold
                    ((x, y), radius) = cv2.minEnclosingCircle(c)
                    M = cv2.moments(c)
                    if M["m00"] > 0:
                        cX = int(M["m10"] / M["m00"])
                        
                        # --- Proportional Control Logic ---
                        # Calculate error (-1.0 to 1.0)
                        error = (cX - center_x) / center_x
                        
                        # Steering sensitivity (Kp)
                        steer_speed = error * args.speed
                        
                        # Forward/Backward logic based on target size
                        # area < 5000: too far (move forward)
                        # area > 15000: too close (move backward)
                        forward_speed = 0
                        if area < 5000:
                            forward_speed = args.speed * 0.6
                        elif area > 15000:
                            forward_speed = -args.speed * 0.6

                        left_speed = forward_speed + steer_speed
                        right_speed = forward_speed - steer_speed

                        # Draw indicators for UI
                        if not args.no_display:
                            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
                            cv2.circle(frame, (cX, int(y)), 5, (255, 0, 0), -1)
                            cv2.putText(frame, f"Area: {int(area)}", (10, 30), 
                                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            
            # 4. Command the Rover
            # Clamp speeds to PWM limits
            left_speed = max(-1023, min(1023, left_speed))
            right_speed = max(-1023, min(1023, right_speed))
            
            send_move(session, base_url, left_speed, right_speed)
            
            if not args.no_display:
                cv2.imshow("Rover Vision", frame)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break

    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        send_move(session, base_url, 0, 0)
        cap.release()
        cv2.destroyAllWindows()
        print("Vision system shut down.")

if __name__ == "__main__":
    main()