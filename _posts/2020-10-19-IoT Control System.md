---
layout: default
modal-id: 2
date: 2020-10-19
img: iot.png
alt: image-alt
project-date: October 2020
author: Håvard Kråkenes
---

## Introduction

The background for this report is to make an embedded Arduino PID controller for a physical airheater process. It will be developed and tested using hardware in the loop principles, and will be tested against a software simulator since the physical airheater is not available. A schematic of the process can be seen in *Figure 1* below.

![Process Drawing](img/projects/scada/Airheater.png){:class="img-responsive img-centered"}
*Figure 1: Process drawing*  
sett inn nytt bilde

## Problem Description

In this report the task is to control the simulated/physical airheater using an embedded arduino PI controller. The simulated airheater is a "Black Box simulator" meaning we can not look at its inner workings or code. The physical system is not available at the time of this report, so that will be tested at a later date.

The simulator setup will communicate with the arduino controller over serial communication. The arduino will have two modes: Analog I/O and Serial I/O.

IoT will be implemented in the system in the form of Thingspeak, which is a platform where you can send and retrieve sensor data and do data analysis in the cloud.
Since the Arduino in use does not have Wi-Fi capabilities, the communication to Thingspeak is done in Labview on the simulator.

A overview of the system including communication can be seen in *Figure 2* below.

![System overview](img/projects/scada/flow.png){:class="img-responsive img-centered"}
*Figure 2: System overview*  
sett inn nytt bilde

## Labview Simulator  
The Labview software has been adapted from the previous project. OPC functionality has been removed and replaced with serial communication and thingspeak.
The simulator can chose between software and hardware regulator, as well as black box and model process.

When running in "Arduino" mode, it initializes serial communication with the Arduino at a baud rate of 2000000 and a timeout of 100ms. The values y, Kp, Ti and r are sent to the Arduino and output from the controller, u, is received back. Serial was chosen because no DAQ device was available.

Some hickups has been discovered when using Serial; it slows down the whole Labview program. It waits until it receives the serial payload from the Arduino before continuing, so the 100ms loop time is greatly increased during simulation. Though the calculations is still valid because it in practice runs everything in 100ms sample time, the real time plotting and response seems much slower.

This means that using serial in this way is only usable towards software, and the physical process should not be implemented in the same solution. For testing and hardware in the loop against a simulator it works great.

See the figures below for an overview of the Labview application:

sett inn bilde

The Labview files can be found on the link at the bottom of this report.


## Arduino Embedded Controller
The Arduino board used in this project is the standard Arduino UNO. It is programmed with two modes, Analog I/O and Serial I/O. This is chosen with a button with a LED showing the chosen mode. A OLED display is also attached to show the current mode, controller parameters and input/output values.

In analog mode it reads input voltage from a designated AI pin, runs the PI controller algorithm and outputs 0-5V analog. Analog output is implemented using PWM and a RC circuit with values: ... . A DAC chip with i2c was not available and could not be sourced in time for this report. An OLED display was attached in its place to implement I2C.

Since the physical airheater is not available at the time of writing this report, the input in analog mode is simulated with a potmeter to the analog input pin, simulating 1-5V input.

In serial mode the analog inputs and outputs are disabled and it instead sends and receives the data over serial. In this mode the controller parameters is also received over serial so they can be set in Labview in real-time.

The controller is implemented as a arduino library with built in filter function. The source code for this library can be seen here: sett inn link

A sketch of the Arduino circuit can be seen in the figure below.

sett inn bilde

The Arduino source code can be seen here: sett inn link

## Hardware in the loop
The Embedded controller is tested and controller parameters fine tuned using hardware in the loop principles. This means involving the hardware controller into a software simulator to test that the controller works as expected before using it on the actual process.

This was done using the serial communication described in the chapters above. Since the PI algorithm from a code standpoint is the same as the previous project, the same PI parameters is used.

A plot of the control loop using hardware in the loop can be seen in the figure below.


## Testing
After hardware in the loop testing, the analog input and output was tested using a multimeter to measure output voltage, and using a potmeter to adjust the simulated input voltage.
This can also be easier seen on the attached OLED display.

The test is done just to verify that the analog code works as expected before connecting it to the physical process later.


## Thingspeak
Thingspeak communication is implemented in the Labview application. This is done since the only Arduino at hand was a UNO without ethernet or Wi-Fi. This also means that thingspeak only works in serial mode. As a proof of concept this works well, but for making a production embedded controller with IoT connectivity one would want to use an Arduino with on-board Wi-Fi and more memory.

In thingspeak the values of y, u, r, Kp and Ti is implemented and sent from Labview. There seem to be a problem that the HTTP request takes longer than Labviews sampling time (100ms) so the resolution of the data in Thingspeak is much lower than in Labview. This is because of how HTTP requests work and how Labview's HTTP module works. It also has a lot to do with the latency to thingspeak's servers, upload speed etc.

All in all it works ok-ish. It should be possible to send a larger range of data in one request to keep up with the process.

## Conclusion


Files:
