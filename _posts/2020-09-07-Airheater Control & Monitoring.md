---
layout: default
modal-id: 1
date: 2020-09-07
img: scada.png
alt: image-alt
project-date: September 2020
author: Håvard Kråkenes
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

![Step response of black box model](img/projects/scada/Stepresponse.png){:class="img-responsive img-centered"}
*Figure 3: Step response of black box model (left) and final model fit (right)*

The final model parameters are derived by trial and error. The final parameters are: $\Theta_{t}=22,\Theta_{d}=2,K_{h}=3.5$ and can be seen in use to the right in *Figure 3*.

A bode diagram of the model with the aquired model parameters can be seen in *Figure 4*.

![Bode diagram of air heater model](img/projects/scada/Bode1.png){:class="img-responsive img-centered"}
*Figure 4: Bode diagram of airheater transfer function*

As we can see from *Figure 4*, since $\omega_{c}<\omega_{180}$ the system is asymptotically stable.

The bode diagram in *Figure 4* is generated using the following code:
```matlab
% Define Constants
t=22;
td=2;
Kh=3.5;

% Define Transfer function
num=[Kh];
den=[t, 1];
H = tf(num, den, 'InputDelay', td)
% Frequency Response
bode(H);
grid on
```

## PI Controller Tuning

The transfer function of the PI controller can be seen in equation *3*.

$$
h_{c}\left ( s \right )=\frac{K_{p}T_{i}s+K_{p}}{T_{i}s}\tag{3}
$$

Parameters for the PI controller is found using Ziegler Nichols method along with trial and error. Ziegler Nichols method is used on the combined transfer function of the system and controller. The ZN parameters is then found to be approximately: $K_{c}=5.12,T_{c}=\frac{2\pi}{\omega_{180}}=7.73$ Thus, the parameters for the PI controller should become: $K_{p}=0.5K_{c}=2.56,T_{i}=\frac{T_{c}}{1.2}=6.44$.

However, by testing and analysing this is not a very good fit as the system oscilates. The Gain Margin and Phase Margins is also at the edge of what is acceptable.

Trial and error reveals the best controller parameters to become: $K_{p}=1.2,T_{i}=12$. Bode diagram for the combined system using these settings can be seen in *Figure 5* and testing on the black box model can be seen in *Figure 6*. Both GM and PM are here within optimal limits.

![Bode diagram of system with PI controller](img/projects/scada/Bode2.png){:class="img-responsive img-centered"}
*Figure 5: Bode diagram of complete system, GM and PM within OK limits*

![Step response with tuned PI controller](img/projects/scada/StepPI.png){:class="img-responsive img-centered"}
*Figure 6: Step response with tuned PI controller*

## Labview

There is a total of three labview programs used for this report. The main controller application which controls the air heater and sends log data to an OPC-UA server, the OPC-UA server and the OPC-UA client which reads from OPC-UA and transmits the data to an Azure SQL server.

![GUI of LabVIEW controller application](img/projects/scada/view1gui.PNG){:class="img-responsive img-centered"}
*Figure 7: GUI of controller application*

An overview of the GUI of the controller application can be seen in *Figure 7*. The different tabs allows the user to change settings of the application, including model parameters, controller parameters and opc connection details.

![Code of LabVIEW controller application](img/projects/scada/view1code.PNG){:class="img-responsive img-centered"}
*Figure 8: Code of controller application*

The inner workings of the labview application can be seen in *Figure 8* including the PI controller, the blackbox model, "simulator" of black box model and OPC-UA connection.

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

In *Figure 9* the OPC-UA server gui and code can be seen. This is just a OPC-UA server running locally on the same machine as the controller.

![Code and GUI of LabVIEW OPC-UA server application](img/projects/scada/view2.PNG){:class="img-responsive img-centered"}
*Figure 9: OPC-UA Server*

In *Figure 10* the OPC-UA and SQL client gui and code can be seen. The application reads the latest value from the local OPC-UA server and transmits it to the cloud based Azure SQL server.

![Code and GUI of LabVIEW OPC-UA and SQL client application](img/projects/scada/view3.PNG){:class="img-responsive img-centered"}
*Figure 10: OPC-UA and SQL client*

