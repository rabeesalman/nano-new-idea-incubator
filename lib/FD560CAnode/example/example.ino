////the ca display need transistors(pnp)array
#include <FD650CA.h>
FD650CA fd(A1,A0);
uint16_t num=0;
void setup()
{
  fd.clean();
  delay(300);
  fd.allOn_bri(8,1); // put your setup code here, to run once:
}

void loop()
{
  fd.shownum(0,num%10,0);
  fd.shownum(1,num/10%10,1);
  fd.shownum(2,num/100%10,0);
  fd.shownum(3,num/1000%10,0);
  delay(100);
  num++;
}
