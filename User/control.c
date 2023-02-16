#include <stm32f10x.h>
#include "GPIO_Init_Configuration.h"
#include "control.h"
#include "Delay.h"

//引脚定义
/*
U/D:    PC0 PC1 控制方式
INC:    PC2 PC3 下边沿触发
CS:     PC4 PC5 控制开关，低电平有效
*/

void UP_024(void)
{
    PC4_OFF;
    PC0_OFF;
    PC2_ON;
    Delay_10us(1);
    PC2_OFF; //下边沿触发
    PC4_ON;
}

void DOWN_024(void)
{
    PC4_OFF;
    PC0_ON;
    PC2_ON;
    Delay_10us(1);
    PC2_OFF;//下边沿触发
    PC4_ON;
}

void UP_135(void)
{
    PC5_OFF;
    PC1_OFF;
    PC3_ON;
    Delay_10us(1);
    PC3_OFF;//下边沿触发
    PC5_ON;
}

void DOWN_135(void)
{
    PC5_OFF;
    PC1_ON;
    PC3_ON;
    Delay_10us(1);
    PC3_OFF;//下边沿触发
    PC5_ON;
}
