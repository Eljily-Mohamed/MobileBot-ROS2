/*
 * QuadEncoder.c
 */
#include "quadEncoder.h"

#define SAMPLING_PERIOD_ms		5
#define TE_ms	SAMPLING_PERIOD_ms
#define USE_QUAD_ENCODER_1250_CPR 1

#if USE_QUAD_ENCODER_1250_CPR
#define COUNT_PER_ROUND 1250
#define MAX_CNT_PER_REV (COUNT_PER_ROUND * 4 - 1)
#define MAX_COUNT (int)(((unsigned long)MAX_CNT_PER_REV*6555)/1000)
#define HALF_MAX_COUNT (MAX_COUNT>>1)
#define COEFF	6555
#endif

#if USE_QUAD_ENCODER_1000_CPR
#define COUNT_PER_ROUND 1000
#define MAX_CNT_PER_REV (COUNT_PER_ROUND * 4 - 1)
#define MAX_COUNT (int)(((unsigned long)MAX_CNT_PER_REV*8192)/1000)
#define HALF_MAX_COUNT (MAX_COUNT>>1)
#define COEFF 8192
#endif

#if USE_QUAD_ENCODER_500_CPR
#define COUNT_PER_ROUND 500
#define MAX_CNT_PER_REV (COUNT_PER_ROUND * 4 - 1)
#define MAX_COUNT (int)(((unsigned long)MAX_CNT_PER_REV*16392)/1000)
#define HALF_MAX_COUNT (MAX_COUNT>>1)
#define COEFF 16392
#endif

#if USE_QUAD_ENCODER_250_CPR
#define COUNT_PER_ROUND 250
#define MAX_CNT_PER_REV (COUNT_PER_ROUND * 4 - 1)
#define MAX_COUNT (int)(((unsigned long)MAX_CNT_PER_REV*32768)/1000)
#define HALF_MAX_COUNT (MAX_COUNT>>1)
#define COEFF 32768
#endif


TIM_HandleTypeDef    TimEncoderHandleLeft;
TIM_HandleTypeDef    TimEncoderHandleRight;


extern int Left_first_index_reached ;
extern int Right_first_index_reached ;

/*******************************************************************************
 * TIMER 1, CHANNEL 1 et 2  --> RIGHT
 * TIMER 2, CHANNEL 1 et 2	--> LEFT
****************************************************************************** */
int indexL=0;
static int indexR=0;

//================================================================
//		TIMER INIT
//================================================================

void quadEncoder_Init(void)
{
	TIM_Encoder_InitTypeDef sConfig;
	//--------------------------------------------------
	// TIMER 1
	//--------------------------------------------------
	TimEncoderHandleLeft.Instance = TIM1;
	TimEncoderHandleLeft.Init.Prescaler = 0;
	TimEncoderHandleLeft.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimEncoderHandleLeft.Init.Period = COUNT_PER_ROUND*4;
	TimEncoderHandleLeft.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV4;
	sConfig.IC1Filter = 0x0F;
	sConfig.IC2Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;//TIM_ICSELECTION_DIRECTTI; //TIM_TI1SELECTION_XORCOMBINATION
	sConfig.IC2Prescaler = TIM_ICPSC_DIV4;
	sConfig.IC2Filter = 0x0F;

	HAL_TIM_Encoder_Init(&TimEncoderHandleLeft, &sConfig);

	__HAL_TIM_SetCounter(&TimEncoderHandleLeft, 0);

	HAL_TIM_Encoder_Start(&TimEncoderHandleLeft,TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&TimEncoderHandleLeft,TIM_CHANNEL_2);

	//--------------------------------------------------
	// TIMER 2
	//--------------------------------------------------
	TimEncoderHandleRight.Instance = TIM2;
	TimEncoderHandleRight.Init.Prescaler = 0;
	TimEncoderHandleRight.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimEncoderHandleRight.Init.Period = COUNT_PER_ROUND*4;
	TimEncoderHandleRight.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV4;
	sConfig.IC1Filter = 0x0F;
	sConfig.IC2Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;//TIM_ICSELECTION_DIRECTTI; //TIM_TI1SELECTION_XORCOMBINATION
	sConfig.IC2Prescaler = TIM_ICPSC_DIV4;
	sConfig.IC2Filter = 0x0F;

	HAL_TIM_Encoder_Init(&TimEncoderHandleRight, &sConfig);

	__HAL_TIM_SetCounter(&TimEncoderHandleRight, 0);

	HAL_TIM_Encoder_Start(&TimEncoderHandleRight,TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&TimEncoderHandleRight,TIM_CHANNEL_2);
}

//================================================================
//		POSITION LEFT CALC
//================================================================

void quadEncoder_PosCalcL(int* AngPos)
{

int POSCNTcopy = 0;
POSCNTcopy = (int)TIM1->CNT;
AngPos[1] = AngPos[0];
AngPos[0] = (unsigned int)(((unsigned long)POSCNTcopy * COEFF)/1000); // 0 <= POSCNT <= 4999 to 0 <= AngPos <= 32767
}

//================================================================
//		POSITION RIGHT CALC
//================================================================

