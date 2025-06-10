# Smart Entry Management System

## Project Description
This project is a **smart entry management system** using the **ESP32-S3 Xiao microcontroller** and **RFID technology**. It enables access control through a **mobile app** and **real-time monitoring** using Firebase.

## Features
- **Remote Door Control:** Admin can unlock and lock the door via **Blynk app**
- **User Management:** Admin can register, update, and delete users using RFID
- **Unauthorized Access Detection:** System detects invalid RFID scans; after **three invalid attempts**, it takes a photo, stores it in **Firebase**, and sends an alert via email
- **Access Logs:** All attempts (authorized and unauthorized) are logged in **Firebase** and viewable via **Blynk app**

## Components Used
- **ESP32-S3 Xiao Microcontroller**
- **RFID RC522 Scanner Module**
- **Solenoid Lock** (Controlled via relay module)
- **20V DC Power Adapter**
- **Blynk App** for UI and Remote Control
- **Firebase Realtime Database** (for data storage)
- **Firebase Storage** (for storing images)
- **Camera Module** for unauthorized access detection

---

## Problem Statement
Traditional entry systems lack **real-time monitoring** and **remote control**. This project implements a **secure, IoT-enabled system** with:
- **Remote access**
- **User management**
- **Alert notifications for unauthorized access**

---

## First-Time Setup Guide
### 1. Prerequisites
Ensure the following are **already set up**:
- **Hardware is fully integrated** and powered
- **Blynk App & Firebase accounts are created**
- **GitHub repository containing the firmware code**
  
### 2. Download Project Code
Clone the GitHub repository and download the firmware code: [https://github.com/kumars21/IoT_System/tree/main/Smart%20Entry%20System]


Refer to the project report for additional project documentation.

### 3. Install Required Software
Install **Arduino IDE** and required libraries:
- ESP32 board support
- RFID RC522 library
- Firebase & Blynk libraries

### 4. Configure Firebase
If Firebase is **not yet set up**, refer to the following guide:
- **Firebase Setup Guide:** [https://firebase.google.com]

### 5. Configure Blynk App
If you **haven’t created a Blynk project**, follow this guide:
- **Blynk Setup Guide:** [https://blynk.io/]

### 6. Upload Firmware to ESP32
1. **Connect ESP32-S3 Xiao to your computer**
2. **Open Arduino IDE**
3. **Load the firmware code (`.ino` file)**
4. **Enter your Firebase & Blynk credentials**
5. **Upload the code to the microcontroller**
6. **Verify that the system initializes correctly**

### 7. Using the System
- **Admin controls the door remotely via Blynk app**
- **User management via Blynk interface**
- **Unauthorized RFID attempts trigger image capture & email alerts**
- **Access logs stored in Firebase and viewable via Blynk**

---

## Future Improvements
- **Facial Recognition Integration** for better security

---

This project provides **a secure, automated entry management system** ideal for home, office, or restricted-access areas. Feel free to contribute improvements via the GitHub repository!

