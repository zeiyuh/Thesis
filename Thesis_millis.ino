#define USE_ARDUINO_INTERRUPTS true
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <NewTone.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define BLYNK_TEMPLATE_ID "TMPL6XBP_nIyx"
#define BLYNK_TEMPLATE_NAME "MEDICINE DISPENSER"
#define BLYNK_AUTH_TOKEN "mhtEiHu2Ll2lfnsTTz-_qrUCxVYb5u-w"
#define BLYNK_PRINT Serial

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

char ssid[] = "Redmi 9T";
char pass[] = "medicine";
#define EspSerial Serial1
#define ESP8266_BAUD 38400
ESP8266 wifi(&EspSerial);

#define buzzer_1 12
#define buzzer_2 13

#define servoMIN1 600
#define servoMAX1 350

#define servoMIN2 350
#define servoMAX2 600

#define light_1 6
#define light_2 7
#define light_3 8
#define light_4 9
#define light_5 10
#define light_6 11

#define indicator_1 26
#define indicator_2 28
#define indicator_3 4
#define indicator_4 5
#define irLED_1 22 
#define irLED_2 24

MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
int storage1=0;
int storage2=0;
int storage3=0;
int storage4=0;

int melody_1[] = { 262, 262, 196, 262, 262, 196, 262, 262 };
int noteDurations_1[] = { 10, 10, 10, 10, 10, 10, 10, 10 };
int melody_2[] = { 196, 196, 262, 196, 196, 262, 196, 196};
int noteDurations_2[] = { 10, 10, 10, 10, 10, 10, 10, 10 };

Adafruit_PWMServoDriver servodriver = Adafruit_PWMServoDriver();
int servo_1 = 0;
int servo_2 = 1;
int servo_3= 2;
int servo_4= 3;

int IRPin_1=A1; //IR
int IRPin_2=A2;

bool off_1= false; //off indicator
bool off_2= false; 
bool off_3= false; 
bool off_4= false; 

bool missed_1 = false; //missed dosage
bool missed_2 = false;
bool missed_3 = false;
bool missed_4 = false;

bool normal1 = false; // bpm display
bool normal2 = false;
bool normal3 = false;
bool normal4 = false;

bool notnormal1 = false; // bpm display
bool notnormal2 = false;
bool notnormal3 = false;
bool notnormal4 = false;

bool on_slot1 = false;
bool on_slot2 = false;
bool on_slot3 = false;
bool on_slot4 = false;

int z=0;
int x=0;
int y=0; 

const byte ROWS = 4; //keypad
const byte COLS = 4; 
DS3231 rtc(SDA,SCL);
char key[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {37, 35, 33, 31}; 
byte colPins[COLS] = {45, 43, 41, 39};
Keypad keypad = Keypad(makeKeymap(key), rowPins, colPins, ROWS, COLS); 

int state= 0;
int ir_2= 0;
int ir_1= 0;

bool dispense_1 = false; //supply counter
bool dispense_2 = false;
bool dispense_3 = false;
bool dispense_4 = false;
bool stopper= false;

String storedHour_1;
String storedMinute_1;
String storedDay_1;
String storedMonth_1;
String storedYear_1;
String alarmDay_1;
String alarmMonth_1;
String alarmYear_1;

String storedHour_2;
String storedMinute_2;
String storedDay_2;
String storedMonth_2;
String storedYear_2;
String alarmDay_2;
String alarmMonth_2;
String alarmYear_2;

String storedHour_3;
String storedMinute_3;
String storedDay_3;
String storedMonth_3;
String storedYear_3;
String alarmDay_3;
String alarmMonth_3;
String alarmYear_3;

String storedHour_4;
String storedMinute_4;
String storedDay_4;
String storedMonth_4;
String storedYear_4;
String alarmDay_4;
String alarmMonth_4;
String alarmYear_4;

String D_app1;
String T_app1;
String D_app2;
String T_app2;
String D_app3;
String T_app3;
String D_app4;
String T_app4;

String status_flag1; // detect med app
String status_flag2;
String status_flag3;
String status_flag4;

String bpmstatus_app1;
String bpmstatus_app2;
String bpmstatus_app3;
String bpmstatus_app4;

int bpmvalue_app1 = 0;
int bpmvalue_app2 = 0;
int bpmvalue_app3 = 0;
int bpmvalue_app4 = 0;

int cursor = 0;

LiquidCrystal_I2C lcd1(0x27,20,4); 
LiquidCrystal_I2C lcd2(0x26,16,2);  

void S_appcontrol1 (int S_app1);
int S_appstore1;
void S_appcontrol2 (int S_app2);
int S_appstore2;
void S_appcontrol3 (int S_app3);
int S_appstore3;
void S_appcontrol4 (int S_app4);
int S_appstore4;

BLYNK_WRITE(V8){
  S_appcontrol1 (param.asInt());
  }
void S_appcontrol1 (int S_app1){
   S_appstore1 = S_app1;
  } 
BLYNK_WRITE(V10){
  S_appcontrol2 (param.asInt());
  }
void S_appcontrol2 (int S_app2){
   S_appstore2 = S_app2;
  } 
  BLYNK_WRITE(V12){
  S_appcontrol3 (param.asInt());
  }
void S_appcontrol3 (int S_app3){
   S_appstore3 = S_app3;
  } 
  BLYNK_WRITE(V14){
  S_appcontrol4 (param.asInt());
  }
void S_appcontrol4 (int S_app4){
   S_appstore4 = S_app4;
  }  
   
void setup() {
        Serial.begin(115200);
        EspSerial.begin(ESP8266_BAUD);
        delay(10);
        
        Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
        Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass, "blynk.cloud", 80);
      
        // Initialize sensor
        if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
        {
          //Serial.println("MAX30105 was not found. Please check wiring/power. ");
          //while (1); ETO UNCOMMENT MO PAG MAY ISSUES
        }
        //Serial.println("Place your index finger on the sensor with steady pressure.");
      
        particleSensor.setup(); //Configure sensor with default settings
        particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
        particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
        
        pinMode(light_1, OUTPUT); //LED
        pinMode(light_2, OUTPUT);
        pinMode(light_3, OUTPUT);
        pinMode(light_4, OUTPUT);
        pinMode(light_5, OUTPUT);
        pinMode(light_6, OUTPUT);
        
        pinMode(indicator_1, OUTPUT);
        pinMode(indicator_2, OUTPUT);
        pinMode(indicator_3, OUTPUT);
        pinMode(indicator_4, OUTPUT);
      
        pinMode(irLED_1,OUTPUT);
        pinMode(IRPin_1,INPUT); // IR
        pinMode(irLED_2,OUTPUT);
        pinMode(IRPin_2,INPUT); // IR
        
        
        servodriver.begin();
        servodriver.setPWMFreq(60);
        servodriver.setPWM(servo_1, 0, servoMIN2);
        servodriver.setPWM(servo_2, 0, servoMIN1);
        servodriver.setPWM(servo_3, 0, servoMIN1);
        servodriver.setPWM(servo_4, 0, servoMIN2);
     
        lcd1.init();
        lcd2.init();
        lcd1.clear();         
        lcd2.clear();         
        lcd1.backlight();
        lcd2.backlight();

        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print ("  BPM Monitor");
        
        rtc.begin();
        rtc.setDOW(THURSDAY);     // Set Day-of-Week to SUNDAY
        rtc.setTime(16, 05, 0);     // Set the dafault time HR.MIN.SEC
        rtc.setDate(15, 10, 2023);   // Set the default date DD.MM.YYYY
        
        pinMode(buzzer_1, OUTPUT);
        digitalWrite(buzzer_1, LOW);
        pinMode(buzzer_2, OUTPUT);
        digitalWrite(buzzer_2, LOW);

        Blynk.virtualWrite(V0,"EMPTY");
        Blynk.virtualWrite(V1,"EMPTY");
        Blynk.virtualWrite(V2,"EMPTY");
        Blynk.virtualWrite(V3,"EMPTY");
        Blynk.virtualWrite(V4,"EMPTY");
        Blynk.virtualWrite(V5,"EMPTY");
        Blynk.virtualWrite(V6,"EMPTY");
        Blynk.virtualWrite(V7,"EMPTY");

        Blynk.virtualWrite(V8,0);
        Blynk.virtualWrite(V10,0);
        Blynk.virtualWrite(V12,0);
        Blynk.virtualWrite(V14,0);

        Blynk.virtualWrite(V20,0);   
      }

