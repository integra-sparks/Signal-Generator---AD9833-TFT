

// spi code for the AD9833 from here  http://www.vwlowen.co.uk/arduino/AD9833-waveform-generator/AD9833-waveform-generator.htm
// if there are other credits missed appologies given
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <URTouch.h>
///////
#define UPDATE_DELAY 3000 // 3 seconds to any storage of changes
#define tft_RST 8
#define tft_DC 9
#define TOUCH_ORIENTATION  LANDSCAPE
#define REF__FREQ 25000000.0           // On-board crystal reference frequency
const int buzzer = 16; //buzzer to massduino pin 16 using a 100ohm resistor in series
int FSYNC =2;
int tft_CS=10;
uint32_t myMillis;
uint8_t updateValues=0;
long freq;
long range;
int SETMEM;
int SCANRUN;
int M1set;
int M2set;
char M1txt [40];
char M2txt [40];
  long m1_freq;
  long m2_freq;
  int waveType;
  int m1_waveType;
  int m2_waveType; 
unsigned short x, y;
// Declare which fonts we will be using
extern uint8_t BigFont[];
//Below are the characters that will be written to the buttons
char FRQup[3] = {"F+"};
char FRQdown[4] = {"F-"};
char Range[6] = {"Range"};
char SIN[4] = {"Sin"};
char TRI[4] = {"Tri"};
char SQR[4] = {"Sqr"};
char M1[3] = {"M1"};
char M2[3] = {"M2"};
char SET[4] = {"SET"};
char SCAN[5] ={"SCAN"};
char MCLR[5] ={"MCLR"};
//Initialize TFT
//This is the bit that will change if you use a different TFT or a non touch TFT, Remember they are two seperate components so there is a bit of variation
//It should however be simple to work out as the naming of the various pins is usually similar. 
Adafruit_ILI9341 tft = Adafruit_ILI9341(tft_CS, tft_DC,tft_RST);
//Setup GFX Buttons the number 11 is important, it is the actual number of buttons +1, if you want to add a button then remember to increment this
Adafruit_GFX_Button buttons[11];
// Initialize touchscreen
// Pins using T_Clk,T_CS, T_Din, T_DO, T_IRQ 
//this is the touch screen part mentioned earlier
URTouch  myTouch( 7, 6, 5, 4, 3);



void setup(void)
{

//Set up SPI CS  
//need control of the chip selects as we have two devices on the SPI
  pinMode(FSYNC, OUTPUT);
  pinMode(tft_CS,OUTPUT);
  //setup of buzzer
//To give feedback from the touch screen, the symbols are on the small side the feedback is when the screen is touched anywhere though... !
  pinMode(buzzer, OUTPUT); // Set buzzer -  as an output
//initialize SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE2);
  delay(50);
 // Initial screen setup
//rememer to turn Serial on if you want debug information.
//Serial.begin(9600);
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_HI);
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(30,30);
  tft.setTextSize(4);
 // create the buttons // x, y, w, h, outline, fill, text
  buttons[0].initButton(&tft, 20,180,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      FRQup, 1); 
          buttons[0].drawButton();
  buttons[1].initButton(&tft, 70,180,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      FRQdown, 1); 
          buttons[1].drawButton();  
  buttons[2].initButton(&tft, 125,180,50,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      Range, 1); 
          buttons[2].drawButton();
  buttons[3].initButton(&tft, 20,220,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      SIN, 1); 
          buttons[3].drawButton();  
  buttons[4].initButton(&tft, 70,220,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      TRI, 1); 
          buttons[4].drawButton();
  buttons[5].initButton(&tft, 120,220,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      SQR, 1); 
          buttons[5].drawButton(); 
  buttons[6].initButton(&tft, 240,220,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      M1, 1); 
          buttons[6].drawButton();
  buttons[7].initButton(&tft, 290,220,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      M2, 1); 
          buttons[7].drawButton();  
  buttons[8].initButton(&tft, 190,220,40,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      SET, 1); 
          buttons[8].drawButton(); 
  buttons[9].initButton(&tft, 270,180,50,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      SCAN, 1); 
          buttons[9].drawButton(); 
  buttons[10].initButton(&tft, 270,140,50,20,ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE,
                      MCLR, 1); 
          buttons[10].drawButton(); 
