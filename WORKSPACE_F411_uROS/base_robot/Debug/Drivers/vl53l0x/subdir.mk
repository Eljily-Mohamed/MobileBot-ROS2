################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/vl53l0x/vl53l0x_api.c \
../Drivers/vl53l0x/vl53l0x_api_calibration.c \
../Drivers/vl53l0x/vl53l0x_api_core.c \
../Drivers/vl53l0x/vl53l0x_api_ranging.c \
../Drivers/vl53l0x/vl53l0x_api_strings.c \
../Drivers/vl53l0x/vl53l0x_platform_log.c 

OBJS += \
./Drivers/vl53l0x/vl53l0x_api.o \
./Drivers/vl53l0x/vl53l0x_api_calibration.o \
./Drivers/vl53l0x/vl53l0x_api_core.o \
./Drivers/vl53l0x/vl53l0x_api_ranging.o \
./Drivers/vl53l0x/vl53l0x_api_strings.o \
./Drivers/vl53l0x/vl53l0x_platform_log.o 

C_DEPS += \
./Drivers/vl53l0x/vl53l0x_api.d \
./Drivers/vl53l0x/vl53l0x_api_calibration.d \
./Drivers/vl53l0x/vl53l0x_api_core.d \
./Drivers/vl53l0x/vl53l0x_api_ranging.d \
./Drivers/vl53l0x/vl53l0x_api_strings.d \
./Drivers/vl53l0x/vl53l0x_platform_log.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/vl53l0x/%.o Drivers/vl53l0x/%.su Drivers/vl53l0x/%.cyclo: ../Drivers/vl53l0x/%.c Drivers/vl53l0x/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I"/home/kerhoas/WORKSPACE_WEB2/ROBOT_ROS_/WORKSPACE_F411_uROS6/base_robot/Drivers/vl53l0x" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I"/home/kerhoas/WORKSPACE_WEB2/ROBOT_ROS_/WORKSPACE_F411_uROS6/base_robot/micro_ros_stm32cubemx_utils/microros_static_library/libmicroros/microros_include" -I"/home/kerhoas/WORKSPACE_WEB2/ROBOT_ROS_/WORKSPACE_F411_uROS6/base_robot/Drivers/vl53l0x" -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-vl53l0x

clean-Drivers-2f-vl53l0x:
	-$(RM) ./Drivers/vl53l0x/vl53l0x_api.cyclo ./Drivers/vl53l0x/vl53l0x_api.d ./Drivers/vl53l0x/vl53l0x_api.o ./Drivers/vl53l0x/vl53l0x_api.su ./Drivers/vl53l0x/vl53l0x_api_calibration.cyclo ./Drivers/vl53l0x/vl53l0x_api_calibration.d ./Drivers/vl53l0x/vl53l0x_api_calibration.o ./Drivers/vl53l0x/vl53l0x_api_calibration.su ./Drivers/vl53l0x/vl53l0x_api_core.cyclo ./Drivers/vl53l0x/vl53l0x_api_core.d ./Drivers/vl53l0x/vl53l0x_api_core.o ./Drivers/vl53l0x/vl53l0x_api_core.su ./Drivers/vl53l0x/vl53l0x_api_ranging.cyclo ./Drivers/vl53l0x/vl53l0x_api_ranging.d ./Drivers/vl53l0x/vl53l0x_api_ranging.o ./Drivers/vl53l0x/vl53l0x_api_ranging.su ./Drivers/vl53l0x/vl53l0x_api_strings.cyclo ./Drivers/vl53l0x/vl53l0x_api_strings.d ./Drivers/vl53l0x/vl53l0x_api_strings.o ./Drivers/vl53l0x/vl53l0x_api_strings.su ./Drivers/vl53l0x/vl53l0x_platform_log.cyclo ./Drivers/vl53l0x/vl53l0x_platform_log.d ./Drivers/vl53l0x/vl53l0x_platform_log.o ./Drivers/vl53l0x/vl53l0x_platform_log.su

.PHONY: clean-Drivers-2f-vl53l0x