void loop() { 
        Blynk.run();
        int z=0;
        int x=0;
        int y=0;
        
        String dateString= rtc.getDateStr();
        String d= dateString.substring (0,2);
        String m= dateString.substring (3,5);
        String yr= dateString.substring (6,10);
        String currentDate= d+"."+ m+"."+ yr;
        
        String timeString = rtc.getTimeStr();
        String realHour = timeString.substring(0, 2);
        String realMinute = timeString.substring(3, 5);
        String currentTime = realHour+":"+realMinute+":00";
      
        String DOWString= rtc.getDOWStr();
        String currentDOW= DOWString;
        
        Main();
       // emptyslots(); PAKI COMMENT BACK NA LANG
        
        bool alarm = false;
        while (alarm == false) {
        IR_1();
        IR_2();
        int z=0;
        int x=0;
        int y=0;
        
        String dateString= rtc.getDateStr();
        String d= dateString.substring (0,2);
        String m= dateString.substring (3,5);
        String yr= dateString.substring (6,10);
        String currentDate= d+"."+ m+"."+ yr;
        
        String timeString = rtc.getTimeStr();
        String realHour = timeString.substring(0, 2);
        String realMinute = timeString.substring(3, 5);
        String currentTime = realHour+":"+realMinute+":00";
      
        String DOWString= rtc.getDOWStr();
        String currentDOW= DOWString;
        
        Main();
        
        delay(2000); //required for print
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("Date:"+ currentDate);
        lcd1.setCursor(0, 1);
        lcd1.print("Time:"+ currentTime);
        lcd1.setCursor(0, 2);
        lcd1.print("Day: " + currentDOW);

        blynkfunc();
        
        if (realHour == storedHour_1 && realMinute == storedMinute_1 && m == alarmMonth_1 && d == alarmDay_1 && yr == alarmYear_1) {
          alarm = true;
          
        } else if (realHour == storedHour_2 && realMinute == storedMinute_2 && m == alarmMonth_2 && d == alarmDay_2 && yr == alarmYear_2) {
          alarm = true;
        } else if (realHour == storedHour_3 && realMinute == storedMinute_3 && m == alarmMonth_3 && d == alarmDay_3 && yr == alarmYear_3) {
          alarm = true;
        } else if (realHour == storedHour_4 && realMinute == storedMinute_4 && m == alarmMonth_4 && d == alarmDay_4 && yr == alarmYear_4) {
          alarm = true;
        }else {
          alarm = false;
        }
        }

      if (realHour == storedHour_1 && realMinute == storedMinute_1 && m == alarmMonth_1 && d == alarmDay_1 && yr == alarmYear_1) { //alarm#1
          Blynk.logEvent("schedule_reminder");
          beatAvg = 0;
          storage1= 0;
          on_slot1 == false;
          ir_1 =digitalRead(IRPin_1);
          if (ir_1 == LOW){ 
            StatusApp1 ();
            }
          else {   
            uint32_t period_1 = 5 * 60000L;       // 5 minutes loop
            for( uint32_t tStart_1 = millis();  (millis()-tStart_1) < period_1;  ){
               digitalWrite(irLED_1,LOW);
               soundAlarm();
               timedisplay();
               
                 bool timeloop1 = false;
                 while (timeloop1 == false){
                 uint32_t pulseperiod_1 = 0.5 * 60000L;       // 1 minute loop
                 for( uint32_t pulseStart_1 = millis();  (millis()-pulseStart_1) < pulseperiod_1;  ){ 
                  long irValue = particleSensor.getIR();
                  if (checkForBeat(irValue) == true){ // check for finger
                          //We sensed a beat!
                      long delta = millis() - lastBeat;
                      lastBeat = millis();
                      beatsPerMinute = 60 / (delta / 1000.0);
                      
                      if (beatsPerMinute < 255 && beatsPerMinute > 20){
                          rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
                          rateSpot %= RATE_SIZE; //Wrap variable
                    
                          //Take average of readings
                          beatAvg = 0;
                          for (byte x = 0 ; x < RATE_SIZE ; x++)
                            beatAvg += rates[x];
                          beatAvg /= RATE_SIZE;
                          }
                           
                        lcd2.clear();
                        lcd2.setCursor(0,0);  
                        lcd2.print("BPM= ");
                        lcd2.print(beatsPerMinute);
                        lcd2.setCursor(0,1);
                        lcd2.print("Avg BPM= ");
                        lcd2.print(beatAvg);
                      }
                      if (irValue < 50000) { 
                          soundAlarm();
                          lcd2.clear();
                          lcd2.setCursor(0,0);
                          lcd2.print("Nothing detected");
                          delay(500);
                          }
                      ir_1 =digitalRead(IRPin_1);
                      if (ir_1==LOW){
                          StatusApp1 ();
                          LED_OFF();
                          digitalWrite(buzzer_1,LOW);
                          digitalWrite(buzzer_2,LOW);
                          break;
                            }}
                            timeloop1=true;
                            }
                      
                      storage1 = beatAvg;
                      if (storage1 >=60 && storage1 <=100){
                        bpmvalue_app1 = storage1;
                        Blynk.virtualWrite(V20,bpmvalue_app1);
                        normal1= true;
                        BPMDisp();
                        soundAlarm();
                        servodriver.setPWM(servo_1, 0, servoMIN1);
                        delay(500);
                        servodriver.setPWM(servo_1, 0, servoMAX1);
                        delay(1000);
                        digitalWrite(indicator_1, HIGH);
                        bpmstatus_app1 = "Normal BPM"; 
                        Blynk.virtualWrite(V21,bpmstatus_app1);
                        off_1 = true;
                        on_slot1 = true;
                        dispense_1 = true;  
                        break;
                        }  
                     else if (storage1 !=0 && storage1 <=60 && storage1 >=100){
                        notnormal1 = true;
                        BPMNotDisp();
                        bpmvalue_app1 = storage1;
                        Blynk.virtualWrite(V20,bpmvalue_app1);
                        bpmstatus_app1 = "Abnormal BPM"; 
                        Blynk.virtualWrite(V21,bpmstatus_app1);
                        Blynk.logEvent("abnormal_bpm");
                        off_1 = true;
                        digitalWrite(indicator_1, HIGH);
                        break;
                      }   
                      
                      ir_1 =digitalRead(IRPin_1);
                      if (ir_1==LOW){ 
                        StatusApp1 ();
                        LED_OFF();
                          break;
                          }}
                          
                   if (storage1 == 0){
                        lcd2.clear();
                        lcd2.setCursor(0,0);
                        lcd2.print ("   Time limit");
                        lcd2.setCursor(0,1);
                        lcd2.print ("  Missed dosage");
                        bpmstatus_app1 = "Missed"; 
                        Blynk.virtualWrite(V21,bpmstatus_app1);
                        digitalWrite(indicator_1, HIGH);
                        off_1 = true;
                        }}
                     
                      LED_OFF();  
                      digitalWrite(buzzer_1,LOW);
                      digitalWrite(buzzer_2,LOW);
                }
                  
           if (realHour == storedHour_2 && realMinute == storedMinute_2 && m == alarmMonth_2 && d == alarmDay_2 && yr == alarmYear_2) { //alarm#2
              Blynk.logEvent("schedule_reminder");
              beatAvg = 0;
              storage2= 0;
              on_slot2 == false;
              ir_1 =digitalRead(IRPin_1);
              if (ir_1 == LOW){
                StatusApp2();
                }
              else {  
                uint32_t period_2 = 5 * 60000L;       // 5 minutes loop
                for( uint32_t tStart_2 = millis();  (millis()-tStart_2) < period_2;  ){
                   digitalWrite(irLED_2,LOW);
                   soundAlarm();
                   timedisplay();
                   
                     bool timeloop2 = false;
                     while (timeloop2 == false){
                     uint32_t pulseperiod_2 = 0.5 * 60000L;       // 1 minute loop
                     for( uint32_t pulseStart_2 = millis();  (millis()-pulseStart_2) < pulseperiod_2;  ){
                      long irValue = particleSensor.getIR();
                      if (checkForBeat(irValue) == true){ // check for finger
                              //We sensed a beat!
                          long delta = millis() - lastBeat;
                          lastBeat = millis();
                          beatsPerMinute = 60 / (delta / 1000.0);
                          
                          if (beatsPerMinute < 255 && beatsPerMinute > 20){
                              rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
                              rateSpot %= RATE_SIZE; //Wrap variable
                        
                              //Take average of readings
                              beatAvg = 0;
                              for (byte x = 0 ; x < RATE_SIZE ; x++)
                                beatAvg += rates[x];
                              beatAvg /= RATE_SIZE;
                              }
                               
                            lcd2.clear();
                            lcd2.setCursor(0,0);  
                            lcd2.print("BPM= ");
                            lcd2.print(beatsPerMinute);
                            lcd2.setCursor(0,1);
                            lcd2.print("Avg BPM= ");
                            lcd2.print(beatAvg);
                          
                          }
                          if (irValue < 50000) { 
                              soundAlarm();
                              lcd2.clear();
                              lcd2.setCursor(0,0);
                              lcd2.print("Nothing detected");
                              delay(500);
                              }
                          ir_1 =digitalRead(IRPin_1);
                          if (ir_1==LOW){
                              StatusApp2();
                              LED_OFF();
                              digitalWrite(buzzer_1,LOW);
                              digitalWrite(buzzer_2,LOW);
                              break;
                                }}
                             timeloop2=true;
                            }
                          
                          storage2 = beatAvg;
                          if (storage2 >=60 && storage2 <=100){
                            bpmvalue_app1 = storage2;
                            Blynk.virtualWrite(V20,bpmvalue_app1);
                            normal2= true;
                            BPMDisp();
                            soundAlarm();
                            servodriver.setPWM(servo_2, 0, servoMIN2);
                            delay(500);
                            servodriver.setPWM(servo_2, 0, servoMAX2);
                            delay(1000);
                            digitalWrite(indicator_2, HIGH); 
                            bpmstatus_app1 = "Normal BPM"; 
                            Blynk.virtualWrite(V21,bpmstatus_app1);
                            off_2 = true;
                            on_slot2 = true;
                            dispense_2 = true; 
                            break;
                            }
                         else if (storage2 !=0 && storage2 <=60 && storage2 >=100) {
                            notnormal2 = true;
                            BPMNotDisp();
                            bpmvalue_app2 = storage2;
                            Blynk.virtualWrite(V20,bpmvalue_app2);
                            bpmstatus_app2 = "Abnormal BPM"; 
                            Blynk.virtualWrite(V21,bpmstatus_app2);
                            Blynk.logEvent("abnormal_bpm");
                            off_2 = true;
                            digitalWrite(indicator_2, HIGH);
                            break;
                          }   
                          
                          ir_1 =digitalRead(IRPin_1);
                          if (ir_1==LOW){
                            StatusApp2();
                            LED_OFF();
                              break;
                              }}
                              
                        if (storage1 == 0){ 
                            lcd2.clear();
                            lcd2.setCursor(0,0);
                            lcd2.print ("   Time limit");
                            lcd2.setCursor(0,1);
                            lcd2.print ("  Missed dosage");
                            bpmstatus_app1 = "Missed"; 
                            Blynk.virtualWrite(V21,bpmstatus_app1);
                            digitalWrite(indicator_2, HIGH);
                            off_2 = true;
                            }}
                              
                           LED_OFF();  
                          digitalWrite(buzzer_1,LOW);
                          digitalWrite(buzzer_2,LOW);
                          digitalWrite(indicator_2, HIGH);  
                          }

            if (realHour == storedHour_3 && realMinute == storedMinute_3 && m == alarmMonth_3 && d == alarmDay_3 && yr == alarmYear_3) { //alarm#1
              Blynk.logEvent("schedule_reminder");
              beatAvg = 0;
              storage3= 0;
              on_slot3 == false;
              ir_1 =digitalRead(IRPin_1);
              if (ir_1 == LOW){
                StatusApp3();
                }
              else { 
                uint32_t period_3 = 5 * 60000L;       // 5 minutes loop
                for( uint32_t tStart_3 = millis();  (millis()-tStart_3) < period_3;  ){
                   digitalWrite(irLED_1,LOW);
                   soundAlarm();
                   timedisplay();
                   
                     bool timeloop3 = false;
                     while (timeloop3 == false){
                     uint32_t pulseperiod_3 = 0.5 * 60000L;       // 1 minute loop
                     for( uint32_t pulseStart_3 = millis();  (millis()-pulseStart_3) < pulseperiod_3;  ){
                      long irValue = particleSensor.getIR();
                      if (checkForBeat(irValue) == true){ // check for finger
                              //We sensed a beat!
                          long delta = millis() - lastBeat;
                          lastBeat = millis();
                          beatsPerMinute = 60 / (delta / 1000.0);
                          
                          if (beatsPerMinute < 255 && beatsPerMinute > 20){
                              rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
                              rateSpot %= RATE_SIZE; //Wrap variable
                        
                              //Take average of readings
                              beatAvg = 0;
                              for (byte x = 0 ; x < RATE_SIZE ; x++)
                                beatAvg += rates[x];
                              beatAvg /= RATE_SIZE;
                              }
                               
                            lcd2.clear();
                            lcd2.setCursor(0,0);  
                            lcd2.print("BPM= ");
                            lcd2.print(beatsPerMinute);
                            lcd2.setCursor(0,1);
                            lcd2.print("Avg BPM= ");
                            lcd2.print(beatAvg);
                          
                          }
                          if (irValue < 50000) { 
                              soundAlarm();
                              lcd2.clear();
                              lcd2.setCursor(0,0);
                              lcd2.print("Nothing detected");
                              delay(500);
                              }
                          ir_1 =digitalRead(IRPin_1);
                          if (ir_1==LOW){
                              StatusApp3();
                              LED_OFF();
                              digitalWrite(buzzer_1,LOW);
                              digitalWrite(buzzer_2,LOW);
                              break;
                                }}
                               timeloop3=true;
                            }
                          
                          storage3 = beatAvg;
                          if (storage3 >=60 && storage3<=100){
                            bpmvalue_app1 = storage3;
                            Blynk.virtualWrite(V20,bpmvalue_app1);
                            normal3= true;
                            BPMDisp();
                            soundAlarm();
                            servodriver.setPWM(servo_3, 0, servoMIN2);
                            delay(500);
                            servodriver.setPWM(servo_3, 0, servoMAX2);
                            delay(1000);
                            digitalWrite(indicator_3, HIGH);
                            bpmstatus_app1 = "Normal BPM"; 
                            Blynk.virtualWrite(V21,bpmstatus_app1); 
                            off_3 = true;
                            on_slot3 = true;
                            dispense_3 = true;
                            break;
                            } 
                         else if (storage3 !=0 && storage3 <=60 && storage3 >=100) {
                            notnormal3 = true;
                            BPMNotDisp();
                            bpmvalue_app3 = storage3;
                            Blynk.virtualWrite(V20,bpmvalue_app3);
                            bpmstatus_app3 = "Abnormal BPM"; 
                            Blynk.virtualWrite(V21,bpmstatus_app3);
                            Blynk.logEvent("abnormal_bpm");
                            off_3 = true;
                            digitalWrite(indicator_3, HIGH);
                            break;
                          }   
                          
                          ir_1 =digitalRead(IRPin_1);
                          if (ir_1==LOW){
                            StatusApp3();
                            LED_OFF();
                              break;
                              }}

                          if (storage3 == true ){
                            lcd2.clear();
                            lcd2.setCursor(0,0);
                            lcd2.print ("   Time limit");
                            lcd2.setCursor(0,1);
                            lcd2.print ("  Missed dosage");
                            bpmstatus_app1 = "Missed"; 
                            Blynk.virtualWrite(V21,bpmstatus_app1);
                            digitalWrite(indicator_1, HIGH);
                            off_3 = true;
                            }
                          }
                            
                           LED_OFF();  
                          digitalWrite(buzzer_1,LOW);
                          digitalWrite(buzzer_2,LOW);
                          digitalWrite(indicator_3, HIGH);  
                    }

        if (realHour == storedHour_4 && realMinute == storedMinute_4 && m == alarmMonth_4 && d == alarmDay_4 && yr == alarmYear_4) { //alarm#1
              Blynk.logEvent("schedule_reminder");
              beatAvg = 0;
              storage4 = 0;
              on_slot4 == false;
              ir_1 =digitalRead(IRPin_1);
              if (ir_1 == LOW){
                StatusApp4();
                }
              else {  
                uint32_t period_4 = 5 * 60000L;       // 5 minutes loop
                for( uint32_t tStart_4 = millis();  (millis()-tStart_4) < period_4;  ){
                   digitalWrite(irLED_1,LOW);
                   soundAlarm();
                   timedisplay();
                   
                     bool timeloop4 = false;
                     while (timeloop4 == false){
                     uint32_t pulseperiod_4 = 1 * 60000L;       // 1 minute loop
                     for( uint32_t pulseStart_4 = millis();  (millis()-pulseStart_4) < pulseperiod_4;  ){
                      long irValue = particleSensor.getIR();
                      if (checkForBeat(irValue) == true){ // check for finger
                              //We sensed a beat!
                          long delta = millis() - lastBeat;
                          lastBeat = millis();
                          beatsPerMinute = 60 / (delta / 1000.0);
                          
                          if (beatsPerMinute < 255 && beatsPerMinute > 20){
                              rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
                              rateSpot %= RATE_SIZE; //Wrap variable
                        
                              //Take average of readings
                              beatAvg = 0;
                              for (byte x = 0 ; x < RATE_SIZE ; x++)
                                beatAvg += rates[x];
                              beatAvg /= RATE_SIZE;
                              }
                               
                            lcd2.clear();
                            lcd2.setCursor(0,0);  
                            lcd2.print("BPM= ");
                            lcd2.print(beatsPerMinute);
                            lcd2.setCursor(0,1);
                            lcd2.print("Avg BPM= ");
                            lcd2.print(beatAvg);
                          
                          }
                          if (irValue < 50000) { 
                              soundAlarm();
                              lcd2.clear();
                              lcd2.setCursor(0,0);
                              lcd2.print("Nothing detected");
                              delay(500);
                              }
                          ir_1 =digitalRead(IRPin_1);
                          if (ir_1==LOW){
                              StatusApp4();
                              LED_OFF();
                              digitalWrite(buzzer_1,LOW);
                              digitalWrite(buzzer_2,LOW);
                              break;
                                }}
                            timeloop4=true;
                            }
                          
                          storage4 = beatAvg;
                          if (storage4 >=60 && storage4 <=100){
                            bpmvalue_app1 = storage4;
                            Blynk.virtualWrite(V20,bpmvalue_app1);
                            normal4= true;
                            BPMDisp();
                            soundAlarm();
                            servodriver.setPWM(servo_4, 0, servoMIN1);
                            delay(500);
                            servodriver.setPWM(servo_4, 0, servoMAX1);
                            delay(1000);
                            digitalWrite(indicator_4, HIGH); 
                            bpmstatus_app1 = "Normal BPM"; 
                            Blynk.virtualWrite(V21,bpmstatus_app1);
                            off_4 = true;
                            on_slot4 = true;
                            dispense_4 = true; 
                            break;
                            }
                         else if (storage4 !=0 && storage4 <=60 && storage4 >=100) {
                            notnormal4 = true;
                            BPMNotDisp();
                            bpmvalue_app4 = storage4;
                            Blynk.virtualWrite(V20,bpmvalue_app4);
                            bpmstatus_app4 = "Abnormal BPM"; 
                            Blynk.virtualWrite(V21,bpmstatus_app4);
                            Blynk.logEvent("abnormal_bpm");
                            off_4 = true;
                            digitalWrite(indicator_4, HIGH);
                            break;
                          }   
                          
                          ir_1 =digitalRead(IRPin_1);
                          if (ir_1==LOW){
                            StatusApp4();
                            LED_OFF();
                              break;
                              }}
                          if (storage4 == 0){
                            lcd2.clear();
                            lcd2.setCursor(0,0);
                            lcd2.print ("   Time limit");
                            lcd2.setCursor(0,1);
                            lcd2.print ("  Missed dosage");
                            bpmstatus_app1 = "Missed"; 
                            Blynk.virtualWrite(V21,bpmstatus_app1);   
                            digitalWrite(indicator_4, HIGH);
                            off_4 = true;
                            }
                          }
                          
                           LED_OFF();  
                          digitalWrite(buzzer_1,LOW);
                          digitalWrite(buzzer_2,LOW);
                          digitalWrite(indicator_4, HIGH);  
                      } 

            if(off_1 == true && off_2 == true && off_3 == true && off_4 == true){
                  LED_ON();
                  soundAlarm();
                  digitalWrite(indicator_1, LOW);  
                  digitalWrite(indicator_2, LOW);  
                  digitalWrite(indicator_3, LOW);  
                  digitalWrite(indicator_4, LOW);  
                  }
             
              }

