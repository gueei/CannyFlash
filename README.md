# CannyFlash
Bootloader that allows flashing Arduino device via onboard CAN Interface (MCP2515)

## Current status
Compiled Bootloader in CannyFlash/bin
Only Atmega32u4 (Arduino Leonardo) (tested)
Atmega328p (Arduino UNO) need to recompile (but tested)

## Bootloader flashing
First need to prepare Arduino ISP, and wire as following instruction:
https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP

Next reset fuse on Target Device. (following for Leonardo)
    avrdude -C{ArduinoDirectory}/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf" -v -patmega32u4 -cstk500v1 -P{COM_Port} -b19200 -e -Ulock:w:0x3F:m -Uefuse:w:0xcb:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m
    
You can also turn on verbose in Arduino IDE (Preferences -> Show Verbose Output during -> Upload), copy and modify the command while uploading some sketch in Arduino

Finally upload bootloader using following command:
    avrdude -C{ArduinoDirectory}/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf" -v -V -patmega32u4 -cstk500v1 -P{COM_Port} -b19200 -Uflash:w:{CannyFlash_Bootloader.hex}:i -Ulock:w:0x2F:m

Your device can now flash via CANBus!

## Arduino IDE setting
Add the following to the end of "programmers.txt" file (under {ArduinoDirectory}/hardware/avr/1.8.3)

    canisp.name=CannyFlash Programmer
    canisp.communication=serial
    canisp.protocol=arduino
    canisp.speed=57600
    canisp.program.protocol=arduino
    canisp.program.speed=57600
    canisp.program.tool=avrdude
    canisp.program.extra_params=-P{serial.port} -b{program.speed}

## Upload CannyFlash
Prepare another Arduino Leonardo (UNO also works but it will fail while verifying the upload) with MCP2515. Upload [CANISP_Programmer](CANISP_Programmer/CANISP_Programmer/CANISP_Programmer.ino), we will now call this device CannyFlash Programmer

Connect Target Device with CannyFlash Programmer via CAN

Connect CannyFlash Programmer Pin 3 to ground, this will put programmer in "Magic Packet" mode.

Poweron or reset target device. If programmer received magic packet from target device, password will be sent. If password is accepted, the Target device will enter "program mode". 

You can now Flash with "Upload with Programmer" option in Arduino IDE. 