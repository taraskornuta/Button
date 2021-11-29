/**
  * MIT License
  * 
  * Copyright (c) 2020 Taras Kornuta (Leon11t)
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

#define BTN_PROCESS_DEF        (10)
#define BTN_DEBOUNCE_DEF       (20)
#define BTN_LONG_PRESS_DEF     (1000)

#define BTN_DEBOUNCE_TIME      (this->debounce_time_ms / this->process_time_ms)
#define BTN_LONG_PRESS_TIME(x) (this->instance[x].long_press_time_ms / this->process_time_ms)


static btn_init_t *this = NULL;

int8_t Button_Init(btn_init_t *init, btn_instance_t *instance, uint8_t count)
{
  if ((NULL == init) || (NULL == instance) || (NULL == init->port_read) || (0 == count))
  {
    return -1;
  }
  // link internal objects
  this = init;
  this->instance = instance;

 // init internal variables, set defaults
  this->count = count;
  this->process_time_ms = (this->process_time_ms) ? this->process_time_ms  : BTN_PROCESS_DEF;
  this->debounce_time_ms = (this->debounce_time_ms) ? this->debounce_time_ms : BTN_DEBOUNCE_DEF;
  this->long_press_def_ms = (this->long_press_def_ms) ? this->long_press_def_ms : BTN_LONG_PRESS_DEF; 

  for(uint8_t i = 0; i < count; i++)
  {
    this->instance[i].state.locked = 0;
    this->instance[i].state.prew = BTN_STATE_NONE;
    this->instance[i].long_count = 0;
    this->instance[i].lock_count = 0;
    this->instance[i].long_press_time_ms = (this->instance[i].long_press_time_ms) ? this->instance[i].long_press_time_ms : this->long_press_def_ms;
  }
  return 0;
}


void Button_Update(void)
{
  uint8_t key = 0;

  while(key < this->count)
  {
    btn_state_t now_pressed = BTN_STATE_NONE;
    const uint32_t *port = this->instance[key].port;
    const uint32_t pin = this->instance[key].pin;

    now_pressed = (this->port_read(port, pin) ? BTN_STATE_SHORT : BTN_STATE_NONE);
    
    if (BTN_STATE_SHORT == now_pressed)                 // Button pressed
    {
      if (!this->instance[key].state.locked)
      {
        if (++this->instance[key].lock_count >= BTN_DEBOUNCE_TIME) // Debounce time
        {
          this->instance[key].state.locked = 1;
        }
      }

      if ((this->instance[key].state.locked) && 
          (BTN_STATE_SHORT == this->instance[key].state.prew))
      {
        if (++this->instance[key].long_count >= BTN_LONG_PRESS_TIME(key))
        {
          this->instance[key].state.act = BTN_STATE_LONG;
          if (NULL != this->long_press) this->long_press(key);
        }
      }
      this->instance[key].state.prew = now_pressed;
    }
    else if (BTN_STATE_NONE == now_pressed)            // Button released
    {
      if (this->instance[key].state.locked)
      {
        if (this->instance[key].lock_count)
        { 
          this->instance[key].lock_count--;
        }
        else
        {
          this->instance[key].state.locked = 0;
          if (this->instance[key].long_count < BTN_LONG_PRESS_TIME(key))
          {
            if (0 == this->instance[key].lock_count)
            {
              this->instance[key].state.act = BTN_STATE_SHORT;
              if (NULL != this->short_release) this->short_release(key);
            }
          }
          else
          {
            if (BTN_STATE_LONG == this->instance[key].state.act)
            {
              if (NULL != this->long_release) this->long_release(key);
            }
          }
        }
      }
      else
      {
        this->instance[key].long_count = 0;
      }
    }
    key++;
  }
}

btn_state_t Button_EventGet(uint8_t key)
{
  if (key >= this->count) return 0;
  return this->instance[key].state.act;
}

