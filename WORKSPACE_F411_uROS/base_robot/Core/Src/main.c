#include "main.h"
#include "groveLCD.h"
#include "robot_control.h"
#include "communication_manager.h"

#define ARRAY_LEN 100
#define SAMPLING_PERIOD_ms 5
#define FIND_MOTOR_INIT_POS 0

//################################################
#define EX1 1
#define EX2 2
#define EX3 3

#define SYNCHRO_EX EX1
//################################################
//################################################
// PARAMETRE A MODIFIER EN FONCTION DU N° ROBOT
#define ROS_DOMAIN_ID 4
//################################################
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern I2C_HandleTypeDef hi2c1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 3000 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

// Déclaration des objets synchronisants !! Ne pas oublier de les créer
xSemaphoreHandle xSemaphore = NULL;
xQueueHandle qh = NULL;

#if SYNCHRO_EX == EX1
xQueueHandle qhL = NULL;
xQueueHandle qhR = NULL;
#endif
struct AMessage // Exemple de type de message pour queues, on peut mettre ce qu'on veut
{
	char command;
	int data;
};


void SystemClock_Config(void);
void microros_task(void *argument);
uint16_t mes_vl53=0;

int Left_first_index_reached = 0;
int Right_first_index_reached = 0;

//========================================================================
#if SYNCHRO_EX == EX1

int tab_speed[200];
int tab_cde[200];
int speed;
int obstacle_detected ;
char command_received ;
int speed_received ;
float values_xy[2] = {0, 0};

//=======================================  Controle Robot

int mode = 1;
int previous =0;
int steps = 0, cpt = 0;
int w_cam = 320;
int h_cam = 240;

//======================================= asservissement de moter ========================

#define Te 5
#define tau_L 230
#define tau_R 210
#define Ti_L (0.1*tau_L)
#define Ti_R (0.1*tau_R)
#define Ki_L (Te/Ti_L)
#define Ki_R (Te/Ti_R)
#define Kp_L 0.01
#define Kp_R 0.01

//========================================================================

int speed_L = 0;
static void task_MG(void *pvParameters) {
    int err_L;
    float up_L;
    static float ui_L = 0.0;
    int commande = 0;
    int consigne=0;
    int start = 0;
    int tmp = 0;
    struct AMessage pxLxedMessage;

    for (;;) {
        xQueueReceive(qhL, &pxLxedMessage, portMAX_DELAY);
        //printf("TASK A \r\n");

		if(pxLxedMessage.command == 'b'){ //consigne recul
			consigne = -pxLxedMessage.data;
		}else if(pxLxedMessage.command =='f'){ // avancer
			consigne = pxLxedMessage.data;
		}else if(pxLxedMessage.command == 'l'){ // rotation gauche
			consigne = -pxLxedMessage.data;;
		}else if(pxLxedMessage.command == 'r'){ // rotation droite
			consigne = pxLxedMessage.data;
		}else{
			consigne = pxLxedMessage.data;
		}

        // Calculate speed control values for left motor
        speed_L = quadEncoder_GetSpeedL();
        err_L = consigne - speed_L;
        up_L = Kp_L * (float)err_L;
        ui_L += Kp_L * Ki_L * (float)err_L;
        commande = (int)(up_L + ui_L);
        motorLeft_SetDuty(commande+100);

        xSemaphoreGive(xSemaphore);
    }
}

int speed_R = 0;
static void task_MD(void *pvParameters) {
    int err_R;
    float up_R;
    static float ui_R = 0.0;
    int commande = 0;
    int consigne=0;
    int start = 0;
    int tmp = 0;
    struct AMessage pxRxedMessage;

    for (;;) {
        xQueueReceive(qhR, &pxRxedMessage, portMAX_DELAY);
        //printf("TASK B \r\n");


		if(pxRxedMessage.command == 'b'){ //consigne recul
			consigne = -pxRxedMessage.data;
		}else if(pxRxedMessage.command =='f'){ // avancer
			consigne = pxRxedMessage.data;
		}else if(pxRxedMessage.command == 'l'){ // rotation gauche
			consigne = pxRxedMessage.data;;
		}else if(pxRxedMessage.command == 'r'){ // rotation droite
			consigne = -pxRxedMessage.data;
		}else {
			consigne = pxRxedMessage.data;
		}

        // Calculate speed control values for right motor
        speed_R = quadEncoder_GetSpeedR();
        err_R = consigne - speed_R;
        up_R = Kp_R * (float)err_R;
        ui_R += Kp_R * Ki_R * (float)err_R;
        commande= (int)(up_R + ui_R);
        motorRight_SetDuty(commande+100);
        xSemaphoreGive(xSemaphore);
    }
}


