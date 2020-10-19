/*
  PiController.h - Library PI controller and lowpass filter.
  Created by Håvard N. Kråkenes, October 11, 2020.
*/
#ifndef PiController_h
#define PiController_h

#include "Arduino.h"

class PiController
{
  public:
    PiController();
    void init(float kp, float ti, float dt);
    float ctrl(float y, float sp);
    float filter(float y, float a);
  private:
    float _kp;
    float _ti;
    float _dt;
    float _error;
    float _integral;
    float _integral1;
    float _uk;
    float _filter;
    int _outputMin;
    int _outputMax;
};

#endif
