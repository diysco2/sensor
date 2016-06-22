/* 

LI-820 data logger code

Based on examples writen by Ladyada.  Works with Adafruit Ultimate GPS v1.0 module


(c) Zoran Nesic                 File created:        Nov 21, 2013
                                Last modification:   Jan, 22, 2015


Revisions

Nov 27, 2014
    - added port control for LI-820 and pump relays.  Currently the program just
      turns them on.  Later on I'll build in the automatic shutdown when the input voltage
      is low.
Dec 11, 2013
    - Reduced the number of bad data files by making sure that
      logging is only done when GPS has a fix!!  
    - SD errors now blink the external LED indicator too.
Dec 10, 2013 
  - Create daily folders
  - Use an LED indicator when SD card is not present 
    After each card change user needs to reset Arduino to 
    get the logging going again.  This also turns off the LED.
Dec 6, 2013
  - Clean up and comment
Dec 5, 2013
  - First version that does it all: parse GPS and LI-820 data 
    and stores them into hhour files on the SD card
Dec 3, 2013
  - Switched from stacked shield setup to jumper-wire setings.
  - Serial #2 connected to GPS
  - Serial #3 connected to LI-820
Nov 30, 2013
  - Switched to Arduino Mega2560 Rev 3

Jan 22, 2015
  - merged Zoran's code with Joey's code
  - adjusted code to fit new "co2" board configs.

Mar 23, 2015
  - add visual light feedback
  
April 30, 2015
  - add delay time flag field
*/  

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <avr/sleep.h>
#include <math.h>; // added by Joey - for converting lat/long
#include <OneWire.h> // added by joey - for temperature sensor
#include <DallasTemperature.h> // added by joey - for temperature sensor


// Initiate SoftwareSerial communication with GPS
Adafruit_GPS GPS(&Serial2);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  true

// Number of decimal places when printing results.
#define DECIMALS 5

// Set the pins used
#define chipSelect 10
#define ledPin 13
#define SD_workingLEDpin 44    // This pin is HIGH if there is an SD writing problem
#define LEDintensity 30        // intensity of LED on the SD_workingLEDpin
#define RELAY_LI820 3          // Pin that turns the LI820 relay ON
#define RELAY_PUMP  4          // Pin that turns the pump relay ON

/* ---- TEMP SENSOR --- */
// Set the pins used for temperature - Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2 //51
#define TEMPERATURE_PRECISION 10

// Set to true if you want two output strings to be printed on Serial using two different 
// methods (troubleshooting only)
#define DEBUG_PRINT  false

// ---- Flag for calculating delay time ---- //
//#define flagPin 9
int flagPin = 9;
int val = 0; 

const int MAX_BUFFER_LEN = 200;

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

char newLI820Line[MAX_BUFFER_LEN];
long lCounter = 0;
int lPointer=0;
char oldFileName[25];
char newFileName[25];
File logfile;

// Visual Feedback LEDs
int led_gpsfix = 5;
int led_datalogging = 6;
//int led_warning = 9;


/* ---- TEMP SENSOR --- */
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// arrays to hold device addresses
DeviceAddress Temp1, Temp2;

// --- debugging --- //
unsigned long millis_old;

/*
========================================================
Setup parameters
========================================================
*/
void setup()  
{
  // Visual LED Feedback
  pinMode(led_gpsfix, OUTPUT);
  // digitalWrite(led_gpsfix,HIGH);  
  pinMode(led_datalogging, OUTPUT);
  // digitalWrite(led_datalogging,HIGH);
  // pinMode(led_warning, OUTPUT);
  // digitalWrite(led_warning,HIGH);
  
  // Set both file names to null
  oldFileName[0]=0;
  newFileName[0]=0;
  
  // PC Serial port output at 115200 baud
  Serial.begin(115200);
  Serial.println("LI-820 Logging program by Zoran Nesic, Biomet/Micromet - UBC");
  Serial.println(freeRam());

  // Select the LED pin mode
  pinMode(ledPin, OUTPUT);
  pinMode(SD_workingLEDpin, OUTPUT);
  
  // Select the RELAY pin mode
  pinMode(RELAY_LI820, OUTPUT);
  pinMode(RELAY_PUMP, OUTPUT);
  // Set relay high (pump and LI-820 are ON)
  digitalWrite(RELAY_LI820,HIGH);
  digitalWrite(RELAY_PUMP,HIGH);
  
  // Select flagPin mode:
  pinMode(flagPin, INPUT_PULLUP);
  //digitalWrite(flagPin, HIGH);
  
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
 // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect, 11, 12, 13)) {
    Serial.println("Card init. failed!");
    error(2);
  }
    // Open a "dummy" file as a place holder.  It will get closed in the main loop.
    logfile = SD.open("dummy", FILE_WRITE);
    if( ! logfile ) {
      Serial.print("Couldnt create "); Serial.println(newFileName);
      error(3);
    }  
  
  // 9600 NMEA is the default baud rate for Adafruit GPS
  GPS.begin(9600);
  
  // Open communication with LI-820
  Serial3.begin(9600);

  /* --- TEMP SENSOR - set up: START -- */
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  // search for devices on the bus and assign based on an index
  if (!sensors.getAddress(Temp1, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(Temp2, 1)) Serial.println("Unable to find address for Device 1"); 
  // set the resolution
  sensors.setResolution(Temp1, TEMPERATURE_PRECISION);
  sensors.setResolution(Temp2, TEMPERATURE_PRECISION);
  // print the temperature resolution to the console
  Serial.print("Heater Thermometer Resolution: ");
  Serial.print(sensors.getResolution(Temp1), DEC); 
  Serial.println();
  Serial.print("Box Thermometer Resolution: ");
  Serial.print(sensors.getResolution(Temp2), DEC); 
  Serial.println();

  Serial.println();
  
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  delay(1000);
  // Ask for firmware version
  Serial2.println(PMTK_Q_RELEASE);
}

uint32_t timer = millis();

