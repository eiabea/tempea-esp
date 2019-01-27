import sys
import time
import machine
import network
import onewire, ds18x20
from machine import Pin, RTC
from network import WLAN
from umqtt.robust import MQTTClient

SSID = "AwesomeWifiName"
PASSWORD = "password"
LED_PIN = 2
DS18B20_DATA_PIN = 5
MQTT_HOST = "192.168.0.8"
MQTT_PORT = 1883
MQTT_CLIENT_ID = "ESP8266"
MQTT_TOPIC = "esp_temp"
SLEEP_SEC = 60
BLINK_ON = 300
BLINK_OFF = 100

def blink(times):
    led = Pin(LED_PIN, Pin.OUT)
    for x in range(0, times):
        led.off()
        time.sleep_ms(BLINK_ON)
        led.on()
        time.sleep_ms(BLINK_OFF)

def do_connect():
    interface = WLAN(network.STA_IF)

    # Stage one: check for default connection
    for t in range(0, 30):
        if interface.isconnected():
            print('Connected to', SSID)
            return interface
        print("Trying to connect to wifi", t)
        if t == 0:
            interface.active(True)
            interface.connect(SSID, PASSWORD)

        time.sleep_ms(1000)

    raise Exception("Unable to connect to ", SSID)

def do_connect_mqtt():
    c = MQTTClient(MQTT_CLIENT_ID, MQTT_HOST, MQTT_PORT)
    c.connect()
    return c

def measure_temp():
    ow = onewire.OneWire(Pin(DS18B20_DATA_PIN))
    ds = ds18x20.DS18X20(ow)
    roms = ds.scan()
    ds.convert_temp()
    time.sleep_ms(750)
    return ds.read_temp(roms[0])
   
def publish_mqtt(client, temp):
    # 85 is the error value when the sensor is not detected
    if(temp != 85):
        client.publish(MQTT_TOPIC, str(temp))
    time.sleep(2)

def esp_sleep(seconds):
    print("Going to sleep for " + str(seconds) + "seconds")
    rtc = RTC()
    rtc.irq(trigger=rtc.ALARM0, wake=machine.DEEPSLEEP)
    time.sleep(2)
    rtc.alarm(rtc.ALARM0,seconds*1000)
    machine.deepsleep()

try:
    do_connect()
    blink(1)

    temp = measure_temp()
    print("Temp: " + str(temp))

    try:
        mqtt = do_connect_mqtt()
        publish_mqtt(mqtt, temp)
        blink(2)
    except:
        print("Unable to publish data to mqtt")
        pass
except Exception as err:
    print(err)
    pass

esp_sleep(SLEEP_SEC)
