#ifndef motor_h
#define motor_h
#include "main.h"
#define MOTOR_CH1 TIM_CHANNEL_1
#define MOTOR_CH2 TIM_CHANNEL_2

/*
电机结构体
*/
typedef struct Motor {
    TIM_HandleTypeDef* htim;
    uint32_t channel1;
    uint16_t ch1_val;
    uint32_t channel2;
    uint16_t ch2_val;
} Motor;


/*
一个channel控制两个PWM
*/
int Motor_Init();
Motor Motor_Construct(TIM_HandleTypeDef* htim, uint32_t channel1,uint32_t channel2);
void Motor_SetDuty(Motor* motor,int16_t value);

#endif
