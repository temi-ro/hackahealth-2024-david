# hackahealth-2024-david

## About David
David is 26 years old and from St. Gallen. He likes going to concerts, for dinner, or drinks. David has participated in the very first Zurich Health Hackathon and is back with many ideas. For this edition, David wants to work on his independence in restaurants.

## Challenge
David has been in a wheelchair since birth and lost his eyesight as a teenager. When he goes to a restaurant and wants to be served, he never knows if a waiter is close to him. So he often misses opportunities to order. He wants a device that allows him to call the waiter's attention at the right time independently. The goal was to obtain a solution that allows him to be treated the same as everybody else. See [here](https://www.hackahealth.ch/project-david-2024) for more information.

## Solution
We created an interactive cube with embedded electronics, including an ESP32, that functions as an access point and hosts a web server. Users (e.g., David) can connect to it and interact with the cube through two buttons:
- **Request Service**
- **Cancel**

When the "Request Service" button is pressed, the cube emits vibrations, beeps, and sounds to alert a nearby waiter. The user can either cancel their request, or the waiter can acknowledge it by pressing the "Snooze" button on the cube. This action sends an alert to the user’s phone.

Once the waiter has acknowledged by pressing "Snooze" (state changes to "COMING"), the user can confirm the notification by pressing either button on their device.

## Code Overview
This repository contains the code for the ESP32-based web server and the communication protocol between the ESP32 and the user’s mobile device. Key components include:
- **Web Server**: The ESP32 hosts a web server that provides the user interface accessible via any mobile device connected to the cube’s Wi-Fi network.
- **Button Actions**: Code to handle "Request Service" and "Cancel" button presses, triggering various actions (e.g., activating alerts on the cube and sending updates to the connected device).
- **Communication Protocol**: Real-time updates are sent between the ESP32 and the user’s mobile device using WebSockets, allowing the user to receive immediate feedback when the waiter acknowledges the service request.

This setup enables interaction between the ESP32 cube and the user’s phone, creating a communication link with the waiter.

## How to use
1. Turn on your smartphone hotspot (or whatever network you want the device to connect to).
2. Turn on the device 
3. Go to the website of the device (e.g., 192.168.43.184). The IP can be configured or read out using the serial monitor as indicated below. 

## Network configuration
1. Use a serial communication tool (e.g., [HTerm](https://www.der-hammer.info/pages/terminal.html) or the Arduino Serial Monitor) and connect to the microcontroller. The baud rate is set to 115200.
2. The following commands are available:
    - `help`: Display the list of available commands.
    - `info`: Display the current configuration.
    - `set_network <network_ssid>`: Define the SSID of the network to which the device should connect. This will prompt you to enter the password.
    - `set_ip <ip_address>`: Define a static IP address for the device.
3. The device will store the configuration in the EEPROM and use it to connect to the network when it boots up.


## Development
The project is best developed in VsCode with the PlatformIO extension. The code is based on the Arduino framework. If you want to upload the code, you need to perform two steps:

1. Build and upload the filesystem image ([see here](https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/)). This will upload the files in the */data* directory to the ESP32.
2. Build and flash the actual code.