/*..........................................................................*/
String getInputFromKeyBoard(int n) {
  String num = "";
  String number;
  String confirmNumber;
  int i = 0;

  while (1) {
    char key_1 = keypad.getKey();
    if (key_1) {
      digitalWrite(buzzer_1, HIGH);
      digitalWrite(buzzer_2, HIGH);
      
      delay(100);
      digitalWrite(buzzer_1, LOW);
      digitalWrite(buzzer_2, LOW);
      if (key_1 == '0' || key_1 == '1' || key_1 == '2' || key_1 == '3' || key_1 == '4' || key_1 == '5' || key_1 == '6' || key_1 == '7' || key_1 == '8' || key_1 == '9') {
        i++;
        if (i <= n) {
          num += key_1;
        }
        lcd1.setCursor(14, 0);
        lcd1.print(num);
        lcd1.setCursor(0, 1);
        lcd1.print("*Cancel #Confirm");
        }

      if (key_1 == '#' && i == n) {
        break;}

      if (key_1 == '*') {
        MainMenu();}

      if ((key_1 == '#' || key_1 == 'A' || key_1 == 'B' || key_1 == 'C' || key_1 == 'D') && i < n) { //u change
        lcd1.clear();
        lcd1.print("Invalid Number!");
        delay(2000);
        number = "";
        confirmNumber = "";
        num = "";
        return "";
      }
      if (i > n) {
        lcd1.clear();
        lcd1.print("Incomplete input!");
        delay(2000);
        number = "";
        confirmNumber = "";
        num = "";
        return "";
      }

      if (i == n && ( key_1 == 'B' || key_1 == 'C' || key_1 == 'D')) {
        lcd1.clear();
        lcd1.print("Invalid Option!");
        delay(2000);
        number = "";
        confirmNumber = "";
        num = "";
        return "";
      }}}
  return num;
  }
   
