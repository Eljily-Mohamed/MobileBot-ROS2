/*
 * QuadEncoder.h
 */

#ifndef INC_QUADENCODER_H_
#define INC_QUADENCODER_H_

#include "main.h"

void quadEncoder_Init(void);
int16_t quadEncoder_GetPos16L(void);
int16_t quadEncoder_GetPos16R(void);
int32_t quadEncoder_GetPos32L(void);
int32_t quadEncoder_GetPos32R(void);
int16_t quadEncoder_GetSpeedL(void);
int16_t quadEncoder_GetSpeedR(void);
void quadEncoder_CallbackIndexL(void);
void quadEncoder_CallbackIndexR(void);
void quadEncoder_PosCalcL(int*);
void quadEncoder_PosCalcR(int*);

#endif /* INC_QUADENCODER_H_ */
