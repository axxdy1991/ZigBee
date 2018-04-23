#include<iocc2530.h>
void delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<30;j++);
}
void main()
{
  P1SEL &= 0xFE;//1111 1110P1_0普通IO口
  P1DIR |= 0x01;//0000 0001P1_0输出状态
  P1_0 = 0;//输入低电平
  while(1)
  {
    P1_0 = 0;
    delay();
    P1_0 = 1;
    delay();
  }
  
}