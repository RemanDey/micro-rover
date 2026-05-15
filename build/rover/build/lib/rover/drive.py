import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

import asyncio
import websockets

ESP_IP = "192.168.4.1"

MAX_PWM = 1023

class TeleopBridge(Node):

    def __init__(self):

        super().__init__('teleop_bridge')

        self.subscription = self.create_subscription(
            Twist,
            '/cmd_vel',
            self.callback,
            10
        )

    async def send(self, data):

        uri = f"ws://{ESP_IP}:81"

        async with websockets.connect(uri) as websocket:

            await websocket.send(data)

    def callback(self, msg):

        linear = msg.linear.x
        angular = msg.angular.z

        # Differential drive
        left = linear - angular
        right = linear + angular

        # Normalize
        left_pwm = int(max(min(left * MAX_PWM, 1023), -1023))
        right_pwm = int(max(min(right * MAX_PWM, 1023), -1023))

        data = f"{left_pwm},{right_pwm}"

        asyncio.run(self.send(data))

def main(args=None):

    rclpy.init(args=args)

    node = TeleopBridge()

    rclpy.spin(node)

    node.destroy_node()

    rclpy.shutdown()

if __name__ == '__main__':
    main()