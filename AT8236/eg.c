

#include "motor.h"
#include "tim.h"

Motor motor1;
Motor motor2;
/*
初始化
*/
void APP_Init()
{
    motor1=Motor_Construct(&htim2,MOTOR_CH1,MOTOR_CH2);
    motor2=Motor_Construct(&htim2,MOTOR_CH3,MOTOR_CH4);
    Motor_SetDuty(&motor1,0);
    Motor_SetDuty(&motor2,0);
}

void APP_Loop()
{
    Motor_SetDuty(&motor1,500);
    Motor_SetDuty(&motor2,500);
}
