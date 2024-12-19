/*
 * MotorCommand.c
 */

#include "motorCommand.h"

static TIM_HandleTypeDef    TimHandle;
static TIM_OC_InitTypeDef   sConfigOC;

//=================================================================
//			PWM INIT
// TIMER 3 (PWM)  : CH1 et CH2
// ENABLE : Sortie Logique (GPIO) PA7
//=================================================================

void motorCommand_Init(void)
{
	unsigned int uwPrescalerValue = 0;

	/* Compute the prescaler value to have TIM4 counter clock equal to 10MHz */
	  uwPrescalerValue = (unsigned int) ((SystemCoreClock / 10000000) - 1);
	  TimHandle.Instance = TIM3;
	  TimHandle.Init.Period = 200 - 1; // 100MHz/200=50kHz
	  TimHandle.Init.Prescaler = uwPrescalerValue;
	  TimHandle.Init.ClockDivision = 0;
	  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;

	  HAL_TIM_Base_Init(&TimHandle);

	  sConfigOC.OCMode = TIM_OCMODE_PWM1;
	  sConfigOC.Pulse = 0x5;// Specifies the pulse value to be loaded into the Capture Compare Register. This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF */

	  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

	 HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfigOC, TIM_CHANNEL_1);
	 HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfigOC, TIM_CHANNEL_2);

	 // CHANGEMENT DU RAPPORT CYCLIQUE
	 __HAL_TIM_SetCompare(&TimHandle, TIM_CHANNEL_1, 100);	// 100 : moteurs au repos
	 __HAL_TIM_SetCompare(&TimHandle, TIM_CHANNEL_2, 100);

	  HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1);	// MOTOR RIGHT
	  HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2); // MOTOR LEFT

	  // ENABLE MOTEUR (SI INVERSEUR)
	  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0);
	  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0);
}

//=================================================================
//			SET DUTY CYCLE LEFT
//=================================================================
void motorLeft_SetDuty(int duty)
{
	__HAL_TIM_SetCompare(&TimHandle, TIM_CHANNEL_1, duty);
}
//=================================================================
//			SET DUTY CYCLE RIGHT
//=================================================================
void motorRight_SetDuty(int duty)
{
	__HAL_TIM_SetCompare(&TimHandle, TIM_CHANNEL_2, duty);
}
//=================================================================


