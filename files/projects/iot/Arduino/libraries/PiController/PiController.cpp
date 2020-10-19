/*
  PiController.cpp - Library PI controller and lowpass filter.
  Created by Håvard N. Kråkenes, October 11, 2020.
*/

#include "Arduino.h"
#include "PiController.h"

PiController::PiController()
{

}

void PiController::init(float kp, float ti, float dt)
{
  _kp = kp;
  _ti = ti;
  _dt = dt;
  _integral1 = 0;
  _uk = 0;
  _outputMin = 0;
  _outputMax = 5;
  _filter = 0;
}

float PiController::ctrl(float y, float sp)
{
  // PI algorithm
  _error=sp-y;
  _integral = _integral1 + _error * _dt;
  _uk=_kp*_error + _kp / _ti * _integral;
  
  if (_uk > _outputMax)
      {
          _uk = _outputMax;
      }
  if (_uk < _outputMin)
      {
          _uk = _outputMin;
      }
  _integral1 = _integral; 
  return _uk;
}
float PiController::filter(float y, float a)
{
  _filter = (1-a)*_filter + a*y;

  return _filter;
}