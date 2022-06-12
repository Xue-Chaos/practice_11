/* ����ͷ�ļ� */
#include "ioCC2530.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* �ĸ�LED�ƵĶ˿ڶ��� */
#define D3 P1_0
#define D4 P1_1
#define D5 P1_3
#define D6 P1_4
/* �����Ķ˿ڶ��� */
#define SW1 P1_2


/* �������� */
unsigned char workMode=0;     //ϵͳ����ģʽ:0�� ֹͣģʽ  1�� ����ģʽ
unsigned int  counter=0;      //��ʱ���жϼ�����

unsigned char uart_rx_buf[20];//���ڽ��ջ�����
unsigned char uart_rx_len=0;  //���ڽ����ַ�����
unsigned char uart_data_len=0;//���ڽ���ʱ����޺�����ݸ���
unsigned char temp;           //�Ӵ��ڽ���һ���ַ�

unsigned char dat;//�����޺����ʾЧ��ֵ
unsigned char showIndex=0;    //�޺��Ч��λ��
unsigned char showLen=2;      //Ԥ��2���޺��Ч���������������δ���Ϳ���ָ�����ʾԤ���Ч��
unsigned char showBuf[20]={0x01,0x03};   
                              /* showBuf����޺����ʾЧ������ Ԥ��2����ʾЧ��
                                              D4 D3 D6 D5 
                                 0x01 -> 0000 0  0  0  1   D5��
                                 0x03 -> 0000 0  0  1  1   D6 D5ͬʱ��
                                 ....                      �Դ����� ��4λ����4��LED
                              */
unsigned char sendByte=0;     //��ʾÿ��Ч��ʱ��ͨ�����������ǰЧ��ֵ


/**********LED�˿ڳ�ʼ��************************/
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

/***********��ʱ����ʼ��************************/
void InittTimer1(void)
{
    T1IF=0;                  //���timer1�жϱ�־
    T1STAT &= ~0x01;         //���ͨ��0�жϱ�־
    
    T1CTL = 0x0A;            //����32��Ƶ��ģ�Ƚϼ�������ģʽ��������ģʽ
    
    T1CCTL0 |= 0x04;         //�趨timer1ͨ��0�Ƚ�ģʽ
    T1CC0L = 10000&0xFF;     //��10000�ĵ�8λд��T1CC0L
    T1CC0H = (10000>>8)&0xFF;//��10000�ĸ�8λд��T1CC0H
    
    T1IE = 1;                //ʹ�ܶ�ʱ��1���жϣ�����дΪIEN1 |= 0x02;
}

/**********����ɨ�� ÿ10msִ��һ��*********************/
void ScanKey(void)
{
    /*.......������3 ��ʼ��Ҫ��ʵ�ְ���ɨ��
    workMode��ʾ������ʽ��Ĭ��Ϊ0��ʾֹͣģʽ
    ��1�� ���빤��ģʽworkMode��1����ʱ showIndexֵҪ��0��ʼ
    �ٰ�1�Σ�����ֹͣģʽworkMode��0  
  ÿ�ΰ��¶�ʱ����ֵcounterҪ��0
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
    /*.......������3 ����.......................*/
}

/**********����ͨ�ų�ʼ��************************/
void InitUART0(void)
{
    PERCFG = 0x00; //usart0 ʹ�ñ���λ��1 TX-P0.3 RX-P0.2  
    P0SEL |= 0x0c; //P0.2 P0.3 �������蹦��
    U0CSR |= 0xC0; //uartģʽ �������
    P2DIR &= ~0xC0;//P0������ΪUART��ʽ  
   
    U0BAUD = 216;
    U0GCR = 10;
    
    U0UCR |= 0x80; //������ ����żУ�� 8λ����λ  1λֹͣλ
    URX0IF = 0;    //����UART0 RX�жϱ�־
    UTX0IF = 0;    //����UART0 TX�жϱ�־
    URX0IE = 1;    //ʹ�ܽ����ж�
}


