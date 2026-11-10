#include "buzzer.h"
void buzzer_Init(void)
{
    HAL_GPIO_WritePin(GPIO,GPIO_PIN_,GPIO_PIN_SET);
}
void buzzer_On(void)
{
    HAL_GPIO_WritePin(GPIO,GPIO_PIN_,GPIO_PIN_RESET);
}
void buzzer_Off(void)
{
    HAL_GPIO_WritePin(GPIO,GPIO_PIN_,GPIO_PIN_SET);
}