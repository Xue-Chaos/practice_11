/* 包含头文件 */
#include "ioCC2530.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* 四个LED灯的端口定义 */
#define D3 P1_0
#define D4 P1_1
#define D5 P1_3
#define D6 P1_4
/* 按键的端口定义 */
#define SW1 P1_2


/* 变量定义 */
unsigned char workMode=0;     //系统工作模式:0： 停止模式  1： 工作模式
unsigned int  counter=0;      //定时器中断计数器

unsigned char uart_rx_buf[20];//串口接收缓冲区
unsigned char uart_rx_len=0;  //串口接收字符长度
unsigned char uart_data_len=0;//串口接收时存放霓虹灯数据个数
unsigned char temp;           //从串口接收一个字符

unsigned char dat;//单个霓虹灯显示效果值
unsigned char showIndex=0;    //霓虹灯效果位置
unsigned char showLen=2;      //预设2种霓虹灯效果，如果串口助手未发送控制指令，将显示预设的效果
unsigned char showBuf[20]={0x01,0x03};   
                              /* showBuf存放霓虹灯显示效果数据 预存2种显示效果
                                              D4 D3 D6 D5 
                                 0x01 -> 0000 0  0  0  1   D5亮
                                 0x03 -> 0000 0  0  1  1   D6 D5同时亮
                                 ....                      以此类推 低4位控制4个LED
                              */
unsigned char sendByte=0;     //显示每种效果时，通过串口输出当前效果值


/**********LED端口初始化************************/
void InitLED(void)
{
    P1SEL &= ~0x1b;
    P1DIR |= 0x1b;
    D3 = D4 = D5 = D6 = 0;
}

void InitKey(void)
{
    P1SEL &= ~0x04;
    P1DIR &= ~0x04;
    P1INP &= ~0x04;
    P2INP &= ~0x40;
}

/***********定时器初始化************************/
void InittTimer1(void)
{
    T1IF=0;                  //清除timer1中断标志
    T1STAT &= ~0x01;         //清除通道0中断标志
    
    T1CTL = 0x0A;            //配置32分频，模比较计数工作模式，正计数模式
    
    T1CCTL0 |= 0x04;         //设定timer1通道0比较模式
    T1CC0L = 10000&0xFF;     //把10000的低8位写入T1CC0L
    T1CC0H = (10000>>8)&0xFF;//把10000的高8位写入T1CC0H
    
    T1IE = 1;                //使能定时器1的中断，或者写为IEN1 |= 0x02;
}

/**********键盘扫描 每10ms执行一次*********************/
void ScanKey(void)
{
    /*.......答题区3 开始：要求实现按键扫描
    workMode表示工作方式，默认为0表示停止模式
    按1次 进入工作模式workMode置1，此时 showIndex值要从0开始
    再按1次，返回停止模式workMode置0  
  每次按下定时计数值counter要清0
    ....................*/
    if (SW1 == 0)
    {
        if (workMode == 0)
        {
            workMode = 1;
            showIndex = 0;
        }
        else
        {
            workMode = 0;
        }
        counter = 0;
    }
    /*.......答题区3 结束.......................*/
}

/**********串口通信初始化************************/
void InitUART0(void)
{
    PERCFG = 0x00; //usart0 使用备用位置1 TX-P0.3 RX-P0.2  
    P0SEL |= 0x0c; //P0.2 P0.3 用于外设功能
    U0CSR |= 0xC0; //uart模式 允许接收
    P2DIR &= ~0xC0;//P0优先作为UART方式  
   
    U0BAUD = 216;
    U0GCR = 10;
    
    U0UCR |= 0x80; //流控无 无奇偶校验 8位数据位  1位停止位
    URX0IF = 0;    //清零UART0 RX中断标志
    UTX0IF = 0;    //清零UART0 TX中断标志
    URX0IE = 1;    //使能接收中断
}


