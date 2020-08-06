#pragma once

#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"
#include "set_screen.h"
#include "screens.h"
#include "button.h"
#include "pwm_.h"

template<class Pins, class Flash_data, class Clock, size_t qty_lines = 4, size_t size_line = 20>
struct Menu : TickSubscriber {
   String_buffer lcd {};
   HD44780& hd44780 {HD44780::make(Pins{}, lcd.get_buffer())};
   Button_event& up;
   Button_event& down;
   Button_event& enter;
   Flash_data&   flash;
   PWM&          pwm;
   Clock&        clock;

   Screen* current_screen {&main_screen};
   size_t tick_count{0};

   Buttons_events buttons_events {
        Up_event    {[this](auto c){   up.set_click_callback(c);}}
      , Down_event  {[this](auto c){ down.set_click_callback(c);}}
      , Enter_event {[this](auto c){enter.set_click_callback(c);}}
      , Out_event   {[this](auto c){enter.set_long_push_callback(c);}}
      , Increment_up_event   {[this](auto c){  up.set_increment_callback(c);}}
      , Increment_down_event {[this](auto c){down.set_increment_callback(c);}}
   };

   Menu (
        Pins pins
      , Button_event& up
      , Button_event& down
      , Button_event& enter
      , Flash_data&   flash
      , PWM&          pwm
      , Clock&        clock
   ) : up{up}, down{down}, enter{enter}
      , flash{flash}, pwm{pwm}, clock{clock}
   {
      tick_subscribe();
      current_screen->init();
      while(not hd44780.init_done()){}
   }

   Main_screen<Clock> main_screen {
          lcd, buttons_events
        , Out_callback { [this]{ change_screen(main_select); }}
        , clock
   };

   Select_screen<2> main_select {
          lcd, buttons_events
        , Out_callback          { [this]{change_screen(main_screen)   ;}}
        , Line {"Настройка"      ,[this]{change_screen(time_screen); time_screen.reset();}}
        , Line {"Будильник"      ,[this]{/*change_screen(alarm_screen)*/  ;}}

   };

   Time_screen<Clock> time_screen {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(main_select);}}
        , Exit_callback      { [this](bool exit) {
           if (exit) change_screen(main_select);
        }}
        , clock
   };
   
   void notify() override {
      every_qty_cnt_call(tick_count, 50, [this]{
         current_screen->draw();
      });
   }

   void change_screen(Screen& new_screen) {
      current_screen->deinit();
      current_screen = &new_screen;
      current_screen->init();
   }

};