## SQL

The Azure SQL server has two tables, one view and one stored procedure. The SQL query used to initialize the database can be seen below:

```sql

/************************************************************************************/
/*Tables.sql*/
/************************************************************************************/

CREATE TABLE [MEASUREMENT]
( 
	[MeasurementId]      int  NOT NULL  IDENTITY ( 1,1 ) Primary Key,
	[MeasurementName]    varchar(50)  NOT NULL UNIQUE 

)
go

CREATE TABLE [MEASUREMENTDATA]
( 
	[MeasurementDataId]  int  NOT NULL  IDENTITY ( 1,1 ) Primary Key,
	[MeasurementId]      int  NOT NULL Foreign Key REFERENCES MEASUREMENT(MeasurementId),
	[MeasurementTimeStamp] datetime  NOT NULL ,
	[MeasurementValue]   float  NOT NULL, 
	[MeasurementUnit]	 varchar(50)  NOT NULL
)
go

	
/************************************************************************************/
/*GetMeasurementData.sql*/
/************************************************************************************/
IF EXISTS (SELECT name 
	   FROM   sysobjects 
	   WHERE  name = 'GetMeasurementData' 
	   AND 	  type = 'V')
	DROP VIEW GetMeasurementData
GO

CREATE VIEW GetMeasurementData
AS

SELECT   TOP (1000) MEASUREMENTDATA.MeasurementDataId, MEASUREMENT.MeasurementName, FORMAT(MEASUREMENTDATA.MeasurementTimeStamp, 'MM.dd HH:mm:ss') 
                         AS MeasurementTimeStamp, MEASUREMENTDATA.MeasurementValue, MEASUREMENTDATA.MeasurementUnit
FROM         dbo.MEASUREMENTDATA INNER JOIN
                         dbo.MEASUREMENT ON MEASUREMENTDATA.MeasurementId = MEASUREMENT.MeasurementId
ORDER BY dbo.MEASUREMENTDATA.MeasurementDataId DESC

GO

	
/************************************************************************************/
/*SaveMeasurementData.sql*/
/************************************************************************************/
IF EXISTS (SELECT name 
   FROM   sysobjects 
   WHERE  name = 'SaveMeasurementData' 
   AND   type = 'P')
DROP PROCEDURE SaveMeasurementData
GO

CREATE PROCEDURE AddMeasurement
@MeasurementName varchar(50),
@MeasurementValue float,
@MeasurementUnit varchar(50)
AS

DECLARE
@MeasurementId int

if not exists (select * from MEASUREMENT where MeasurementName = @MeasurementName)
	insert into MEASUREMENT (MeasurementName) values (@MeasurementName)
else
	select @MeasurementId = MeasurementId from MEASUREMENT where MeasurementName = @MeasurementName


insert into MEASUREMENTDATA (MeasurementId, MeasurementValue, MeasurementUnit, MeasurementTimeStamp) values (@MeasurementId, @MeasurementValue, @MeasurementUnit, getdate())

GO

```

## ASP.NET

Monitoring of the stored data values is implemented in a ASP.NET Core web application hosted on Azure Web Services. The logging application can be accessed here: [Labdata](https://labdata.azurewebsites.net/)

![Screenshot of monitoring web application](img/projects/scada/aspnet.PNG){:class="img-responsive img-centered"}
*Figure 11: ASP.NET Core monitoring web application*

A screenshot of the web application can be seen in *Figure 11*. It receives the last 1000 values from the database and plots it. This is a basic single page application with basic features and some styling done with bootstrap. For this implementation it was concidered to be "good enough" for showing temperature data. A table showing all the 1000 data points can be opened by clicking the button on the page.

## Conclusion

There are many applications, interfaces and technologies at work in this setup. They all speak with each other and fulfill its own task as a standalone application and talking with the other via known industry protocols. The usage of a cloud infrastructure such as Azure shows how easy it is to move applications from large local servers to the cloud, this also means that the applications can be scaled up almost infinitely. Industry 4.0 is definately the way to go for easy 24/7 online monitoring.
