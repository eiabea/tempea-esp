# Tempea slave implementation on an ESP8266

## Flash ESP8266 with Arduino (NONOS)

This How-To is based on [this](https://tttapa.github.io/Pages/Arduino/ESP8266/Flashing/Flashing-With-an-Arduino.html) great explanations by [tttapa](https://github.com/tttapa)

- Install [esptool](https://github.com/espressif/esptool)

- Get the latest NONOS firmware from [here](https://github.com/espressif/ESP8266_NONOS_SDK/releases/)

- Extract the firmware and navigate into the _bin_ directory of the extracted firmware
```
$ cd ~/Downloads/ESP8266_NONOS_SDK-3.0/bin
```

- Build up the schematic shown [here](https://github.com/eiabea/tempea-esp/tree/master/schematic/FlashEspWithArduino.fzz)

- Connect the Arduino via USB to your pc and check the port with
```
dmesg
```

- Note the port, it should look something like _/dev/ttyACM3_

- Put the ESP into program mode by following these steps:

  1. Press and hold the "Reset"-Button
  2. Press and hold the "Progam"-Button
  3. Release the "Reset"-Button
  4. Release the "Program"-Button

- Flash the latest firmware with the following command:
```
$ esptool -p /dev/ttyACM3 --chip esp8266 write_flash -fm dio 0x00000 boot_v1.7.bin 0x10000 at/1024+1024/user1.2048.new.5.bin 0xfc000 esp_init_data_default_v08.bin 0xfe000 blank.bin
```

# C++ Version
## Installation for Arduino IDE (C++ Version)
- Start Arduino and open Preferences window.
- Enter `https://arduino.esp8266.com/stable/package_esp8266com_index.json` into *Additional Board Manager URLs* field. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and find *esp8266* platform.
- Select the version you need from a drop-down box.
- Click *install* button.
- Don’t forget to select your ESP8266 board from Tools > Board menu after installation.

## Installation of the PubSubClient for Arduino IDE (C++ Version)
- Open Manage Libraries... from Tools and find *PubSubClient* by Nick O' Leary.
- Select the version you need from a drop-down box.
- Click *install* button.

## Installation of the OneWire for Arduino IDE (C++ Version)
- Open Manage Libraries... from Tools and find *OneWire* by Jim Studt.
- Select the version you need from a drop-down box.
- Click *install* button.

## Dependencies
- *board* esp8266 (v2.3.0)
- *lib* PubSubClient (v2.6.0)
- *lib* OneWire (v2.3.4)

# Contribute

Every help is appreciated, take a look at the [Contributing file](https://github.com/eiabea/tempea-esp/tree/master/CONTRIBUTION.md).