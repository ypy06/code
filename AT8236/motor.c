#include "motor.h"
#include "tim.h"
extern TIM_HandleTypeDef htim3;   //定时器通道


/*
电机初始化
*/
int Motor_Init(void)
{
 
}

/*
构造
TIM_HandleTypeDef 句柄

*/
Motor Motor_Construct(TIM_HandleTypeDef* htim, uint32_t channel1,uint32_t channel2)
{
    Motor motor;
    motor.htim=htim;
    motor.ch1_val=0;
    motor.ch2_val=0;
    motor.channel1=channel1;
    motor.channel2=channel2;
    
    HAL_TIM_PWM_Start(htim,channel1);
    HAL_TIM_PWM_Start(htim,channel2);
    return motor;
}


/*
设置占空比
*/

void Motor_SetDuty(Motor* motor,int16_t value){

    if(value>=0){
            motor->ch1_val=value;
            motor->ch2_val=0;
            __HAL_TIM_SET_COMPARE(motor->htim,motor->channel1,motor->ch1_val);
            __HAL_TIM_SET_COMPARE(motor->htim,motor->channel2,0);

    } else {
            motor->ch1_val=0;
            motor->ch2_val=-value;
            __HAL_TIM_SET_COMPARE(motor->htim,motor->channel1,0);
            __HAL_TIM_SET_COMPARE(motor->htim,motor->channel2,motor->ch2_val);
    }
}