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

BLEPeripheral     blePeripheral = BLEPeripheral();
BLEService        main_service  = BLEService("190A");
BLECharacteristic Depth = BLECharacteristic("3842fdbd-8feb-4e8e-b90d-8d8770bdee00", BLEWrite, 8);
BLECharacteristic DepthSubscribe = BLECharacteristic("db276bb3-ca7e-44eb-9990-ccf2bd630cf7", BLERead | BLENotify, 1);
BLECharacteristic DateTime = BLECharacteristic("1b879018-3228-45a9-969e-7fd647e45327", BLEWrite, 14);
BLECharacteristic TXchar = BLECharacteristic("0002", BLERead | BLENotify, 20);
BLECharacteristic RXchar = BLECharacteristic("0001", BLEWriteWithoutResponse, 20);
BleEventHandler* handler = nullptr;
bool vars_ble_connected = false;
String lastLogEntry = "";

void init_ble() {
   blePeripheral.setLocalName("BowWatch");
   blePeripheral.setAdvertisingInterval(500); // in ms
   blePeripheral.setDeviceName("BowWatchV1");
   blePeripheral.setAdvertisedServiceUuid(main_service.uuid());
   // when someone connects to the watch
   blePeripheral.setEventHandler(BLEConnected, ble_ConnectHandler);
   // when someone disconnects from the watch
   blePeripheral.setEventHandler(BLEDisconnected, ble_DisconnectHandler);
   // main_service is what things connect to
   blePeripheral.addAttribute(main_service);
   // TXchar is a notification. It should tell the remote device when it is updated
   blePeripheral.addAttribute(TXchar);
   // RXchar is a "WriteWithoutResponse"
   blePeripheral.addAttribute(RXchar);
   // when RXchar receives something
   RXchar.setEventHandler(BLEWritten, ble_written);

   // Depth Subscribe / unsubscribe
   char val[1] = {0};
   DepthSubscribe.setValue(val);
   blePeripheral.addAttribute(DepthSubscribe);

   // Depth
   char depthVal[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
   Depth.setValue( depthVal );
   Depth.setEventHandler(BLEWritten, DepthWritten);
   blePeripheral.addAttribute(Depth);

   // Date/Time
   DateTime.setValue("");
   DateTime.setEventHandler(BLEWritten, DateTimeWritten);
   blePeripheral.addAttribute(DateTime);

   blePeripheral.begin();
   ble_feed();
}

void ble_feed() {
  blePeripheral.poll();
}

void ble_set_event_handler(BleEventHandler* in)
{
   handler = in;
   char val[1] = {0};
   if (in != nullptr)
      val[0] = 1;
   DepthSubscribe.setValue(val);
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
void ble_written(BLECentral& central, BLECharacteristic& characteristic) 
{
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

void DepthWritten(BLECentral& central, BLECharacteristic& characteristic)
{
   if (handler != nullptr)
   {
      unsigned const char* value = characteristic.value();
      // unpack the 8 byte long into a 4 byte long
      unsigned long l = value[7] | (value[6] << 8) | (value[5] << 16) | (value[4] << 24);
      handler->onDepth(l);
      lastLogEntry = "ReceivedDepth";
   } else {
      lastLogEntry = "NoHandlerDepth";
   }

}

void DateTimeWritten(BLECentral& central, BLECharacteristic& characteristic)
{
   if (characteristic.valueLength() == 14)
   {
      char latestValue[15];
      memset(latestValue, 0, 15);
      memcpy(latestValue, characteristic.value(), 15);
      String tempString = latestValue;
      SetDateTimeString(tempString);
   }
   else
   {
      lastLogEntry = "InvalidDateTime";
   }
   if (handler != nullptr)
      handler->onDateTime(lastLogEntry);
}

String get_last_message()
{
   return lastLogEntry;
}

/*****
 * Write to TXchar so someone that subscribed to be notified
 * receives the message
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