void quadEncoder_PosCalcR(int* AngPos)
{

int POSCNTcopy = 0;
POSCNTcopy = (int)TIM2->CNT;
AngPos[1] = AngPos[0];
AngPos[0] = (unsigned int)(((unsigned long)POSCNTcopy * COEFF)/1000); // 0 <= POSCNT <= 4999 to 0 <= AngPos <= 32767
}

//================================================================
//		POSITION LEFT 16 BITS
//================================================================

int16_t quadEncoder_GetPos16L(void)
{
	uint16_t PosL = 0;
	PosL=TIM1->CNT;
	return (int16_t)PosL;

	}

//================================================================
//		POSITION RIGHT 16 BITS
//================================================================

int16_t quadEncoder_GetPos16R(void)
{
	uint16_t PosR = 0;
	PosR=TIM2->CNT;
    return (int16_t)PosR;
}
//================================================================
//		POSITION LEFT 32 BITS	(pos 16 bits + nombre de tours)
//================================================================

int32_t quadEncoder_GetPos32L(void)
{
	int32_t  PosL = 0;
	PosL=indexL*4*COUNT_PER_ROUND + (int32_t) quadEncoder_GetPos16L();
	return PosL;
}

//================================================================
//		POSITION RIGHT 32 BITS	(pos 16 bits + nombre de tours)
//================================================================

int32_t quadEncoder_GetPos32R(void)
{
	int32_t PosR = 0;
	PosR=indexR*4*COUNT_PER_ROUND + (int32_t) quadEncoder_GetPos16R();
	return PosR;
}

//================================================================
//		SPEED LEFT
//--> must be called every Te
//================================================================

int16_t quadEncoder_GetSpeedL(void)
{
	static int AngPos[2] = {0,0};
	static int16_t SpeedL=0;

	quadEncoder_PosCalcL(AngPos);
	SpeedL = AngPos[0] - AngPos[1];
	if (SpeedL >= 0)
	{
		if (SpeedL >= HALF_MAX_COUNT)
			{
			SpeedL = SpeedL - MAX_COUNT;
			}
	}
	else
	{
		if (SpeedL < -HALF_MAX_COUNT)
			{
			SpeedL = SpeedL + MAX_COUNT;
			}
	}
	
	//***********************************
	// CONVERT RPM
	// 1 tour = 32767
	// Nbre de Tours pendant Te: DELTA_pos/32767
	// Nbre de Tours pendant 1s (Te en ms) : (DELTA_pos/32767)*(1000/Te)
	// Nbre de Tours par minute : : (DELTA_pos/32767)*((60*1000)/Te)

	SpeedL=(SpeedL*60*1000)/(32767*TE_ms);
	return SpeedL;
}

//================================================================
//		SPEED RIGHT
//-->  must be called every Te
//================================================================

int16_t quadEncoder_GetSpeedR(void)
{
	static int AngPos[2] = {0,0};
	static int16_t SpeedR=0;


	quadEncoder_PosCalcR(AngPos);
	SpeedR = AngPos[0] - AngPos[1];
	if (SpeedR >= 0)
	{
		if (SpeedR >= HALF_MAX_COUNT)
			{
			SpeedR = SpeedR - MAX_COUNT;
			}
	}
	else
	{
		if (SpeedR < -HALF_MAX_COUNT)
			{
			SpeedR = SpeedR + MAX_COUNT;
			}
	}
	//***********************************
	// CONVERT RPM
	// 1 tour = 32767
	// Nbre de Tours pendant Te: DELTA_pos/32767
	// Nbre de Tours pendant 1s (Te en ms) : (DELTA_pos/32767)*(1000/Te)
	// Nbre de Tours par minute : : (DELTA_pos/32767)*((60*1000)/Te)

	SpeedR=(SpeedR*60*1000)/(32767*TE_ms);
	return SpeedR;
}

//================================================================
//		MAJ index Left
//================================================================

void quadEncoder_CallbackIndexL()
{
				if (__HAL_TIM_DIRECTION_STATUS(&TimEncoderHandleLeft)==1)
				{
					indexL--;
				}
				else
				{
					indexL++;
				}


				__HAL_TIM_SetCounter(&TimEncoderHandleLeft, 0);		// RAZ Counter
				HAL_TIM_Encoder_Start(&TimEncoderHandleLeft,TIM_CHANNEL_1);
				HAL_TIM_Encoder_Start(&TimEncoderHandleLeft,TIM_CHANNEL_2);

				Left_first_index_reached = 1;

}
//================================================================
//		MAJ index Right
//================================================================

void quadEncoder_CallbackIndexR()
{
				if (__HAL_TIM_DIRECTION_STATUS(&TimEncoderHandleRight)==1)
				{
					indexR--;

				}
				else
				{
					indexR++;
				}

				__HAL_TIM_SetCounter(&TimEncoderHandleRight, 0);			// RAZ Counter
				HAL_TIM_Encoder_Start(&TimEncoderHandleRight,TIM_CHANNEL_1);
				HAL_TIM_Encoder_Start(&TimEncoderHandleRight,TIM_CHANNEL_2);

				Right_first_index_reached = 1;

}
//================================================================



