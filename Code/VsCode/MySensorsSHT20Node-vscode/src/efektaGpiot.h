/*
 Name:		                    efektaGPIOT
 Created:	                    25.01.2020
 Modified:                      15.02.2020
 Autor of an idea & creator:    Berk (Efekta)
 Programming:                   Alex aka Sr.FatCat

Library GPIOT
NRF5x lower power interruble in sleep.
*/
#ifndef EFEKTA_GPIOT_H
#define EFEKTA_GPIOT_H

extern "C" {
#include "app_gpiote.h"
#include "nrf_gpio.h"
}

#include <stdint.h>

#define dream(x, y) interruptedSleep.run((x), (y))
#define dream(x) interruptedSleep.run((x))
#define addDreamPin(x, y, z) interruptedSleep.addPin((x), (y), (z))
#define initDream() interruptedSleep.init()


#ifndef APP_GPIOTE_MAX_USERS
#define APP_GPIOTE_MAX_USERS 1
#endif
// nrf_gpio_pin_pull_t = {NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_PULLDOWN}
 
extern class CDream {
public:
    enum ePinsIntEvent { NRF_PIN_HIGH_TO_LOW, NRF_PIN_LOW_TO_HIGH, NRF_PIN_GHANGE };
    struct sIntGPIOT {
        uint8_t gpio;
        nrf_gpio_pin_pull_t pull;
        ePinsIntEvent event;
    };
private:    
    static app_gpiote_user_id_t m_gpiote_user_id;
    static uint32_t wakeUpRegim;
    sIntGPIOT *pins;
    uint8_t pinsNum;
    uint8_t pinsAdded;
public:  
    //CDream() {}
    CDream(const uint8_t pinsNum_) : pinsNum(pinsNum_) { pins = new sIntGPIOT[pinsNum]; pinsAdded = 0;}
    //CDream(const uint8_t pinsNum_, sIntGPIOT *pins_) : pinsNum(pinsNum_) { pins = pins_; pinsAdded = pinsNum;}   
    void addPin(uint8_t, nrf_gpio_pin_pull_t, ePinsIntEvent);
    void init();
    int8_t run(uint32_t , bool = true);
    inline uint8_t getPinsNum() {return pinsNum;}
    inline sIntGPIOT * getPins() {return pins;}
    inline ePinsIntEvent getPinEvent(uint8_t idx) {return pins[idx].event;}
    inline uint8_t getPinGPIO(uint8_t idx) {return pins[idx].gpio;}
    inline void setWakeupRegim(uint8_t regim) {wakeUpRegim = pins[regim].gpio;}
} interruptedSleep;

uint32_t CDream::wakeUpRegim = 0;
app_gpiote_user_id_t CDream::m_gpiote_user_id = 0;

void CDream::addPin(uint8_t gpio_, nrf_gpio_pin_pull_t pull_, ePinsIntEvent event_){
    if (pinsNum == 0 || pinsAdded == pinsNum) return;
    pins[pinsAdded].gpio = gpio_;
    pins[pinsAdded].pull = pull_;
    pins[pinsAdded].event = event_;
    pinsAdded++;
}

void gpiote_event_handler(uint32_t, uint32_t);

void CDream::init(){
  if (pinsAdded != pinsNum) return;
  uint32_t pins_low_to_high_mask = 0, pins_high_to_low_mask = 0;

  for (int i = 0; i < pinsNum; i++){
      nrf_gpio_cfg_input(pins[i].gpio, pins[i].pull);
      if (pins[i].event == NRF_PIN_HIGH_TO_LOW ) pins_high_to_low_mask |= (1 << pins[i].gpio);
      else if (pins[i].event == NRF_PIN_LOW_TO_HIGH )  pins_low_to_high_mask |= (1 << pins[i].gpio);
      else {
          pins_high_to_low_mask |= (1 << pins[i].gpio);
          pins_low_to_high_mask |= (1 << pins[i].gpio);
      }
  }
  
  APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
 
  app_gpiote_user_register(&m_gpiote_user_id, pins_low_to_high_mask, pins_high_to_low_mask, gpiote_event_handler);
  app_gpiote_user_enable(m_gpiote_user_id);
}

void gpiote_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
    MY_HW_RTC->CC[0] = (MY_HW_RTC->COUNTER + 2); // Taken from d0016 example code, ends the sleep delay
    for (int i = 0; i < interruptedSleep.getPinsNum(); i++){
       if (interruptedSleep.getPinEvent(i) == CDream::NRF_PIN_HIGH_TO_LOW || interruptedSleep.getPinEvent(i) == CDream::NRF_PIN_GHANGE) {
          if ((1 << interruptedSleep.getPinGPIO(i)) & event_pins_high_to_low){
               interruptedSleep.setWakeupRegim(i);
               return;
          }
       }
       if (interruptedSleep.getPinEvent(i) == CDream::NRF_PIN_LOW_TO_HIGH || interruptedSleep.getPinEvent(i) == CDream::NRF_PIN_GHANGE) {
          if ((1 << interruptedSleep.getPinGPIO(i)) & event_pins_low_to_high){
               interruptedSleep.setWakeupRegim(i);
               return;
          }
       }
       
    }
}

int8_t CDream::run(uint32_t sleep_time, bool isSmartSleep){
    wakeUpRegim = 0;
    int retSleep = sleep(sleep_time, isSmartSleep);
    if (retSleep == MY_SLEEP_NOT_POSSIBLE) return MY_SLEEP_NOT_POSSIBLE;
    if (wakeUpRegim > 0) return wakeUpRegim; else return MY_WAKE_UP_BY_TIMER;
}

#endif