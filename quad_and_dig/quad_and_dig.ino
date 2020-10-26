#include <Servo.h> 
/*
 * This is a servo and ECO signal manager for dual servo/dual ESC vehicles.
 * 
 * Rear servo signal is the front signal normal/zero/reversed depending on
 * the servo modifier. For quad steer/normal/crabwalk.
 * 
 * Rear ESC signal is the front signal normal/reduced/zero depending on 
 * the esc modifier. This is a "dig" controller.
 * 
 */

// For rear servo
volatile unsigned long leadingedge1;
volatile unsigned long leadingedge2;
volatile int pulsetime1; 
volatile int pulsetime2;

// For rear esc
volatile unsigned long leadingedge3;
volatile unsigned long leadingedge4;
volatile int pulsetime3; 
volatile int pulsetime4;


//declare servo pins
int servoin1  = 2; // pin 2 - steering - signal from esc channel 1
int servoin2  = 3; // pin 3 - inversion channel - signal from esc channel used for quadsteer/crab/norm
int servoout = 9; // output rear servo is pin 9, front is pin 2 
int escin1  = 5; // pin 5 - esc - signal from esc channel 2
int escin2  = 6; // pin 6 - esc multiplier channel - signal from esc channel used for ESC dig
int escout = 11; // output rear esc is pin 11, front is pin 5
Servo rearservo;
Servo rearesc;

String outline;  //for debug output
int ch1_hist[3]; //servo input 1
int ch2_hist[3]; //servo input 2
int ch3_hist[3]; //esc input 1
int ch4_hist[3]; //esc input 2
int timeoffset1=0; //time offset is subtracted from timed pulses due to the board adding time
int timeoffset2=0;
int timeoffset3=0;
int timeoffset4=0;
int timeaccum1=0; //sum of pulse timings for initializing offsets
int timeaccum2=0;
int timeaccum3=0;
int timeaccum4=0;

long RServo=1500; //value to write to rear servo
long RESC=1500;   //value to write to rear servo

void setup()
{
  pinMode(servoin1, INPUT);    // digital pin for front servo
  pinMode(servoin2, INPUT);    // set pin for servo modifier
  pinMode(servoout, OUTPUT);   // pin for rear servo
  pinMode(escin1, INPUT);      // digital pin for front ESC
  pinMode(escin2, INPUT);      // set pin for ESC modifier
  pinMode(escout, OUTPUT);     // pin for rear ESC
  leadingedge1 = 0;//servo leading edge
  leadingedge2 = 0;//servo manipulator leading edge
  leadingedge3 = 0;//esc leading edge
  leadingedge4 = 0;//esc manipulator leading edge
  pulsetime1 = 1500;//servo pulse timer
  pulsetime2 = 1500;//servo inversion pulse timer
  pulsetime3 = 1500;//esc pulse timer
  pulsetime4 = 1500;//esc multiplier pulse timer
  timeoffset1=0;  //init the time offsets to 0
  timeoffset2=0;
  timeoffset3=0;
  timeoffset4=0;
  timeaccum1=0;   //set accumulation for pulse timings to 0
  timeaccum2=0;
  timeaccum3=0;
  timeaccum4=0;
  attachInterrupt(digitalPinToInterrupt(servoin1), chan1, CHANGE); // attach interrupts
  attachInterrupt(digitalPinToInterrupt(servoin2), chan2, CHANGE); // these are inputs
  attachInterrupt(digitalPinToInterrupt(escin1), chan3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(escin2), chan4, CHANGE);
  rearservo.attach(servoout); // attach output pins
  rearesc.attach(escout);
  Serial.begin(115200);       // for debugging
  int offcnt;
  delay(1000);                // sleep a little at the start. assume stuff isn't working yet
  for (offcnt=0;offcnt<100;offcnt++){
    delay(20);               // 2 second wait to gather pulse timings to determine offset
    timeaccum1-=1500-pulsetime1;
    timeaccum2-=1500-pulsetime2;
    timeaccum3-=1500-pulsetime3;
    timeaccum4-=1500-pulsetime4;
  }
  timeoffset1=int(timeaccum1/100); //Assume controls are all neutral
  timeoffset2=int(timeaccum2/100);
  timeoffset3=int(timeaccum3/100);
  timeoffset4=int(timeaccum4/100);
}

