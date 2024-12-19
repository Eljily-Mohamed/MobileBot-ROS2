/*
 * robot_control.h
 *
 *      Author: eljily
 */

#ifndef INC_ROBOT_CONTROL_H_
#define INC_ROBOT_CONTROL_H_

#include "motorCommand.h"
#include "quadEncoder.h"
#include "captDistIR.h"
#include "VL53L0X.h"

// Enumération pour définir les états du robot
typedef enum {
    MOVING_FORWARD,
    MOVING_BACKWARD,
    TURNING_LEFT,
    TURNING_RIGHT,
    STOPPED
} RobotState;

void move_forward(int id,int consigne);
void move_backward(int id,int consigne);
void move_left(int id,int consigne);
void move_right(int id,int consigne);
void stop_robot(int id);
int detect_obstacle_forward();
int detect_obstacle_backward();

#endif /* INC_ROBOT_CONTROL_H_ */
