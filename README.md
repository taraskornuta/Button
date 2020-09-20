# Button
### Embedded MCU library to handle buttons pressing.

### Description: 
This is a high-level abstraction layer library which provides easy to use driver 
to handle simple signal buttons connected directly to MCU GPIO.

The library is provide:
- Up to 255 buttons handling
- Buttons debounce filtering
- Single short press handling
- Single long press handling
- Short release button callback function
- Long press button callback function
- Long release button callback function
 
## Setup:
User should provide function which can read the GPIO port and return state: 
- true  - button active pressed
- false - button unactive released

In STM HAL - can be used HAL_GPIO_ReadPin(). In other hand user can provide 
wrapper function for other SDK
The library can work with array of buttons. The maximum is 255 buttons. 
Thay can be connected to different ports in any order. 

There are a few settings which should be adjusted by the user:
- BTN_PROCESSING_TIME_MS - set the time in millisecond interval which used to 
                         call Button_Update() function
- BTN_DEBOUNCE_TIME_MS   - set the time in millisecond to process button debounce
                         Default is 20ms
- BTN_LONG_PRESS_TIME_MS - set the time millisecond to generate long press event

## Example:
```C
int main(void)
{
  btnInstance_t keys[4] = 
  {
    BTN_INIT_INSTANCE(GPIOA, GPIO_PIN_0),
    BTN_INIT_INSTANCE(GPIOB, GPIO_PIN_1),
    BTN_INIT_INSTANCE(GPIOC, GPIO_PIN_2),
    BTN_INIT_INSTANCE(GPIOD, GPIO_PIN_3),
  };
  
  Button_Init(keys, (portReadCallback_t)&HAL_GPIO_ReadPin, 4);

  while(1)
  {
    if (EV_SHORT == Button_EventGet(0)) printf("Button 0 SHORT press\n");
    if (EV_SHORT == Button_EventGet(1)) printf("Button 1 SHORT press\n");
    // ...
    if (EV_LONG == Button_EventGet(0)) printf("Button 0 LONG press\n");
    if (EV_LONG == Button_EventGet(1)) printf("Button 1 LONG press\n");
  }
}

void Button_ShortRelease(uint8_t btnCode)
{
  printf("BUTTON_PIN_%d state ShortRelease", btnCode);
}

void Button_LongPress(uint8_t btnCode)
{
  printf("BUTTON_PIN_%d state LongPress", btnCode);
}

void Button_LongRelease(uint8_t btnCode)
{
  printf("BUTTON_PIN_%d state LongRelease", btnCode);
}

void TIM1_UP_IRQHandler(void)  // 10 milliseconds timer interrupt handler
{
   Button_Update();
}
```



## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
MIT