static void task_Superviseur(void *pvParameters) {
    struct AMessage pxMessage;
    int obstacle_detected_forward, obstacle_detected_backward;
    int mode_navigate_end;
    char first;
    char last;
    vTaskDelay(5);
    for (;;) {
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 1);
        //printf("TASK C \r\n");
    	// Check for obstacles
    	obstacle_detected = 0;
    	obstacle_detected_forward = detect_obstacle_forward();
    	obstacle_detected_backward = detect_obstacle_backward();

        // Set mode to auto
        if(mode == 0){
//        			if (obstacle_detected_forward >= 0) {
//					// Obstacle detected in forward direction (0: left, 1: right)
//					//printf("Obstacle detected in front! Stopping.\r\n");
//					pxMessage.command = 's';
//					pxMessage.data =0;
//					obstacle_detected = 1;
//					} else if (obstacle_detected_backward) {
//							//printf("Obstacle detected behind! Stopping.\r\n");
//							pxMessage.command = 's';
//							pxMessage.data =0;
//							obstacle_detected = 2;
//					} else {

							pxMessage.command = command_received;
							pxMessage.data = speed_received;
//					}
        }else if(mode == 1){
        	if(obstacle_detected_forward >= 0 && obstacle_detected_backward){
        		pxMessage.command = 's';
        		pxMessage.data =0;
        		obstacle_detected=3;
        	}
        	else if (obstacle_detected_forward >= 0) {
				//printf("Obstacle front ! .\r\n");
				//printf("steps %d .\r\n",steps);
        		first = (obstacle_detected_forward == 0) ? 'l' : 'r'; last = (obstacle_detected_forward == 0) ? 'r' : 'l';
				pxMessage.command = 'b';
				pxMessage.data = 500;
				obstacle_detected = 1;
				mode_navigate_end = 1;
			} else if (obstacle_detected_backward) {
				//printf("Obstacle detected behind! Stopping.\r\n");
				pxMessage.command = 's';
				pxMessage.data = 0;
				obstacle_detected = 2;
			} else if(mode_navigate_end){
				if (cpt < 500) {
					pxMessage.command = first;
					pxMessage.data = 200;
				} else if (cpt > 300 && cpt < 900) {
					pxMessage.command = 'f';
					pxMessage.data = 400;
				} else if(cpt > 900 && cpt < 1400){
					pxMessage.command = last;
					pxMessage.data = 100;
				}(cpt)++;
				if(cpt > 700){
					mode_navigate_end = 0;
					cpt=0;
					//printf("end  \r\n",cpt);
				}
				//printf("capt = %d \r\n",cpt);
			}else {
				//printf("move_in_square_pattern\r\n");
									if (cpt > 5000 && cpt < 6000) {
								        pxMessage.command = 'l';
								        pxMessage.data = 100;
								    } else if (cpt > 6000 && cpt < 7000) {
								    	//printf("move right\r\n");
								        pxMessage.command = 'r';
								        pxMessage.data = 100;
								        cpt = 0;
								    } else {
								    	//printf("move forward\r\n");
								        pxMessage.command = 'f';
								        pxMessage.data = 500;
								    }
								    (cpt)++;
								    //printf("capt = %d \r\n",cpt);
			}
        } else {
        	if (obstacle_detected_forward >= 0) {
        			pxMessage.command = 's';
        			pxMessage.data =0;
					obstacle_detected = 1;
			} else if (obstacle_detected_backward) {
				pxMessage.command = 's';
				pxMessage.data =0;
				obstacle_detected = 2;
			}
            if (values_xy[0] == 0 && values_xy[1] == 0) {
                pxMessage.command = 's';
                pxMessage.data = 0;
            }
            // Move forward condition
            else if (values_xy[0] > (w_cam / 2 - 120) && values_xy[0] < (w_cam / 2 + 120)) {
                pxMessage.command = 'f';
                pxMessage.data = 300;
            }
            // Move right condition (if values_xy[0] is more than halfway of w_cam)
            else if (values_xy[0] > w_cam / 2) {
                pxMessage.command = 'r';
                pxMessage.data = 100;
            }
            // Move left condition (if values_xy[0] is less than halfway of w_cam)
            else {
                pxMessage.command = 'l';
                pxMessage.data = 100;
            }
        }

        // Send command to left motor task
        xQueueSend(qhL, (void *)&pxMessage, portMAX_DELAY);
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        // Send command to right motor task
        xQueueSend(qhR, (void *)&pxMessage, portMAX_DELAY);
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0);
        // Wait before the next iteration
        vTaskDelay(SAMPLING_PERIOD_ms);
    }
}


