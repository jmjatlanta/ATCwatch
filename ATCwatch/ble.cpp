/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ble.h"
#include "pinout.h"
#include <BLEPeripheral.h>
#include "sleep.h"
#include "time.h"
#include "battery.h"
#include "inputoutput.h"
#include "backlight.h"
#include "bootloader.h"
#include "push.h"
#include "accl.h"

BLEPeripheral                   blePeripheral           = BLEPeripheral();
BLEService                      main_service     = BLEService("190A");
BLECharacteristic   TXchar        = BLECharacteristic("0002", BLENotify, 20);
BLECharacteristic   RXchar        = BLECharacteristic("0001", BLEWriteWithoutResponse, 20);

bool vars_ble_connected = false;

void init_ble() {
  blePeripheral.setLocalName("BowWatchV1");
  blePeripheral.setAdvertisingInterval(500); // in ms
  blePeripheral.setDeviceName("BowWatch");
  blePeripheral.setAdvertisedServiceUuid(main_service.uuid());
  // There are 3 attributes here...
  // main_service is what things connect to
  // TXchar is a notification
  // RXchar is a "WriteWithoutResponse"
  blePeripheral.addAttribute(main_service);
  blePeripheral.addAttribute(TXchar);
  blePeripheral.addAttribute(RXchar);
  // handle events
  // when RXchar receives something
  RXchar.setEventHandler(BLEWritten, ble_written);
  // when someone connects to the watch
  blePeripheral.setEventHandler(BLEConnected, ble_ConnectHandler);
  // when someone disconnects from the watch
  blePeripheral.setEventHandler(BLEDisconnected, ble_DisconnectHandler);
  blePeripheral.begin();
  ble_feed();
}

void ble_feed() {
  blePeripheral.poll();
}

/***
 * handles a connection event (when a device connects to the watch)
 * @param central info about the foreign device
 */
void ble_ConnectHandler(BLECentral& central) {
  sleep_up(WAKEUP_BLECONNECTED);
  set_vars_ble_connected(true);
}

/***
 * handles a disconnect event
 * @param central the foreign device
 */
void ble_DisconnectHandler(BLECentral& central) {
  sleep_up(WAKEUP_BLEDISCONNECTED);
  set_vars_ble_connected(false);
}

String answer = "";
String tempCmd = "";
int tempLen = 0, tempLen1;
boolean syn;

/***
 * Called when RXchar gets written to
 * @param central where it came from
 * @param characteristic the incoming data
 */
void ble_written(BLECentral& central, BLECharacteristic& characteristic) {
  char remoteCharArray[22];
  tempLen1 = characteristic.valueLength();
  tempLen = tempLen + tempLen1;
  memset(remoteCharArray, 0, sizeof(remoteCharArray));
  memcpy(remoteCharArray, characteristic.value(), tempLen1);
  tempCmd = tempCmd + remoteCharArray;
  if (tempCmd[tempLen - 2] == '\r' && tempCmd[tempLen - 1] == '\n') {
    answer = tempCmd.substring(0, tempLen - 2);
    tempCmd = "";
    tempLen = 0;
    filterCmd(answer);
  }
}

/*****
 * Write to TXchar
 * @param Command what to write
 */
void ble_write(String Command) {
  Command = Command + "\r\n";
  while (Command.length() > 0) {
    const char* TempSendCmd;
    String TempCommand = Command.substring(0, 20);
    TempSendCmd = &TempCommand[0];
    TXchar.setValue(TempSendCmd);
    Command = Command.substring(20);
  }
}

/*****
 * See if a device is connected to the watch
 * @returns true if there is a connection
 */
bool get_vars_ble_connected() {
  return vars_ble_connected;
}

/****
 * Set the connection status
 * @param state true if there is a connection, false if not
 */
void set_vars_ble_connected(bool state) {
  vars_ble_connected = state;
}

/****
 * convert a command into something the BLE device can use
 * I believe this could better be handled by separate characteristics.
 * I need to test to see if memory requirements get better or worse
 * @param Command the command
 */
void filterCmd(String Command) {
  if (Command == "AT+BOND") {
    ble_write("AT+BOND:OK");
  } else if (Command == "AT+ACT") {
    ble_write("AT+ACT:0");
  } else if (Command.substring(0, 7) == "BT+UPGB") {
    start_bootloader();
  } else if (Command.substring(0, 8) == "BT+RESET") {
    set_reboot();
  } else if (Command.substring(0, 7) == "AT+RUN=") {
    ble_write("AT+RUN:" + Command.substring(7));
  } else if (Command.substring(0, 8) == "AT+USER=") {
    ble_write("AT+USER:" + Command.substring(8));
  } else if (Command == "AT+PACE") {
    accl_data_struct accl_data = get_accl_data();
    ble_write("AT+PACE:" + String(accl_data.steps));
  } else if (Command == "AT+BATT") {
    ble_write("AT+BATT:" + String(get_battery_percent()));
  } else if (Command.substring(0, 8) == "AT+PUSH=") {
    ble_write("AT+PUSH:OK");
    show_push(Command.substring(8));
  } else if (Command == "BT+VER") {
    ble_write("BT+VER:P8");
  } else if (Command == "AT+VER") {
    ble_write("AT+VER:P8");
  } else if (Command == "AT+SN") {
    ble_write("AT+SN:P8");
  } else if (Command.substring(0, 12) == "AT+CONTRAST=") {
    String contrastTemp = Command.substring(12);
    if (contrastTemp == "100")
      set_backlight(1);
    else if (contrastTemp == "175")
      set_backlight(3);
    else set_backlight(7);
    ble_write("AT+CONTRAST:" + Command.substring(12));
  } else if (Command.substring(0, 10) == "AT+MOTOR=1") {
    String motor_power = Command.substring(10);
    if (motor_power == "1")
      set_motor_power(50);
    else if (motor_power == "2")
      set_motor_power(200);
    else set_motor_power(350);
    ble_write("AT+MOTOR:1" + Command.substring(10));
    set_motor_ms();
  } else if (Command.substring(0, 6) == "AT+DT=") {
     // set date and time
    SetDateTimeString(Command.substring(6));
    ble_write("AT+DT:" + GetDateTimeString());
  } else if (Command.substring(0, 5) == "AT+DT") {
     // get date and time
    ble_write("AT+DT:" + GetDateTimeString());
  } else if (Command.substring(0, 8) == "AT+HTTP=") {
    show_http(Command.substring(8));
  }
}