int getData() {
  lcd1.setCursor (0,1);
  lcd1.print ("*Cancel #Confirm");
  
  String container = "";
  lcd1.setCursor(15, 0);
  while (true) {
      char c = keypad.getKey();
    if (c == '*'){
      break;
      }  
    if (c == '#') {
      break;
  } else if (isDigit(c)) {
      container += c;
      lcd1.print(c);
  } else {
      //Nothing
    }
  }
  return container.toInt();
}

/*---------------------------------------------------------------------------------*/
  
void soundAlarm(){
  for (int thisNote_1 = 0; thisNote_1 < 8; thisNote_1++) { // Loop through the notes in the array.
    int noteDuration_1 = 1000/noteDurations_1[thisNote_1];
    NewTone(buzzer_1, melody_1[thisNote_1], noteDuration_1); // Play thisNote for noteDuration. 
    delay(noteDuration_1 * 4 / 3); // Wait while the tone plays in the background, plus another 33% delay between notes.
  }
   for (int thisNote_2 = 0; thisNote_2 < 8; thisNote_2++) { // Loop through the notes in the array.
    int noteDuration_2 = 1000/noteDurations_2[thisNote_2];
    NewTone(buzzer_2, melody_2[thisNote_2], noteDuration_2); // Play thisNote for noteDuration.
    delay(noteDuration_2 * 4 / 3); // Wait while the tone plays in the background, plus another 33% delay between notes.
  }
  LED_ON();
  }
