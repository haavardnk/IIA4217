---
layout: default
modal-id: 2
date: 2020-10-19
img: iot.png
alt: iot-header
project-date: October 2020
author: Håvard Kråkenes
---

## Introduction

The background for this report is to make an embedded Arduino PI controller with IoT connectivity for a physical airheater process. It will be developed and tested using hardware in the loop principles, and will be tested against a software simulator since the physical airheater is not available remotely.

## Problem Description

In this report the task is to control the simulated/physical airheater using an embedded arduino PI controller. The simulated airheater is a "Black Box simulator" meaning we can not look at its inner workings or code. The physical system is not available at the time of this report, so that will be tested at a later date.

The simulator setup will communicate with the arduino controller over serial communication. The arduino will have two modes: Analog I/O and Serial I/O.

IoT will be implemented in the system in the form of Thingspeak, which is a platform where you can send and retrieve sensor data and do data analysis in the cloud.
Since the Arduino in use does not have Wi-Fi capabilities, the communication to Thingspeak is done in LabVIEW on the simulator.

A overview of the system including communication can be seen in *Figure 1* below.

![Process-Drawing](img/projects/iot/process.png){:class="img-responsive img-centered" style="width: 100%;"}
*Figure 1: Process drawing*  

## LabVIEW Simulator  
The LabVIEW software has been adapted from the previous project. OPC functionality has been removed and replaced with serial communication and thingspeak.
The simulator can chose between software and hardware regulator, as well as black box and model process.

When running in "Arduino" mode, it initializes serial communication with the Arduino at a baud rate of $4000000$ and a timeout of $250ms$. The values $y$, $Kp$, $Ti$ and $r$ are sent to the Arduino and output from the controller and $u$ is received back. Serial was chosen because no DAQ device was available.

Some hickups has been discovered when using Serial; it slows down the whole LabVIEW program. It waits until it receives the serial payload from the Arduino before continuing, so the 100ms loop time is greatly increased during simulation. Though the calculations is still valid because it in practice runs everything in $100ms$ sample time, the real time plotting and response seems much slower.

This means that using serial in this way is only usable towards software, and the physical process should not be implemented in the same solution. For testing and hardware in the loop against a simulator it works great.

See *Figure 2* below for a "schematic" of the LabVIEW application.

