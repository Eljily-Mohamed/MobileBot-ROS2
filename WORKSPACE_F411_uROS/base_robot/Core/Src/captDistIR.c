/*
 * IRMeasure.c
 */


#include "captDistIR.h"

ADC_HandleTypeDef 	adcHandle;
ADC_HandleTypeDef 	adcHandle_12;
ADC_HandleTypeDef 	adcHandle_13;
ADC_ChannelConfTypeDef   sConfig;

//=================================================================
//			ADC INIT FOR IR SENSOR SHARP GP2D12
//=================================================================

void  captDistIR_Init(void)
{
	adcHandle.Instance		= ADC1;

	adcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	adcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adcHandle.Init.Resolution = ADC_RESOLUTION12b;
	// Don't do continuous conversions - do them on demand
	adcHandle.Init.ContinuousConvMode    = DISABLE;  // Continuous mode disabled to have only 1 conversion at each conversion trig
	// Disable the scan conversion so we do one at a time */
	adcHandle.Init.ScanConvMode = DISABLE;
	//Say how many channels would be used by the sequencer
	adcHandle.Init.NbrOfConversion = 2;
	adcHandle.Init.DiscontinuousConvMode = DISABLE;  // Parameter discarded because sequencer is disabled
	adcHandle.Init.NbrOfDiscConversion = 2;
	adcHandle.Init.ExternalTrigConvEdge =  ADC_EXTERNALTRIGCONVEDGE_NONE ;
	//Start conversion by software, not an external trigger
	adcHandle.Init.ExternalTrigConv = 0;
	adcHandle.Init.DMAContinuousRequests = DISABLE;
	adcHandle.Init.EOCSelection = DISABLE;

	HAL_ADC_Init(&adcHandle);
}

//=================================================================
//			IR GET (POLL METHOD)
//=================================================================

int  captDistIR_Get(int* tab)
{
	sConfig.Channel      = ADC_CHANNEL_4;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
	HAL_ADC_ConfigChannel(&adcHandle, &sConfig);

	HAL_ADC_Start(&adcHandle);					//Start the conversion
	HAL_ADC_PollForConversion(&adcHandle,10);	//Processing the conversion
	tab[0]=HAL_ADC_GetValue(&adcHandle); 		//Return the converted data

	sConfig.Channel      = ADC_CHANNEL_8;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
	HAL_ADC_ConfigChannel(&adcHandle, &sConfig);

	HAL_ADC_Start(&adcHandle);						//Start the conversion
	HAL_ADC_PollForConversion(&adcHandle,10);		//Processing the conversion
	tab[1]=HAL_ADC_GetValue(&adcHandle); 			//Return the converted data

	return 0;
}
//=================================================================
