#include <Servo.h> 

volatile unsigned long leadingedge1;
volatile unsigned long leadingedge2;
volatile int pulsetime1; 
volatile int pulsetime2;


//declare servo pins
int servoin1  = 2; // pin 2 - steering
int servoin2  = 3; // pin 3 - inversion channel
int servoout1 = 9; // output read servo is pin 9, front is pin 2
Servo rearservo;


int ch1_hist[3];
int ch2_hist[3];

int execute=0;
long RServo=1500; //value to write to rear servo

void setup()
{
  pinMode(servoin1, INPUT);      // sets the digital pin 1 as input
  pinMode(servoin2, INPUT);      // sets the digital pin 1 as input
  pinMode(servoout1, OUTPUT);    // sets the digital pin 9 as output
//  pinMode(servoout2, OUTPUT);    // sets the digital pin 9 as output
  leadingedge1 = 0;
  leadingedge2 = 0;
  pulsetime1 = 1500;
  pulsetime2 = 1500;
  attachInterrupt(0, chan1, CHANGE);
  attachInterrupt(1, chan2, CHANGE);
  rearservo.attach(servoout1);
  Serial.begin(115200);        // for debugging
}

int amode(int a[]){ //mode or average of an array - use to smooth glitches
  if (a[0]==a[1]) { return a[0]; }
  if (a[0]==a[2]) { return a[0]; }
  if (a[1]==a[2]) { return a[1]; }
  return (a[0]+a[1]+a[2])/3;
}

int pusharr(int a[],int pushval) {
  a[2]=a[1];
  a[1]=a[0];
  a[0]=pushval;
  return 0;
}

void dostuff(){
  execute=0;
  RServo = 1500+((((long)pulsetime1-1500)*((long)pulsetime2-1500))/500);
  if (RServo > 2050) {
    RServo=2050;
  } else {
    if (RServo < 950) {
      RServo=950;
    }
  }
  Serial.print("P1.");
  Serial.print(pulsetime1);
  Serial.print("--P2.");
  Serial.print(pulsetime2);
  Serial.print("--SRear.");
  Serial.print(RServo);
  Serial.print("\n" );
  execute=1;
}

void chan1()
{
  if(digitalRead(servoin1) == HIGH)
  {
    leadingedge1 = micros();
  } else {
    if (leadingedge1 > 0)
    {
      pulsetime1 = ((volatile long)micros() - leadingedge1)-14; //14 us added from other operations? center needed normalizing to 1500
      if ((pulsetime1 > 800 and pulsetime1 < 2200)) {pusharr(ch1_hist,pulsetime1);}
      leadingedge1 = 0;
      pulsetime1= amode(ch1_hist);
    }
  }
}

void chan2()
{
  if(digitalRead(servoin2) == HIGH)
  {
    leadingedge2 = micros();
  }
  else
  {
    if(leadingedge2 > 0)
    {
      pulsetime2 = ((volatile long)micros() - leadingedge2)-14;
      if ((pulsetime2 > 800 and pulsetime2 < 2200)) {pusharr(ch2_hist,pulsetime2);}
      leadingedge2 = 0;
      pulsetime2= amode(ch2_hist);
    }
  }
}

void loop()
{  
  delay(2);  //delay is non blocking
  dostuff();  //do stuff after the receiver has sent all pulses
  if (execute==1) { rearservo.write(RServo);}
}

