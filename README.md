# hackahealth-2024-david

## About David
David is 26 years old and from St. Gallen. He likes going to concerts, for dinner, or drinks. David has participated in the very first Zurich Health Hackathon and is back with many ideas. For this edition, David wants to work on his independence in restaurants.

## Challenge
David has been in a wheelchair since birth and lost his eyesight as a teenager. When he goes to a restaurant and wants to be served, he never knows if a waiter is close to him. So he often misses opportunities to order. He wants a device that allows him to call the waiter's attention at the right time independently.

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
