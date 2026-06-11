# bike-rear-traffic-detector

## Background
The purpose of this project is to assist a member of the team’s mom, Paula Ould. Paula was suddenly diagnosed in 2022 with a benign but hemorrhaging brain tumor. The tumor has since been successfully removed, but the process has limited her eyesight and degraded her motor function, rendering her legally unable to drive.
Since the surgeries, she has purchased an e-trike from RadPower to safely travel short distances from her house. She has added a mirror to increase her field of vision and a flag for visibility, among other modifications. She is able to ride safely in low-traffic environments, but is still nervous about biking on busier roads or at busier times of day because it is harder for her to detect objects at the very edge of her peripherals or behind her.
Therefore, our team’s goal is to create a system that detects and communicates the presence of rear oncoming traffic to Paula while she is on the road. This system is intended to act as an additional layer of information to support her existing safety measures, helping improve awareness and confidence, rather than encouraging riding in higher-risk conditions.


## Introduction

Rear-facing bicycle safety and rider awareness system using mmWave radar, an STM32F411, and directional haptic feedback. Detects nearby vehicles in real time and alerts the rider through handlebar vibration motors using onboard radar and IMU sensor fusion.

# Major Hardware
## PCB overview

This PCB is a compact embedded control and power-distribution board for a rear object detection and rider alert system mounted on a bicycle. A 2-cell lithium-ion battery feeds the board through a resettable PTC fuse and reverse-polarity PMOS protection stage before entering a TPS542025 buck converter that generates a regulated 5 V rail for the rear mmWave radar module and the onboard 3.3 V regulator. An NCP1117 LDO then derives a low-noise 3.3 V rail used by the STM32F411 microcontroller, the BNO055 inertial measurement unit, and two DRV2605L haptic motor driver ICs. The STM32 serves as the central controller, receiving rear radar data over UART and motion/orientation data from the BNO055 over I2C, then processing that information to determine when and how to alert the rider. The DRV2605L drivers independently control left and right haptic motors mounted in the handlebars, allowing directional vibration cues to indicate approaching vehicles or hazards. The board also includes SWD debug/programming access, USB connectivity, multiple test points, local decoupling and bulk capacitors for power integrity, and locking JST-GH connectors for vibration-resistant off-board connections suitable for bicycle use.

# Major Software

## Radar System Development

The radar subsystem is built around the Texas Instruments IWR6843AOP mmWave radar sensor, which serves as the primary environmental sensing device for the project. The radar was selected because it can directly measure target motion using Doppler velocity information while remaining compact enough for integration into a wearable system. Unlike traditional proximity sensors, the radar can distinguish moving objects and provide velocity measurements, allowing the system to respond differently depending on how quickly a target is approaching.

## Hardware configuration

Before software development could begin, the IWR6843AOP radar evaluation module required several hardware configuration changes to enable communication with the custom STM32-based PCB. The UART routing and operating mode switches on the radar evaluation board were configured to allow direct communication between the radar and the microcontroller. This enabled the radar to operate independently of a host PC during normal operation while still allowing access to the radar's command-line interface for configuration and testing.

The switch positions used throughout development are shown below:
<div align="center">
    <img src="radar swithes.jpg" width="600">
    <br>
    <b>Figure 1:</b> IWR6843AOP Evaluation Module switch locations used during development.
</div>

 Table 1. Switch configuration
| Switch | Position |
|---------|----------|
| S1.1 | ON |
| S1.2 | ON |
| S1.3 | ON |
| S1.4 | OFF |
| S2.1 | OFF |
| S2.2 | ON |
| S2.3 | OFF |
| S2.4 | OFF |
| S3   | OFF |                        

These settings configured the radar module for UART communication with the STM32 and were required before software testing and radar configuration development could begin.

