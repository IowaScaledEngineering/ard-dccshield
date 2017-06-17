

#include "RelayPulser.h"

#define PIN_PULSER_SLOT_EMPTY 255

void RelayPulser::init(uint16_t onMs, uint16_t cduRechargeMs, uint8_t activeOutputState)
{
  this->onMs = onMs;
  this->cduRechargeMs = cduRechargeMs;
  this->activeOutputState = activeOutputState;
  Wire.begin();
  //  If you're using a standard Iowa Scaled shield to connect the I2C lines to the Arduino,
  //  the /IORST line is likely tied to Digital IO line 4.  Use the initializer below.
  this->relayBoard.begin(LOW, LOW, LOW, -1);
  this->relayBoard.allOff();
  if (LOW == activeOutputState)
     this->relayBoard.allOn();
  else
     this->relayBoard.allOff();

  
  state = PP_IDLE;
  targetMs = 0;
  memset(pinQueue, PIN_PULSER_SLOT_EMPTY, PIN_PULSER_MAX_PINS + 1);
}

uint8_t RelayPulser::addPin(uint8_t Pin)
{
//  Serial.print(" PinPulser::addPin: "); Serial.print(Pin,DEC);
  
  for(uint8_t i = 0; i < PIN_PULSER_MAX_PINS; i++)
  {
    if(pinQueue[i] == Pin)
    {
//      Serial.print(" Already in Index: "); Serial.println(i,DEC);
      return i;
    }

    else if(pinQueue[i] == PIN_PULSER_SLOT_EMPTY)
    {
//      Serial.print(" pinQueue Index: "); Serial.println(i,DEC);
      pinQueue[i] = Pin;
      process();
      return i;
    }
  }  

//  Serial.println();
  return PIN_PULSER_SLOT_EMPTY;
}

PP_State RelayPulser::process(void)
{
  unsigned long now;
  
  switch(state)
  {
  case PP_IDLE:
    if(pinQueue[0] != PIN_PULSER_SLOT_EMPTY)
    {
      Serial.print(" PinPulser::process: PP_IDLE: Pin: "); Serial.println(pinQueue[0],DEC);
      
      if (LOW == activeOutputState)
        this->relayBoard.relayOff(pinQueue[0]);
      else
        this->relayBoard.relayOn(pinQueue[0]);      

      targetMs = millis() + onMs;
      state = PP_OUTPUT_ON_DELAY;
    }
    break;
      
  case PP_OUTPUT_ON_DELAY:
    now = millis();
    if(now >= targetMs)
    {
      Serial.print(" PinPulser::process: PP_OUTPUT_ON_DELAY: Done Deactivate Pin: "); Serial.println(pinQueue[0],DEC);
      
      if (LOW == activeOutputState)
        this->relayBoard.relayOff(pinQueue[0]);
      else
        this->relayBoard.relayOff(pinQueue[0]);
        
      targetMs = now + cduRechargeMs;
      memmove(pinQueue, pinQueue + 1, PIN_PULSER_MAX_PINS);
      state = PP_CDU_RECHARGE_DELAY;
    }
    break;
      
  case PP_CDU_RECHARGE_DELAY:
    now = millis();
    if(now >= targetMs)
    {
      if(pinQueue[0] != PIN_PULSER_SLOT_EMPTY)
      {
        Serial.print(" PinPulser::process: PP_CDU_RECHARGE_DELAY: Done Deactivate Pin: "); Serial.println(pinQueue[0],DEC);
        
        if (LOW == activeOutputState)
          this->relayBoard.relayOn(pinQueue[0]);
        else
          this->relayBoard.relayOff(pinQueue[0]);   
        targetMs = now + onMs;
        state = PP_OUTPUT_ON_DELAY;
      }
      else
      {
//        Serial.println(" PinPulser::process: PP_CDU_RECHARGE_DELAY - Now PP_IDLE");
        state = PP_IDLE;
      }
    }
    break;
  }
  return state;
}

