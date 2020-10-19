---
layout: default
modal-id: 1
date: 2020-09-07
img: scada.png
alt: scada-header
project-date: September 2020
author: Håvard Kr

---

## Introduction

The background for this report was to make a simple control and monitoring system for for a airheater using known industry 4.0 and IoT technology. The airheater is a physical system made for use in a lab environment, but in this case a simulator of the physical system was used. A schematic of the airheater system can be seen in *Figure 1* below.

![Airheater Drawing](img/projects/scada/Airheater.png){:class="img-responsive img-centered"}
*Figure 1: Air Heater System*

## Problem Description

In this report the task is to control the simulated airheater in Matlab Simulink using a PI controller. The simulated airheater is a "Black Box simulator" meaning we can not look at its inner workings or code. The simulator is however supplied with a mathematical model.

The step response method is used on the transfer function of the mathematical model to find a parametrized model which replicates the black box.

Frequency response analysis and Stability analysis is used to find optimal controller parameters for controlling the airheater. The controller is then implemented as C code inside Simulink.

The temperature output of the airheater model are be stored in a database and used for data analysis. This is accomplished by using modern IoT technologies including OPC UA, Azure SQL Server and a web application using .NET Core hosted on Azure App Services.

A flow of the applications can be seen in *Figure 2* below.

![Application system flow](img/projects/scada/flow.png){:class="img-responsive img-centered"}
*Figure 2: Application system flow*

## Model Analysis

The mathematic model for the airheater can be seen in equation *1*.

$$
\dot{T}_{out}=\frac{1}{\Theta_{t}}\left \{ -T_{out}+ \left [ K_{h}u\left ( t-\Theta _{d} \right ) +T_{env}\right ]\right \}\tag{1}

$$

The equation is then rearranged to a the transfer function shown in equation *2*.

$$
H\left ( s \right )=\frac{K_{h}}{\Theta _{t}s+1}e^{-\Theta _{d}s}\tag{2}

$$

The model parameters are found using the step response method on the black box model as shown to the left in *Figure 3*.