/*
========================================
Main loop
========================================
*/
void loop()                     
{
  char *p,*p1;
  char tmpStr[20];
  char newPathName[10];

  // Call GPS Checker & Blink LED if GPS fix exists
  // gpsChecker();
  
  // if a it's time to receive GPS's data then we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) { 

    if (!GPS.parse(GPS.lastNMEA())){   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
    }  
    else{
      // by adding GPS.fix && I have disabled the option of logging LI-820 data without
      // when GPS is not having fix.  Maybe not a bad thing here but it may mess with
      // my thinking later on if I try to use the same setup in another setting
      if ( GPS.fix && GPS.year > 12 && GPS.year < 99 && GPS.month > 0 && GPS.month < 13 
          && GPS.hour >-1 && GPS.hour < 25 && GPS.minute > -1 && GPS.minute < 60) {

        // If proper GPS data has been received and the clock data makes sense
        // open a new file for writing
        getFileName(newFileName);
        // if the new file is different than the old file name then
        // flush the buffer and close the old file
        if (strcmp(newFileName,oldFileName)!=0){
          Serial.println("Old file flushed and closed");
          if (logfile) {
            // if logfile is open then flush and close
            logfile.flush();
            logfile.close();
          }
          // do oldFileName = newFileName
          strcpy(oldFileName,newFileName);
        }
         
        if (! logfile) {
          // If there is no logfile then open a new one.
          getPathName(newPathName);
          getFileName(newFileName);
          SD.mkdir(newPathName);
          Serial.print("Made new folder: ");Serial.println(newPathName);
          Serial.print("Atempt to open: "); Serial.println(newFileName);
          logfile = SD.open(newFileName, FILE_WRITE);
          if( ! logfile ) {
            Serial.print("Couldnt create - "); Serial.println(newFileName);
            error(3);
          }
          Serial.print("Writing to "); Serial.println(newFileName);
          strcpy(oldFileName,newFileName);
        }
      }  
    }
  }

  // if the line grows well over the regular length of LI820 line
  // reset it.  Most likely caused by errors in comm. like not checking 
  // the comm port buffer often enough.
  if (lPointer > MAX_BUFFER_LEN - 2){
    lPointer = 0;
    newLI820Line[lPointer] = '\0';
  }

  // // Check if a new character from LI-820 has arrived
  // if (Serial3.available()>0) {  
  //   // Read an incoming byte
  //   char cNew = Serial3.read();
  //   if (cNew > -1) {
  //     // add it to the current line
  //     newLI820Line[lPointer] = cNew;
  //     lPointer += 1;
  //     newLI820Line[lPointer] = '\0';     
  //   }

  
  // Check if a new character from LI-820 has arrived
  while (Serial3.available()>0) {  
    // Read an incoming byte
    char cNew = Serial3.read();
    if (cNew > -1) {
      // add it to the current line
      newLI820Line[lPointer] = cNew;
      lPointer += 1;
      newLI820Line[lPointer] = '\0';   
    }  

    // when end of the LI820 line ("</li8") is received do the parsing
    // I didn't use here "</li820>" because I wanted it to be generic and
    // work with "</li840>" too
    p = strstr(newLI820Line,"</li8");
    if (lPointer> 20 && p) {
      // If LI820 line has the proper end mark ("/li8")
      p1 = strstr(newLI820Line,"<li8");
      if (p1) {  
        // And if LI820 line has a proper start mark ("li8") then
        // extract the values (co2, Tcell, Pcell, Vin)
        float co2   = parseFloat(newLI820Line,"<co2>");
        float Tcell = parseFloat(newLI820Line,"<celltemp>");      
        float Pcell = parseFloat(newLI820Line,"<cellpres>");
        float Vin   = parseFloat(newLI820Line,"<ivolt>");
        
        char buffGPSPos[100];
        char myBuffer[150];
        char myNumber[12];
        myBuffer[0] = 0;
        
        // First create myBuffer string by adding GPS date, time, fix and fix_quality
        char buff1[50];sprintf(buff1,"%02i/%02i/%4i,%02i:%02i:%02i.%03i,%i,%i",GPS.day,GPS.month,GPS.year+2000,
                          GPS.hour,GPS.minute,GPS.seconds,GPS.milliseconds,
                          GPS.fix,
                          GPS.fixquality
                );
        strcat(myBuffer, buff1);
        
        if (DEBUG_PRINT){      
          Serial.println("-----------------------------");
          Serial.print(GPS.day, DEC); Serial.print('/');
          Serial.print(GPS.month, DEC); Serial.print("/20");
          Serial.print(GPS.year, DEC);Serial.print(", ");
          Serial.print(GPS.hour, DEC); Serial.print(':');
          Serial.print(GPS.minute, DEC); Serial.print(':');
          Serial.print(GPS.seconds,DEC);Serial.print('.');Serial.print(GPS.milliseconds);
          Serial.print(", "); Serial.print((int)GPS.fix);
          Serial.print(", "); Serial.print((int)GPS.fixquality); Serial.print(", ");
        }      
        
        if (GPS.fix) { 
          strcat(myBuffer,",");

          // if all of the above are true - light on
          digitalWrite(led_gpsfix,HIGH);      

          // Add minus sign for the South hemisphere and store to myBuffer
          if (GPS.lat == 'N'){
            dtostrf(geoConvert(GPS.latitude),10,DECIMALS,myNumber);
          }
          else{
            dtostrf(-geoConvert(GPS.latitude),10,DECIMALS,myNumber);
          }            
          
          strcat(myBuffer,myNumber);         
          strcat(myBuffer,",");
          
          // add minus sign for West longitudes and store to myBuffer
          if (GPS.lon == 'E'){
            dtostrf(geoConvert(GPS.longitude),10,DECIMALS,myNumber);
          }
          else{
            dtostrf(-geoConvert(GPS.longitude),10,DECIMALS,myNumber);
          }            
          strcat(myBuffer,myNumber);
          
          // add GPS speed, altitude and # of satellites to the output buffer (myBuffer)
          strcat(myBuffer,",");dtostrf(GPS.speed*1.852,6,DECIMALS,myNumber);strcat(myBuffer,myNumber);  // to convert knots to kmh
          strcat(myBuffer,",");dtostrf(GPS.altitude,6,DECIMALS,myNumber);strcat(myBuffer,myNumber);
          strcat(myBuffer,",");dtostrf(GPS.satellites,2,0,myNumber);strcat(myBuffer,myNumber);          
          if (DEBUG_PRINT){
            Serial.print(GPS.latitude, 5); Serial.print(GPS.lat);
            Serial.print(", "); 
            Serial.print(GPS.longitude, 5); Serial.print(GPS.lon);
            Serial.print(","); Serial.print(GPS.speed);
            Serial.print(", "); Serial.print(GPS.altitude);
            Serial.print(", "); Serial.print((int)GPS.satellites);
          }
        }
        else{
          // No GPS data.  Print out empty fields.
          sprintf(tmpStr,"%s",",,,,,");
          strcat(myBuffer,tmpStr);

          // if no gps - then blink light
          digitalWrite(led_gpsfix,HIGH);
          delay(500);
          digitalWrite(led_gpsfix,LOW); 
          delay(500);

          if (DEBUG_PRINT){
            Serial.print(",,,,");
          }
        }
        if (DEBUG_PRINT){
          Serial.print(", ");
          Serial.print(co2,DECIMALS);
          Serial.print(", ");
          Serial.print(Tcell,DECIMALS);
          Serial.print(", ");
          Serial.print(Pcell,DECIMALS);
          Serial.print(", ");
          Serial.println(Vin,DECIMALS);
        }
        // Add co2,Tcell, Pcell and Vin to the output buffer (myBuffer)
        sprintf(tmpStr,",%s",dtostrf(co2,6,DECIMALS,myNumber));strcat(myBuffer,tmpStr);
        sprintf(tmpStr,",%s",dtostrf(Tcell,6,DECIMALS,myNumber));strcat(myBuffer,tmpStr);
        sprintf(tmpStr,",%s",dtostrf(Pcell,6,DECIMALS,myNumber));strcat(myBuffer,tmpStr);
        sprintf(tmpStr,",%s",dtostrf(Vin,6,DECIMALS,myNumber));strcat(myBuffer,tmpStr);
        //Serial.println(myBuffer);

        //temp sensor
        sensors.requestTemperatures(); // Send the command to get temperatures
        float Temp1C = sensors.getTempC(Temp1);
        float Temp2C = sensors.getTempC(Temp2);
        if (DEBUG_PRINT){
          Serial.print(", ");
          Serial.print(Temp1C, DECIMALS);
          Serial.print(", ");
          Serial.println(Temp2C, DECIMALS);
          Serial.print(", ");
        } 
        sprintf(tmpStr,",%s",dtostrf(Temp1C,6,DECIMALS,myNumber));strcat(myBuffer,tmpStr);
        sprintf(tmpStr,",%s",dtostrf(Temp2C,6,DECIMALS,myNumber));strcat(myBuffer,tmpStr);
        
        // ----- Flag system during delay time test ------ //
        
        val = digitalRead(flagPin);
        if (val == LOW){
          sprintf(tmpStr,",%s",dtostrf(1,1,0,myNumber));strcat(myBuffer,tmpStr);
        } else if (val == HIGH){
          sprintf(tmpStr,",%s",dtostrf(0,1,0,myNumber));strcat(myBuffer,tmpStr);
        } else{
          sprintf(tmpStr,",%s",dtostrf(2,1,0,myNumber));strcat(myBuffer,tmpStr);
        }
        
        Serial.println(myBuffer);

        // Visual Output for LEDs logging data
        if(!myBuffer){
          digitalWrite(led_datalogging,HIGH);
          delay(500);
          digitalWrite(led_datalogging,LOW); 
          delay(500);
        }
        if(myBuffer){
          // Visual Output for LEDs logging data
          digitalWrite(led_datalogging,HIGH);
        }

        // Save the buffer to SD card:
        if (logfile) {
          // if logfile is open then save the new line and flush the buffer
          int bytesWriten = logfile.println(myBuffer);
          logfile.flush();

          if (bytesWriten < getLen(myBuffer)){
            Serial.println("Error writing on SD");
            // If there is an error writing on the SD card while logfile != false
            // then user must have removed the card.  Turn on the external LED to
            // make sure that user will notice this and reset the module once the card
            // is back and ready.

            analogWrite(SD_workingLEDpin,LEDintensity);
          }
          else{
            // Card writing worked.  Turn OFF the external LED.
            analogWrite(SD_workingLEDpin,0);
          }
        }
    
        // Clear (already processed) newLI820Line
        lPointer = 0;
        newLI820Line[lPointer] = '\0';

        // Print out free memory after every 15 lines are captured.
        /*
        lCounter += 1;
        if (lCounter > 15){
            Serial.print("Free mem = ");
            Serial.println(freeRam());
            lCounter = 0;        
              } 
        */
      }
      else{ 
        // Otherwise an incomplete line has been received from LI820.
        // Delete the incomplete line and keep waiting for the next one
        lPointer = 0;
        newLI820Line[lPointer] = '\0';

        Serial.println("-9");
      }
    }  
  }
} // End of main loop