[![Labview](img/projects/iot/labview.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/iot/labview.png){:target="_blank"}
*Figure 2: LabVIEW schematics*  

The LabVIEW files can be found on the link at the bottom of this report.

## Arduino Embedded Controller
The Arduino board used in this project is the standard Arduino UNO. It is programmed with two modes, Analog I/O and Serial I/O. This is chosen with a button with a LED showing the chosen mode. A OLED display is also attached to show the current mode, controller parameters and input/output values.

In analog mode it reads input voltage from a designated AI pin, runs the PI controller algorithm and outputs $0-5V$ analog. Analog output is implemented using two different methods; PWM and DAC. The PWM output is filtered with a lowpass filter to convert it to DC using a RC circuit with values $100µF$ and $1kΩ$ as these values was available and gave a good result. The DAC chip used is a MCP4921 12 bit dac which is connected using the SPI protocol.

The OLED display is connected to the Arduino using the I2C protocol.

Since the physical airheater is not available at the time of writing this report, the input in analog mode is simulated with a potmeter to the analog input pin, simulating a $1-5V$ input.

The mode selector is a simple if else selector, based on the value of the toggle state of the button. The code for the serial mode can be seen below:
```c
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
```
In serial mode the analog inputs and outputs are disabled and it instead sends and receives the data with serial over USB. As seen in the code above, the Arduino first sets the led light low, analog outputs to zero and then reads serial if available. Since the serial message is a string, the next part is to decompose the string and extract the different values from the string. The PI controller is then ran based on the values received over serial, and the output of the controller, $u$, is sent back over serial. Lastly the display mode is changed after the first iteration of the serial mode. The rest of the screen is updated on a general basis for both modes, and can be seen in the source code.

The controller is implemented as a arduino library with built in filter function. The source code for this library can be seen here: [PI Controller Library](https://github.com/haavardnk/IIA4217/tree/gh-pages/files/projects/iot/Arduino/libraries/PiController){:target="_blank"}

A sketch of the Arduino circuit can be seen in *Figure 3* below.

![Arduino](img/projects/iot/arduino.png){:class="img-responsive img-centered"}
*Figure 3: Arduino circuit schematics*  

A picture of the actual Arduino circuit can be seen in *Figure 4* below.

[![Arduino-photo](img/projects/iot/arduino_pic.jpg){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/iot/arduino_pic.jpg){:target="_blank"}
*Figure 4: LabVIEW hardware in the loop plot*

The Arduino source code can be seen here: [Arduino Code](https://github.com/haavardnk/IIA4217/blob/gh-pages/files/projects/iot/Arduino/airheater/airheater.ino){:target="_blank"}

## Hardware in the loop
The Embedded controller is tested and controller parameters fine tuned using hardware in the loop principles. This means involving the hardware controller into a software simulator to test that the controller works as expected before using it on the actual process.

This was done using the serial communication described in the chapters above. Since the PI algorithm from a code standpoint is the same as the previous project, the same PI parameters is used. These are: $K_{p}=1.2, T_{i}=12s$

A plot of the control loop using hardware in the loop can be seen in *Figure 5*.

[![Labview-plot](img/projects/iot/labview2.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/iot/labview2.png){:target="_blank"}
*Figure 5: LabVIEW hardware in the loop plot*

## Testing
After hardware in the loop testing, the analog input and output was tested using a multimeter to measure output voltage, and using a potmeter to adjust the simulated input voltage.
This can also be easier seen on the attached OLED display.

The test is done just to verify that the analog code works as expected before connecting it to the physical process later.

## Thingspeak
Thingspeak communication is implemented in the LabVIEW application. This is done since the only Arduino at hand was a UNO without ethernet or Wi-Fi. This also means that thingspeak only works in serial mode. As a proof of concept this works well, but for making a production embedded controller with IoT connectivity one would want to use an Arduino with on-board Wi-Fi and more memory.

Two different channels is made in Thingspeak, one for plotting and storing history, and one for updating the control parameters.

The first channel are updated by sending $y$, $u$, $r$, $Kp$ and $Ti$ from LabVIEW. The values are sent to Thingspeak using MQTT where it publishes all 5 values at once. There is a limit to thingspeak free plan that only supports updating the value every 15 seconds, so the data is of low resolution.

[![Thingspeak1](img/projects/iot/thingspeak.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/iot/thingspeak.png){:target="_blank"}
*Figure 6: Thingspeak GUI of channel 1* 

The second channel are read by LabVIEW using HTTP with a GET request. It gets the last value of both $Kp$ and $Ti$ as CSV and updates the values in the running VI. On Thingspeak the values can be updated using a GET request in the web browser, so the parameters can be updated from anywhere in the world. There is a separate while loop in LabVIEW for reading from Thingspeak, this is because the HTTP requests take longer and slows down the VI, and so the separate while loop has a longer sampling time. Details can be seen in *Figure 2* or in the source code. A screenshot of this channel on Thingspeak can be seen in *Figure 7*.

[![Thingspeak2](img/projects/iot/thingspeak2.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/iot/thingspeak2.png){:target="_blank"}
*Figure 7: Thingspeak GUI of channel 2* 

MQTT was chosen for writing instead of HTTP because HTTP was not reliable for this use case. This was because the response time was too big compared to the 100ms loop timer, and thus slowed down the whole VI. MQTT solved this by being responsive and fast without hiccups.

All in all it works great. Using a premium plan would enable high frequency MQTT publishing which would help a lot.

## Cyber Security
The implementation which is done here can be said to be quite secure, especially when the Arduino is in stand alone mode. The serial protocol in use is not safe at all, especially since it is over USB. If anyone got physical access to the device or the serial cable, they could inject faulty values for the controller, or in worst case reprogram the device. This can however easily be avoided by having control of the physical location of the hardware.

As for Thingspeak there would be possible to hijack this traffic, but the attacker would then need the private write codes on Thingspeak. This can therefore be considered safe.

## Conclusion
In this report we have implemented an IoT embedded controller as a proof of concept. This shows how easy it is to make small embedded circuits and IoT gadgets in modern times. Some problems occured by not having the right hardware and software licensing, but all in all it shows the general idea of how it should work.


All the files for this project can be seen here: [Project files](https://github.com/haavardnk/IIA4217/tree/gh-pages/files/projects/iot){:target="_blank"}
