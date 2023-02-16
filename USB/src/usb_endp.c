/**
 ******************************************************************************
 * @file    usb_endp.c
 * @author  MCD Application Team
 * @version V4.0.0
 * @date    21-January-2013
 * @brief   Endpoint routines
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "control.h"
#include "GPIO_Init_Configuration.h"
#include "R_data.h"
#include "Delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL 5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 Receive_Flag;     // 接收数据成功标志位
u16 Receive_Num;     // 接收到的数据个数
u16 Laser_Power = 0; // 激光器的能量值

u16 i;     // 循环变量
u16 temp1; // 临时变量
u16 temp2;

uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
extern uint8_t USART_Rx_Buffer[];
extern uint32_t USART_Rx_ptr_out;
extern uint32_t USART_Rx_length;
extern uint8_t USB_Tx_State;
extern void USB_Send_string(u8 *data_buffer, u8 N);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
 * Function Name  : EP1_IN_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void EP1_IN_Callback(void) // EP1_IN_Callback 回调函数
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;

  if (USB_Tx_State == 1)
  {
    if (USART_Rx_length == 0)
    {
      USB_Tx_State = 0;
    }
    else
    {
      if (USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE)
      {
        USB_Tx_ptr = USART_Rx_ptr_out;
        USB_Tx_length = VIRTUAL_COM_PORT_DATA_SIZE;

        USART_Rx_ptr_out += VIRTUAL_COM_PORT_DATA_SIZE;
        USART_Rx_length -= VIRTUAL_COM_PORT_DATA_SIZE;
      }
      else
      {
        USB_Tx_ptr = USART_Rx_ptr_out;
        USB_Tx_length = USART_Rx_length;

        USART_Rx_ptr_out += USART_Rx_length;
        USART_Rx_length = 0;
      }
      UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], ENDP1_TXADDR, USB_Tx_length);
      SetEPTxCount(ENDP1, USB_Tx_length);
      SetEPTxValid(ENDP1);
    }
  }
}

u32 Instruction_Decoding(void) // 指令编码函数
{
  u32 returning = 0;
  if (USB_Rx_Buffer[0] == (uint8_t)'1' && USB_Rx_Buffer[1] == (uint8_t)'1')
  {
    returning += 1; // Power
  }
  else if (USB_Rx_Buffer[0] == (uint8_t)'0' && USB_Rx_Buffer[1] == (uint8_t)'0')
  {
    returning += 0; // Resistance
  }
  else
  {
    return (u32)-1; // Error
  }
  returning *= 10;
  if (USB_Rx_Buffer[2] == (uint8_t)'1' && USB_Rx_Buffer[3] == (uint8_t)'1')
  {
    returning += 1; // Up
  }
  else if (USB_Rx_Buffer[2] == (uint8_t)'0' && USB_Rx_Buffer[3] == (uint8_t)'0')
  {
    returning += 0; // Down
  }
  else if (USB_Rx_Buffer[2] == (uint8_t)'2' && USB_Rx_Buffer[3] == (uint8_t)'2')
  {
    returning += 2; // Set
  }
  else if (USB_Rx_Buffer[2] == (uint8_t)'3' && USB_Rx_Buffer[3] == (uint8_t)'3')
  {
    returning += 3; // Set
  }
  else if (USB_Rx_Buffer[2] == (uint8_t)'4' && USB_Rx_Buffer[3] == (uint8_t)'4')
  {
    returning += 4; // Set
  }
  else
  {
    return (u32)-1; // Error
  }
  return returning;
}

void Resistance_Change(u16 target) // 电位器调整函数
{
  if (target < 961 && target > 0)
  {

    for (i = 0; i < 100; i++)
    {
      DOWN_024();
      DOWN_135();
    }
    for (i = 0; i < r1[target]; i++)
    {
      UP_024();
    }
    for (i = 0; i < r2[target]; i++)
    {
      UP_135();
    }
  }
}

void Laser_Power_Change(u16 target) // 激光功率调整函数
{
  int j;
  int temp_laser;
  PA_OFF(9); // 这边假设是置1锁功率，如果是置零锁功率的话还要调整
  for (j = 0; j <= 7; j++)
  {
    temp_laser = (target & 1);
    target >>= 1;
    if (temp_laser)
    {
      PA_ON(j);
    }
    else
    {
      PA_OFF(j);
    }
  }
  PA_ON(9);
}

u8 *Message_compile(u16 target, u8 *message) // 数值至字符串转换
{
  u16 index = 0;
  message[index] = target % 10 + '0';
  while ((u16)(target /= 10))
  {
    index += 1;
    message[index] = target % 10 + '0';
  }
  message[index + 1] = 0;
  return message;
}

u8 *Message_Info(u16 number, u8 *message) // info的信息填充
{                                         // 真想换一种方式，这也太丑了
  switch (number)
  {
  case 1: // 激光器成功开启
    message[0] = "L";
    message[1] = "a";
    message[2] = "s";
    message[3] = "e";
    message[4] = "r";
    message[5] = " ";
    message[6] = "O";
    message[7] = "N";
    message[8] = 0;
    break;
  case 2: // 激光器成功关闭
    message[0] = "L";
    message[1] = "a";
    message[2] = "s";
    message[3] = "e";
    message[4] = "r";
    message[5] = " ";
    message[6] = "O";
    message[7] = "F";
    message[8] = "F";
    message[9] = 0;
    break;
  }
}

u8 *Message_error(u8 *message) // 错误信息填充
{
  message[0] = 'E';
  message[1] = 'r';
  message[2] = 'r';
  message[3] = 'o';
  message[4] = 'r';
  message[5] = 0;
  return message;
}

u16 Message_len(u8 *message) // 信息长度
{
  u16 len = 0;
  while (message[len] != 0)
  {
    len++;
    if (len >= 128)
    {
      break;
    }
  }
  return len;
}

/*******************************************************************************
 * Function Name  : EP3_OUT_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void EP3_OUT_Callback(void)
{
  uint16_t USB_Rx_Cnt;             // 接收数据个数
  uint8_t Message_send[128] = {0}; // 发送缓冲区

  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = USB_SIL_Read(EP3_OUT, USB_Rx_Buffer);

  if (USB_Rx_Cnt != 0)
  {
    Receive_Flag = 1;         // 接收标志
    Receive_Num = USB_Rx_Cnt; // 接收数据大小
  }
  else
    Receive_Flag = 0;
  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the USART Xfer */
  if (Receive_Flag)
  {
    switch (Instruction_Decoding())
    {
    case 1: // 调大电阻
      if (R_index < 961)
      {
        R_index++;
        Message_compile(R_index, Message_send);
      }
      break;

    case 0: // 调小电阻
      if (R_index > 0)
      {
        R_index--;
        Message_compile(R_index, Message_send);
      }
      break;

    case 2: // 电阻调整至对应值
      if (USB_Rx_Buffer[4] <= (uint8_t)'9')
      {
        R_index = (USB_Rx_Buffer[4] - '0') * 100 + (USB_Rx_Buffer[5] - '0') * 10 + (USB_Rx_Buffer[6] - '0');
        Message_compile(R_index, Message_send);
      }
      else
      {
        Message_error(Message_send);
      }
      break;

    case 10: // 调小激光功率
      if (Laser_Power > 0)
      {
        Laser_Power--;
        Message_compile(Laser_Power, Message_send);
      }
      break;

    case 11: // 调大激光功率
      if (Laser_Power < 255)
      {
        Laser_Power++;
        Message_compile(Laser_Power, Message_send);
      }
      break;

    case 12: // 激光功率调整到对应值

      temp1 = (USB_Rx_Buffer[5] - '0') * 100 + (USB_Rx_Buffer[6] - '0') * 10 + (USB_Rx_Buffer[7] - '0');
      temp2 = (USB_Rx_Buffer[9] - '0') * 100 + (USB_Rx_Buffer[10] - '0') * 10 + (USB_Rx_Buffer[11] - '0');
      if ((temp1 == temp2) && temp1 >= 1 && temp1 <= 255)
      {
        Laser_Power = temp1;
        Message_compile(Laser_Power, Message_send);
      }
      else
      {
        Message_error(Message_send);
      }
      break;
    case 13:                         // 打开激光器
      PA_ON(9);                      // 先打开18针
      Delay_1ms(5);                  // 按照说明需要延迟5ms
      PA_ON(10);                     // 再打开19针
      Message_Info(1, Message_send); // 填充成功开启激光器
      break;
    case 14:                         // 关闭激光器
      PA_OFF(10);                    // 先关闭19针
      Delay_1ms(5);                  // 这边八成也要延时5ms
      PA_OFF(9);                     // 再关闭18针
      Message_Info(2, Message_send); // 填充成功关闭激光器
      break;
    default: // 其他情况报错
      Message_error(Message_send);
    }
    Resistance_Change(R_index);
    Laser_Power_Change(Laser_Power);
    USB_Send_string(Message_send, Message_len(Message_send)); // 发送消息至上位机
  }

  /* Enable the receive of data on EP3 */
  SetEPRxValid(ENDP3);
}

/*******************************************************************************
 * Function Name  : SOF_Callback / INTR_SOFINTR_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void SOF_Callback(void)
{
  static uint32_t FrameCount = 0;

  if (bDeviceState == CONFIGURED)
  {
    if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
    {
      /* Reset the frame counter */
      FrameCount = 0;

      /* Check the data to be sent through IN pipe */
      Handle_USBAsynchXfer();
    }
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