void BPMDisp(){ 
      if (normal1 == true){
      lcd2.clear();
      lcd2.setCursor(0,0); 
      lcd2.print (storage1); 
      lcd2.print(" is normal BPM");
      delay(1000);
      normal1 = false; 
      }

      if (normal2 == true){
      lcd2.clear();
      lcd2.setCursor(0,0); 
      lcd2.print (storage2); 
      lcd2.print(" is normal BPM");
      delay(1000); 
      normal2 = false;
      }

      if (normal3 == true){
      lcd2.clear();
      lcd2.setCursor(0,0); 
      lcd2.print (storage3); 
      lcd2.print(" is normal BPM");
      delay(1000); 
      normal3 = false;
      }

      if (normal4 == true){
      lcd2.clear();
      lcd2.setCursor(0,0); 
      lcd2.print (storage4); 
      lcd2.print(" is normal BPM");
      delay(1000); 
      normal4 = false;
      }
  }
void BPMNotDisp(){
   if (notnormal1== true){
    lcd2.setCursor(0,0);  
    lcd2.print (storage1);
    lcd2.print(" abnormal BPM");
    lcd2.setCursor(0,1);  
    lcd2.print("Try again later");
    delay(1000);
    LED_OFF();
    digitalWrite(buzzer_1,LOW);
    digitalWrite(buzzer_2,LOW);            
    servodriver.setPWM(servo_1, 0, servoMIN2);
    notnormal1 = false;
    }
    
    if (notnormal2== true){
    lcd2.setCursor(0,0);  
    lcd2.print (storage2);
    lcd2.print(" abnormal BPM");
    lcd2.setCursor(0,1);  
    lcd2.print("Try again later");
    delay(1000);
    LED_OFF();
    digitalWrite(buzzer_1,LOW);
    digitalWrite(buzzer_2,LOW);            
    servodriver.setPWM(servo_2, 0, servoMIN1);
    notnormal2 = false;
    }
    
    if (notnormal3== true){
    lcd2.setCursor(0,0);  
    lcd2.print (storage3);
    lcd2.print(" abnormal BPM");
    lcd2.setCursor(0,1);  
    lcd2.print("Try again later");
    delay(1000);
    LED_OFF();
    digitalWrite(buzzer_1,LOW);
    digitalWrite(buzzer_2,LOW);            
    servodriver.setPWM(servo_3, 0, servoMIN1);
    notnormal3 = false;
    }
    
    if (notnormal4== true){
    lcd2.setCursor(0,0);  
    lcd2.print (storage4);
    lcd2.print(" abnormal BPM");
    lcd2.setCursor(0,1);  
    lcd2.print("Try again later");
    delay(1000);
    LED_OFF();
    digitalWrite(buzzer_1,LOW);
    digitalWrite(buzzer_2,LOW);            
    servodriver.setPWM(servo_4, 0, servoMIN2);
    notnormal4 = false;
    }}
    
void LED_ON (){
  digitalWrite(light_1, HIGH); 
  digitalWrite(light_2, HIGH); // Turn on LED2 
  digitalWrite(light_3, HIGH); // Turn on LED3 
  digitalWrite(light_4, HIGH); // Turn on LED4
  digitalWrite(light_5, HIGH);
  digitalWrite(light_6, HIGH); 
  delay (500);
  LED_OFF();

  digitalWrite(light_1, HIGH); 
  digitalWrite(light_2, HIGH); // Turn on LED2 
  digitalWrite(light_3, HIGH); // Turn on LED3 
  digitalWrite(light_4, HIGH); // Turn on LED4
  digitalWrite(light_5, HIGH);
  digitalWrite(light_6, HIGH); 
  delay (500);
  LED_OFF();
  }
  
void LED_OFF(){
  digitalWrite(light_1, LOW); 
  digitalWrite(light_2, LOW); // Turn on LED2 
  digitalWrite(light_3, LOW); // Turn on LED3 
  digitalWrite(light_4, LOW); // Turn on LED4
  digitalWrite(light_5, LOW); 
  digitalWrite(light_6, LOW); 
  delay (500);
  }
      
void Main(){
 // Serial.print("press");
  char key_Main = keypad.getKey();
      if (key_Main) {
        digitalWrite(buzzer_1, HIGH);
        digitalWrite(buzzer_2, HIGH);
        delay(100);
        digitalWrite(buzzer_1, LOW);
        digitalWrite(buzzer_2, LOW);
        
     switch (key_Main){
        case 'A':
        MainMenu();
        break;
    }}
  }
     
void MainMenu(){
        char key_MainMenu = keypad.getKey();
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("A: Schedule");
        lcd1.setCursor(0, 1);
        lcd1.print("B: Supply");
        lcd1.setCursor(0, 2);
        lcd1.print("C: Alarm");
        lcd1.setCursor(0, 3);
        lcd1.print("D: Back");
        
 while (y<1){
      char key_MainMenu = keypad.getKey();
           
    if (key_MainMenu== 'A'){ 
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("  Schedule");
        Schedule();
        break;}
        
   else if (key_MainMenu== 'B'){
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print(" Supply Monitor");
        supplymonitor();
        break;}

    if (key_MainMenu== 'C'){
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("      Alarms ");
        alarmlist();
        break;}
        
     if (key_MainMenu== 'D'){
        break;}    
        
        }
      }    
void Schedule(){
        char key_Schedule = keypad.getKey();
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("A: Schedule #1");
        lcd1.setCursor(0,1);
        lcd1.print("B: Schedule #2");
        lcd1.setCursor(0,2);
        lcd1.print("C: Schedule #3");
        lcd1.setCursor(0,3);
        lcd1.print("D: Schedule #4");
       
while (y<1){
  char key_Schedule = keypad.getKey();
    if (key_Schedule== 'A'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("   Schedule #1");
        delay(1000);
        zone_1();
        MainMenu();
        break;}

   if (key_Schedule== 'B'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("  Schedule #2");
        delay(1000);
        zone_2();
        MainMenu();
        break;}
    
    if (key_Schedule== 'C'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("  Schedule #3");
        delay(1000);
        zone_3();
        MainMenu();
        break;}
        
    if (key_Schedule== 'D'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("  Schedule #4");
        delay(1000);
        zone_4();
        MainMenu();
        break;}
  }}
  