static void task_LCD(void *pvParameters)
{
	struct AMessage pxRxedMessage;
	for(;;) {

		if(mode != previous){
			groveLCD_display();
			groveLCD_setCursor(0,0);
			if(mode ==0){
			groveLCD_term_printf("MODE Manuel");
			}
			if(mode ==1){
			groveLCD_term_printf("MODE Auto");
			}
			if(mode ==2){
			groveLCD_term_printf("MODE Track");
			}
			previous = mode;
		}

	}
}

//========================================================
#elif SYNCHRO_EX == EX2

static void task_C( void *pvParameters )
{
	for (;;)
	{
		 printf("TASK C \n\r");
		xSemaphoreTake( xSemaphore, portMAX_DELAY );
	}
}

static void task_D( void *pvParameters )
{
	for (;;)
	{
		 printf("TASK D \n\r");
		xSemaphoreGive( xSemaphore );
	}
}

//========================================================
#elif SYNCHRO_EX == EX3

static void task_E( void *pvParameters )
{
	struct AMessage pxMessage;
	pxMessage.command='a';
	pxMessage.data=10;
	vTaskDelay(1000);
	for (;;)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 1);
	    printf("TASK E \r\n");
		xQueueSend( qh, ( void * ) &pxMessage,  portMAX_DELAY );
		xSemaphoreTake( xSemaphore, portMAX_DELAY );

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0);
		vTaskDelay(SAMPLING_PERIOD_ms);
	}
}

static void task_F(void *pvParameters)
{
	struct AMessage pxRxedMessage;

	for (;;)
	{
		xQueueReceive( qh,  &( pxRxedMessage ) , portMAX_DELAY );
		 printf("TASK F \r\n");
		xSemaphoreGive( xSemaphore );
	}
}
#endif


