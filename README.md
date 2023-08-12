
![Photo of board](https://i.imgur.com/III7pWA.jpg)
# Description
This repository holds the code for the LU Player Tracker board. You can find the PCB repository [here](https://github.com/HailStorm32/LU-Player-Tracker-PCB). 

This board shows the player count and locations in real time by lighting up the respective world RGB LED and seven segment display. The player location and count is sent via MQTT from a Darkflame Universe private server. 

> Darkflame Universe is a server emulator for the defunct LEGO Universe MMO. You can find more info about the project [here](https://github.com/DarkflameUniverse/DarkflameServer)


# Setup
In order to flash the board and get a working product, there are a few steps that need to be taken.
<br>

### Environment setup
There are multiple ways to setup the development environment. I would recommend reading [Espressif's documentation](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/get-started/index.html#manual-installation) 

However, the quickest and most straightforward way is to use the [ESP-IDF Tools Installer](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/get-started/windows-setup.html#esp-idf-tools-installer) (For Windows). 

<br>

### Config

 1. Navigate to the code directory and create a file called `credentials.h`
 2. Open the file and add the following
 
```
// MQTT
#define  BROKER_ADDR  "mqtt://<brokerIP>"
#define  BROKER_UNAME  "<brokerUname>"
#define  BROKER_PASS  "<brokerPass>"

// WIFI
#define  WIFI_SSID  "<WIFISSID>"
#define  WIFI_PASS  "<WIFIPass>"
```
Example
```
// MQTT
#define  BROKER_ADDR  "mqtt://192.168.0.0"
#define  BROKER_UNAME  "UserName"
#define  BROKER_PASS  "password123"

// WIFI
#define  WIFI_SSID  "wifissid"
#define  WIFI_PASS  "wifiPassword123"
```
3. Save and close the file

<br>

### Flashing

 1. Open the ESP-IDF 5.0 CMD (if you used the installer) and navigate to the code directory.
 2. In the code directory, run `idf.py set-target esp32s3`
 3. Then run `idf.py build`
 4. Once that has finished building, find connect the board to the PC via the USB connection and note the COM port
 

> NOTE: The USB does **not** supply power. You will also need to connect the power cable to the board to flash.
5. Then run `idf.py -p COM# flash` where `#` is the COM port number
6. If nothing failed, your board is now flashed and you can disconnect the USB cable

<br>

### Server Side
You will need the following setup:

 - Functional [Darkflame Server](https://github.com/DarkflameUniverse/DarkflameServer) *(running on Linux)*
 - MQTT Broker
 - Tracker script

The last two items are covered below.

 <br>
  
#### Installing MQTT broker

1. run `sudo apt install -y mosquitto mosquitto-clients`  
  
2. run `sudo systemctl enable --now mosquitto.service`  
  

#### Configuring Broker 

1. Open the broker config file using  
`sudo nano /etc/mosquitto/mosquitto.conf`  
  
2. Add the following line to the **top** of the file  
`per_listener_settings true`  
  
3. Add the following lines at the **bottom** of the file  

```  
allow_anonymous false  
listener 1883  
password_file /etc/mosquitto/passwd  
```  
  

4. Set a username and password for the broker by running 
`sudo mosquitto_passwd -c /etc/mosquitto/passwd YOUR_USERNAME` 
  
5.  Restart the broker with `sudo systemctl restart mosquitto`  
  
6. Make sure its running with `systemctl status mosquitto`  
  

#### Install The Script

1. Clone the [this](https://github.com/HailStorm32/hailstorms-darkflame-server-scripts) repository and follow [these](https://github.com/HailStorm32/hailstorms-darkflame-server-scripts#playercntdisplaypy) instructions
  
  

#### Last Step

1. Allow tcp port `1883` through the firewall  


<br>

# Use

#### Power Supply
The board is powered via the 5.5mmx2.1mm barrel jack on the left side of the board. It requires a [5V 2A supply](https://www.amazon.com/gp/product/B0B6PQV88X).

> The USB does **NOT** supply power, and is only for flashing and serial in/out



#### Connecting to WiFi
> *Currently only available on the `wifiConfigAP` branch*

The board will allow you to enter your WiFi credentials via a website broadcasted by the board.

 1. Power on the board
 2. Then, while holding down the MODE button, briefly press the RESET button. 
	 > *Its easier to press the reset button with a small flat object like a LEGO 1x4 tile*

	 ![enter image description here](https://i.imgur.com/kQesXeW.png)
 3. Keep holding the MODE button until you see two solid dashed lines in the seven segment
	 ![enter image description here](https://i.imgur.com/4qJ6HoI.png)
 4. Your board should now be broadcasting a hotspot with the name `LU Player Tracker`
 5. Connect to the hotspot with the password `LUisthebest`
 6. Navigate to `192.168.4.1`
 7. Enter your WiFi's SSID and password and click save
 8. You should get message saying your credentials are saved 
 9. Power cycle the board by either briefly pressing reset, or unplugging/plugging in the barrel jack
 10. A couple seconds after reboot, you should see the seven segment change to showing numbers and the world LEDs turning on (if they are populated) 

---

Credit to `FutronBob` for the map layout that inspired this project
