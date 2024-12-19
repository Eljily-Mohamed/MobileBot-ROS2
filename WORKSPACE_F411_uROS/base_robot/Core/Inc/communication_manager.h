/*
 * communication_manager.h
 *
 *  Created on: Apr 29, 2024
 *      Author: eljily
 */

#include <stdint.h>
#include <stddef.h>

#ifndef INC_COMMUNICATION_MANAGER_H_
#define INC_COMMUNICATION_MANAGER_H_

//return la 1ere elemente de buffer
char process_command_data(char* buffer);
//return a vitess comme entier
int process_vitess_data(char* buffer);
void get_vitess(char c);


#endif /* INC_COMMUNICATION_MANAGER_H_ */