//=========================================================================
int main(void)
{
  int ret=0;
  int tab_dist[2];

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  motorCommand_Init();
  quadEncoder_Init();
  captDistIR_Init();

  HAL_Delay(500);

  // Affichage via UART2 sur Terminal série $ minicom -D /dev/ttyACM0
  printf("hello\r\n"); // REM : ne pas oublier le \n

  VL53L0X_init();

  ret = VL53L0X_validateInterface();
  if(ret ==0)
  {
	  printf("VL53L0X OK\r\n");
  }
  else
  {
	  printf("!! PROBLEME VL53L0X !!\r\n");
  }
  VL53L0X_startContinuous(0);

  int a, b;
  groveLCD_begin(16,2,0); // !! cette fonction prend du temps
  HAL_Delay(100);
  groveLCD_display();
  a=5; b=2;
  groveLCD_term_printf("%d+%d=%d",a,b,a+b);
  groveLCD_setCursor(0,0);
  groveLCD_term_printf("hello");


  HAL_Delay(50);

#if FIND_MOTOR_INIT_POS

  int16_t speed=0;
// RECHERCHE DE LA POSITION INITIALE ( 1er signal 'index' du capteur )
// Evite une erreur pour une mesure de vitesse 
	Left_first_index_reached = 0;
	 while( Left_first_index_reached != 1 )
	 {
		motorLeft_SetDuty(130);
		HAL_Delay(SAMPLING_PERIOD_ms);
		speed = quadEncoder_GetSpeedL();
	 }

	Right_first_index_reached = 0;
	 while( Right_first_index_reached != 1 )
	 {
		motorRight_SetDuty(130);
		HAL_Delay(SAMPLING_PERIOD_ms);
		speed = quadEncoder_GetSpeedR();
	 }

	 motorLeft_SetDuty(100);
	 motorRight_SetDuty(100);
	 HAL_Delay(200);

	 speed = quadEncoder_GetSpeedL();
	 speed = quadEncoder_GetSpeedR();
#endif

  osKernelInitialize();

  xTaskCreate( microros_task, ( const portCHAR * ) "microros_task", 3000 /* stack size */, NULL,  25, NULL );
  //
  //xTaskCreate( task_A, ( const portCHAR * ) "task A", 128 /* stack size */, NULL, 28, NULL );
 // xTaskCreate( task_B, ( const portCHAR * ) "task B", 128 /* stack size */, NULL, 27, NULL );
#if SYNCHRO_EX == EX1
    xTaskCreate( task_LCD, ( const portCHAR * ) "task LCD", 128 /* stack size */, NULL, 24, NULL );
	xTaskCreate( task_MG, ( const portCHAR * ) "task_MG", 128 /* stack size */, NULL, 27, NULL );
	xTaskCreate( task_MD, ( const portCHAR * ) "task_MD", 128 /* stack size */, NULL, 26, NULL );
	xTaskCreate( task_Superviseur, ( signed portCHAR * ) "task_Superviseur", 128 /* stack size */, NULL, 28, NULL );
#elif SYNCHRO_EX == EX2
	xTaskCreate( task_C, ( signed portCHAR * ) "task C", 128 /* stack size */, NULL, 28, NULL );
	xTaskCreate( task_D, ( signed portCHAR * ) "task D", 128 /* stack size */, NULL, 27, NULL );
#elif SYNCHRO_EX == EX3
	xTaskCreate( task_E, ( signed portCHAR * ) "task E", 128 /* stack size */, NULL, 30, NULL );
	xTaskCreate( task_F, ( signed portCHAR * ) "task F", 128 /* stack size */, NULL, 29, NULL );
#endif

	vSemaphoreCreateBinary(xSemaphore);
	xSemaphoreTake( xSemaphore, portMAX_DELAY );

	qh = xQueueCreate( 1, sizeof(struct AMessage ) );

#if SYNCHRO_EX == EX1
	qhL = xQueueCreate( 1, sizeof(struct AMessage ) );
	qhR = xQueueCreate( 1, sizeof(struct AMessage ) );
#endif

  osKernelStart();

  while (1)
  {}

}
//=========================================================================
bool cubemx_transport_open(struct uxrCustomTransport * transport);
bool cubemx_transport_close(struct uxrCustomTransport * transport);
size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);

void * microros_allocate(size_t size, void * state);
void microros_deallocate(void * pointer, void * state);
void * microros_reallocate(void * pointer, size_t size, void * state);
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);

void subscription_callback1(const void * msgin)
{
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;
  // Process message
  //printf("Received from HOST: %s\n\r", msg->data.data);
  mode = process_vitess_data(msg->data.data);
}

void subscription_callback2(const void * msgin)
{
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;

  // Process message
  //printf("Received from HOST: %s\n\r", msg->data.data);
  command_received = process_command_data(msg->data.data);
  speed_received = process_vitess_data(msg->data.data);

}

void subscription_callback3(const void * msgin)
{
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;
  get_xy(msg->data.data, values_xy);
  // Process message
  //printf("Received from HOST: %f\n\r",values_xy[0]);
}

