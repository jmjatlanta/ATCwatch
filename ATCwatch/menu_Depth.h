/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "Arduino.h"
#include "class.h"
#include "images.h"
#include "menu.h"
#include "display.h"
#include "ble.h"
#include "time.h"
#include "battery.h"
#include "accl.h"
#include "push.h"
#include "heartrate.h"


class DepthScreen : public Screen, public BleEventHandler
{
  public:
    virtual void pre()
    {
      ble_set_event_handler(this);
      lv_style_copy( &bigFont, &lv_style_plain );
      bigFont.text.color = lv_color_hsv_to_rgb(10, 5, 95);
      bigFont.text.font = &mksd50;

      lv_style_copy( &littleFont, &lv_style_plain );
      littleFont.text.color = lv_color_hsv_to_rgb(10, 5, 95);
      littleFont.text.font = &sans_bold28;

      label_screen = lv_label_create(lv_scr_act(), NULL);
      lv_label_set_text(label_screen, "Depth");
      lv_obj_align(label_screen, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

      label_big_number = lv_label_create(lv_scr_act(), NULL);
      lv_obj_set_style(label_big_number, &bigFont);
      lv_label_set_text(label_big_number, "Ready");
      lv_obj_align(label_big_number, NULL, LV_ALIGN_CENTER, 0, -30);

      label_little_number = lv_label_create(lv_scr_act(), NULL);
      lv_obj_set_style(label_little_number, &littleFont);
      lv_label_set_text(label_little_number, "");
      lv_obj_align(label_little_number, NULL, LV_ALIGN_CENTER, 50, -30);
    }

    virtual void main()
    {
       running = true;
    }

    virtual void up()
    {
    }

    virtual void down()
    {
    }

    virtual void right()
    {
      display_home();
    }

    virtual void post() {
       ble_set_event_handler(nullptr);
    }

    virtual uint32_t sleepTime() override {
       return  300 * 1000; // 5 minutes
    }

   virtual void lv_event_class(lv_obj_t * object, lv_event_t event) {
   }

   virtual void onDepth(unsigned long depth) override {
      if (running)
      {
         unsigned long large = depth / 100;
         lv_label_set_text_fmt(label_big_number, "%i", large);
         lv_obj_align(label_big_number, NULL, LV_ALIGN_CENTER, -30, -30);

         unsigned long small = depth % 100;
         lv_label_set_text_fmt(label_little_number, "%i", small);
         lv_obj_align(label_little_number, NULL, LV_ALIGN_CENTER, 10, -30);
      }
   }

   virtual void onDateTime(String in) override  { }

  private:
    lv_obj_t *label_screen;
    lv_obj_t *label_big_number;
    lv_obj_t *label_little_number;
    lv_style_t bigFont;
    lv_style_t littleFont;
};

DepthScreen depthScreen;
