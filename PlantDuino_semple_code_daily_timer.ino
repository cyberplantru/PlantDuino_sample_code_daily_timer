/*
  Example code for the PlantDuino v2.0: daily timer
  
    http://www.cyberplant.info
  by CyberPlant LLC, 21 September 2014
  This example code is in the public domain.
  
  To navigate the menu items click buttons "prev" and "next". To change the parameters click buttons "up" and "down"
  
   menu item 0  .   menu item 1  .  menu item 2   .   menu item 3  .   menu item 4  .   menu item 5  .   menu item 6  
  ____________  .  ____________  .  ____________  .  ____________  .  ____________  .  ____________  .  ____________  
  | 18:41:37 |  .  | Set Hour |  .  | Set Min  |  .  | Set Date |  .  |   Mode   |  .  | Relay On |  .  | RelayOff |    
  | 21/09/14 |  .  | 18:41:37 |  .  | 18:41:37 |  .  | 21/09/14 |  .  |off       |  .  | >14:16   |  .  | >14:16   |  
  ------------  .  ------------  .  ------------  .  ------------  .  ------------  .  ------------  .  ------------  
  actual time   .                .  click         .  for correct   .  relay on,     .  click         .  click         
  and date in   .                .  button "S"    .  date change   .  off,          .  button "S"    .  button "S"    
  format        .                .  to nulling    .  add one day   .  or auto by    .  to choose     .  to choose     
  hh:mm:ss      .                .  seconds       .  Click         .  timer         .  change for    .  change for    
  dd/mm/yy      .                .                .  button "S"    .                .  hour or       .  hour or       
                .                .                .  to save the   .                .  minutes       .  minutes       
                .                .                .  date to actual.                .                .
  */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <EEPROMVar.h>
#include <EEPROMex.h> 

// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input

// ADC readings expected for the 5 buttons on the ADC input
#define SAVE_10BIT_ADC           0  // right
#define NEXT_10BIT_ADC            145  // up
#define UP_10BIT_ADC          329  // down
#define DOWN_10BIT_ADC          505  // left
#define PREV_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_SAVE              1  // 
#define BUTTON_NEXT                 2  // 
#define BUTTON_UP               3  // 
#define BUTTON_DOWN               4  // 
#define BUTTON_PREV             5  // 

/*--------------------------------------------------------------------------------------
  Variables
--------------------------------------------------------------------------------------*/
byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events


LiquidCrystal_I2C lcd(0x38,8,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
RTC_DS1307 rtc;
long previousMillis = 0;
long interval = 200;  // delay 
int i=0;

int m=0; //menu item
int sm=0;

int addressRelayMode = 0;
int addressHrOn = addressRelayMode+sizeof(int);
int addressHrOff = addressHrOn+sizeof(int);
int addressMinOn = addressHrOff+sizeof(int);
int addressMinOff = addressMinOn+sizeof(int);

int RelayMode = EEPROM.readInt(addressRelayMode);
int HrOn = EEPROM.readInt(addressHrOn);
int HrOff = EEPROM.readInt(addressHrOff);
int MinOn = EEPROM.readInt(addressMinOn);
int MinOff = EEPROM.readInt(addressMinOff);

char * mode[3] = {"on", "auto", "off"};

int hr;
int mi;
int sc;
int dy;
int mo;
int yh;
int ft=0;
int ftdy;
int ftmo;
int ftyh;
int ftmi;

void setup()
{
 
  lcd.init();                      // initialize the lcd 

  Serial.begin(57600);
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); 
#endif
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
    //button adc input
   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
   //lcd relay control


   //Print some initial text to the LCD.
   lcd.setCursor( 0, 0 );
   lcd.print( " Plant " );
  lcd.setCursor( 0, 1 );
  lcd.print( " Duino " );
   delay (2000);
   lcd.clear ();
    lcd.setCursor( 0, 0 );
   lcd.print( "ver. 2.0" );
   delay (2000);
   lcd.clear ();
   

}