void alarmlist(){
  bool leave = false;
   while(leave== false){
      lcd1.clear();
      lcd1.setCursor(0, 0);
      lcd1.print("A#1 Date:");                   
      String storedAlarmDate_1= alarmDay_1+"."+ alarmMonth_1 +"." + alarmYear_1;
      lcd1.print(storedAlarmDate_1);
      lcd1.setCursor(0, 1);
      lcd1.print("A#1 Time:");
      String storedAlarmTime_1 = storedHour_1 + ":" + storedMinute_1 + ":00";
      lcd1.print(storedAlarmTime_1);
      lcd1.setCursor(0, 2);
      lcd1.print("A#2 Date:");
      String storedAlarmDate_2= alarmDay_2+"."+ alarmMonth_2 + "." + alarmYear_2;
      lcd1.print(storedAlarmDate_2);
      lcd1.setCursor(0, 3);
      lcd1.print("A#2 Time:");
      String storedAlarmTime_2 = storedHour_2 + ":" + storedMinute_2 + ":00";
      lcd1.print(storedAlarmTime_2);
      
      delay(3000);
      lcd1.clear();
      lcd1.setCursor(0, 0);
      lcd1.print("A#3 Date:");                   
      String storedAlarmDate_3= alarmDay_3+"."+ alarmMonth_3 + "." + alarmYear_3;
      lcd1.print(storedAlarmDate_3);
      lcd1.setCursor(0, 1);
      lcd1.print("A#3 Time:");
      String storedAlarmTime_3 = storedHour_3 + ":" + storedMinute_3 + ":00";
      lcd1.print(storedAlarmTime_3);
       lcd1.setCursor(0, 2);
      lcd1.print("A#4 Date:");                   
      String storedAlarmDate_4= alarmDay_4+"."+ alarmMonth_4 + "." + alarmYear_4;
      lcd1.print(storedAlarmDate_4);
      lcd1.setCursor(0, 3);
      lcd1.print("A#4 Time:");
      String storedAlarmTime_4 = storedHour_4 + ":" + storedMinute_4 + ":00";
      lcd1.print(storedAlarmTime_4);
      //MainMenu();
      break;
   }
      } 
  
void zone_1(){
    char key_zone_1 = keypad.getKey();
    lcd1.clear();
    lcd1.setCursor(0,0);
    lcd1.print("*: Change time");
    lcd1.setCursor(0,1);
    lcd1.print("A: Change date");
    
   while (y<1){
      char key_zone_1 = keypad.getKey();
      String getInputFromKeyBoard(int n);
    
    if (key_zone_1 == '*') {
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("*Set Hour:");
        String enteredHour_1 = getInputFromKeyBoard(2);
  
          if (enteredHour_1.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Minute:");
       
          String enteredMinute_1 = getInputFromKeyBoard(2);
          
          if (enteredMinute_1.length() == 2) {
            storedHour_1 = enteredHour_1;
            storedMinute_1 = enteredMinute_1;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("  Alarm Time set to");
            lcd1.setCursor(0, 1);
            String alarmTime_1 = "     "+enteredHour_1 + ":" + enteredMinute_1 + ":00";
            lcd1.print(alarmTime_1);
            delay(1000);
            T_app1 = enteredHour_1 + ":" + enteredMinute_1 + ":00";
            Blynk.virtualWrite(V1,T_app1);
          }}break;}
  
    if (key_zone_1 == 'A'){
          lcd1.clear();
          lcd1.setCursor(0, 0);
          lcd1.print("*Set Month:");
          String enteredMonth_1 = getInputFromKeyBoard(2);
            
         if (enteredMonth_1.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Day:");
            String enteredDay_1 = getInputFromKeyBoard(2);
          
          if (enteredDay_1.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Year:");
            String enteredYear_1 = getInputFromKeyBoard(4);

          if (enteredYear_1.length()== 4){
            storedMonth_1= enteredMonth_1;
            storedDay_1= enteredDay_1; 
            storedYear_1= enteredYear_1;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print(" Alarm Date set to");
            lcd1.setCursor(0, 1);
            alarmMonth_1= enteredMonth_1;
            alarmDay_1= enteredDay_1;
            alarmYear_1= enteredYear_1;
            
            String alarmDate_1= "    "+ enteredMonth_1 + "." + enteredDay_1 + "." + enteredYear_1;
            lcd1.print(alarmDate_1);
            delay(1000);
            D_app1 = enteredMonth_1 + "." + enteredDay_1 + "." + enteredYear_1;
            Blynk.virtualWrite(V0,D_app1);
            
          }}}break;}}
          }
void zone_2(){
      char key_zone_2 = keypad.getKey();
      lcd1.clear();
      lcd1.setCursor(0,0);
      lcd1.print("*: Change time");
      lcd1.setCursor(0,1);
      lcd1.print("A: Change date");
      
 while (y<1){
    char key_zone_2 = keypad.getKey();
    String getInputFromKeyBoard(int n);
  
    if (key_zone_2 == '*') {
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("*Set Hour:");
        String enteredHour_2 = getInputFromKeyBoard(2);
  
        if (enteredHour_2.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Minute:");
           String enteredMinute_2 = getInputFromKeyBoard(2);
          
          if (enteredMinute_2.length() == 2) {
            storedHour_2 = enteredHour_2;
            storedMinute_2 = enteredMinute_2;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("  Alarm Time set to");
            lcd1.setCursor(0, 1);
            String alarmTime_2 = "     "+enteredHour_2 + ":" + enteredMinute_2 + ":00";
            lcd1.print(alarmTime_2);
            delay(1000);
            T_app2 = enteredHour_2 + ":" + enteredMinute_2 + ":00";
            Blynk.virtualWrite(V3,T_app2);
          }}break;}
  
    if (key_zone_2 == 'A'){
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("*Set Month:");
        String enteredMonth_2 = getInputFromKeyBoard(2);
            
         if (enteredMonth_2.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Day:");
       
          String enteredDay_2 = getInputFromKeyBoard(2);
          
          if (enteredDay_2.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Year:");
            String enteredYear_2 = getInputFromKeyBoard(4);

          if (enteredYear_2.length()== 4){
            storedMonth_2= enteredMonth_2;
            storedDay_2= enteredDay_2; 
            storedYear_2= enteredYear_2;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print(" Alarm Date set to");
            lcd1.setCursor(0, 1);
            alarmMonth_2= enteredMonth_2;
            alarmDay_2= enteredDay_2;
            alarmYear_2= enteredYear_2;
            String alarmDate_2= "     "+ enteredMonth_2 + "." + enteredDay_2 + "." + enteredYear_2;
            lcd1.print(alarmDate_2);
            delay(1000);
            D_app2 = enteredMonth_2 + "." + enteredDay_2 + "." + enteredYear_2;
            Blynk.virtualWrite(V2,D_app2); 
          }}}break;}}
          }
          
