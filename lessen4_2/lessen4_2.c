#include<iocc2530.h>
char ch;
void delayus()
{
  char k = 63;
  while(k--);
}
void Cfg32M()
{
  SLEEPCMD &= 0xFB;//1111 1011 开启2个高频时钟源
  while(0 == (SLEEPSTA & 0x40));//0100 0000等待32M稳定
  delayus();
  CLKCONCMD &=0xF8;//1111 1000不分频输出
  CLKCONCMD &=0xBF;//1011 1111设置32M作为系统主时钟
  while(CLKCONSTA & 0x40);//0100 0000等待32M成功成为当前系统主时钟
  SLEEPCMD |=0x40;//0000 0100(关闭片内16M的时钟源）
}
void UartCfg()
{
  PERCFG &= 0xFE;//1111 1110选中串口0的备用位置1
  P0SEL |= 0x0C;//0000 1100  P0_2、P0_3为片上外设功能
  U0CSR |=0xC0;//1100 0000  8个数据位、1个停止位、无流控、无效验确定
  U0GCR |= 8;
  U0BAUD |= 59;//(设置波特率9600）
  EA = 1;//开CPU总中断
  URX0IE = 1;//开串口0中断
  UTX0IF = 0;//UART0 TX中断标志初始置位0
}
void main()
{
  Cfg32M();
  UartCfg();
  while(1);
}


#pragma vector=URX0_VECTOR
__interrupt void URX0_ISR(void)
{
  URX0IF = 0;//串口0来数据的标志位，硬件会置1，需要软件清0.
  ch =U0DBUF;//从接受寄存器里取字节存入变量ch
  U0DBUF = ch;//把变量ch里的值赋给串口0发送数据寄存器
  while(0 == UTX0IF);//等待发送完成
  UTX0IF = 0;//串口0发送数据的标志位，硬件会置1，需要软件清0.
}