#include<iocc2530.h>
void delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<30;j++);
}
void main()
{
  P0SEL &=0xDF;//1101 1111//P0_5普通IO口
  P0DIR &=0xDF;//1101 1111//P0_5输入模式
  P0INP &=0xDF;//1101 1111//P0_5上下拉模式
  P2INP &=0xDF;//1101 1111//P0_5上拉电阻
  EA = 1;//中断总开关
  P0IE = 1;//P0组中断开关
  P0IEN |= 0x20;//0010 0000//P0_5中断开关
  PICTL |= 0x01;//0000 0001把P0这一组配置成下降沿触发
  
  P1DIR |= 0x01;//0000 0001设置P1_0输出模式
  while(1);
}
//中断函数
#pragma vector = P0INT_VECTOR
__interrupt void fsdfas()
{
  if(P0IFG & 0x20)//0010 0000//判断P0_5是否有中断
  {
    //P0组的P0_5引发了外部中断
    delay();
    if(0==P0_5)
    {
      //说明确实是连接在P0_5的按钮触发了外部中断
      P1_0 ^= 1;//异或操作
    }
    
  }
  P0IFG = 0;
  P0IF = 0;
}