/*---------------------------------------------------------------
Function parseFloat:
  - Parses a string looking for a start string (like: "<co2>"")
  - grabs the characters that follow the start string and
    converts them to a float.
-----------------------------------------------------------------*/
float parseFloat( char *cnewLine, char *startString){
  float result;
  char *p;
  p = strstr(cnewLine,startString) + getLen(startString);
  result = atof(p);
  return result;
  
}  

/*---------------------------------------------------------
Function getLen:
  - returns the length of an char array (before the NULL)
-----------------------------------------------------------*/
int getLen( char *buffer){
  int i=0;
  while(buffer[i])
    i++;
  return i;  
}

/*---------------------------------------------------------
Function getFileName:
  - creates a file name for each hhour of data.  
  - The time stamp refers to the end of half hour period
-----------------------------------------------------------*/
void getFileName(char fileName[]){
  // Create file name of the format: yymmddqq.dat
  // where qq goes from 02 to 96
  // 00:00 - 00:29 -> qq = 02
  // 00:30 - 00:59 -> qq = 04
  // 01:00 - 01:29 -> qq = 06
  // 23:30 - 23:59 -> qq = 96
  
  // Return in the array is too short
/*  if (sizeof(*fileName) < 15 ){
    Serial.println(sizeof(*fileName));
    Serial.println("Didnt get the file name!");
   return;
  } 
*/  
  int qq;
  qq = GPS.hour * 4;
  if (GPS.minute >= 0 && GPS.minute <=29){
    qq += 2;
  }
  else{
    qq += 4;
  }
  sprintf(fileName,"%02i%02i%02i/%02i%02i%02i%02i.dat",GPS.year,GPS.month,GPS.day,GPS.year,GPS.month,GPS.day,qq);
//  Serial.println(fileName);
}

