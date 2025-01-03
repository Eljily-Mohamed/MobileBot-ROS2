#include "robot_control.h"

RobotState robot_state = STOPPED;

void move_forward(int id, int consigne) {
	int dutyBoth = consigne + 100;
    if (id == 2) {
        motorLeft_SetDuty(dutyBoth);
    } else if (id == 1) {
        motorRight_SetDuty(dutyBoth);
    }
    robot_state = MOVING_FORWARD;
}

void move_backward(int id, int consigne) {
	int dutyBoth = -consigne + 100;
    if (id == 2) {
        motorLeft_SetDuty(dutyBoth);
    } else if (id == 1) {
        motorRight_SetDuty(dutyBoth);
    }
    robot_state = MOVING_BACKWARD;
}

void move_left(int id, int consigne) {
    int dutyLeft = -consigne + 100;
    int dutyRight = consigne + 100;
    if (id == 2) {
        motorLeft_SetDuty(dutyLeft);
    } else if (id == 1) {
        motorRight_SetDuty(dutyRight);
    }
    robot_state = TURNING_LEFT;
}

void move_right(int id, int consigne) {
	  int dutyLeft = consigne + 100;
	  int dutyRight= -consigne + 100;

    if (id == 2) {
        motorLeft_SetDuty(dutyLeft);
    } else if (id == 1) {
        motorRight_SetDuty(dutyRight);
    }
    robot_state = TURNING_RIGHT;
}

void stop_robot(int id) {
    int dutyBoth = 100;
    if (id == 2) {
        motorLeft_SetDuty(dutyBoth);
    } else if (id == 1) {
        motorRight_SetDuty(dutyBoth);
    }
    robot_state = STOPPED;
}

// Returns 0 if obstacle detected on left, 1 for right, -1 if no obstacle
int detect_obstacle_forward() {
    int tab[2];
    int threshold = 2000;
    //tab[0] = left
    //tab[1] = droit
    captDistIR_Get(tab);
    if (tab) {
        for (int i = 0; i < 2; i++) {
            if (tab[i] > threshold) {
                return i;
            }
        }
    }
    return -1;
}


int detect_obstacle_backward() {
    uint16_t range = VL53L0X_readRangeContinuousMillimeters();
    uint16_t threshold = 100;
    if (range > 0 && range < threshold) {
        return 1;
    }
    return 0;
}


