rosdep install -i --from-path src --rosdistro humble -y  
colcon build --packages-select py_ihm  
. install/setup.bash  
ros2 run py_ihm ihm_ros2

# test topic
ros2 topic echo /sensor/motor_speed
ros2 topic echo /sensor/recive_obstacl
ros2 topic echo /camera/src_xy

openocd -s /share/openocd/scripts -f board/st_nucleo_f4.cfg -f flash_board.cfg -c "myFlash base_robot.elf"
#ssh
rm -rv ~/.ssh/
#wifi
ssh rpi@192.168.50.85
#local
ssh rpi@192.168.0.4
# Login RPI : rpi
# Mot de Passe : robotrose021

# #Côté RPI, il est nécéssaire de lancer micro_ros_agent :  
cd ~/microros_ws
. install/setup.bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 115200

#camera
cd ~/Desktop/py_send_camera_ws
. install/setup.bash
ros2 run py_send_camera send_camera

#motor info
# D = 9.8 cm == 98 mm 

#vitese km/h = vitese tr/min * pi * D/1000 * 60