The radar communicates with the STM32 through two UART interfaces. A 115200 baud Command Line Interface (CLI) UART is used to transmit configuration commands to the radar, while a separate 921600 baud data UART is used to receive real-time object detection data. This dual-UART architecture separates radar configuration traffic from high-speed sensor data, ensuring reliable communication during operation. The CLI UART is used only during initialization and configuration, while the high-speed data UART continuously streams radar detection packets during normal operation.

## Configuration Development and Testing

A significant portion of the radar development process involved determining an appropriate configuration for the application. Texas Instruments provides a large number of configurable radar parameters that influence range resolution, velocity resolution, frame rate, transmit power, and target detection performance.

To evaluate these parameters, the TI mmWave Demo Visualizer software was used extensively during development. This software provides real-time displays of radar point clouds, detected objects, and Doppler velocity information, allowing different configurations to be tested and compared quickly. Multiple configurations were evaluated to determine which settings provided the most reliable detection of approaching targets while minimizing false detections and unnecessary processing overhead.

Particular attention was given to Doppler measurements because the project relies on detecting approaching motion rather than generating a detailed environmental map. By observing velocity data within the Demo Visualizer, threshold values and radar operating parameters were tuned to provide stable and repeatable motion classification for the wearable warning system.

Once a suitable configuration was identified, the resulting command sequence was incorporated directly into the STM32 firmware. During startup, the radar driver automatically transmits the complete configuration to the radar module, eliminating the need for a separate configuration utility during normal operation.

## Drivers

## Radar module information
One of the most important software components of the project is the custom radar driver developed for the IWR6843AOP sensor. The driver abstracts the complexity of radar communication and packet processing, providing the rest of the application with simple motion-detection states.

During initialization, the driver transmits the complete radar configuration over the CLI UART and prints the radar responses to the USB debugging interface. This allows configuration progress and communication status to be monitored during development and testing. Once configured, the driver continuously receives binary data packets from the radar over the high-speed data UART.

The incoming byte stream is searched for the Texas Instruments radar packet magic word, which marks the beginning of a valid radar frame. The driver then parses the packet header and iterates through the packet's Type-Length-Value (TLV) structures to locate detected-point data. Doppler velocity measurements are extracted from each detected point and analyzed to determine the maximum observed target velocity within the frame.

For this project, only negative Doppler values are considered because they correspond to objects moving toward the radar sensor. The driver identifies the fastest approaching object and classifies the result into three application-level states:

RADAR_NO_OBJECT – No approaching object detected above the minimum velocity threshold.
RADAR_YIPPEE_1 – An approaching object detected with a velocity between approximately 0.05 m/s and 3.129 m/s.
RADAR_YIPPEE_2 – An approaching object detected with a velocity greater than 3.129 m/s.

This approach greatly simplifies the rest of the software architecture by converting complex radar point
## Haptic Driver
The haptic driver provides a clean interface for controlling the two DRV2605 haptic motor drivers used in the wearable feedback system. The driver handles all low-level I2C communication and motor initialization, including configuring both DRV2605 devices for Real-Time Playback (RTP) mode. This allows vibration intensity to be controlled directly through software. Higher-level functions such as Haptic_PlayForMs(), Haptic_Off(), and Haptic_LongWarningPattern() allow the main application to generate feedback without needing to interact with device registers or communication protocols.

Different feedback patterns are created by varying vibration intensity, pulse duration, and timing. The Haptic_PlayForMs() function generates a continuous vibration for a specified duration, while Haptic_LongWarningPattern() creates a multi-pulse warning consisting of two short vibration bursts followed by a longer pulse. By encapsulating all hardware-specific details within the driver, the haptic subsystem remains modular, reusable, and easy to integrate with the radar and IMU sensing systems.

## IMU Driver
The IMU driver is written in C using STM32 HAL and communicates with a BNO055 sensor over I2C. The driver begins by defining the BNO055 I2C address and the internal register addresses needed to configure and read from the sensor. During IMU_Init(), the driver stores the I2C handle, waits for the BNO055 to boot, places the sensor into configuration mode, sets the power mode and unit selection, and then switches the sensor into AMG mode. AMG mode enables accelerometer, magnetometer, and gyroscope operation.