//reset AD9833 and initiaze to 1Khz and Sinewave 1Khz range
AD9833reset();
//allow short time for device to settle
delay (50);
//Set up defaults as 1KHz and Sine with 1Khz range
waveType=0x2000;
range = 1000L;
freq=1000L;
//this is the bit that runs the AD9833 passing frequency and whatever 'wave' you have selected
AD9833setFrequency(freq, waveType); 
//Setting up the screen
  tft.setCursor (100,55);
  tft.print ("Settings");
  M1set=LOW;//Set up flag to show if memory is set
  M2set=LOW;
  tft.setCursor (20,135);
  tft.print ("SCAN");
  tft.setTextColor(ILI9341_RED);
  tft.setCursor (80,135);
  tft.print ("NOT ARMED");
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 195);tft.print ("*");//waveform select identifier
}

// Start main loop read buttons (IRQ triggered) and call subroutines to set selected values
void loop() {
  
 waitfortouch(&x,&y);  //Wait for a screen touch and receive co-ordinates in x & y
    //loop to identify screen has been touched an return the position, the routine then aligns this to a specific button
    //which has been previously set up then if statement are used to take specific action.
    //the piece of buzzer code is a beep beep to show a finger press has occured
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(200);        // ...for 200 msec
  noTone(buzzer);     // Stop sound...
  delay(25);        // ...for 25 msec
    tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(50);        // ...for 50 msec
  noTone(buzzer);     // Stop sound...
//   Serial.print(x); Serial.print(","); Serial.println(y); //for debugging show x & y received
   
    if (buttons[0].contains(x,y)) //switch LED dependent on which button is pressed.
    {
      frequencyUp();
    }
    if (buttons[1].contains(x,y))
    {
      frequencyDown();    
          }
    if (buttons[2].contains(x,y)) //switch LED dependent on which button is pressed.
    {

      rangeSet();   
 delay (100);
    }
    if (buttons[3].contains(x,y))
    {
      waveType=0x2000;
      AD9833setFrequency(freq,waveType);
      armUpdate();
  //Set wave marker to correct position for selection
  //the pre set ot black and re 'write' is to clear the previous selection
      tft.setTextColor(ILI9341_BLACK);
      tft.setCursor(115, 195);tft.print ("*");
      tft.setCursor(65, 195);tft.print ("*");
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(15, 195);tft.print ("*");
 
    }   
     if (buttons[4].contains(x,y)) //switch LED dependent on which button is pressed.
    {
      waveType=0x2002;
      AD9833setFrequency(freq,waveType);
  armUpdate();
  //Set wave marker to correct position for selection
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(115, 195);tft.print ("*");
        tft.setCursor(15, 195);tft.print ("*");
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(65, 195);tft.print ("*");

    
    }
    if (buttons[5].contains(x,y))
    {
      waveType=0x2028;
      AD9833setFrequency(freq,waveType);
      armUpdate();  
  //Set wave marker to correct position for selection
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(65, 195);tft.print ("*");
        tft.setCursor(15, 195);tft.print ("*");
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(115, 195);tft.print ("*");
 
       
    }   
       if (buttons[6].contains(x,y))
    {    
 useM1 ();
    }  
       if (buttons[7].contains(x,y))
    {     
 useM2 ();
    }  
       if (buttons[8].contains(x,y))
    {     
SETMEM=HIGH;
    }  
 if (buttons[9].contains(x,y))
    { 
//set up for sweeping (SCAN)   
useM1();  
SCANRUN=HIGH;
scanFrq ();
    }  
 if (buttons[10].contains(x,y))
    {   
//this clears memory parameters and sets freq and waveType to default
mem_reset();
    }  
//set screen status for SCAN to available (ARMED)

if (M1set and M2set){
if (m1_freq < m2_freq)
{
  //Clear display then write armed as both memories are set and M2 is larger than M1
    tft.fillRect(80,130,110 ,35,ILI9341_BLACK);
    tft.setCursor (80,135); 
    tft.setTextColor(ILI9341_GREEN);
    tft.print ("ARMED");
    tft.setTextColor(ILI9341_WHITE);
  //special beep for scan function
  tone(buzzer, 500); // Send 1KHz sound signal...
  delay(200);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(25);        // ...for 1sec
    tone(buzzer, 500); // Send 1KHz sound signal...
  delay(50);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  }
  else
  {
    //conditions not met to run a sweep
    tft.fillRect(80,130,110 ,35,ILI9341_BLACK);
    tft.setCursor (80,135);
    tft.setTextColor(ILI9341_RED);
    tft.print ("NOT ARMED");
    tft.setTextColor(ILI9341_WHITE);
  }
}
   
processUpdate();//millis timer update

}

