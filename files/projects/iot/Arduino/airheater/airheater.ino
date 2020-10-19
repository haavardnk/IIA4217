#include <MCP492X.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <PiController.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define PIN_SPI_CHIP_SELECT_DAC 10

/**********************************************************************
 Control air heater with arduino and hardware in the loop simulator.
 author  Håvard N. Kråkenes
 date    11.10.2020
************************************************************************/
//PI algorithm
float sp;
float y;
float uk = 0;
float dt = 0.1;
float kp = 1.2;
float ti = 12;
float filterWeight = 0.5;
PiController pi = PiController();

//Push button toggle serial/physical
int buttonPin = 2; 
int ledPin = 4;
int toggleState = HIGH;      // the current state of the output pin, high = Physical
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

//Serial communication
String readSerial, data, printString;
int ind1, ind2, ind3, ind4;

//Analog input / output
int outputPin = 9;
int inputPin = A0;
//DAC
MCP492X myDac(PIN_SPI_CHIP_SELECT_DAC);


//OLED Display
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // init serial port baud rate
  Serial.begin(4000000);
  Serial.setTimeout(50);

  // init pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(outputPin, OUTPUT);

  // init pi controller
  pi.init(kp, ti, dt);
  // init dac
  myDac.begin();
  
  //Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}
void loop() {

  // Toggle button, added time delay to not read multiple states
  if (digitalRead(buttonPin) == HIGH && millis() - time > debounce) {
    if (toggleState == HIGH)
      toggleState = LOW;
    else
      toggleState = HIGH;
    time = millis(); 
  }
  
  // Clear display
  display.clearDisplay();
  display.setCursor(0,0); 
  if(toggleState == HIGH) // ALL CODE FOR PHYSICAL CONNECTION GOES HERE
  {
    digitalWrite(ledPin, HIGH);
    sp = 25;
    y = 12.5 + ((pi.filter(analogRead(inputPin), filterWeight) * (5.0 / 1023.0))*7.5);

    // Get new controller output
    uk = pi.ctrl(y,sp);

    // Write analog output
    analogWrite(outputPin, 51*uk);
    myDac.analogWrite(819*uk);

    // Update display mode
    display.println(F("Mode: Analog"));
  }
  else // ALL CODE FOR SERIAL HARDWARE IN THE LOOP GOES HERE
  {
    digitalWrite(ledPin, LOW);
    analogWrite(outputPin, 0);
    myDac.analogWrite(0);
    
    // check serial 
    if ( Serial.available() ){
      // Read serial string 
      readSerial = Serial.readString();

      // Index string and get variables
      ind1 = readSerial.indexOf(',');  //finds location of first ,
      y = readSerial.substring(0, ind1).toFloat();   //captures first data String
      ind2 = readSerial.indexOf(',', ind1+1 );   //finds location of second ,
      sp = readSerial.substring(ind1+1, ind2+1).toFloat();   //captures second data String
      ind3 = readSerial.indexOf(',', ind2+1 );   //finds location of third ,
      kp = readSerial.substring(ind2+1, ind3+1).toFloat();   //captures third data String
      ind4 = readSerial.indexOf(',', ind3+1 );   //finds location of fourth ,
      ti = readSerial.substring(ind3+1, ind4).toFloat();   //captures fourth data String
    }

    // PI Controller get new output
    uk = pi.ctrl(y,sp);
    
    // Send u over serial
    Serial.println (uk);

    // Update display mode
    display.println(F("Mode: Serial"));
  }

    display.print(F("Kp: "));display.println(kp);
    display.print(F("Ti: "));display.println(ti);
    display.print(F("sp: "));display.println(sp);
    display.print(F("y: "));display.println(y);
    display.print(F("u: "));display.println(uk);
    display.display();
    
    delay(100);
}
