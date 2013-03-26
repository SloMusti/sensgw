/**
 * wlan slovenija sensor gateway
 * for sensgw v0.2 board and v0.5 board
 * for use with MPL115A2 I2C temperature and pressure sensor, controlls an LED
 * Instructions
 * see http://dev.wlan-si.net/wiki/Telemetry/sensgw
 * Author: Luka Mustafa - Musti (musti@wlan-si.net)
 * Revision history
 * 2.3.2013 - version 0.1 released
 */

// The following libraries are required for communication and sensors
// Include libraries required for your sensors

// I2C library used is not standard Wire, but an improved version of it
//http://dsscircuits.com/articles/arduino-i2c-master-library.html
#include "I2C.h"
// MPL115A2 sensor library modified to use the correct I2C library
#include <MPL115A2.h>
// 1-wire library as on Arduino vebsite
#include <OneWire.h>
// http://milesburton.com/Dallas_Temperature_Control_Library
#include <DallasTemperature.h> 

// System variables
#define hwversion 0.2 // this must equal the version printed on the board
#define fwversion 0.1 // this must equal to the version of this code

//***************************************************************
// Definitions of variables and objects for sensors

// 1WIRE: data wire is plugged into D3
#define ONE_WIRE_BUS 3
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature onewire(&oneWire);

//***************************************************************
// System variables - DO NOT EDIT
// UART variables
char RxBuffer[20]; //RX buffer
char data; // storing intermediate chars
byte index = 0; // buffer index

//for loop variable
char i=0;

//other variables
boolean ledstat=0;
boolean i2cpwr=0; //I2c power transistor PNP control, low=ON

//***************************************************************
// Pointers to functions called via serial port

// Command "ACOM /x calls" GETfunctions array
// ACOM /0 must always be (int)testGET, where x is a positive integer

// Command "ACOM /x /v" calls SETfunctions array, where v is an integer
// ACOM /0 v must always be (int)testSET

int GETfunctions[4]={(int)testGET,(int)readPressure,(int)readTemp,(int)read1Wtemp};
int SETfunctions[2]={(int)testSET,(int)setLED};

//***************************************************************
// Functions for reading/writing and sensor communication

//**************************
// GET: 1Wire temperature
void read1Wtemp(){
  onewire.requestTemperatures();
  Serial.print(onewire.getTempCByIndex(0));//can be expanded for multiple sensors
  Serial.print("\n\n"); //terminator
}

//**************************
// GET: MPL115A2 teperature
void readTemp(){
  MPL115A2.ReadSensor();
  MPL115A2.shutdown();
  Serial.print(MPL115A2.GetTemperature());
  Serial.print("\n\n"); //terminator
}
//**************************
// GET: MPL115A2 pressure
// beware, measurement is not sea level compensated
void readPressure(){
  MPL115A2.ReadSensor();
  MPL115A2.shutdown();
  Serial.print(MPL115A2.GetPressure());
  Serial.print("\n\n"); //terminator
}

//**************************
// SET: Control LED
void setLED(int data){
  digitalWrite(5, data);
  Serial.print("OK"); //ever SET function must return something
  Serial.print("\n\n"); //terminator
}

//****************************************************************
// Initialization sequence, edit ONLY the specified part
void setup() {
    //initialize UART
    Serial.begin(115200);
    
    //initialize I2C
    I2c.begin();
    I2c.pullup(true);
    I2c.setSpeed(1); //400kHz
    //initialize I2C power transistor
    pinMode(4, OUTPUT);
    digitalWrite(4, i2cpwr);
    
    //**************************
    // EDIT from here!!!
    
    //initialize sensors PUT YOUR SENSOR INITIALIZATION CODE HERE
    onewire.begin();
    MPL115A2.begin();
    
    //setup pins
    pinMode(5, OUTPUT); //LED
}



//****************************************************************
// do not edit from here on, unless you know what your are doing

// tests the communication and returns version
void testGET(){
  Serial.print("sensgw hw ");
  Serial.print(hwversion);
  Serial.print(" fw ");
  Serial.print(fwversion);
  Serial.print("\n\n"); //terminator  
}

void testSET(int data){
  Serial.print("test write, decimal value: ");
  Serial.print(data);
  Serial.print("\n\n"); //terminator
}

void loop()
{
  // handles incoming UART commands
  // wait for at least 7 characters:
  while (Serial.available() > 7) {
    int x = 0;
    int v = 0;
    // Check for "ACOM /" string
    if(Serial.find("ACOM /")){
      // parse command number after /
      x = Serial.parseInt(); 
      //if termianted, call GET
      if (Serial.read() == '\n'){
        // call GET function
        ((void (*)()) GETfunctions[x])();
      }
      //if nto terminated, read value argument and call SET
      else{
        v = Serial.parseInt();
        ((void (*)(int)) SETfunctions[x])(v); 
      }
      //more arguments can be supported in the same manner
    }
  }
}