/************��ʱ��T1�жϷ����ӳ���**************/
#pragma vector = T1_VECTOR //�жϷ����ӳ���
__interrupt void T1_ISR(void)
{   
    counter++;
    ScanKey();
}



/*************** �����ڷ���ָ�����ȵ�����  ***************/
void uart_tx_string(char *data_tx,int len)  
{   
  unsigned int j;  
  for(j=0;j<len;j++)  
  {   
    U0DBUF = *data_tx++; // ��Ҫ���͵�1�ֽ�����д��U0DBUF
    while(UTX0IF == 0);  // �ȴ�TX�жϱ�־����U0DBUF����
    UTX0IF = 0;          // ����TX�жϱ�־
  }
}

void uart_tx_byte(char data_tx)  
{   
    U0DBUF = data_tx;   // ��Ҫ���͵�1�ֽ�����д��U0DBUF
    while(UTX0IF == 0); // �ȴ�TX�жϱ�־����U0DBUF����
    UTX0IF = 0;         // ����TX�жϱ�־
} 


/************UART0 �����жϷ����ӳ���**************/

#pragma vector = URX0_VECTOR //�ж�����
__interrupt void UART0_RX_ISR(void)
{
  
    URX0IF=0;
    temp = U0DBUF;
  
    
    if((temp&0xf0) == 0xf0)//�ж��ǲ�����F��ͷ
    {
      uart_rx_len=0;
      uart_data_len=temp&0x0f;//ȡ���޺��Ч��������
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
            
            U0CSR &= ~0x40;//��ֹ����
            
            uart_tx_stirng(showBuf, showLen);
            
            U0CSR |= 0x40;//�������
        }
        
        if(uart_rx_len>sizeof(uart_rx_buf))
            uart_rx_len=0; 
    }
}


/************ֹͣģʽ**************/
void StopMode(void)
{
    //�ɿ�����д ��ֹͣģʽʱ��D5����˸����0.5s��0.5s����������Ϩ��
    /*.......������7 ��ʼ��....................*/
    D5 = ~D5;
    D3 = D4 = D6 = 0;
    counter = 0;
    while (counter != 50);
   /*.......������7 ����.......................*/
}

/***************����ģʽ*****************/
void WorkMode(void)
{
   
    if(counter<100)
    {
        D3 = D4 = D5 = D6 = 1;
    
        sendByte=1;//���ý��빤��ģʽʱ��ֵ����sendByte=1ʱ������빤��ģʽ
    }
    else
    {
      dat=showBuf[showIndex];//ȡ��һ���޺��Ч������ֵ��ŵ�����dat��
        /*.......������9 ��ʼ��
      ��������ʱ����һ���޺�����ݷ��ظ����ڣ�
      ���������޺��Ч������ֵ��ʾ��4��LED���ϡ�....................*/ 
        //1.ÿһ���޺�����ݷ��ظ�����
        if(sendByte)
        {
            uart_tx_byte(dat);
            sendByte=0;
        }
        
        //2.ʵ�ֶ�Ӧ���޺��Ч��
        if(dat&0x01) D5=1;else D5=0;
        if(dat&0x02) D6=1;else D6=0;
        if(dat&0x04) D3=1;else D3=0;
        if(dat&0x08) D4=1;else D4=0;
     /*.......������9 ����.......................................*/
    
        if(counter>=200){
            counter=100;
            showIndex++;
            if(showIndex>=showLen)showIndex=0;
            
            sendByte=1;
        }
    }
  }

/************main�������**************************/
void main(void)
{
    //ʱ�ӳ�ʼ��,�ٶ�����Ϊ32MHz
    CLKCONCMD &= 0X80;
    while(CLKCONSTA&0X40);
    
    InitLED();
    InitKey();
    InittTimer1();      //��ʼ��Timer1
    InitUART0();        //UART0��ʼ��

    EA = 1;             //ʹ��ȫ���ж�
    
    while(1)
    {
        if (workMode == 0)
            StopMode();
        if (workMode == 1)
            WorkMode();
    }
}
