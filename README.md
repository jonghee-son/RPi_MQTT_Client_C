# RPi_MQTT_Client_C
MQTT Client for Raspberry Pi written in C

# How to use
1. Clone this repository & install dependency
```bash
$ wget https://github.com/eclipse/paho.mqtt.c/releases/download/v1.3.10/Eclipse-Paho-MQTT-C-1.3.10-Linux.tar.gz
$ tar -xvzf Eclipse-Paho-MQTT-C-1.3.10-Linux.tar.gz
$ cd Eclipse-Paho-MQTT-C-1.3.10-Linux
$ make
$ sudo make install
$ cd ~
$ git clone https://github.com/jonghee-son/RPi_MQTT_Client_C.git
```

2. Edit the code to fit your environment
```
#define ADDRESS "tcp://172.30.1.50:1883" //Broker IP Address
#define ADDRESS_SECURE "tcp://172.30.1.50:8883" //Broker IP Address but with TLS encryption
#define CLIENTID "CLIENT_SUB" //Client Name
#define TOPIC_1 "lab/light" //Topic Configuration (Light)
#define TOPIC_2 "light/stat" //Topic for current status of light
#define PAYLOAD_1 "Switch is ON" //Defining message for status message publication
#define PAYLOAD_2 "Switch is OFF"
```
Edit these pre-defined parameters.

3. compile editted code with preferred c compiler (gcc, clang etc.)
```bash
$ gcc -o client src.c -lwiringPi -lpaho-mqtt3c
$ ./client
```
