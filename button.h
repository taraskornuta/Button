#ifndef BUTTON_H
#define BUTTON_H

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

/**
  * Name: Button library 
  * Version: v1.0
  * Author: Kornuta Taras
  * E-Mail: taraskornuta@gmail.com
  *
  * Description: 
  * This is a high-level abstraction layer library which provides easy to use driver 
  * to handle simple signal buttons connected directly to MCU GPIO.
  *
  * The library is provide: 
  * - Single short press handling
  * - Single long press handling
  * - Short release button callback function
  * - Long press button callback function
  * - Long release button callback function
  * - Ability to configure debounce time
  * - Up to 255 buttons handling
  *
  * Setup:
  * User should provide function which can read the GPIO port and return state: 
  * true  - button active pressed
  * false - button unactive released
  * In STM HAL - can be used HAL_GPIO_ReadPin(). In other hand user can provide 
  * wrapper function for other SDK
  * The library can work with array of buttons. The maximum is 255 buttons. 
  * Thay can be connected to different ports in any order. 
  * 
  * There are a few settings which should be adjusted by the user:
  * BTN_PROCESSING_TIME_MS - set the time in millisecond interval which used to 
  *                          call Button_Update() function
  * BTN_DEBOUNCE_TIME_MS   - set the time in millisecond to process button debounce
  *                          Default is 20ms
  * BTN_LONG_PRESS_TIME_MS - set the time millisecond to generate long press event
  *
  * Example:
  *
  * int main(void)
  * {
  *   btnInstance_t keys[4] = 
  *   {
  *     BTN_INIT_INSTANCE(GPIOA, GPIO_PIN_0),
  *     BTN_INIT_INSTANCE(GPIOB, GPIO_PIN_1),
  *     BTN_INIT_INSTANCE(GPIOC, GPIO_PIN_2),
  *     BTN_INIT_INSTANCE(GPIOD, GPIO_PIN_3),
  *   };
  *   
  *   Button_Init(keys, (portReadCallback_t)&HAL_GPIO_ReadPin, 4);
  *
  *   while(1)
  *   {
  *     if (EV_SHORT == Button_EventGet(0)) printf("Button 0 SHORT press\n");
  *     if (EV_SHORT == Button_EventGet(1)) printf("Button 1 SHORT press\n");
  *     // ...
  *     if (EV_LONG == Button_EventGet(0)) printf("Button 0 LONG press\n");
  *     if (EV_LONG == Button_EventGet(1)) printf("Button 1 LONG press\n");
  *   }
  * }
  *
  * void Button_ShortRelease(uint8_t btnCode)
  * {
  *   printf("BUTTON_PIN_%d state ShortRelease", btnCode);
  * }
  * 
  * void Button_LongPress(uint8_t btnCode)
  * {
  *   printf("BUTTON_PIN_%d state LongPress", btnCode);
  * }
  * 
  * void Button_LongRelease(uint8_t btnCode)
  * {
  *   printf("BUTTON_PIN_%d state LongRelease", btnCode);
  * }
  *
  * void TIM1_UP_IRQHandler(void)  // 10 milliseconds timer interrupt handler
  * {
  *    Button_Update();
  * }
  *
**/


#include <stdint.h>
#include "stdbool.h"

#define BTN_PROCESSING_TIME_MS (10UL)
#define BTN_DEBOUNCE_TIME_MS   (20UL)	  
#define BTN_LONG_PRESS_TIME_MS (1000UL)

#define BTN_INIT_INSTANCE(PORT, PIN)\
  { \
    .port = (uint32_t *)PORT,\
    .pin = PIN\
  }

typedef enum
{
  BTN_STATE_NONE = 0,  // button not pressed
  BTN_STATE_SHORT,     // button short pressed detected
  BTN_STATE_LONG       // button long pressed detected
}btnState_t;

typedef struct
{
  btnState_t state;      // actual button state
  btnState_t prewState;  // debounce button state
  bool       locked;     // debounce button lock
  uint8_t    lockCount;  // debounce time counter
  uint16_t   longCount;  // long press time counter
  uint32_t   *port;      // GPIO port
  uint32_t   pin;        // GPIO pin
}btnInstance_t;

/**
  * @brief portReadCallback_t user provided port read function callback
  */
typedef bool (*portReadCallback_t)(uint32_t *port, uint32_t pin);

/**
  * @brief Button_Init Buttons initialization function
  * Should be called before all others button module functions 
  * @param btnInstanceArray: pointer to buttons init instances array
  * @param callback: GPIO port read function pointer
  * @param amount: buttons count. Max number of buttons is 255
  * @retval None
  */
void Button_Init(void *btnInstanceArray, portReadCallback_t callback, const uint8_t amount);

/**
  * @brief Button_Update buttons poll and handling fuction
  * Should be places in the loop or timer interrupt routine with 
  * interval about 10 milliseconds. This mean that button polling interval will
  * be 100Hz
  * @param None
  * @retval None
  */
void Button_Update(void);

/**
  * @brief Button_EventGet get the button event status function
  * Used to check status of the particular button
  * @param btnCode: button code
  * @retval button status: BTN_STATE_NONE, BTN_STATE_SHORT, BTN_STATE_LONG
  */
btnState_t Button_EventGet(uint8_t btnCode);

/**
  * @brief Button_ShortRelease optional user defined Short button release function
  * The body of this function is implemented as __weak and user can implement 
  * this function inside his own code with addition logic, to get Short button release event
  * Used to check Short button release event for particular button code
  * @param btnCode: button code
  * @retval None
  */
void Button_ShortRelease(uint8_t btnCode);

/**
  * @brief Button_LongPress optional user defined Long button press function
  * The body of this function is implemented as __weak and user can reimplement 
  * this function inside his own code with addition logic, to get Long button press event
  * Used to check Long button press event for particular button code
  * @param btnCode: button code
  * @retval None
  */
void Button_LongPress(uint8_t btnCode);

/**
  * @brief Button_LongRelease optional user defined Long button release function
  * The body of this function is implemented as __weak and user can reimplement 
  * this function inside his own code with addition logic, to get Long button release event
  * Used to check Long button release event for particular button code
  * @param btnCode: button code
  * @retval None
  */
void Button_LongRelease(uint8_t btnCode);

#endif //BUTTON_H
