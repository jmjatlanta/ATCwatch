all: ATCwatch.ino.zip

ATCwatch.ino.zip: ATCwatch.ino
	~/arduino-1.8.13/arduino --verify --board sandeepmistry:nRF5:p8watch:softdevice=stockFW -v /home/jmjatlanta/Development/cpp/ATCwatch/ATCwatch/ATCwatch.ino

install: ATCwatch.ino.zip
	adb push ~/arduino-1.8.13/ATCwatch.ino.zip /mnt/sdcard/Download/
