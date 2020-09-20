/**
  * MIT License
  * 
  * Copyright (c) 2020 Tatas Kornuta (Leon11t)
  * 
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * 
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
**/

#include "button.h"
#include "stdio.h"

#define BTN_DEBOUNCE_TIME   (BTN_DEBOUNCE_TIME_MS / BTN_PROCESSING_TIME_MS)
#define BTN_LONG_PRESS_TIME (BTN_LONG_PRESS_TIME_MS / BTN_PROCESSING_TIME_MS)

static uint8_t btnAmount = 0;
static btnInstance_t *btnInstance = NULL;
static portReadCallback_t btnPortRead = NULL;


void Button_Init(void *btnInstanceArray, portReadCallback_t callback, const uint8_t amount)
{
  btnInstance = (btnInstance_t*)btnInstanceArray;
  btnPortRead = callback;
  btnAmount   = amount;
  // init internal variables
  for(uint8_t keyNumber = 0; keyNumber < amount; keyNumber++)
  {
    btnInstance[keyNumber].locked = false;
    btnInstance[keyNumber].longCount = 0;
    btnInstance[keyNumber].lockCount = 0;
    btnInstance[keyNumber].prewState = BTN_STATE_NONE;
  }
}

void Button_Update(void)
{  
  uint8_t keyNumber = 0;

  while(keyNumber < btnAmount)
  {
    btnState_t btnNowPressed = BTN_STATE_NONE;
    
    if (true ==  btnPortRead(btnInstance[keyNumber].port, btnInstance[keyNumber].pin))  
    {
      btnNowPressed = BTN_STATE_SHORT;
    }
    
    if ((BTN_STATE_SHORT == btnNowPressed))                               // Button pressed
    {
	    if (false == btnInstance[keyNumber].locked)
      {
        if (++btnInstance[keyNumber].lockCount >= BTN_DEBOUNCE_TIME) // Debounce time
        {
	    	  btnInstance[keyNumber].locked = true;
	      }
      }
      
      if ((true == btnInstance[keyNumber].locked) && 
          (BTN_STATE_SHORT == btnInstance[keyNumber].prewState))
      {
        if (++btnInstance[keyNumber].longCount >= BTN_LONG_PRESS_TIME)
        {
          btnInstance[keyNumber].state = BTN_STATE_LONG;
          if (btnInstance[keyNumber].longCount == BTN_LONG_PRESS_TIME)
          {
            Button_LongPress(keyNumber);
          }
        }
      }
      btnInstance[keyNumber].prewState = btnNowPressed;     
    }
    else if (BTN_STATE_NONE == btnNowPressed)                            // Button released
    {
      if (true == btnInstance[keyNumber].locked)
      {
        if (0 != btnInstance[keyNumber].lockCount)
        { 
	        btnInstance[keyNumber].lockCount--;         
	      }
        else
        {
          btnInstance[keyNumber].locked = false;
          if (btnInstance[keyNumber].longCount < BTN_LONG_PRESS_TIME)
          {
            btnInstance[keyNumber].state = BTN_STATE_SHORT;
            
            if ((BTN_STATE_SHORT == btnInstance[keyNumber].state) && 
                (0 == btnInstance[keyNumber].lockCount))
            {
              Button_ShortRelease(keyNumber);
            }     
          }
          else
          {
            if (BTN_STATE_LONG == btnInstance[keyNumber].state)
            {
              Button_LongRelease(keyNumber);
            }
          }
        }  
      }
      else
      {
        btnInstance[keyNumber].longCount = 0;
      }
    }
    keyNumber++;
  }
}   

btnState_t Button_EventGet(uint8_t btnCode)
{
  return btnInstance[btnCode].state;
}


__weak void Button_ShortRelease(uint8_t btnCode)
{
  /* Prevent unused argument(s) compilation warning */
  (void)btnCode;
  /* NOTE: This function should not be modified, when the callback is needed,
           the Button_ShortRelease could be implemented in the user file
  */
}

__weak void Button_LongPress(uint8_t btnCode)
{
  /* Prevent unused argument(s) compilation warning */
  (void)btnCode;
  /* NOTE: This function should not be modified, when the callback is needed,
           the Button_LongPress could be implemented in the user file
  */
}

__weak void Button_LongRelease(uint8_t btnCode)
{
  /* Prevent unused argument(s) compilation warning */
  (void)btnCode;
  /* NOTE: This function should not be modified, when the callback is needed,
           the Button_LongRelease could be implemented in the user file
  */
}