/************定时器T1中断服务子程序**************/
#pragma vector = T1_VECTOR //中断服务子程序
__interrupt void T1_ISR(void)
{   
    counter++;
    ScanKey();
}



/*************** 往串口发送指定长度的数据  ***************/
void uart_tx_string(char *data_tx,int len)  
{   
  unsigned int j;  
  for(j=0;j<len;j++)  
  {   
    U0DBUF = *data_tx++; // 将要发送的1字节数据写入U0DBUF
    while(UTX0IF == 0);  // 等待TX中断标志，即U0DBUF就绪
    UTX0IF = 0;          // 清零TX中断标志
  }
}

void uart_tx_byte(char data_tx)  
{   
    U0DBUF = data_tx;   // 将要发送的1字节数据写入U0DBUF
    while(UTX0IF == 0); // 等待TX中断标志，即U0DBUF就绪
    UTX0IF = 0;         // 清零TX中断标志
} 


/************UART0 接收中断服务子程序**************/

#pragma vector = URX0_VECTOR //中断向量
__interrupt void UART0_RX_ISR(void)
{
  
    URX0IF=0;
    temp = U0DBUF;
  
    
    if((temp&0xf0) == 0xf0)//判读是不是以F开头
    {
      uart_rx_len=0;
      uart_data_len=temp&0x0f;//取出霓虹灯效果的组数
    }
    
    
    else
    {
        uart_rx_buf[uart_rx_len++] = temp;
        
        if(uart_rx_len==uart_data_len)
        {
            memcpy(showBuf,uart_rx_buf,uart_data_len);
            showLen=uart_data_len;
            showIndex=0;
            sendByte=1;
            
            U0CSR &= ~0x40;//禁止接收
            
            uart_tx_stirng(showBuf, showLen);
            
            U0CSR |= 0x40;//允许接收
        }
        
        if(uart_rx_len>sizeof(uart_rx_buf))
            uart_rx_len=0; 
    }
}


/************停止模式**************/
void StopMode(void)
{
    //由考生编写 ，停止模式时，D5灯闪烁（亮0.5s灭0.5s），其他灯熄灭
    /*.......答题区7 开始：....................*/
    D5 = ~D5;
    D3 = D4 = D6 = 0;
    counter = 0;
    while (counter != 50);
   /*.......答题区7 结束.......................*/
}

/***************工作模式*****************/
void WorkMode(void)
{
   
    if(counter<100)
    {
        D3 = D4 = D5 = D6 = 1;
    
        sendByte=1;//设置进入工作模式时的值，当sendByte=1时代表进入工作模式
    }
    else
    {
      dat=showBuf[showIndex];//取出一组霓虹灯效果数据值存放到变量dat中
        /*.......答题区9 开始：
      正常工作时，将一组霓虹灯数据返回给串口，
      并将该组霓虹灯效果数据值显示到4个LED灯上。....................*/ 
        //1.每一组霓虹灯数据返回给串口
        if(sendByte)
        {
            uart_tx_byte(dat);
            sendByte=0;
        }
        
        //2.实现对应的霓虹灯效果
        if(dat&0x01) D5=1;else D5=0;
        if(dat&0x02) D6=1;else D6=0;
        if(dat&0x04) D3=1;else D3=0;
        if(dat&0x08) D4=1;else D4=0;
     /*.......答题区9 结束.......................................*/
    
        if(counter>=200){
            counter=100;
            showIndex++;
            if(showIndex>=showLen)showIndex=0;
            
            sendByte=1;
        }
    }
  }

/************main函数入口**************************/
void main(void)
{
    //时钟初始化,速度设置为32MHz
    CLKCONCMD &= 0X80;
    while(CLKCONSTA&0X40);
    
    InitLED();
    InitKey();
    InittTimer1();      //初始化Timer1
    InitUART0();        //UART0初始化

    EA = 1;             //使能全局中断
    
    while(1)
    {
        if (workMode == 0)
            StopMode();
        if (workMode == 1)
            WorkMode();
    }
}
