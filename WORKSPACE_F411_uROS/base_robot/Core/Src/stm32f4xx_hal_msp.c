#include "main.h"


#define USART2_IRQ_PRIO	9
#define USART6_IRQ_PRIO	10
//#define EXTI1_IRQ_PRIO	7
#define EXTI0_IRQ_PRIO	6
#define EXTI15_10_IRQ_PRIO	7
#define I2C1_ER_IRQ_PRIO	2
#define I2C1_EV_IRQ_PRIO	11
#define TIM5_IRQ_PRIO		12

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

void HAL_PWM_Timer3_MspInit(void);
void HAL_Encoder_Timer1_MspInit(void);
void HAL_Encoder_Timer2_MspInit(void);
void HAL_adcir_MspInit(void);
void HAL_GPIO_MspInit(void);

void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  HAL_PWM_Timer3_MspInit();
  HAL_Encoder_Timer1_MspInit();
  HAL_Encoder_Timer2_MspInit();
  HAL_adcir_MspInit();
  HAL_GPIO_MspInit();

}
/******************************************************************
			GPIO : contrôle période d'échantillonage
GPIO OUTPUT	-->	PB4
******************************************************************/
void HAL_GPIO_MspInit(void)
{
	  GPIO_InitTypeDef  GPIO_InitStruct;

	  GPIO_InitStruct.Pin = GPIO_PIN_4;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // hal_gpio.h
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;

	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
/******************************************************************
			ENCODER - TIMER1
PWM1/1	-->	PA8		-- Encodeur Voie A
PWM1/2	--> PA9		-- Encodeur Voie B
EXTI1	--> PB10		-- Index encodeur
******************************************************************/
void HAL_Encoder_Timer1_MspInit(void)
{
	  GPIO_InitTypeDef  GPIO_InitStruct;

	  __TIM1_CLK_ENABLE();

	  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // hal_gpio.h
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	  GPIO_InitStruct.Alternate =  GPIO_AF1_TIM1 ; // hal_gpio_ex.h

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = GPIO_PIN_10;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
	  HAL_NVIC_SetPriority(EXTI15_10_IRQn, EXTI15_10_IRQ_PRIO, 0);
	  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
/******************************************************************
				ENCODER - TIMER2
PWM2/1	-->	PA0		-- Encodeur Voie A
PWM2/2	--> PA1		-- Encodeur Voie B
EXTI 	--> PC0		-- Index Moteur
******************************************************************/
void HAL_Encoder_Timer2_MspInit(void)
{
	  GPIO_InitTypeDef  GPIO_InitStruct;

	  __TIM2_CLK_ENABLE();

	  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // hal_gpio.h
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	  GPIO_InitStruct.Alternate =  GPIO_AF1_TIM2 ; // hal_gpio_ex.h

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = GPIO_PIN_0;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
	  HAL_NVIC_SetPriority(EXTI0_IRQn, EXTI0_IRQ_PRIO, 0);
	  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/******************************************************************
//			PWM - TIMER4 COMMANDE MOTEURS
PA6 --> PWM3/1
PC7 --> PWM3/2
PB3 --> ENABLE MOTEUR (actif état Bas)
******************************************************************/
void HAL_PWM_Timer3_MspInit(void)
{
	  GPIO_InitTypeDef  GPIO_InitStruct;

	  __TIM3_CLK_ENABLE();

	  GPIO_InitStruct.Pin = GPIO_PIN_6;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	  GPIO_InitStruct.Alternate =  GPIO_AF2_TIM3 ; // hal_gpio_ex.h

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	  GPIO_InitStruct.Alternate =  GPIO_AF2_TIM3 ; // hal_gpio_ex.h

	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  // ENABLE MOTEUR : SORTIE LOGIQUE PB3
	/*  GPIO_InitStruct.Pin = GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);*/

	  GPIO_InitStruct.Pin = GPIO_PIN_3;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1);

	  GPIO_InitStruct.Pin = GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 1);




}

/******************************************************************
			ADC
ADC1_4	--> PA4
ADC1_8	--> PB0
http://stm32f4-discovery.com/2014/04/library-06-ad-converter-on-stm32f4xx/
******************************************************************/
void HAL_adcir_MspInit(void)
{
	  GPIO_InitTypeDef  GPIO_InitStruct;
	  /* Peripheral clock enable */
	  __ADC1_CLK_ENABLE();

	  GPIO_InitStruct.Pin = GPIO_PIN_4 ;
	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	  GPIO_InitStruct.Pin = GPIO_PIN_0 ;
	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}


/******************************************************************
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
******************************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hi2c->Instance==I2C1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_RCC_I2C1_CLK_ENABLE();

	  HAL_NVIC_SetPriority(I2C1_ER_IRQn, I2C1_ER_IRQ_PRIO, 0);
	  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	  HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C1_EV_IRQ_PRIO, 0);
	  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  }

}

/******************************************************************
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
******************************************************************/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }

}

/******************************************************************
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
******************************************************************/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }

}

/******************************************************************
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
******************************************************************/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_DMA_DeInit(huart->hdmatx);

    /* USART1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_DMA_DeInit(huart->hdmatx);

    /* USART2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
