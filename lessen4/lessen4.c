#include<iocc2530.h>
#include"74LS164_8LED.h"
void delayus()
{
  char k = 63;
  while(k--);
}
void delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<800;j++);
}
void Init32M()
{
  SLEEPCMD &= 0xFB;//1111 1011 开启2个高频时钟源
  while(0 == (SLEEPSTA & 0x40));//0100 0000等待32M稳定
  delayus();
  CLKCONCMD &=0xF8;//1111 1000不分频输出
  CLKCONCMD &=0xBF;//1011 1111设置32M作为系统主时钟
  while(CLKCONSTA & 0x40);//0100 0000等待32M成功成为当前系统主时钟
  SLEEPCMD |=0x40;//0000 0100(关闭片内16M的时钟源）
}
void main()
{
  char i;
  LS164_Cfg();//LED初始化
  Init32M();//配置片外32M晶振
  while(1)
  {
    for(i=0;i<10;i++)
    {
      LS164_BYTE(i);
      delay();
      
    }
  }
  
}