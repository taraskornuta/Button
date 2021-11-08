#ifndef BUTTON_H
#define BUTTON_H

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

/**
  * Name: Button library 
  * Version: v2.0
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
  * - Adjustable long press time for each button
  * - Adjustable debounce time
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
  *   btn_instance_t btn_inst[2] = {
  *     {
  *         .port = GPIOA,
  *         .pin = GPIO_PIN_0,
  *     },
  *     {
  *         .long_press_time_ms = 2050,
  *         .port = GPIOB,
  *         .pin = GPIO_PIN_1,
  *     },
  *   };
  *
  *   btn_init_t btn_init = {
  *     .process_time_ms = 10,
  *     .debounce_time_ms = 20,
  *     ..port_read = (port_read_cb_t)&HAL_GPIO_ReadPin,
  *     .short_release = Button_ShortRelease,
  *     .long_release = Button_LongRelease,
  *     .long_press = Button_LongPress,
  *   };
  * 
  *   Button_Init(&btn_init, btn_inst, 2);
  *
  *   while(1)
  *   {
  *     if (BTN_STATE_SHORT == Button_EventGet(0)) printf("Button 0 SHORT press\n");
  *     if (BTN_STATE_SHORT == Button_EventGet(1)) printf("Button 1 SHORT press\n");
  *     // ...
  *     if (BTN_STATE_LONG == Button_EventGet(0)) printf("Button 0 LONG press\n");
  *     if (BTN_STATE_LONG == Button_EventGet(1)) printf("Button 1 LONG press\n");
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


typedef enum {
  BTN_STATE_NONE = 0,  // button not pressed
  BTN_STATE_SHORT,     // button short pressed detected
  BTN_STATE_LONG       // button long pressed detected
} btn_state_t;

/**
  * @brief port_read_cb_t user provided port read function callback
  */
typedef uint8_t (*port_read_cb_t)(const uint32_t *port, const uint32_t pin);

/**
 * @brief btn_event_cb_t provide button event callback  
 */
typedef void (*btn_ev_cb_t)(uint8_t btn_code);

/**
 * @brief Button instance configuration parameters and operation data
 * The user provided parameters:
 * @param port: pointer to GPIO port
 * @param pin: number of the pin
 * @param long_press_time_ms: optionaly user can define individual long press time 
 *       for each button 
 * Multiple button instances can be created by grouping them into array
 * btn_instance_t btn_inst[2] = {
 *  {.port = GPIOA,.pin = 13},
 *  {.port = GPIOC,.pin = 5} 
 * };   
 */
typedef struct {
  struct {
    btn_state_t  act  : 2;     // actual button state
    btn_state_t  prew : 2;     // previeus button state
    uint8_t      locked : 1;   // debounce button lock
  } state;

  uint8_t        lock_count;   // debounce time counter
  uint16_t       long_count;   // long press time counter

  uint16_t long_press_time_ms; // optional individual button long press time
  const uint32_t *port;        // GPIO port
  const uint32_t pin;          // GPIO pin
} btn_instance_t;

/**
 * @brief Init struct
 * 
 */
typedef struct {
  btn_instance_t *instance;     // configured buttons instancess 
  uint8_t  count;               // configured ammount of used buttons
  uint8_t  process_time_ms;     // configured button processing time
  uint8_t  debounce_time_ms;    // configured contacts debounce time
  uint16_t long_press_def_ms;   // configured default time for long press 

  port_read_cb_t port_read;     // configured callback function to read port 
  btn_ev_cb_t    short_release; // optional callback function for short release event
  btn_ev_cb_t    long_release;  // optional callback function for long press release event
  btn_ev_cb_t    long_press;    // optional callback function for long press detection event
} btn_init_t;


/**
  * @brief Button_Init Buttons initialization function
  * Should be called before all others button module functions 
  * @param init: pointer to init struct
  * @param instance: pointer to button instance struct
  * @param count: buttons count. Max number of buttons is 255
  * @retval -1 - Init error, 0 - Init Ok
  */
int8_t Button_Init(btn_init_t *init, btn_instance_t *instance, uint8_t count);

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
  * @param key: button code
  * @retval button status: BTN_STATE_NONE, BTN_STATE_SHORT, BTN_STATE_LONG
  */
btn_state_t Button_EventGet(uint8_t key);

#endif //BUTTON_H
