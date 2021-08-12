# CannyFlash
Bootloader that allows flashing Arduino device via onboard CAN Interface (MCP2515)

## Current status
Compiled Bootloader stored in [CannyFlash/Bootloader_bin](CannyFlash/Bootloader_bin)

Only Atmega32u4 (Arduino Leonardo) (tested) 
Atmega328P (UNO) - developing

## Arduino IDE preparation

### Copy Bootloader directory
under {ArduinoDirectory}/hardware/avr/1.8.3/bootloaders
create folder cannyflash, put file "cannyflash_v1_leonardo.hex" in the folder

### Add custom board

Add the following to the end of "boards.txt" file (under {ArduinoDirectory}/hardware/avr/1.8.3)

    ## CannyFlash Bootloader
    canny.name=CannyFlash v1 (Arduino)
    canny.build.board=AVR_UNO

    canny.upload.tool=avrdude
    canny.upload.protocol=arduino
    canny.bootloader.tool=avrdude

    ## CannyFlash Bootloader / 32u4
    ## ----------------------------------------------
    canny.menu.cpu.atmega32u4=ATmega32u4 (Leonardo)
    canny.menu.cpu.atmega32u4.upload.maximum_size=28672
    canny.menu.cpu.atmega32u4.upload.maximum_data_size=2560
    canny.menu.cpu.atmega32u4.upload.speed=19200

    canny.menu.cpu.atmega32u4.bootloader.low_fuses=0xff
    canny.menu.cpu.atmega32u4.bootloader.high_fuses=0xd8
    canny.menu.cpu.atmega32u4.bootloader.extended_fuses=0xcb
    canny.menu.cpu.atmega32u4.bootloader.file=cannyflash/cannyflash_v1_leonardo.hex
    canny.menu.cpu.atmega32u4.bootloader.unlock_bits=0x3F
    canny.menu.cpu.atmega32u4.bootloader.lock_bits=0x2F

    canny.menu.cpu.atmega32u4.build.mcu=atmega32u4
    canny.menu.cpu.atmega32u4.build.f_cpu=16000000L
    canny.menu.cpu.atmega32u4.build.vid=0x2341
    canny.menu.cpu.atmega32u4.build.pid=0x8036
    canny.menu.cpu.atmega32u4.build.usb_product="Arduino Leonardo"
    canny.menu.cpu.atmega32u4.build.board=AVR_LEONARDO
    canny.menu.cpu.atmega32u4.build.core=arduino
    canny.menu.cpu.atmega32u4.build.variant=leonardo
    canny.menu.cpu.atmega32u4.build.extra_flags={build.usb_flags}

Currently only supports Leonardo (32u4)

### Add Programmer (for uploading custom firmware)

Add the following to the end of "programmers.txt" file (under {ArduinoDirectory}/hardware/avr/1.8.3)

    canisp.name=CannyFlash Programmer
    canisp.communication=serial
    canisp.protocol=arduino
    canisp.speed=19200
    canisp.program.protocol=arduino
    canisp.program.speed=19200
    canisp.program.tool=avrdude
    canisp.program.extra_params=-P{serial.port} -b{program.speed}

## Flash CannyFlash Bootloader

To Flash bootloader, you need another Arduino UNO and upload Arduino ISP (from example)
Connection please refer to [Arduino ISP](https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP)

Once everything is done, back to Arduino IDE, tools -> Programmer choose CannyFlash Programmer
Don't forget the port should choose the UNO that contains Arduino ISP
then tools -> Burn Bootloader

## Upload CannyFlash
Prepare another Arduino Leonardo (UNO also works but it will fail while verifying the upload) with MCP2515. Upload [CANISP_Programmer](CANISP_Programmer/CANISP_Programmer/CANISP_Programmer.ino), we will now call this device CannyFlash Programmer

Connect Target Device with CannyFlash Programmer via CAN

In Arduino IDE, change the port to the CannyFlash Programmer (i.e. not the one you flash firmware).

CannyFlash Programmer should be fast blinking it's LED, meaning that it is waiting for the Magic Packet from CAN bus.

Connect target to the CAN bus, poweron or reset target device. If programmer received magic packet from target device, password will be sent. If password is accepted, the Target device will enter "program mode" and stops blinking. 

In Arduino IDE, Tools > Board > CannyFlash V1 Arduino, choose the right processor, and the COM port is the CannyFlash Programmer. 

You can hit Upload and it should work like usual Arduino variants!

### Remember

Every time before upload, you need to reset both CannyFlash Programmer and target device. 