/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"
#include <BLEPeripheral.h>

class BleEventHandler
{
public:
   virtual void onDepth(unsigned long depth) = 0;
   virtual void onDateTime(String newDateTime);
};

void init_ble();
void ble_feed();
void ble_ConnectHandler(BLECentral& central);
void ble_DisconnectHandler(BLECentral& central);
void ble_DisconnectHandler(BLECentral& central);
void ble_written(BLECentral& central, BLECharacteristic& characteristic);
void ble_set_event_handler(BleEventHandler* handler);
void DepthWritten(BLECentral& central, BLECharacteristic& characteristic);
void DateTimeWritten(BLECentral& central, BLECharacteristic& characteristic);
void ble_write(String Command);
bool get_vars_ble_connected();
void set_vars_ble_connected(bool state);
void filterCmd(String Command);
String get_last_message();