The main function in the driver is IMU_Update(). This function reads six bytes of linear acceleration data from the BNO055, corresponding to the X, Y, and Z acceleration axes. The raw sensor values are converted into acceleration values in meters per second squared by dividing by 100. These values are stored as the most recent acceleration readings so that other parts of the program can access them later.

After reading the acceleration data, the driver calculates the total acceleration magnitude using the X, Y, and Z components. This magnitude is compared to a motion threshold of 0.25 m/s². If the acceleration magnitude is above this threshold, the driver considers the device to be moving and updates last_motion_time. If the acceleration stays below the threshold for at least 15 seconds, the driver returns IMU_STOPPED.

## Main Loop and System Intelligence
The intelligence of the system is implemented through an event-driven main loop that combines information from both the radar and IMU subsystems. During each iteration, the radar driver processes incoming Doppler measurements and classifies nearby motion as either moderate motion, fast motion, or no detection. At the same time, the IMU driver evaluates the bike's motion state and determines whether the device has remained stable for an extended period.

The main application uses information from both sensors to select an appropriate haptic response. When a radar detection occurs while the IMU indicates normal movement, the system generates a continuous vibration warning. If the radar detects motion while the IMU reports that the device has remained stable for an extended period, a more noticeable multi-pulse warning pattern is generated. This allows the system to provide context-aware feedback rather than responding solely to radar detections.


## Overview of the main loop

The main control loop continuously receives processed radar data over the I²C interface and evaluates the motion of nearby objects relative to the user. The software extracts the Doppler velocity information from detected targets and determines whether an object is approaching. The magnitude of the approach speed is then mapped to a corresponding vibration intensity for the handlebar-mounted haptic motors. Faster approaching objects produce stronger vibrations, allowing the user to quickly assess the severity of a potential hazard without diverting attention from the road. This approach transforms raw radar measurements into intuitive feedback, enabling the system to communicate both the presence and urgency of nearby objects in real time. The continuous execution of this loop allows the device to adapt immediately to changing traffic conditions and provide responsive warnings to the user.
## Coding Style and Software Architecture
The project is implemented in embedded C using the STM32 HAL framework. The software follows a modular driver-based architecture in which each hardware subsystem—including the radar, IMU, haptic motors, and USB communication interface—is isolated within its own source and header files. This organization improves code readability, simplifies debugging, and allows individual components to be modified without affecting the rest of the system.

Although the project does not use object-oriented programming or a real-time operating system, it incorporates many object-oriented design principles such as abstraction and encapsulation. Hardware-specific details are hidden inside each driver, exposing only simple public interfaces to the application layer. The overall system operates using an event-driven polling loop, where sensor inputs are continuously evaluated and used to determine the appropriate system response. This lightweight architecture is well suited for real-time embedded applications while remaining easy to understand and maintain.

# Testing

##USB Debugging Interface

To simplify development and testing, the custom PCB includes a USB connection that enables direct communication between the STM32 microcontroller and a host computer. The project uses the STM32 USB CDC (Communications Device Class) interface to create a virtual serial port that can be accessed through terminal software such as PuTTY.

A custom debug_print() function was implemented to transmit diagnostic messages over USB. Throughout development, this interface was used to monitor radar detections, verify IMU operation, confirm successful peripheral initialization, and observe the system's decision-making logic in real time. This capability significantly reduced debugging time by providing immediate visibility into the internal state of the system without requiring additional hardware debugging tools.

The USB debugging interface was particularly valuable during integration testing, allowing rapid verification that the radar, IMU, and haptic subsystems were communicating correctly and producing the expected responses under different operating conditions.

# Lessons learned
radar required configuration so take time.