int amode(int a[]){ //mode or average of an array - use to smooth glitches
  if (a[0]==a[1]) { return a[0]; }
  if (a[0]==a[2]) { return a[0]; }
  if (a[1]==a[2]) { return a[1]; }
  return int((a[0]+a[1]+a[2])/3);
}

int pusharr(int a[],int pushval) {
  a[2]=a[1];
  a[1]=a[0];
  a[0]=pushval;
  return 0;
}

void dostuff(){
  RServo = 1500+((((long)pulsetime1-1500)*((long)pulsetime2-1500))/500); // multiply rear steering by -1, 0 or 1
  RESC = 1500+((((long)pulsetime3-1500)*((long)pulsetime4-1000))/1000); // multiply rear esc by 0, 1/2 or 1
  //if (RESC > pulsetime3) { RESC = pulsetime3; } // dont lock the reverse
  outline="Sin:"+String(pulsetime1)+"  Smod:"+String(pulsetime2)+"  SRear:"+String(RServo);
  outline+="   Ein:"+String(pulsetime3)+"  Emod:"+String(pulsetime4)+"  ERear:"+String(RESC);
  outline+="   off1:"+String(timeoffset1)+" off2:"+String(timeoffset2)+" off3:"+String(timeoffset3)+" off4:"+String(timeoffset4);
  Serial.print(outline);
  Serial.print("\n" );
}

void chan1() // read pulse by timing rise/fall of pulse edge
{
  if(digitalRead(servoin1) == HIGH)
  {
    leadingedge1 = micros();
  } else {
    if (leadingedge1 > 0)
    {
      pulsetime1 = (micros() - leadingedge1)-timeoffset1; //us added from other operations? center needed normalizing to 1500
      pusharr(ch1_hist,pulsetime1); // push to array
      leadingedge1 = 0; // reset pulse timer
      pulsetime1 = amode(ch1_hist); // take the mode of the last 3 values. This is to smooth any gitches.
    }
  }
}

void chan2() // limited to 1000-2000 to limit the multiplier
{
  if(digitalRead(servoin2) == HIGH)
  {
    leadingedge2 = micros();
  }
  else
  {
    if(leadingedge2 > 0)
    {
      pulsetime2 = (micros() - leadingedge2)-timeoffset2;
      if (pulsetime2 > 2000) {pulsetime2=2000;}
      if (pulsetime2 < 1000) {pulsetime2=1000;}
      pusharr(ch2_hist,pulsetime2);
      pulsetime2 = amode(ch2_hist);
      leadingedge2 = 0;
    }
  }
}

void chan3() //as in chan1
{
  if(digitalRead(escin1) == HIGH)
  {
    leadingedge3 = micros();
  }
  else
  {
    if(leadingedge3 > 0)
    {
      pulsetime3 = (micros() - leadingedge3)-timeoffset3;
      pusharr(ch3_hist,pulsetime3);
      leadingedge3 = 0;
      pulsetime3 = amode(ch3_hist);
    }
  }
}

void chan4() //limited like chan2
{
  if(digitalRead(escin2) == HIGH)
  {
    leadingedge4 = micros();
  }
  else
  {
    if(leadingedge4 > 0)
    {
      pulsetime4 = (micros() - leadingedge4)-timeoffset4;
      if (pulsetime4 > 2000) {pulsetime4=2000;}
      if (pulsetime4 < 1000) {pulsetime4=1000;}
      pusharr(ch4_hist,pulsetime4);
      pulsetime4 = amode(ch4_hist);
      leadingedge4 = 0;
    }
  }
}

void loop()
{  
  delay(20);  //delay is non blocking, interrupts ok
  dostuff();  //do stuff after the receiver has sent all pulses
  rearservo.write(RServo);
  rearesc.write(RESC);
}