/*---------------------------------------------------------
Function getPathName:
  - creates a path name for the current time.
-----------------------------------------------------------*/
void getPathName(char pathName[]){
  // Create path name of the format: yymmdd
  
  sprintf(pathName,"%02i%02i%02i",GPS.year,GPS.month,GPS.day);
//  Serial.println(pathName);
}

/*------------------------------
Function error:
  - blinks out an error code
--------------------------------*/
void error(uint8_t errno) {
/*
  if (SD.errorCode()) {
    putstring("SD error: ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }
  */
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(ledPin, HIGH);
      analogWrite(SD_workingLEDpin,LEDintensity);
      delay(100);
      digitalWrite(ledPin, LOW);
      analogWrite(SD_workingLEDpin,0);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}


/*---------------------------------------
Function freeRam:
  - Calculates available RAM memory
  - Arduino Mega 2560 starts with 8k free
-----------------------------------------*/
int freeRam () {
extern int __heap_start, *__brkval; 
int v; 
return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


/*-----------------------------------------------
Interrupt routine
  - Interrupt is called once a millisecond, 
    looks for any new GPS data, and stores it
-------------------------------------------------*/
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
//Serial.print(c);  
#ifdef UDR0
//  if (GPSECHO)
//    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

/*-----------------------------------------------
Function useInterrupt
  - Initiates or stops the interrupts
-------------------------------------------------*/
void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}


// Create lat/long conversion function (see: http://arduinodev.woofex.net/2013/02/06/adafruit_gps_forma/) 
double geoConvert( float degMin ) {
  double min = 0.0;
  double decDeg = 0.0;
          
  // get the minutes, fmod() requires double
  min = fmod( (double)degMin, 100.0);
          
  // rebuild coordinates in decimal degrees
  degMin = (int)( degMin / 100 );
  decDeg = degMin + ( min / 60 );
          
  return decDeg;
}


/* --- Temperature functions -- */
// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress));
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// // // Blink LED if There is GPS Fix
// void gpsChecker(){
//     if(GPS.fix){
//       digitalWrite(led_gpsfix,HIGH);
//     }
//     if(!GPS.fix){
//       digitalWrite(led_gpsfix,HIGH);
//       delay(500);
//       digitalWrite(led_gpsfix,LOW); 
//       delay(500);
//     }
// }


/*
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    Serial3.println(GPS.lastNMEA());
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }
  */

































