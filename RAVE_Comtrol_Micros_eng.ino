
/*    ///RAVE CONTROL Servo///
// |**********************************************************************;
* Project           : Einstellbare Auslasssteuerung für die Aprilia RS 125 mit Modellbauservo ohne RAVE Steuergerät und Solenoid
*                     Auch für andere Motorräder mit Powervalve anwendbar  ***adjustable powervalve controller with rc servo, designed for Aprilia rs 125 but also usefull in every other kind of engine
* 
* Program name      : RAVE_Control_Micros_eng.ino
*
* Author            : InterlinkKnight
*
* Date created      : 05/23/2019
*
* Modified by       : el bodo es loco
*
* Revision History  :
*
* Date modifiied    Vers.   Revision Date  
* 06/08/2020         2.0     06/11/2020
*
|**********************************************************************;
*/
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#define CONTROL_PIN 10  // optional output to the RAVE 7800 unit (it is better to use the other RAVE_Control)
#define LED_PIN 13 // LED "Valve open"
LiquidCrystal lcd (12, 11, 6, 5, 4, 2);
int PIN_ADJUST = A1;
int set;
int setsmooth;
int Setpoint;
Servo pvservo;

//##### Kalibrierung #####// 
//########################//  Importand parameters are signed with "####"


//SERVO Einstellung//
int auf = 95; //#### Servo endpoint open valve in degree, (RS 125: round about 90)
int zu = 145;//#### Servo endpoint closed valve in degree, (RS 125: round about 140)

//Einstellung Drehzahlabnahme//
const byte PulsesPerRevolution = 1;  // #### 1 for reading on Pickup or Ignition coil; 6 is for reading on the Alternator (ROTAX)
// at the Mito, the value is 2 for Alternator or Ignition coil (wasted spark Ignition)



//Timeout und Genauigkeit//
int valvesmooth = 100; //#### trigger value in Rpm to smooth the valve, for example: 100 means opening at the desired 8400rpm but closing at 8400rpm-100rpm = 8300rpm
const unsigned long ZeroTimeout = 100000;  
const byte numReadings = 5;  //Anzahl der Messwertblöcke zur Mittelwert bildung Je höher des to genauer aber auch trägerzwischen 3-10 ist in Ordnung
//no need to change, readings to count for average

/////////////
// Variablen:
/////////////

volatile unsigned long LastTimeWeMeasured;  
volatile unsigned long PeriodBetweenPulses = ZeroTimeout+1000;  
volatile unsigned long PeriodAverage = ZeroTimeout+1000;  
unsigned long FrequencyRaw;  
unsigned long FrequencyReal; 
unsigned long RPM; 
unsigned int PulseCounter = 1;  

unsigned long PeriodSum; 
unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;  
unsigned long CurrentMicros = micros(); 
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;  
unsigned long readings[numReadings];  
unsigned long readIndex;  
unsigned long total;  
unsigned long average;  

void setup() 
{

  Serial.begin(9600); 
  lcd.begin(16, 2);
  Wire.begin(); 
  pvservo.attach(9);
  pvservo.write(auf);
  delay(700);
  pvservo.write(zu);
  delay(700);
  attachInterrupt(digitalPinToInterrupt(3), Pulse_Event, RISING);  
  pinMode (3, INPUT_PULLUP);

  delay(1000);  
}  

void loop()
{
LastTimeCycleMeasure = LastTimeWeMeasured;
CurrentMicros = micros(); 
if(CurrentMicros < LastTimeCycleMeasure)
  {
    LastTimeCycleMeasure = CurrentMicros;
  }

  FrequencyRaw = 10000000000 / PeriodAverage;
  if(PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra)
  {  
    FrequencyRaw = 0;   
    ZeroDebouncingExtra = 2000;
  }
  else
  {
    ZeroDebouncingExtra = 0;  
  }
 FrequencyReal = FrequencyRaw / 10000;  
  RPM = FrequencyRaw / PulsesPerRevolution * 60;  
  RPM = RPM / 10000; 
  total = total - readings[readIndex];  
  readings[readIndex] = RPM;  
  total = total + readings[readIndex];  
  readIndex = readIndex + 1;  

  if (readIndex >= numReadings) 
  {
    readIndex = 0;  
  }
  average = total / numReadings; 
int r = RPM ;
Setpoint = analogRead (PIN_ADJUST);
set = map(Setpoint, 0, 1024, 6000, 10500 );
setsmooth = set - valvesmooth ;
      lcd.setCursor(0, 0);
lcd.clear();
lcd.print("Rotation: ");
lcd.print(RPM);   //  displays the actualy rev per minute
lcd.setCursor(0, 1);
lcd.print("RAVE: "); //displays the desired opening point
lcd.print(set);
if (RPM <= setsmooth){ // Valve closed under this Value
        digitalWrite(LED_PIN,LOW);
        tone (CONTROL_PIN, 500); 
        pvservo.write(zu);
      
      }
    if (RPM >= set){ // Valve Open above this Value
        digitalWrite(LED_PIN,HIGH);
        tone (CONTROL_PIN, 1000); 
         pvservo.write(auf);
        
      }

Serial.print("rpm.val=");
      Serial.println(r);
      Serial.print("set=");
      Serial.println(set);
     
}  
void Pulse_Event()  
{

  PeriodBetweenPulses = micros() - LastTimeWeMeasured;  

  LastTimeWeMeasured = micros();  

 if(PulseCounter >= AmountOfReadings)  
  {
    PeriodAverage = PeriodSum / AmountOfReadings; 
    PulseCounter = 1;  
    PeriodSum = PeriodBetweenPulses; 
    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);  
    
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;  
  }
  else
  {
    PulseCounter++; 
    PeriodSum = PeriodSum + PeriodBetweenPulses; 
  }

}  