[![Step](img/projects/scada/Stepresponse.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/scada/Stepresponse.png){:target="_blank"}
*Figure 3: Step response of black box model (left) and final model fit (right)*

The final model parameters are derived by trial and error. The final parameters are: $\Theta_{t}=22s, \Theta_{d}=2s, K_{h}=3.5$ and can be seen in use to the right in *Figure 3*.

## PI Controller Tuning

The transfer function of the PI controller can be seen in equation *3*.

$$
h_{c}\left ( s \right )=\frac{K_{p}T_{i}s+K_{p}}{T_{i}s}\tag{3}

$$

Parameters for the PI controller is found using Ziegler Nichols method along with trial and error. Ziegler Nichols method is used on the combined transfer function of the system and controller. The ZN parameters is then found to be approximately: $K_{c}=5.12, T_{c}=\frac{2\pi}{\omega_{180}}=7.73s$ Thus, the parameters for the PI controller should become: $K_{p}=0.5K_{c}=2.56, T_{i}=\frac{T_{c}}{1.2}=6.44s$.

However, by testing and analysing this is not a very good fit as the system oscilates. The Gain Margin and Phase Margins is also at the edge of what is acceptable.

Trial and error reveals the best controller parameters to become: $K_{p}=1.2,T_{i}=12s$. Testing of the controller on the black box model can be seen in *Figure 4*.

[![Step2](img/projects/scada/StepPI.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/scada/StepPI.png){:target="_blank"}
*Figure 4: Step response with tuned PI controller*

## Frequency Response and Stability Analysis

A bode plot of the combined system can be seen in Figure 5 below. From the figure we can see that $\omega_{c}=0.202rad/s$, $\omega_{180}=0.761rad/s$, $GM=12dB$ and $PM=57.2°$.

Since $\omega_{c}<\omega_{180}$ the system is asymptotically stable.

The Gain Margin (GM) and Phase Margin (PM) are both within the "Golden Rules", though the Gain Margin is at the edge of the range. $GM: 6dB<GM<12dB$, $PM: 30°<PM<60°$

[![Bode2](img/projects/scada/Bode2.png){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/scada/Bode2.png){:target="_blank"}
*Figure 5: Bode diagram of complete system, GM and PM within OK limits*

```matlab
% Define Constants
t=22;
td=2;
Kh=3.5;
Ti=12;
Kp=1.2;

% Define Transfer functions
num1=[Kh];
den1=[t, 1];
Hp = tf(num1, den1, 'InputDelay', td);

num2=[Kp*Ti, Kp];
den2=[Ti, 0];
Hc = tf(num2, den2);

L = series(Hc, Hp);
% Frequency Response
margin(L);
grid on
```

## Labview

There is a total of three labview programs used for this report. The main controller application which controls the air heater and sends log data to an OPC-UA server, the OPC-UA server and the OPC-UA client which reads from OPC-UA and transmits the data to an Azure SQL server.

[![Labview-gui](img/projects/scada/view1gui.PNG){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/scada/view1gui.PNG){:target="_blank"}
*Figure 6: GUI of controller application*

An overview of the GUI of the controller application can be seen in *Figure 6. The different tabs allows the user to change settings of the application, including model parameters, controller parameters and opc connection details.

[![Labview-code](img/projects/scada/view1code.PNG){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/scada/view1code.PNG){:target="_blank"}
*Figure 7: Code of controller application*

The inner workings of the labview application can be seen in *Figure 7* including the PI controller, the blackbox model, "simulator" of black box model and OPC-UA connection.

The C code for the controller can be seen below.

```c
//PI algorithm
float error;
float integral;
float uk;

error = sp - y;
integral = integral1 + error * dt;
uk = Kp*error + Kp / Ti * integral;

if (uk > OutputMax)
    {
        uk = OutputMax;
    }
if (uk < OutputMin)
    {
        uk = OutputMin;
```

In *Figure 8* the OPC-UA server gui and code can be seen. This is just a OPC-UA server running locally on the same machine as the controller.

![opc-server](img/projects/scada/view2.PNG){:class="img-responsive img-centered"}
*Figure 8: OPC-UA Server*

In *Figure 9 the OPC-UA and SQL client gui and code can be seen. The application reads the latest value from the local OPC-UA server and transmits it to the cloud based Azure SQL server.

![opc-sql](img/projects/scada/view3.PNG){:class="img-responsive img-centered"}
*Figure 9: OPC-UA and SQL client*

## SQL

The Azure SQL server has two tables, one view and one stored procedure. The SQL query used to initialize the database can be seen below:

The full code for initializing the database can be seen here: [SQL Code](https://github.com/haavardnk/IIA4217/blob/gh-pages/files/projects/scada/Database.sql){:target="_blank"}

## ASP.NET

Monitoring of the stored data values is implemented in a ASP.NET Core web application hosted on Azure Web Services. The logging application can be accessed here: [Labdata](https://labdata.azurewebsites.net/){:target="_blank"}

[![asp](img/projects/scada/aspnet.PNG){:class="img-responsive img-centered"}](https://raw.githubusercontent.com/haavardnk/IIA4217/gh-pages/img/projects/scada/aspnet.PNG){:target="_blank"}
*Figure 10: ASP.NET Core monitoring web application*

A screenshot of the web application can be seen in *Figure 10*. It receives the last 1000 values from the database and plots it. This is a basic single page application with basic features and some styling done with bootstrap. For this implementation it was concidered to be "good enough" for showing temperature data. A table showing all the 1000 data points can be opened by clicking the button on the page.

## Conclusion

There are many applications, interfaces and technologies at work in this setup. They all speak with each other and fulfill its own task as a standalone application and talking with the other via known industry protocols. The usage of a cloud infrastructure such as Azure shows how easy it is to move applications from large local servers to the cloud, this also means that the applications can be scaled up almost infinitely. Industry 4.0 is definately the way to go for easy 24/7 online monitoring.

All the files for this project can be seen here: [Project files](https://github.com/haavardnk/IIA4217/tree/gh-pages/files/projects/scada){:target="_blank"}
