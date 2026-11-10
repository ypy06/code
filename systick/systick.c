#include "systick.h"
/**
  * @brief  初始化DWT计数器（用于微秒级延时）
  * @retval 无
  */
static void DWT_Init(void)
{
    /* 使能DWT调试组件 */
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
  * @brief  微秒级延时（基于DWT时钟周期计数器）
  * @param  us 延时时长，单位：微秒（最大约59.6秒，受32位计数器限制）
  * @retval 无
  */
void Delay_us(uint32_t us)
{
    static uint8_t init_flag = 0;
    if (!init_flag)
    {
        DWT_Init();
        init_flag = 1;
    }

    uint32_t cycles = (SystemCoreClock / 1000000) * us;   // 所需周期数
    uint32_t start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < cycles);
}

/**
  * @brief  毫秒级延时（基于HAL_Delay）
  * @param  ms 延时时长，单位：毫秒（最大约49.7天，受32位计数器限制）
  * @retval 无
  */
void Delay_ms(uint32_t ms)
{
    while (ms--)
    {
        HAL_Delay(1);
    }
}

/**
  * @brief  秒级延时（基于HAL_Delay）
  * @param  s 延时时长，单位：秒（最大约49.7天，受32位计数器限制）
  * @retval 无
  */
void Delay_s(uint32_t s)
{
    while (s--)
    {
        HAL_Delay(1000);
    }
}