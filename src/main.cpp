#define STM32F030x6
#define F_OSC   8000000UL
#define F_CPU   48000000UL

#include "init_clock.h"
#include "periph_rcc.h"
#include "periph_flash.h"
#include "pin.h"
#include "literals.h"
#include "button.h"
#include "clock.h"
#include "menu.h"
#include "flash.h"

using Right = mcu::PC13;
using Enter = mcu::PC14;
using Left  = mcu::PC15;
using RS    = mcu::PA0;  
using E     = mcu::PA1;       
using RW    = mcu::PA2;           
using DB4   = mcu::PA3;       
using DB5   = mcu::PA4;
using DB6   = mcu::PA5;    
using DB7   = mcu::PA6;
using Vent  = mcu::PB4;
using TXD   = mcu::PA9;
using RXD   = mcu::PA10;

extern "C" void init_clock () { init_clock<F_OSC, F_CPU>(); }

int main()
{
   struct Flash_data {
      
      uint16_t hours = 0;
      
   } flash;
   
   [[maybe_unused]] auto _ = Flash_updater<
        mcu::FLASH::Sector::_26
      , mcu::FLASH::Sector::_25
   >::make (&flash);

   Date date{};

   volatile decltype(auto) clock = Clock<Date>::make(date); 

   volatile decltype(auto) pwm = PWM::make<mcu::Periph::TIM3, Vent>(990);

   auto up    = Button<Right>();
   auto down  = Button<Left>();
   auto enter = Button<Enter>();
   constexpr auto hd44780_pins = HD44780_pins<RS, RW, E, DB4, DB5, DB6, DB7>{};
   [[maybe_unused]] auto menu = Menu (hd44780_pins, up, down, enter, flash, pwm, clock);
   
   while(1){
      __WFI();
   }
}
