#!/usr/bin/python3
import sys
import numpy as np
import pyqtgraph as pg
from PyQt5 import QtCore
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QColor, QPixmap, QImage
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QLabel, QLineEdit,
    QPushButton, QVBoxLayout, QComboBox, QHBoxLayout, QGridLayout,
    QGraphicsScene, QGraphicsView, QSplitter
)
import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from sensor_msgs.msg import Image
import pygame
import time


class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)

        pygame.mixer.init()
        # Load the audio file
        try:
            pygame.mixer.music.load("assets/alert_me.mp3")
        except pygame.error as e:
            print(f"Error loading music file: {e}")
        # Set the initial volume (value between 0.0 and 1.0)
        pygame.mixer.music.set_volume(0.5)  # Example: 50% volume

        # Window settings
        self.setMinimumSize(800, 800)
        self.setWindowTitle("ROS2 IHM")
        self.setStyleSheet("""
            QMainWindow { background-color: #f4f4f9; }
            QLabel { font-size: 16px; }
            QLineEdit { font-size: 14px; padding: 5px; border: 1px solid #ccc; border-radius: 5px; background-color: #fff; }
            QPushButton { font-size: 12px; padding: 6px; border: none; border-radius: 5px; background-color: #0056b3; color: white; }
            QPushButton#btnUp { background-color: #4CAF50; }
            QPushButton#btnDown { background-color: #FF5733; }
            QPushButton#btnLeft { background-color: #FFC300; }
            QPushButton#btnRight { background-color: #337ab7; }
            QPushButton#btnStop { background-color: #ff0000; color: white; font-weight: bold; }
            QComboBox { font-size: 14px; padding: 5px; border: 1px solid #ccc; border-radius: 5px; background-color: #fff; }
        """)

        # Create main widget
        
        widget = QWidget()
        self.setCentralWidget(widget)
        main_layout = QVBoxLayout(widget)

        # Create top layout for controls and graphs
        top_layout = QVBoxLayout()

        # Mode selection
        self.mode_label = QLabel('Mode de Fonctionnement:', self)
        self.mode_combo = QComboBox(self)
        self.mode_combo.addItems(['Manuel', 'Aléatoire', 'Tracking'])
        self.mode_combo.currentIndexChanged.connect(self.onModeChange)
        top_layout.addWidget(self.mode_label)
        top_layout.addWidget(self.mode_combo)

        # Speed input
        self.speed_label = QLabel('Changer la vitesse (tr/min):', self)
        self.speed_input = QLineEdit(self)
        self.speed_input.setPlaceholderText('Enter new speed')
        self.speed_btn = QPushButton('Set Speed', self)
        self.speed_btn.clicked.connect(self.onSetSpeed)
        top_layout.addWidget(self.speed_label)
        top_layout.addWidget(self.speed_input)
        top_layout.addWidget(self.speed_btn)

       # Robot data
        labels_and_displays = [
            ('Vitesse de rotation (km/h):', 'rotation_speed_display'),
            ('Vitesse de déplacement gauche (tr/min):', 'left_speed_display'),
            ('Vitesse de déplacement droite (tr/min):', 'right_speed_display'),
            ('Vitesse de déplacement (m/s):', 'robot_speed_display'),
            ('Obstacle:', 'obstacle_display')
        ]

        for label_text, attr_name in labels_and_displays:
            label = QLabel(label_text, self)
            display = QLineEdit(self)
            display.setReadOnly(True)
            setattr(self, attr_name, display)  
            top_layout.addWidget(label)
            top_layout.addWidget(display)

        # Movement control
        self.movement_label = QLabel('Contrôle de Mouvement:')
        self.movement_layout = QGridLayout()
        self.btn_stop = QPushButton("STOP")
        self.btn_stop.setObjectName("btnStop")
        self.btn_up = QPushButton("↑")
        self.btn_up.setObjectName("btnUp")
        self.btn_down = QPushButton("↓")
        self.btn_down.setObjectName("btnDown")
        self.btn_left = QPushButton("←")
        self.btn_left.setObjectName("btnLeft")
        self.btn_right = QPushButton("→")
        self.btn_right.setObjectName("btnRight")
        self.btn_up.clicked.connect(lambda: self.send_movement_command("f"))
        self.btn_down.clicked.connect(lambda: self.send_movement_command("b"))
        self.btn_left.clicked.connect(lambda: self.send_movement_command("l"))
        self.btn_right.clicked.connect(lambda: self.send_movement_command("r"))
        self.btn_stop.clicked.connect(lambda: self.send_movement_command("s"))
        self.movement_layout.addWidget(self.btn_up, 0, 1)
        self.movement_layout.addWidget(self.btn_left, 1, 0)
        self.movement_layout.addWidget(self.btn_right, 1, 2)
        self.movement_layout.addWidget(self.btn_down, 2, 1)
        self.movement_layout.addWidget(self.btn_stop, 1, 1)
        top_layout.addWidget(self.movement_label)
        top_layout.addLayout(self.movement_layout)

        # Graph setup
        self.graph_widget = pg.PlotWidget()
        self.graph_widget.setBackground('w')
        self.graph_widget.setTitle("Vitesse de Déplacement", color="b", size="15pt")
        self.graph_widget.setLabel('left', 'Speed', color='red', size=30)
        self.graph_widget.setLabel('bottom', 'Time', color='red', size=30)
        self.speed_data = np.zeros(100)
        self.ptr = 0
        self.curve = self.graph_widget.plot(self.speed_data, pen=pg.mkPen(color='b', width=2))
        top_layout.addWidget(self.graph_widget)

        # Camera feed setup
        self.graphicsScene = QGraphicsScene()
        self.graphicsView = QGraphicsView()
        self.graphicsScene.setBackgroundBrush(QColor(0, 0, 0))  # Black background for camera feed
        self.graphicsView.setScene(self.graphicsScene)
        self.graphicsView.setFixedSize(320, 240)  # Set fixed size for the camera feed
        self.qpixmap = QPixmap()

        # Center the camera feed in the layout
        camera_layout = QHBoxLayout()
        camera_layout.addStretch(1)  # Add stretch to the left
        camera_layout.addWidget(self.graphicsView)
        camera_layout.addStretch(1)  # Add stretch to the right
        top_layout.addLayout(camera_layout)

        # Add top layout to main layout
        main_layout.addLayout(top_layout)

        # Main widget to hold everything
        main_widget = QWidget()
        main_widget.setLayout(main_layout)
        self.setCentralWidget(main_widget)

        # Window settings
        self.setWindowTitle("Camera & Controls")
        self.setGeometry(100, 100, 640, 480)  # Set window size and position

        # ROS2 setup
        rclpy.init(args=None)
        self.node = Node('py_ihm_node')
        self.publisher_mode = self.node.create_publisher(String, '/command/mode', 10)
        self.publisher_speed_movement = self.node.create_publisher(String, '/command/move', 10)
        self.subscription_speed = self.node.create_subscription(String, '/sensor/motor_speed', self.movement_speed_callback, 10)
        self.subscription_obstacle = self.node.create_subscription(String, '/sensor/recive_obstacl', self.obstacle_callback, 10)
        self.subscription_camera = self.node.create_subscription(Image, '/camera/src_frame', self.img_callback, 10)
        self.timer = QTimer()
        self.timer.timeout.connect(self.onTimerTick)
        self.timer.start(100)

    def img_callback(self, img):
        image = QImage(img.data, img.width, img.height, QImage.Format_RGB888)
        self.graphicsScene.clear()
        self.graphicsScene.addPixmap(self.qpixmap.fromImage(image))
    def onModeChange(self):
        mode = self.mode_combo.currentText()
        mode_map = {
            'Manuel': '0',
            'Aléatoire': '1',
            'Tracking': '2'
        }
        mode_value = mode_map.get(mode, '-1') # Default to '-1' if not found
        msg = String()  
        msg.data = mode_value
        self.publisher_mode.publish(msg)
        print(f'Publishing Mode: "{msg.data}"')

    def onSetSpeed(self):
        speed = self.speed_input.text()
        if speed:
            msg = String()
            msg.data = f'v{speed}'
            self.publisher_speed_movement.publish(msg)
            print(f'Publishing Speed: "{msg.data}"')

    def send_movement_command(self, direction):
        speed = self.speed_input.text() or "100"  # Default speed is 100 if not entered
        if direction == "f":
            msg = String()
            msg.data = f'f{speed}'  # Forward with speed
            self.publisher_speed_movement.publish(msg)
            print(f'Publishing Movement: "{msg.data}"')
        elif direction == "b":
            msg = String()
            msg.data = f'b{speed}'  # Backward with speed
            self.publisher_speed_movement.publish(msg)
            print(f'Publishing Movement: "{msg.data}"')
        elif direction == "l":
            msg = String()
            msg.data = f'l{speed}'  # Left with speed
            self.publisher_speed_movement.publish(msg)
            print(f'Publishing Movement: "{msg.data}"')
        elif direction == "r":
            msg = String()
            msg.data = f'r{speed}'  # Right with speed
            self.publisher_speed_movement.publish(msg)
            print(f'Publishing Movement: "{msg.data}"')
        elif direction == "s":
            msg = String()
            msg.data = f's'
            self.publisher_speed_movement.publish(msg)
            print(f'Publishing Movement: "{msg.data}"')


    def movement_speed_callback(self, msg):
        # Diamètre de la roue en m
        D = 0.098  #
        # Extraire les vitesses gauche et droite du message reçu
        message_text = msg.data
        print(msg.data)
        try:
            left_speed_str = message_text.split("speedL=#")[1].split(",")[0]
            right_speed_str = message_text.split("speedR=#")[1]
            
            # Conversion en tr/min
            left_speed_rpm = float(left_speed_str.strip())
            right_speed_rpm = float(right_speed_str.strip())

            # Calcul de la vitesse moyenne en tr/min
            average_speed_rpm = (left_speed_rpm + right_speed_rpm) / 2

            # Calcul de la vitesse moyenne en km/h
            average_speed_kmh = average_speed_rpm * np.pi * D / 1000 * 60
            average_speed_ms = average_speed_kmh / 3.6

            # Mise à jour des champs GUI
            self.left_speed_display.setText(f'{left_speed_rpm:.2f} tr/min')
            self.right_speed_display.setText(f'{right_speed_rpm:.2f} tr/min')
            self.rotation_speed_display.setText(f'{average_speed_kmh:.2f} km/h')
            self.robot_speed_display.setText(f'{average_speed_ms:.2f} m/s')


            # Mise à jour du graphique
            self.update_graph(average_speed_kmh)
            
        except (IndexError, ValueError) as e:
            print(f"Error parsing speed data: {e}")


    def obstacle_callback(self, msg):
        # Parse the obstacle data from the message
        try:
            obstacle_code = int(msg.data.strip("#"))  # Assuming the format is "#<code>"
        except ValueError:
            self.obstacle_display.setText("Invalid message format")  # Update display for invalid data
            return

        # Update the GUI and play the corresponding sound
        if obstacle_code == 1:
            self.obstacle_display.setText("Obstacle detected: Front")
        elif obstacle_code == 2:
            self.obstacle_display.setText("Obstacle detected: Back")
        elif obstacle_code == 3:
            self.obstacle_display.setText("Obstacle detected: Front and Back")
            # pygame.mixer.music.play()
        else:
            self.obstacle_display.setText("No obstacle")

    def update_graph(self, speed):
        self.speed_data[:-1] = self.speed_data[1:]
        self.speed_data[-1] = speed
        self.ptr += 1
        if self.ptr >= len(self.speed_data):
            self.ptr = 0
        self.curve.setData(self.speed_data)

    def onTimerTick(self):
        rclpy.spin_once(self.node, timeout_sec=0.1)

def main(args=None):
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
