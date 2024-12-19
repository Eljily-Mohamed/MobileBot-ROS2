import random
import time
import threading

import rclpy # ROS library
from rclpy.node import Node

import std_msgs.msg
from std_msgs.msg import String
import sensor_msgs.msg

from cv_bridge import CvBridge # To convert to open cv image

import cv2
import numpy as np


class CameraPublisher(Node):

    def __init__(self):
        super().__init__('RPI_Node')

        self.br = CvBridge()

        self.frame_rate = 50  # Max framerate
        self.mode_track_active = False
        self.publisher_src_frame = self.create_publisher(sensor_msgs.msg.Image, 'camera/src_frame', 10) # Raw image

        self.subscription_mode = self.create_subscription(String, '/command/mode',self.get_mode,10)

        self.lock_color = threading.Lock() # For safe sharing of the color config variables between the main loop and the callback.
        self.enable = threading.Condition() # To make the main loop wait passively for the correct mode.
        self.lock_mode = threading.Lock() # For safe sharing of the self.mode variable between the main loop and the callback.
        self.mode = 0
        self.i = 0 # Count iterations in the main loop

        self.cam = cv2.VideoCapture(0)  # Initialise the video device
        self.cam.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
        self.cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)
        self.publisher_src_xy = self.create_publisher(String, 'camera/src_xy', 10)

    def get_mode(self, msg):
        mode = int(msg.data)
        if mode == 2:
            self.mode_track_active = True
        else:
            self.mode_track_active = False
            
    def loop(self):
        while rclpy.ok():
            t0 = time.perf_counter()

            self.i += 1

            # Image capture
            succes, frame = self.cam.read()
            if not succes:
                print("failed to grab frame")
                return
            if self.mode_track_active:
                self.image_traitement(frame)

            if self.i % 10 == 0:
                self.publisher_src_frame.publish(self.br.cv2_to_imgmsg(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), encoding="rgb8"))

            delta_time = time.perf_counter()-t0 # Time to run entire loop
            time_sleep = 1/self.frame_rate - delta_time # Time left for chosen framerate
            if time_sleep > 0:
                time.sleep(time_sleep)

            # Exit the loop if 'q' is pressed
            if cv2.waitKey(1) == ord('q'):
                break

        self.cam.release()
        cv2.destroyAllWindows()

    def image_traitement(self, frame):
        # Get the frame dimensions
        width = int(self.cam.get(3))
        height = int(self.cam.get(4))

        # Convert the frame to HSV color space
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # Define the color range for detecting blue in HSV space
        lower_blue = np.array([100, 150, 50])  # Lower bound for blue
        upper_blue = np.array([140, 255, 255])  # Upper bound for blue

        # Create binary masks for detecting blue color
        mask = cv2.inRange(hsv, lower_blue, upper_blue)

        # Calculate moments to get the barycenter of the detected blue color
        moments = cv2.moments(mask)
        
        coordinates = String()
        if moments["m00"] >= 15000:  # If the mask has detected blue
            # Calculate the center of mass (barycenter)
            cx = int(moments["m10"] / moments["m00"])
            cy = int(moments["m01"] / moments["m00"])
            coordinates.data = f'{cx},{cy}'
            # Draw the barycenter (centroid) as a blue dot on the frame
            cv2.circle(frame, (cx, cy), 5, (255, 0, 0), -1)  # Blue color
            cv2.putText(frame, f"Barycenter: ({cx}, {cy})", (cx + 10, cy),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 0, 0), 1)  # Smaller font size and blue color
        else:
            # Display message if no blue color is detected
            cv2.putText(frame, "Blue color not detected", (50, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 0, 0), 1)  # Smaller font size and blue color
            coordinates.data = "0,0"

        self.publisher_src_xy.publish(coordinates)

        # Display the resulting frames
        # cv2.imshow('frame', frame)  # Show the original frame with the barycenter or message
        # cv2.imshow('mask', mask)    # Show the binary mask (for debugging)


def main(args=None):
    rclpy.init(args=args)

    camera_publisher = CameraPublisher()
    # Starts the loop in a thread because rclpy.spin is blocking (never comes back) and needs to be called for the callbacks subscriptions to work.
    threading.Thread(target=camera_publisher.loop).start()
    rclpy.spin(camera_publisher)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically when the garbage collector destroys the node object)
    camera_publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()