void microros_task(void *argument)
{
  rmw_uros_set_custom_transport( true, (void *) &huart1, cubemx_transport_open,  cubemx_transport_close,  cubemx_transport_write, cubemx_transport_read);

  rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
  freeRTOS_allocator.allocate = microros_allocate;
  freeRTOS_allocator.deallocate = microros_deallocate;
  freeRTOS_allocator.reallocate = microros_reallocate;
  freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

  if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
      printf("Error on default allocators (line %d)\n", __LINE__);
  }

  // micro-ROS app
  rclc_support_t support;
  rcl_allocator_t allocator;
  allocator = rcl_get_default_allocator();

  // create node
  rcl_node_t node;
  rcl_node_options_t node_ops = rcl_node_get_default_options();
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  rcl_init_options_init(&init_options, allocator);
  rcl_init_options_set_domain_id(&init_options, ROS_DOMAIN_ID);

  rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator);
  rclc_node_init_default(&node, "STM32_Node","", &support);

  // create publisher
   rcl_publisher_t publisher1;
   std_msgs__msg__String sensor_speed__msg;
   rclc_publisher_init_default(&publisher1, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),"/sensor/motor_speed");

   rcl_publisher_t publisher2;
   std_msgs__msg__String sensor_obs_msg;
   rclc_publisher_init_default(&publisher2, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),"/sensor/recive_obstacl");

   // create subscriber
   rcl_subscription_t subscriber1;
   std_msgs__msg__String str_msg1;
   rclc_subscription_init_default(&subscriber1,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),"/command/mode");
   rcl_subscription_t subscriber2;
   std_msgs__msg__String str_msg2;
   rclc_subscription_init_default(&subscriber2,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),"/command/move");
   rcl_subscription_t subscriber3;
    std_msgs__msg__String str_msg3;
    rclc_subscription_init_default(&subscriber3,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),"/camera/src_xy");
   // Add subscription to the executor
   rclc_executor_t executor;
   rclc_executor_init(&executor, &support.context, 3, &allocator); // ! 'NUMBER OF HANDLES' A MODIFIER EN FONCTION DU NOMBRE DE TOPICS
   rclc_executor_add_subscription(&executor, &subscriber1, &str_msg1, &subscription_callback1, ON_NEW_DATA);
   rclc_executor_add_subscription(&executor, &subscriber2, &str_msg2, &subscription_callback2, ON_NEW_DATA);
   rclc_executor_add_subscription(&executor, &subscriber3, &str_msg3, &subscription_callback3, ON_NEW_DATA);


   sensor_speed__msg.data.data = (char * ) malloc(ARRAY_LEN * sizeof(char));
   sensor_speed__msg.data.size = 0;
   sensor_speed__msg.data.capacity = ARRAY_LEN;

   sensor_obs_msg.data.data = (char * ) malloc(ARRAY_LEN * sizeof(char));
   sensor_obs_msg.data.size = 0;
   sensor_obs_msg.data.capacity = ARRAY_LEN;

   str_msg1.data.data = (char * ) malloc(ARRAY_LEN * sizeof(char));
   str_msg1.data.size = 0;
   str_msg1.data.capacity = ARRAY_LEN;

   str_msg2.data.data = (char * ) malloc(ARRAY_LEN * sizeof(char));
   str_msg2.data.size = 0;
   str_msg2.data.capacity = ARRAY_LEN;

   str_msg3.data.data = (char * ) malloc(ARRAY_LEN * sizeof(char));
   str_msg3.data.size = 0;
   str_msg3.data.capacity = ARRAY_LEN;

  for(;;)
  {
//	  sprintf(sensor_speed__msg.data.data, "from STM32 : speedL=#%d , speedR=#%d", (int32_t)speed_L,(int32_t)speed_R);
	  sprintf(sensor_speed__msg.data.data, "speedL=#%d , speedR=#%d", (int32_t)speed_L,(int32_t)speed_R);
	  sensor_speed__msg.data.size = strlen(sensor_speed__msg.data.data);
//	  sprintf(sensor_obs_msg.data.data, "from STM32 : obstacle detected : #%d", (int32_t)obstacle_detected);
	  sprintf(sensor_obs_msg.data.data, "#%d", (int32_t)obstacle_detected);
	  sensor_obs_msg.data.size = strlen(sensor_obs_msg.data.data);

	  // Publish sensor data
	  rcl_ret_t pub_ret1 = rcl_publish(&publisher1, &sensor_speed__msg, NULL);
	  rcl_ret_t pub_ret2 = rcl_publish(&publisher2, &sensor_obs_msg, NULL);

	  if (pub_ret1 != RCL_RET_OK || pub_ret2 != RCL_RET_OK) {
		  printf("Error publishing (line %d)\n\r", __LINE__);
	  }
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
    osDelay(10);
  }
}
//=========================================================================
/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
}
//=========================================================================
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {}
}
//=========================================================================
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

