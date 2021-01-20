/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "Arduino.h"
#include "menu.h"
#include <lvgl.h>

#define DEFAULT_SLEEP_TIMEOUT 10000
#define DEFAULT_REFRESH_TIME 40

class Screen_def
{
public:
   /***
    * On system startup
    */   
   virtual void pre()
   {
   }

   /***
    * On app selected
    */
   virtual void main()
   {
   }

   /***
    * On app unselected
    */
   virtual void post()
   {
      running = false;
   }

   /***
    * On swipe up
    */
   virtual void up()
   {
   }

   /****
    * On swipe down
    */
   virtual void down()
   {
   }

   /****
    * On swipe right to left
    */
   virtual void left()
   {
   }

   /***
    * On swipe left to right
    */
   virtual void right()
   {
   }

   /***
    * On single press
    */
   virtual void click(touch_data_struct touch_data)
   {
   }

   /****
    * On long press
    */
   virtual void long_click(touch_data_struct touch_data)
   {
   }

   /****
    * On hardware button press
    * @param length
    */ 
   virtual void button_push(int length)
   {
   }

   /***
    * How much idle time should pass before going to sleep
    */
   virtual uint32_t sleepTime()
   {
      return DEFAULT_SLEEP_TIMEOUT;
   }

   /****
    * How long to refresh
    * @returns refresh time
    */ 
   virtual uint32_t refreshTime()
   {
      return DEFAULT_REFRESH_TIME;
   }

   /***
    * Process a graphical event
    * @param object the object where the event took place
    * @param event details of what took place
    */ 
   virtual void lv_event_class(lv_obj_t * object, lv_event_t event)
   {
      
   }

   virtual void pre_display()
   {
      lv_obj_clean(lv_scr_act());
   }    
protected:
   bool running = false;
};
