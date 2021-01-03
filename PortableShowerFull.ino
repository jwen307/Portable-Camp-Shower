//PortableShowerFull
//Jeffrey Wen
//7-11-19

//This code is for the portable shower project. It integrates code from all of the Subsystem (SSD#) modules

//Libraries for the OLED display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Libaries for the Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>

#define HEATER 9         //Heater is on pin 9

#define WATERPUMP 10     //Pump is on pin 10

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Creates a OLED object called display

//Button Definitions
#define ModePin 2     //Only pins 2 and 3 can be used for interrupts
#define SECOND 3      //Second button will change the pump speed or lower the temp
#define THIRD 11       //Third button will turn the flow on or off or increase the temp

//Temp data wire is plugged into port 5 on the Arduino
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//Global Functions
int desiredTemp = 100;      //Variable for the desired temp set by the user
int currentTemp;            //Variable for the current temp
int pumpSpeed = 1;          //Variable to track the pump speed set by the user
volatile byte mode = LOW;   //Variable to keep track if it is in heat or shower mode
volatile byte flow = LOW;   //Variable to turn the pump on or off
volatile byte heat = LOW;   //Variable to turn the heat on or off




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OLED Functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HeatModeDisplay()            //Function to display the heat on the OLED
{
  //Clear the display
  display.clearDisplay();


  display.setTextSize(2);         //Set the text size
  display.setTextColor(WHITE);    //Set the text color (must be white)
  display.setCursor(10,0);        //Set the starting location of the cursor
  display.print("Heat: ");        //Prints the words
  if(heat == HIGH)
  {
    display.print("ON");
  }
  else
  {
    display.print("OFF");
  }
  display.setCursor(10,20);
  display.setTextSize(1);
  display.print("Des:");
  display.print(desiredTemp);     //Display the Desired Temperature
  display.print(" Cur:");
  display.print(currentTemp);     //Display the Current Temperature

  display.display();              //Loads the display buffer to the screen
}

void ShowerModeDisplay()          //Function to display the shower mode
{
  //Clear the display
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Shower:");
  if(flow == HIGH)
  {
    display.print("ON");
  }
  else
  {
    display.print("OFF");
  }
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("Pump Speed: ");
  display.print(pumpSpeed);

  display.display();
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Button Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This interrupt function changes the mode from heating to shower
void modeChange()
{
  mode = !mode;
  
}

//This interrupt function changes the temperature or pump speed
void secondButton()
{
  if(mode == LOW)
  {
    heat = !heat;
  }
  else
  {
    flow = !flow;
  }
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Temperature Function
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Function to read and give the temperature in fahrenheit
int getTemp(void)
{
  sensors.requestTemperatures();   //Send command to get the temperatures
  int tempF = int(DallasTemperature::toFahrenheit(sensors.getTempCByIndex(0)));

  return tempF;
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Heater Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void heaterOn(void)
{
  digitalWrite(HEATER,HIGH);
}

void heaterOff(void)
{
  digitalWrite(HEATER,LOW);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pump Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Function to run the water pump at half power
void waterPump50(void)
{
  analogWrite(WATERPUMP,127);
  delay(3000);
  analogWrite(WATERPUMP,0);
  delay(3000);
}

//Function to run the water pump at full power
void waterPump100(void)
{
  analogWrite(WATERPUMP,255);
  delay(3000);
  analogWrite(WATERPUMP,0);
  delay(3000);
}

//Function to run the water pump at speed of 1gal/7min
void waterPump7Min(void)
{
  analogWrite(WATERPUMP,36);
}

//Function to turn pump off
void waterPumpOff(void)
{
  analogWrite(WATERPUMP,0);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Setup
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
   Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //Button Setups
  pinMode(ModePin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ModePin), modeChange,FALLING);  //Change the state when the signal falls
  pinMode(SECOND,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SECOND), secondButton,FALLING);  //Change the state when the signal falls
  pinMode(THIRD,INPUT_PULLUP);
  

  //Start up the temperature library
  sensors.begin();

  //Set the heater pin 9 to output
  pinMode(HEATER,OUTPUT);   

  //Set the waterpump pin 10 to output
  pinMode(WATERPUMP,OUTPUT);      
}







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  //Heat Mode Procedure
  while(mode == LOW)
  {
    currentTemp = getTemp();        //Update the current temp
    HeatModeDisplay();              //Display the heating mode screen

    //Check if the third button has been pressed
    if(digitalRead(THIRD) == LOW)
    {
      if(desiredTemp <= 108)
      {
        desiredTemp++;
      }
      else
      {
        desiredTemp = 95;
      }
      delay(200);
    }

    //Turn the heater on if the current temp is below the desired temp
    if((currentTemp < desiredTemp) && (heat == HIGH))
    {
      heaterOn();
    }
    else
    {
      heaterOff();
    }
    
  }


  //Shower Mode Procedure
  while(mode == HIGH)
  {
    heaterOff();                //Turn the heater off in shower mode
    ShowerModeDisplay();        //Show the shower display on the OLED

    //Check if the third button has been pressed
    if(digitalRead(THIRD) == LOW)
    {
      if(pumpSpeed <3)
      {
        pumpSpeed++;
      }
      else
      {
        pumpSpeed = 1;
      }
      delay(200);
    }

    //If the flow in activated, turn the pump on at the specified speed
    if(flow == HIGH)
    {
       if(pumpSpeed == 1)
       {
          waterPump7Min();
       }
       else if(pumpSpeed == 2)
       {
          waterPump50();
       }
       else
       {
          waterPump100();
       }
    }
    else
    {
      waterPumpOff();
    }
    
  }

}