void loop()
{
  unsigned long currentMillis = millis();       // for reading interval get time
  if ((currentMillis - previousMillis) > interval)
  {  
    previousMillis = currentMillis;
       DateTime now = rtc.now();
  
    
      
   hr = now.hour();
   mi = now.minute();
   sc = now.second();
   dy = now.day();
   mo = now.month();
   yh = now.year();
   
   DateTime future (now.unixtime() + ft * 86400L + 00);
   
   ftdy = future.day();
   ftmo = future.month();
   ftyh = future.year();
   

  //----------------------------------function timer----------------------------------------------------- 
   if (RelayMode==0)
{
  lcd.relayOn(); //Relay On
}
  if (RelayMode==1){
    if ((HrOff*60+MinOff)-(HrOn*60+MinOn) > 0)
{
    if ((hr*60+mi) >= (HrOn*60+MinOn) && (hr*60+mi) < (HrOff*60+MinOff))
    {
      lcd.relayOn(); //Relay On

    }
    else
    {
      lcd.relay(); //Relay off
    }
   
    if ((hr*60+mi) >= (HrOff*60+MinOff))
    {    
      lcd.relay(); //Relay off

    }
}
else if ((HrOff*60+MinOff)-(HrOn*60+MinOn) < 0)
{
  if ((hr*60+mi) >= (HrOn*60+MinOn))
    {
      lcd.relayOn(); //Relay On

    }
   
    if ((hr*60+mi) >= (HrOff*60+MinOff) && (hr*60+mi) < (HrOn*60+MinOn))
    {    
      lcd.relay(); //Relay off

    } 
}
else if ((HrOff*60+MinOff)-(HrOn*60+MinOn) == 0)
{
  lcd.relay(); //Relay off
}
  }
  if (RelayMode==2)
  {
    lcd.relay(); //Relay Off
  }
   
  }

//----------------------------------------------------------------------------------------------------------

  byte button;
   
   
   //get the latest button pressed, also the buttonJustPressed, buttonJustReleased flags
   button = ReadButtons();

   //show text label for the button pressed
   switch( button )
   {
      case BUTTON_NONE:
      {

        

      break;
      }

 case BUTTON_NEXT:
      {
  m++;//увеличиваем переменную уровня меню

  if (m>6)//если уровень больше 3
  {
  m=0;// то вернуться к началу
  }

  delay (200);
  lcd.clear();
break;
      }
 case BUTTON_PREV:
     {
        m--;//увеличиваем переменную уровня меню

  if (m<0)//если уровень больше 3
  {
  m=6;// то вернуться к началу
  }

  delay (200);
  lcd.clear();
  break;
      }
       case BUTTON_UP:
      {
          if (m==1)
        {

          hr++;

  if (hr > 23)
  {
  hr = 0;
  }
  rtc.adjust(DateTime(yh, mo, dy, hr, mi, sc));
  delay (200);
  lcd.clear ();
         break;
        }
         if (m==2)
        {

          mi++;

  if (mi > 59)
  {
  mi = 0;
  }
  rtc.adjust(DateTime(yh, mo, dy, hr, mi, sc));
  delay (200);
  lcd.clear ();
         break;
        }
         if (m==3)
        {

          ft++;
          

  delay (200);
  lcd.clear ();
         break;
        }
   if (m==4)
        {

          RelayMode++;
     if (RelayMode>2)
RelayMode=2;
EEPROM.updateInt(addressRelayMode, RelayMode);

  delay (200);
  lcd.clear ();
         break;
        }
        if (m==5 && sm==0)
        {

          HrOn++;
          
          if (HrOn > 23)
          HrOn = 0;
          EEPROM.updateInt(addressHrOn, HrOn);

  delay (200);
  lcd.clear ();
         break;
        }
         if (m==5 && sm==1)
        {

          MinOn++;
          
          if (MinOn > 59)
          MinOn = 0;
          EEPROM.updateInt(addressMinOn, MinOn);

  delay (200);
  lcd.clear ();
         break;
        }
         if (m==6 && sm==0)
        {

          HrOff++;
          
          if (HrOff > 23)
          HrOff = 0;
          EEPROM.updateInt(addressHrOff, HrOff);

  delay (200);
  lcd.clear ();
         break;
        }
         if (m==6 && sm==1)
        {

          MinOff++;
          
          if (MinOff > 59)
          MinOff = 0;
          EEPROM.updateInt(addressMinOff, MinOff);

  delay (200);
  lcd.clear ();
         break;
        }
      }
      
//---------------------------------------------------------------------------------------------------------
       case BUTTON_DOWN:
      {
         if (m==1)
        {

          hr--;

  if (hr < 0)
  {
  hr = 23;
  }
  rtc.adjust(DateTime(yh, mo, dy, hr, mi, sc));
  delay (200);
  lcd.clear ();
         break;
        }
         if (m==2)
        {

          mi--;

  if (mi < 0)
  {
  mi = 59;
  }
  rtc.adjust(DateTime(yh, mo, dy, hr, mi, sc));
  delay (200);
  lcd.clear ();
         break;
        }
          if (m==3)
        {
         ft--;
  delay (200);
  lcd.clear ();
         break;
      }
       if (m==4)
        {

          RelayMode--;
     if (RelayMode<0)
RelayMode=0;
EEPROM.updateInt(addressRelayMode, RelayMode);

  delay (200);
  lcd.clear ();
         break;
        }
        if (m==5 && sm==0)
        {

          HrOn--;
          
          if (HrOn < 0)
          HrOn = 23;
          EEPROM.updateInt(addressHrOn, HrOn);

  delay (200);
  lcd.clear ();
         break;
        }
         if (m==5 && sm==1)
        {

          MinOn--;
          
          if (MinOn < 0)
          MinOn = 59;
          EEPROM.updateInt(addressMinOn, MinOn);

  delay (200);
  lcd.clear ();
         break;
        }
        if (m==6 && sm==0)
        {

          HrOff--;
          
          if (HrOff < 0)
          HrOff = 23;
          EEPROM.updateInt(addressHrOff, HrOff);

  delay (200);
  lcd.clear ();
         break;
        }
         if (m==6 && sm==1)
        {

          MinOff--;
          
          if (MinOff < 0)
          MinOff = 59;
          EEPROM.updateInt(addressMinOff, MinOff);

  delay (200);
  lcd.clear ();
         break;
        }
      }
//------------------------------------------------------------------------------------------------------
      case BUTTON_SAVE:
      {

if (m==2){

  rtc.adjust(DateTime(yh, mo, dy, hr, mi, 0));

  lcd.clear();
         break;
}
if (m==3){


  rtc.adjust(DateTime(ftyh, ftmo, ftdy, hr, mi, sc));
lcd.setCursor(0, 0);
lcd.print("  SAVE  ");
delay(1000);
ft=0;
  lcd.clear();
         break;
}
if (m==5 || m==6){


sm++;
if (sm > 1)
sm=0;
delay(200);
  lcd.clear();
  
         break;
}
      }
      //-------------------------------------------------------------------------------------------------------
      default:
     {
        break;
     }     
   }
 if (m==0)
  {   
      lcd.setCursor (0,0);
   if (hr<10)
    lcd.print('0');
     lcd.print(hr);
    lcd.print(':');
    if (mi<10)
    lcd.print('0');
    lcd.print(mi);
    lcd.print(':');
    if (sc<10)
    lcd.print('0');
    lcd.print(sc);  
    lcd.setCursor (0,1);
    if (dy<10)
    lcd.print('0');
    lcd.print(dy);
    lcd.print('/');
     if (mo<10)
    lcd.print('0');
    lcd.print(mo);
    lcd.print('/');
    int y = (yh-2000);
    lcd.print(y);

  } 
  else if (m==1)
  {

      lcd.setCursor(0, 0);
  lcd.print("Set Hour");
   lcd.setCursor(0, 1);
  if (hr<10)
    lcd.print('0');
 lcd.print(hr);
 lcd.print(':');
    if (mi<10)
    lcd.print('0');
    lcd.print(mi);
    lcd.print(':');
    if (sc<10)
    lcd.print('0');
    lcd.print(sc);  
  
  }
  
  else if (m==2)
  {  
  lcd.setCursor(0, 0);
  lcd.print("Set Min");
  lcd.setCursor(0, 1);
  if (hr<10)
  lcd.print('0');
  lcd.print(hr);
  lcd.print(':');
  if (mi<10)
  lcd.print('0');
  lcd.print(mi);
  lcd.print(':');
  if (sc<10)
  lcd.print('0');
  lcd.print(sc);  
  }
 else if (m==3)
  {
  lcd.setCursor(0, 0);
  lcd.print("Set Date");
  lcd.setCursor (0,1);
  if (ftdy<10)
  lcd.print('0');
  lcd.print(ftdy);
  lcd.print('/');
  if (ftmo<10)
  lcd.print('0');
  lcd.print(ftmo);
  lcd.print('/');
  int y = (ftyh-2000);
  lcd.print(y);
  }
 else if (m==4)
  {
  lcd.setCursor(0, 0);
  lcd.print("  Mode  ");
  lcd.setCursor(0, 1);
  lcd.print(mode[RelayMode]);
  }
  else if (m==5)
  {
   lcd.setCursor(0, 0);
  lcd.print("Reley On");
  
  if (sm==0)
  {
  lcd.setCursor(0, 1);
  lcd.print(">");
  }
  lcd.setCursor(1, 1);
   if (HrOn<10)
    lcd.print('0');
  lcd.print(HrOn);
  lcd.print(':');
  if (MinOn<10)
    lcd.print('0');
   lcd.print(MinOn);
   if (sm==1)
  lcd.print("<");
  }
  
 else if (m==6)
  {
  lcd.setCursor(0, 0);
  lcd.print("ReleyOff");
  
  if (sm==0)
  {
  lcd.setCursor(0, 1);
  lcd.print(">");
  }
  lcd.setCursor(1, 1);
   if (HrOff<10)
    lcd.print('0');
  lcd.print(HrOff);
  lcd.print(':');
  if (MinOff<10)
    lcd.print('0');
   lcd.print(MinOff);
   if (sm==1)
  lcd.print("<");
  }
  

      if( buttonJustPressed )
      buttonJustPressed = false;
   if( buttonJustReleased )
      buttonJustReleased = false;
      
//--------------------------------------------------------------------------------------

      
      }
/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
  Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
--------------------------------------------------------------------------------------*/
byte ReadButtons()
{
   unsigned int buttonVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
   
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   //sense if the voltage falls within valid voltage windows
   if( buttonVoltage < ( SAVE_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SAVE;
   }
   else if(   buttonVoltage >= ( NEXT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( NEXT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_NEXT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( PREV_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( PREV_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_PREV;
   }
   //handle button flags for just pressed and just released events
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
      //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }
   
   //save the latest button value, for change event detection next time round
   buttonWas = button;
   
   return( button );
}

