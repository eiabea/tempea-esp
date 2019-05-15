# Tempea slave implementation on an ESP8266

## Flash ESP8266 with Arduino (NONOS)

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

# Contribute

Every help is appreciated, take a look at the [Contributing file](https://github.com/eiabea/tempea-esp/tree/master/CONTRIBUTION.md).