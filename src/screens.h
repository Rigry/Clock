#pragma once

#include "screen_common.h"
#include "timers.h"
// #include "limited_int.h"
#include <tuple>
#include <array>
#include <bitset>

// constexpr auto day_of_week = std::array {
//     "no", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вск"
// };

// constexpr std::string_view day_of_week_to_string(int i) {
//     return day_of_week[i];
// }

template <class Clock> 
struct Main_screen : Screen {
   String_buffer& lcd;
   Buttons_events eventers;
   Callback<> out_callback;
   Clock& clock;

   Main_screen(
        String_buffer& lcd
      , Buttons_events eventers
      , Out_callback out_callback
      , Clock&       clock
      
   ) : lcd          {lcd}
     , eventers     {eventers}
     , out_callback {out_callback.value}
     , clock        {clock}
   {}

   void init() override {
      eventers.enter ([this]{ });
      eventers.up    ([this]{ });
      eventers.down  ([this]{ });
      eventers.out   ([this]{ out_callback(); });
      lcd.clear();
   }

   void deinit() override {
      eventers.enter (nullptr);
      eventers.up    (nullptr);
      eventers.down  (nullptr);
      eventers.out   (nullptr);
   }

   void draw() override {
      lcd.line(0).cursor(4)._10(clock.hour());
      lcd.line(0).cursor(6) << ":";
      lcd.line(0).cursor(7)._10(clock.minute());
      lcd.line(0).cursor(9) << ":";
      lcd.line(0).cursor(10)._10(clock.second());
      lcd.line(1).cursor(4)._10(clock.date());
      lcd.line(1).cursor(6) << ".";
      lcd.line(1).cursor(7)._10(clock.month());
      lcd.line(1).cursor(9) << ".";
      lcd.line(1).cursor(10)._10(clock.year());
   }
};

template<class Int = size_t>
class Limited_int {
    Int value  {0};
    size_t min {0};
    size_t max {0};
public:
    Limited_int(size_t min, size_t max) : min{min}, max{max}{}
    inline Int operator++(int)   { return value = (value++ < max) ? value : min; }
    inline Int operator--(int)   { return value = (value-- > min) ? value : max; }
    inline operator Int() const  { return value; }
    inline Int operator= (Int v) { return value = v; }
};

struct Date {
  Limited_int<uint16_t> year  {0,99};
  Limited_int<uint16_t> month {1,12};     
  Limited_int<uint16_t> date  {1,31};   
  Limited_int<uint16_t> hour  {1,23};
  Limited_int<uint16_t> minute{0,59}; 
  Limited_int<uint16_t> second{0,59};
  Limited_int<uint16_t> day   {1,7};
};

using Exit_callback = Construct_wrapper<Callback<bool>>;

template <class Clock> 
struct Time_screen : Screen {
    String_buffer& lcd;
    Buttons_events eventers;
    Callback<> out_callback;
    Callback<bool> exit_callback;
    Clock& clock;
    Timer blink{500_ms};
    bool blink_{false};
    union {
      Date date_struct;
      std::array<Limited_int<uint16_t>, 6> date;
    };

    Time_screen (
          String_buffer& lcd
        , Buttons_events eventers
        , Out_callback out_callback
        , Exit_callback exit_callback
        , Clock& clock
    ) : lcd          {lcd}
      , eventers     {eventers}
      , out_callback {out_callback.value}
      , exit_callback{exit_callback.value}
      , clock        {clock}
    {}

    void init() override {
        eventers.up    ([this]{ up();    });
        eventers.down  ([this]{ down();  });
        eventers.enter ([this]{ enter();       });
        eventers.out   ([this]{ out_callback();});
        date_struct = clock.date_struct();
        lcd.clear();
        lcd.line(0).cursor(4)._10(date_struct.hour);
        lcd.line(0).cursor(6) << ":";
        lcd.line(0).cursor(7)._10(date_struct.minute);
        lcd.line(0).cursor(9) << ":";
        lcd.line(0).cursor(10)._10(date_struct.second);
        lcd.line(1).cursor(4)._10(date_struct.date);
        lcd.line(1).cursor(6) << ".";
        lcd.line(1).cursor(7)._10(date_struct.month);
        lcd.line(1).cursor(9) << ".";
        lcd.line(1).cursor(10)._10(date_struct.year);
    }
    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {
      blink_ ^= blink.event();
      blink_ ? lcd.line(line).cursor(cursor)._10(date[index]) : lcd.line(line).cursor(cursor) << "  ";
    }

private:
   
  int index{0};
  int line{1};
  int cursor{10};
  bool exit {false};
  void up() {
    date[index]++;
  }
  void down() {
    date[index]--;
  }
  void enter() {
    if (++index > 5) index = 0;
    exit = false;
    if (line == 1) {
      cursor == 4 ? line = 0 : cursor -= 3;
    } else if (line == 0) {
      if (cursor == 10) {
        line = 1;
        exit = true;
        exit_callback(exit);
        clock.set_time(date_struct);
      } else 
        cursor += 3;
    }
  }

};