void zone_3(){
    char key_zone_3 = keypad.getKey();
    lcd1.clear();
    lcd1.setCursor(0,0);
    lcd1.print("*: Change time");
    lcd1.setCursor(0,1);
    lcd1.print("A: Change date");
    
   while (y<1){
      char key_zone_3 = keypad.getKey();
      String getInputFromKeyBoard(int n);
    
    if (key_zone_3 == '*') {
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("*Set Hour:");
        String enteredHour_3 = getInputFromKeyBoard(2);
  
          if (enteredHour_3.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Minute:");
       
          String enteredMinute_3 = getInputFromKeyBoard(2);
          
          if (enteredMinute_3.length() == 2) {
            storedHour_3 = enteredHour_3;
            storedMinute_3 = enteredMinute_3;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("  Alarm Time set to");
            lcd1.setCursor(0, 1);
            String alarmTime_3 = "     "+enteredHour_3 + ":" + enteredMinute_3 + ":00";
            lcd1.print(alarmTime_3);
            delay(1000);
            T_app3 = enteredHour_3 + ":" + enteredMinute_3 + ":00";
            Blynk.virtualWrite(V5,T_app3);
          }}break;}
  
    if (key_zone_3 == 'A'){
          lcd1.clear();
          lcd1.setCursor(0, 0);
          lcd1.print("*Set Month:");
          String enteredMonth_3 = getInputFromKeyBoard(2);
            
         if (enteredMonth_3.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Day:");
            String enteredDay_3 = getInputFromKeyBoard(2);
          
          if (enteredDay_3.length() == 2) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("*Set Year:");
            String enteredYear_3 = getInputFromKeyBoard(4);

          if (enteredYear_3.length()== 4){
            storedMonth_3 = enteredMonth_3;
            storedDay_3 = enteredDay_3; 
            storedYear_3 = enteredYear_3;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print(" Alarm Date set to");
            lcd1.setCursor(0, 1);
            alarmMonth_3= enteredMonth_3;
            alarmDay_3= enteredDay_3;
            alarmYear_3= enteredYear_3;
            
            String alarmDate_3= "    "+ enteredMonth_3 + "." + enteredDay_3 + "." + enteredYear_3;
            lcd1.print(alarmDate_3);
            delay(1000);
            D_app3 = enteredMonth_3 + "." + enteredDay_3 + "." + enteredYear_3;
            Blynk.virtualWrite(V4,D_app3);
            
          }}}break;}}
          }
void zone_4(){
    char key_zone_4 = keypad.getKey();
    lcd1.clear();
    lcd1.setCursor(0,0);
    lcd1.print("*: Change time");
    lcd1.setCursor(0,1);
    lcd1.print("A: Change date");
    
     while (y<1){
      char key_zone_4 = keypad.getKey();
      String getInputFromKeyBoard(int n);
      
      if (key_zone_4 == '*') {
          lcd1.clear();
          lcd1.setCursor(0, 0);
          lcd1.print("*Set Hour:");
          String enteredHour_4 = getInputFromKeyBoard(2);
    
          if (enteredHour_4.length() == 2) {
              lcd1.clear();
              lcd1.setCursor(0, 0);
              lcd1.print("*Set Minute:");
              String enteredMinute_4 = getInputFromKeyBoard(2);
            
            if (enteredMinute_4.length() == 2) {
              storedHour_4 = enteredHour_4;
              storedMinute_4 = enteredMinute_4;
              lcd1.clear();
              lcd1.setCursor(0, 0);
              lcd1.print("  Alarm Time set to");
              lcd1.setCursor(0, 1);
              String alarmTime_4 = "     "+enteredHour_4 + ":" + enteredMinute_4 + ":00";
              lcd1.print(alarmTime_4);
              delay(1000);
              T_app4 = enteredHour_4 + ":" + enteredMinute_4 + ":00";
              Blynk.virtualWrite(V7,T_app4);
            }}break;}
    
      if (key_zone_4 == 'A'){
           lcd1.clear();
           lcd1.setCursor(0, 0);
           lcd1.print("*Set Month:");
           String enteredMonth_4 = getInputFromKeyBoard(2);
              
           if (enteredMonth_4.length() == 2) {
              lcd1.clear();
              lcd1.setCursor(0, 0);
              lcd1.print("*Set Day:");
              String enteredDay_4= getInputFromKeyBoard(2);
            
           if (enteredDay_4.length() == 2) {
              lcd1.clear();
              lcd1.setCursor(0, 0);
              lcd1.print("*Set Year:");
              String enteredYear_4 = getInputFromKeyBoard(4);

           if (enteredYear_4.length() == 4){
              storedMonth_4= enteredMonth_4;
              storedDay_4= enteredDay_4; 
              storedYear_4= enteredYear_4;
              lcd1.clear();
              lcd1.setCursor(0, 0);
              lcd1.print(" Alarm Date set to");
              lcd1.setCursor(0, 1);
              alarmMonth_4= enteredMonth_4;
              alarmDay_4= enteredDay_4;
              alarmYear_4= enteredYear_4;
              
              String alarmDate_4= "     "+ enteredMonth_4 + "." + enteredDay_4 + "." + enteredYear_4;
              lcd1.print(alarmDate_4);
              delay(1000); 
              D_app4 = enteredMonth_4 + "." + enteredDay_4 + "." + enteredYear_4;
              Blynk.virtualWrite(V6,D_app4);
           }}}break;}}
            } 
                  
void supplymonitor(){
        char key_supplymonitor = keypad.getKey();
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("A: Slot#1");
        lcd1.setCursor(0,1);
        lcd1.print("B: Slot#2");
        lcd1.setCursor(0,2);
        lcd1.print("C: Slot#3");
        lcd1.setCursor(0,3);
        lcd1.print("D: Slot#4");
       
while (y<1){
   char key_supplymonitor = keypad.getKey();
    if (key_supplymonitor == 'A'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("     Slot #1");
        delay(500);
        slot_1();
        MainMenu();
        break;}

   if (key_supplymonitor == 'B'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("    Slot #2");
        delay(500);
        slot_2();
        MainMenu();
        break;}
    
    if (key_supplymonitor == 'C'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("    Slot #3");
        delay(500);
        slot_3();
        MainMenu();
        break;}
        
    if (key_supplymonitor == 'D'){
        lcd1.clear();
        lcd1.setCursor(0,0);
        lcd1.print("    Slot #4");
        delay(500);
        slot_4();
        MainMenu();
        break;}
  }}

void slot_1(){
     lcd1.clear();
     lcd1.setCursor(0, 0);
     lcd1.print("Medicine available   ");   
     lcd1.setCursor(0,1);
     lcd1.print(S_appstore1); 
     delay(2000);  
     } 
void slot_2(){
     lcd1.clear();
     lcd1.setCursor(0, 0);
     lcd1.print("Medicine available   ");   
     lcd1.setCursor(0,1);
     lcd1.print(S_appstore2); 
     delay(2000); 
  }
void slot_3(){
     lcd1.clear();
     lcd1.setCursor(0, 0);
     lcd1.print("Medicine available   ");   
     lcd1.setCursor(0,1);
     lcd1.print(S_appstore3); 
     delay(2000);
     } 
  
void slot_4(){
    lcd1.clear();
     lcd1.setCursor(0, 0);
     lcd1.print("Medicine available   ");   
     lcd1.setCursor(0,1);
     lcd1.print(S_appstore4); 
     delay(2000); 
  } 