// AD9833 documentation advises a 'Reset' on first applying power.
void AD9833reset() {
  WriteRegister(0x100);   // Write '1' to AD9833 Control register bit D8.
  delay(20);
}

// Set the frequency and waveform registers in the AD9833. most of this code comes from the weblink above and has been used in other version and saved a lot of learning
void AD9833setFrequency(long frequency, int Waveform) {

  long FreqWord = (frequency * pow(2, 28)) / REF__FREQ;

  int MSB = (int)((FreqWord & 0xFFFC000) >> 14);    //Only lower 14 bits are used for data
  int LSB = (int)(FreqWord & 0x3FFF);

  //Set control bits 15 ande 14 to 0 and 1, respectively, for frequency register 0
  LSB |= 0x4000;
  MSB |= 0x4000;

delay(10);
  WriteRegister(0x2100);
  WriteRegister(LSB);                  // Write lower 16 bits to AD9833 registers
  WriteRegister(MSB);                  // Write upper 16 bits to AD9833 registers.
  WriteRegister(0xC000);               // Phase register
  WriteRegister(Waveform);             // Exit & Reset to SINE, SQUARE or TRIANGLE


  char txt[40];
 // tft.setTextColor(ILI9341_BLACK);
 //Clear frequency display area using background colour
   tft.fillRect(0,10,360,30,ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
   tft.setTextSize(4);
   tft.setCursor(2, 10);
//sprintf frequency information  into long character array called txt
  sprintf(txt, " %d,%03d,%03dHz", uint16_t(freq / 1000000), uint16_t((freq % 1000000) / 1000), uint16_t(freq % 1000));
  tft.print(txt);
  tft.setTextSize(2);
  tft.setCursor(170, 170);
//Output text to show which range has been selected, using reverse write of previous data to clear screen
  switch (range)
  {
  // this is from the original design and has been ammended to clear the old value on the TFT
    case 1: tft.setTextColor(ILI9341_BLACK);tft.print("100KHz");tft.setCursor(170, 170);tft.setTextColor(ILI9341_WHITE);tft.print("1Hz"); break;
    case 10: tft.setTextColor(ILI9341_BLACK);tft.print("1Hz"); tft.setCursor(170, 170);tft.setTextColor(ILI9341_WHITE);tft.print("10Hz");break;
    case 100: tft.setTextColor(ILI9341_BLACK);tft.print("10Hz");tft.setCursor(170, 170);tft.setTextColor(ILI9341_WHITE);tft.print("100Hz"); break;
    case 1000: tft.setTextColor(ILI9341_BLACK);tft.print("100Hz");tft.setCursor(170, 170);tft.setTextColor(ILI9341_WHITE);tft.print("1KHz");  break;
    case 10000: tft.setTextColor(ILI9341_BLACK);tft.print("1KHz");tft.setCursor(170, 170);tft.setTextColor(ILI9341_WHITE);tft.print("10KHz"); break;
    case 100000: tft.setTextColor(ILI9341_BLACK);tft.print("10KHz");tft.setCursor(170, 170);tft.setTextColor(ILI9341_WHITE);tft.print("100KHz");break;
 //
       
  }


}
// Here are all the subroutines
//First off the original write to 9833 set up, there were problems with this until the SPI.setDataMode was added here specifically 

void WriteRegister(int dat) {
  digitalWrite(tft_CS, HIGH);
  SPI.setDataMode(SPI_MODE2); 
  digitalWrite(FSYNC, LOW);           // Set FSYNC low before writing to AD9833 registers
  delayMicroseconds(20);              // Give AD9833 time to get ready to receive data.
  SPI.transfer(highByte(dat));        // Each AD9833 register is 32 bits wide and each 16
  SPI.transfer(lowByte(dat));         // bits has to be transferred as 2 x 8-bit bytes.
  digitalWrite(FSYNC, HIGH);          //Write done. Set FSYNC high
  digitalWrite(tft_CS, LOW);
}
//Set up two memory sections if already set it just uses the data otherwise it performs a store activity. 
void useM1()
{
  if (SETMEM){setM1();
  SETMEM=LOW;}
  else
  {
    AD9833setFrequency(m1_freq,m1_waveType);
     armUpdate();

  }//for else     
}
//the final AD9833 call might not be required something to look at later, note the continued use of fillRect to clear old data
void setM1()
{
    
    m1_freq=freq;
    m1_waveType=waveType;
    tft.fillRect(20,85,360 ,15,ILI9341_BLACK);
    tft.setCursor (20,85);
    tft.print ("M1");
    sprintf(M1txt, " %d,%03d,%03dHz", uint16_t(m1_freq / 1000000), uint16_t((m1_freq % 1000000) / 1000), uint16_t(m1_freq % 1000));
    tft.setCursor (40,85);
    tft.print (M1txt);
    M1set=HIGH;
    AD9833setFrequency(m1_freq, m1_waveType);   
}

void useM2()
{
  if (SETMEM){setM2();
    SETMEM=LOW;}
  else

    AD9833setFrequency(m2_freq, m2_waveType);
    armUpdate();
    
}

void setM2()
{
    m2_freq=freq;
    m2_waveType=waveType;
     tft.fillRect(20,110,360 ,15,ILI9341_BLACK);
    tft.setCursor (20,110);
    tft.print ("M2");
     tft.setCursor (40,110);
    sprintf(M2txt, " %d,%03d,%03dHz", uint16_t(m2_freq / 1000000), uint16_t((m2_freq % 1000000) / 1000), uint16_t(m2_freq % 1000));
    tft.print (M2txt);
    M2set=HIGH;
    AD9833setFrequency(m2_freq,m2_waveType);
    armUpdate();
}


void frequencyUp()
{
  if (freq < 4000000L) freq += range; else freq = 4000000L;
  AD9833setFrequency(freq, waveType);
  armUpdate();
}

void frequencyDown()
{
  if (freq >= range) freq -= range; else freq = 1L;
  AD9833setFrequency(freq, waveType);
  armUpdate();
}

void rangeReset()
{
  range = 1000L;
  AD9833setFrequency(freq, waveType);
  armUpdate();
}

void rangeSet()
{
  switch (range)
  {
    case 1: range = 10L;break;
    case 10: range = 100L;break;
    case 100: range = 1000L;break;
    case 1000: range = 10000L;break;
    case 10000: range = 100000L;break;
    case 100000: range = 1L;break;
    default: range = 1000L;break;
      
  }
  AD9833setFrequency(freq,waveType);
  armUpdate();
}


void frequencyReset()
{
  freq = 1000L;
  AD9833setFrequency(freq,waveType);
  armUpdate();
}



void waitfortouch(unsigned short int *x,unsigned short int *y){
  do
  {    
    delay(10);
    if (myTouch.dataAvailable() == true)
    {
      myTouch.read();
      *x = myTouch.getX();  //Get touch point  
      *y = myTouch.getY();
      return;
      }
  }while(myTouch.dataAvailable()==false); 
}
void armUpdate()
{
  myMillis=millis()+UPDATE_DELAY;
  updateValues=1;
}

void processUpdate()
{
  if ((updateValues==1) && (myMillis<millis()))
  {
    updateValues=0;

  }
}
//Reset Memory
void mem_reset (){
M1set=LOW;
M2set=LOW;
m1_freq=freq;
m2_freq=freq;
m1_waveType=waveType;
m2_waveType=waveType;
//most of this is self evident however it was necessary to re use the clear screen code at this point, might better be a function of its own. 
    tft.fillRect(80,130,110 ,35,ILI9341_BLACK);
    tft.setCursor (80,135);
    tft.setTextColor(ILI9341_RED);
    tft.print ("NOT ARMED");
    tft.setTextColor(ILI9341_WHITE);
tft.fillRect(20,110,360 ,20,ILI9341_BLACK);
tft.fillRect(20,85,360 ,20,ILI9341_BLACK);
tft.setCursor (20,85);
}
//Scan version this is quite simple it checks that conditions are met then increases by whatever the range is until complete with a comforting beep each time
void scanFrq (){
long tempfreq;
tempfreq=m1_freq;

while (SCANRUN)
{
if (m1_freq >= m2_freq) {SCANRUN=LOW;}
tempfreq=tempfreq+range;
if (tempfreq > m2_freq) {SCANRUN=LOW;}
else
{freq=tempfreq;//not sure why i did this
AD9833setFrequency(freq,waveType);
armUpdate();
  tone(buzzer, 2000); // Send 2KHz sound signal...
  delay(200);        // ...for 200ms
  noTone(buzzer);     // Stop sound...
  delay(25);        // ...for 25msec
    tone(buzzer, 2000); // Send 2KHz sound signal...
  delay(50);        // ...for 50 msec
  noTone(buzzer);     // Stop sound...
  }
}//scan while end
}//prog end
//------------------------------------------------------------------
