/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"

struct time_data_struct {
  int year;
  int month;
  int day;
  int hr;
  int min;
  int sec;
};

/****
 * Initialize the time to 2020-01-01 00:00:00
 */
void init_time();

/****
 * get the time as a struct
 * @returns the date and time as a time_data_struct
 */
time_data_struct get_time();

/****
 * Set date and time
 * @param datetime timestamp in format YYYYMMDDHHMMSS
 */
void SetDateTimeString(String datetime);

/****
 * Set the date
 * @param year the year
 * @param month the month
 * @param day the day
 */
void SetDate(int year, int month, int day);

/****
 * Set the time
 * @param hr the hour
 * @param min the minute
 */
void SetTime(int hr, int min);

/*****
 * Retrieve the date and time (no seconds)
 * @returns the date in the format YYYYMMDDHHMM
 */
String GetDateTimeString();