void IR_1(){
  bool timesup1= false;
  bool timesup2= false;
  bool timesup3= false;
  bool timesup4= false;
  
  ir_1 =digitalRead(IRPin_1);
          if (ir_1 == LOW){ //detect
            
           digitalWrite(irLED_1,HIGH);
           
           if(on_slot1 == true){
            uint32_t period_5 = 10 * 60000L;       // 10 minutes loop
                for( uint32_t tStart_5 = millis();  (millis()-tStart_5) < period_5;){
                  timedisplay();
                  StatusApp1();

                  ir_1 =digitalRead(IRPin_1);
                  if (ir_1 == HIGH){  //wala
                    digitalWrite(irLED_1,LOW);
                    on_slot1 = false;
                    break;
                    }}
                   ir_1 =digitalRead(IRPin_1);
                   if (ir_1 == LOW){
                      timesup1= true;
                      digitalWrite(irLED_1,HIGH);
                      }
                      
                    if (timesup1 == true){
                    status_flag1 = "WARNING"; 
                    Blynk.virtualWrite (V16, status_flag1 );
                    Blynk.logEvent("10_minutes_late");
                    on_slot1 = false;
                    }}
           
            if(on_slot2 == true){
            uint32_t period_6 = 10 * 60000L;       // 5 minutes loop
                for( uint32_t tStart_6 = millis();  (millis()-tStart_6) < period_6;){
                  timedisplay();
                  StatusApp2();

                  ir_1 =digitalRead(IRPin_1);
                  if (ir_1 == HIGH){  //wala
                    digitalWrite(irLED_1,LOW);
                    on_slot2 = false;
                    break;
                    }}
                   ir_1 =digitalRead(IRPin_1);
                   if (ir_1 == LOW){
                      timesup2= true;
                      digitalWrite(irLED_1,HIGH);
                      }
                      
                    if (timesup2 == true){
                    status_flag2 = "WARNING"; 
                    Blynk.virtualWrite (V17, status_flag2 );
                    Blynk.logEvent("10_minutes_late");
                    on_slot2 = false;
                    }}
                    
            if(on_slot3 == true){
            uint32_t period_7 = 10 * 60000L;       // 5 minutes loop
                for( uint32_t tStart_7 = millis();  (millis()-tStart_7) < period_7;){
                  timedisplay();
                  StatusApp3();

                  ir_1 =digitalRead(IRPin_1);
                  if (ir_1 == HIGH){  //wala
                    digitalWrite(irLED_1,LOW);
                    on_slot3 = false;
                    break;
                    }}
                   ir_1 =digitalRead(IRPin_1);
                   if (ir_1 == LOW){
                      timesup3= true;
                      digitalWrite(irLED_1,HIGH);
                      }
                      
                    if (timesup3 == true){
                    status_flag3 = "WARNING"; 
                    Blynk.virtualWrite (V18, status_flag3);
                    Blynk.logEvent("10_minutes_late");
                    on_slot3 = false;
                    }}
                    
            if(on_slot4 == true){
            uint32_t period_8 = 10 * 60000L;       // 5 minutes loop
                for( uint32_t tStart_8 = millis();  (millis()-tStart_8) < period_8;){
                  timedisplay();
                  StatusApp4();

                  ir_1 =digitalRead(IRPin_1);
                  if (ir_1 == HIGH){  //wala
                    digitalWrite(irLED_1,LOW);
                    on_slot4 = false;
                    break;
                    }}
                   ir_1 =digitalRead(IRPin_1);
                   if (ir_1 == LOW){
                      timesup4= true;
                      digitalWrite(irLED_1,HIGH);
                      }
                      
                    if (timesup4 == true){
                    status_flag4 = "WARNING"; 
                    Blynk.virtualWrite (V19, status_flag4 );
                    Blynk.logEvent("10_minutes_late");
                    on_slot4 = false;
                    }}
            }
            
          else {  
           digitalWrite(irLED_1,LOW);
           status_flag1 = "Not detected"; 
           Blynk.virtualWrite (V16, status_flag1 );
           status_flag2 = "Not detected"; 
           Blynk.virtualWrite (V17, status_flag2 );
           status_flag3 = "Not detected"; 
           Blynk.virtualWrite (V18, status_flag3 );
           status_flag4 = "Not detected"; 
           Blynk.virtualWrite (V19, status_flag4 );
            }} 
void IR_2(){
   ir_2=digitalRead(IRPin_2); 
     if (ir_2==LOW) {
         digitalWrite(irLED_2,HIGH);
         dispensecount();
         }
     else{
        digitalWrite(irLED_2,LOW);
  }}
  
void dispensecount(){
  if (dispense_1 == true){
    S_appstore1 = S_appstore1 - 1; 
    Blynk.virtualWrite(V9, S_appstore1);
    Supplyapp_notif1();
    dispense_1 = false;
    }
  if (dispense_2 == true){
     S_appstore2 = S_appstore2 - 1;
     Blynk.virtualWrite(V11, S_appstore2);
     Supplyapp_notif2();
     dispense_2 = false;
    }
  if (dispense_3 == true){
     S_appstore3 = S_appstore3 - 1;
     Blynk.virtualWrite(V13, S_appstore3);
     Supplyapp_notif3();
     dispense_3 = false;
    }
  if (dispense_4 == true){
     S_appstore4 = S_appstore4 - 1;
     Blynk.virtualWrite(V15, S_appstore4);
     Supplyapp_notif4();
     dispense_4= false;
    }
}

void Supplyapp_notif1(){
  if (S_appstore1 <= 3){
    Blynk.logEvent("slot1_low_supply","The medicines are running low in Slot#1. Please refill as soon as possible. Stay healthy!");
    }}
void Supplyapp_notif2(){    
  if (S_appstore2 <= 3){
    Blynk.logEvent("slot2_low_supply","The medicines are running low in Slot#2. Please refill as soon as possible. Stay healthy!");
    }}
void Supplyapp_notif3(){    
 if (S_appstore3 <= 3){
  Blynk.logEvent("slot3_low_supply","The medicines are running low in Slot#3. Please refill as soon as possible. Stay healthy!");
    }}
void Supplyapp_notif4(){       
  if (S_appstore4 <= 3){
  Blynk.logEvent("slot4_low_supply","The medicines are running low in Slot#4. Please refill as soon as possible. Stay healthy!");
  }}

void emptyslots(){
    if (S_appstore1 == 0){
     Blynk.logEvent("slot1_low_supply","Slot#1 is empty. Please refill as soon as possible. Stay healthy!");
    }
    if (S_appstore1 == 0){
     Blynk.logEvent("slot2_low_supply","Slot#2 is empty. Please refill as soon as possible. Stay healthy!");
    }
    if (S_appstore1 == 0){
     Blynk.logEvent("slot3_low_supply","Slot#3 is empty. Please refill as soon as possible. Stay healthy!");
    }
    if (S_appstore1 == 0){
     Blynk.logEvent("slot4_low_supply","Slot#4 is empty. Please refill as soon as possible. Stay healthy!");
    }}       
    
void StatusApp1(){
  digitalWrite(irLED_1,HIGH);
  status_flag1 = "Detected";
  Blynk.virtualWrite (V16,status_flag1);
  } 
void StatusApp2(){
  digitalWrite(irLED_1,HIGH);
  status_flag2 = "Detected";
  Blynk.virtualWrite (V17,status_flag2);
  } 
void StatusApp3(){
  digitalWrite(irLED_1,HIGH);
  status_flag3 = "Detected";
  Blynk.virtualWrite (V18,status_flag3);
  } 
void StatusApp4(){
  digitalWrite(irLED_1,HIGH);
  status_flag4 = "Detected";
  Blynk.virtualWrite (V19,status_flag4);
  }   

void timedisplay(){
        String dateString= rtc.getDateStr();
        String d= dateString.substring (0,2);
        String m= dateString.substring (3,5);
        String yr= dateString.substring (6,10);
        String currentDate= d+"."+ m+"."+ yr;
        
        String timeString = rtc.getTimeStr();
        String realHour = timeString.substring(0, 2);
        String realMinute = timeString.substring(3, 5);
        String currentTime = realHour+":"+realMinute+":00";
      
        String DOWString= rtc.getDOWStr();
        String currentDOW= DOWString;
        
        Main();
        
        delay(2000); //required for print
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("Date:"+ currentDate);
        lcd1.setCursor(0, 1);
        lcd1.print("Time:"+ currentTime);
        lcd1.setCursor(0, 2);
        lcd1.print("Day: " + currentDOW);
  }
void blynkfunc(){
        Blynk.virtualWrite (V9, S_appstore1); //supply
        Blynk.virtualWrite (V11, S_appstore2);
        Blynk.virtualWrite (V13, S_appstore3);
        Blynk.virtualWrite (V15, S_appstore4);
        
        Blynk.virtualWrite (V16, status_flag1); // detected or not
        Blynk.virtualWrite (V17, status_flag2);
        Blynk.virtualWrite (V18, status_flag3);
        Blynk.virtualWrite (V19, status_flag4);

        Blynk.virtualWrite (V20, bpmvalue_app1);
        Blynk.virtualWrite (V21, bpmstatus_app1);
  }  
