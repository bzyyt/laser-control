//-----------------------------------------------------------------
// 程序描述:
//     GPIO驱动程序
// 作    者:
// 开始日期: 2022-12-28
// 完成日期: 2022-12-28
// 修改日期: 2022-12-28
// 当前版本: V1.0.1
// 历史版本:
//  - V1.0: (2014-02-07)LED IO 配置
// - V1.0.1:(2014-02-16)头文件中不包含其他头文件
// 调试工具: 凌智STM32核心开发板、LZE_ST_LINK2
// 说    明:
//
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// 头文件包含
//-----------------------------------------------------------------
#include <stm32f10x.h>
#include "GPIO_Init_Configuration.h"

//-----------------------------------------------------------------
// 初始化程序区
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//
// 函数功能: GPIO配置
// 入口参数: 无
// 返 回 值: 无
// 全局变量: 无
// 调用模块: RCC_APB2PeriphClockCmd();GPIO_Init();
// 注意事项: 无
//
//-----------------------------------------------------------------
void GPIO_Init_Configuration(void)
{
  int i = 0;
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructureLight; // 激光模块 这里采用了A组

  // 使能IO口时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  // GPIOC的0|1|2|3|4|5打开
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  // 推挽输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  PC0_ON;
  PC1_ON;
  PC2_ON;
  PC3_ON;
  PC4_ON;
  PC5_ON;
  // 初始化GPIO口

  // GPIOA的0~7口打开
  GPIO_InitStructureLight.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructureLight.GPIO_Speed = GPIO_Speed_50MHz;
  // 推挽输出
  GPIO_InitStructureLight.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructureLight);
  // 初始化GPIO口
  for (i = 0; i <= 10; i++)
  {
    PA_OFF(i);
  }
}

//-----------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------
