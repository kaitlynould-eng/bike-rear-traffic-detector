# bike-rear-traffic-detector

## Background

The purpose of this project is to assist a member of the team’s mom, Paula Ould. Paula was suddenly diagnosed in 2022 with a benign but hemorrhaging brain tumor. The tumor has since been successfully removed, but the process has limited her eyesight and degraded her motor function, rendering her legally unable to drive.
Since the surgeries, she has purchased an e-trike from RadPower to safely travel short distances from her house. She has added a mirror to increase her field of vision and a flag for visibility, among other modifications. She is able to ride safely in low-traffic environments, but is still nervous about biking on busier roads or at busier times of day because it is harder for her to detect objects at the very edge of her peripherals or behind her.
Therefore, our team’s goal is to create a system that detects and communicates the presence of rear oncoming traffic to Paula while she is on the road. This system is intended to act as an additional layer of information to support her existing safety measures, helping improve awareness and confidence, rather than encouraging riding in higher-risk conditions.


## Introduction

Rear-facing bicycle safety and rider awareness system using mmWave radar, an STM32F411, and directional haptic feedback. Detects nearby vehicles in real time and alerts the rider through handlebar vibration motors using onboard radar and IMU sensor fusion.

# Hardware Design

## Overview

The hardware architecture of the rider-awareness system is centered around a custom printed circuit board that integrates sensing, processing, power regulation, and haptic feedback subsystems into a compact assembly suitable for bicycle use. The system is powered by a 2-cell lithium-ion battery pack and is designed to operate reliably in a vibration-prone outdoor environment through the use of connectorized wiring harnesses, regulated power distribution, and mechanically secured components.

A rear-facing TI IWR6843 mmWave radar module serves as the primary sensing device for detecting approaching vehicles and other rear hazards. An onboard BNO055 inertial measurement unit (IMU) provides orientation and motion data that allow the system to distinguish between riding and stationary conditions. Sensor data is processed by an STM32F411 microcontroller, which coordinates system operation and controls the rider notification subsystem.

Rider alerts are communicated through two handlebar-mounted vibration motors driven by dedicated DRV2605L haptic motor driver ICs. This haptic feedback approach was selected to provide intuitive notifications without requiring the rider to divert visual attention from the road or rely on audible alerts that may be masked by environmental noise.

All electronic components are housed within a custom 3D-printed enclosure mounted to the rear cargo rack of a substitute bicycle. Paula’s e-trike was unavailable during testing, and so the prototype enclosure will be updated in the future to match the geometry of the intended vehicle. The enclosure incorporates dedicated mounting features and attachment points to securely retain the PCB, battery, radar module, and associated wiring. During prototype testing, the enclosure was secured to a substitute bicycle using zip ties attached to a rear basket rack, providing a simple and robust mounting solution without requiring permanent modifications to the bicycle. Internal cable management and connectorized harnesses were used to minimize strain on electrical connections and improve serviceability.

The hardware is organized into six primary subsystems: power distribution, radar sensing, inertial measurement, haptic feedback, programming and debug interfaces, and wiring and harnessing. The following sections describe the design considerations and implementation details of each subsystem.

## Power Distribution

The power distribution system is responsible for converting and regulating battery power for all sensing, processing, and feedback electronics within the rider-awareness system. Power is supplied by a 2-cell lithium-ion battery pack with a nominal voltage of 7.4 V. Battery power enters the PCB through a connectorized interface and first passes through a 2 A resettable PTC fuse to provide overcurrent protection. A P-channel MOSFET configured for reverse-polarity protection prevents damage to the electronics if the battery is connected incorrectly.

Following the protection circuitry, battery voltage is regulated by a TPS542025 switching buck converter configured to generate a 5 V power rail. A switching regulator was selected instead of a linear regulator to improve efficiency and reduce heat dissipation, which is especially important for a battery-powered system. The 5 V rail directly powers the TI IWR6843 mmWave radar module and serves as the input to the secondary voltage regulation stage.

A NCP1117 low-dropout linear regulator generates a regulated 3.3 V rail from the 5 V supply. This rail powers all low-voltage digital electronics, including the STM32F411 microcontroller, BNO055 inertial measurement unit, and both DRV2605L haptic motor driver ICs. The use of a dedicated LDO provides a clean and stable supply for the digital and sensing subsystems while maintaining compatibility with the operating voltage requirements of these devices.

To maintain power integrity throughout the system, bulk capacitance is placed at the outputs of both regulators, while local decoupling capacitors are located near integrated circuits and connector interfaces. These capacitors help suppress voltage transients, reduce switching noise, and ensure stable operation during dynamic load conditions such as radar operation and vibration motor actuation.

Current consumption estimates indicate a steady-state 3.3 V load of approximately 202 mA, increasing to approximately 282 mA during vibration motor startup. The selected power architecture provides sufficient current capacity for all system components while maintaining efficient operation and protecting the electronics from common power-related faults.

## Radar Sensor

Rear traffic detection is performed using a Texas Instruments IWR6843 mmWave radar module mounted at the rear of the bicycle enclosure. The radar operates in the 60 GHz frequency band and was selected for its ability to reliably detect and track moving objects under a wide range of environmental conditions. Unlike optical sensors or cameras, mmWave radar performance is largely unaffected by lighting conditions, shadows, glare, or moderate weather effects, making it well suited for bicycle safety applications.

The radar module is powered from the regulated 5 V rail generated by the onboard power distribution system. Communication between the radar and the STM32F411 microcontroller is accomplished through dedicated UART interfaces. One UART connection is used for radar configuration and initialization, while a second UART interface is used to transmit processed target detection data to the microcontroller. Additional reset and GPIO signals are routed through the radar connector to support system control and status monitoring. The radar is connected to the PCB through a locking JST-GH harness to improve reliability in the vibration-prone bicycle environment.

The radar is mounted within the rear enclosure and oriented to monitor traffic approaching from behind the rider. By continuously measuring target range, velocity, and relative motion, the sensor provides the primary situational-awareness input for the system. These measurements enable the rider-awareness system to identify approaching vehicles and generate appropriate haptic notifications through the handlebar vibration motors.

The IWR6843 was selected because it combines a compact form factor with integrated signal processing capabilities and sufficient detection range for bicycle applications. Its ability to provide direct velocity measurements through Doppler processing is particularly valuable for distinguishing approaching vehicles from stationary roadside objects, reducing false detections and improving the relevance of rider alerts.

## Inertial Measurement Unit

System motion and orientation are measured using a Bosch BNO055 inertial measurement unit (IMU). The BNO055 was selected because it integrates a 3-axis accelerometer, 3-axis gyroscope, and 3-axis magnetometer into a single package while also providing onboard sensor fusion. This reduces computational requirements on the STM32F411 microcontroller and simplifies the implementation of orientation and motion tracking.

The IMU operates from the regulated 3.3 V power rail and communicates with the STM32F411 over an I²C interface. In addition to power, ground, and communication signals, a dedicated reset line is routed between the microcontroller and IMU to allow software-controlled reinitialization if necessary. The sensor is connected to the PCB through a locking JST-GH connector and wiring harness to ensure reliable operation in the vibration-prone bicycle environment.

The primary purpose of the IMU is to determine whether the bicycle is in motion and to provide orientation information that can be used to improve system awareness. This information allows the system to differentiate between riding and stationary conditions, enabling different notification behaviors depending on the rider’s state. For example, approaching traffic while the bicycle is moving generates continuous haptic feedback, whereas approaching traffic while stopped generates a brief notification alert.

The BNO055 was selected due to its integrated sensor-fusion capabilities, ease of implementation, and proven reliability in embedded motion-sensing applications. By providing stable orientation and acceleration data, the IMU serves as an important secondary sensing subsystem that complements the radar-based traffic detection system and improves the overall contextual awareness of the device.

## Haptic Feedback System

Rider notifications are communicated through a dual-channel haptic feedback system consisting of two vibration motors mounted within the bicycle handlebars. Haptic feedback was selected as the primary alert method because it provides immediate and intuitive notifications without requiring the rider to divert visual attention from the road or rely on audible signals that may be masked by traffic noise or environmental conditions.

Each vibration motor is controlled by a dedicated Texas Instruments DRV2605L haptic motor driver IC. The DRV2605L devices operate from the regulated 3.3 V power rail and communicate with the STM32F411 microcontroller over an I²C interface. Using dedicated driver ICs eliminates the need for the microcontroller to directly drive the motors and provides consistent vibration performance while reducing electrical stress on the control electronics. The motors are connected to the PCB through locking JST-GH connectors and dedicated wiring harnesses designed to withstand the vibration and movement encountered during bicycle operation.

The haptic feedback subsystem converts processed radar and motion-sensing information into tactile alerts that can be easily interpreted by the rider. When the bicycle is moving and rear traffic is detected, the vibration motors generate alerts whose intensity varies according to the relative speed of the approaching object. This allows the rider to receive information about nearby traffic without the need to monitor a display or listen for audio cues. When the bicycle is stationary, the system instead provides a brief notification vibration to indicate the presence of approaching rear traffic.

The vibration motors are housed within dedicated 3D-printed handlebar mounts positioned near the rider's hands to maximize perceptibility and ensure alerts can be detected even in noisy outdoor environments. By providing a direct tactile communication method, the haptic feedback subsystem serves as the primary interface between the rider-awareness system and the user.

## Communications and Debug Interfaces

The rider-awareness system incorporates dedicated communication and debugging interfaces to support firmware development, testing, and troubleshooting. These interfaces provide access to the STM32F411 microcontroller for programming, debugging, and system monitoring throughout development.

A Serial Wire Debug (SWD) header is included on the PCB to allow programming and real-time debugging of the STM32F411 microcontroller using an ST-Link debugger. The interface exposes the SWDIO, SWCLK, NRST, 3.3 V, and ground signals required for firmware upload, breakpoint debugging, memory inspection, and device recovery. This interface was used throughout development to validate sensor communication, test control algorithms, and diagnose hardware issues.

In addition to the SWD interface, the PCB includes a USB Mini-B connector that provides a direct connection between the microcontroller and a host computer. The USB interface enables serial communication through a virtual COM port, allowing sensor data, diagnostic information, and system status messages to be monitored during testing and development. This connection simplifies system validation by providing a convenient method for observing device behavior and recording system outputs during operation.

Together, the SWD and USB interfaces provide the primary tools used for firmware deployment, debugging, data collection, and system verification. Including both interfaces improved development efficiency and enabled thorough testing of the sensing, processing, and haptic feedback subsystems throughout the project lifecycle.

## Wiring and Harnessing

The rider-awareness system utilizes a connectorized wiring architecture to distribute power and communication signals between the PCB and external subsystems. Because the system is intended for operation on a bicycle, wiring reliability and resistance to vibration were important design considerations throughout the development process. Connectorized harnesses simplify assembly, maintenance, and troubleshooting while reducing the likelihood of intermittent electrical connections caused by cable movement or vibration.

Power is delivered from the battery pack to the PCB through dedicated high-current wiring sized to support the system load with minimal voltage drop. Lower-current sensor and communication connections use smaller-gauge wiring to reduce overall cable bulk and improve routing flexibility within the enclosure and along the bicycle frame. Wire gauges and connector selections were chosen based on expected current requirements, mechanical robustness, and ease of assembly.
The design incorporates several connector families, including XT30, JST-XH, JST-GH, and standard header interfaces. Locking JST-GH connectors were selected for sensor and motor connections because they provide secure retention and are well suited for environments subject to vibration and movement. These connectors also allow subsystems to be disconnected individually for testing, replacement, or maintenance without requiring modification to the wiring harness.

[Insert Figure X: System Wiring Diagram]

The wiring architecture is divided into eight primary harnesses that connect the battery, power switch, radar module, inertial measurement unit, vibration motors, USB interface, and debugging hardware to the main control board. Each harness is documented with connector types, wire gauges, and signal assignments to support manufacturing, assembly, and future maintenance activities.

[Insert Figure X: Harness Routing Overview]

Careful attention was given to harness routing within the enclosure and along the bicycle structure to minimize cable strain, prevent interference with moving components, and reduce excess wire length. Connector placement on the PCB was coordinated with enclosure layout to simplify assembly and improve serviceability. During prototype testing, harnesses were secured using cable management features and routing paths designed to prevent entanglement while maintaining access to critical system components.

[Insert Figure X: Individual Harness Diagrams (H1–H8)]

Detailed harness documentation, including signal definitions, connector pinouts, wire gauges, and routing information for each cable assembly, is provided in the subsequent figures and tables.

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

## USB Debugging Interface

To simplify development and testing, the custom PCB includes a USB connection that enables direct communication between the STM32 microcontroller and a host computer. The project uses the STM32 USB CDC (Communications Device Class) interface to create a virtual serial port that can be accessed through terminal software such as PuTTY.

A custom debug_print() function was implemented to transmit diagnostic messages over USB. Throughout development, this interface was used to monitor radar detections, verify IMU operation, confirm successful peripheral initialization, and observe the system's decision-making logic in real time. This capability significantly reduced debugging time by providing immediate visibility into the internal state of the system without requiring additional hardware debugging tools.

The USB debugging interface was particularly valuable during integration testing, allowing rapid verification that the radar, IMU, and haptic subsystems were communicating correctly and producing the expected responses under different operating conditions.

## Bike Testing
The testing process began with the bicycle stationary to verify that the radar module could reliably detect approaching objects and trigger the appropriate haptic warnings. During these initial tests, both YIPPEE 1 and YIPPEE 2 activated as expected, demonstrating that the radar and warning system were functioning correctly. However, when testing was expanded to include a moving bicycle, the radar began detecting environmental objects such as bushes, resulting in unwanted activations. To address this issue, the detection range was modified to only consider objects between 1 m and 8.92 m from the bicycle. This adjustment significantly improved performance by filtering out very close detections while still providing adequate warning distance for approaching hazards. Additionally, a minimum velocity threshold of 0.1 m/s was implemented to reduce false detections caused by moving bushes or tree branches in windy conditions. Based on feedback from Paula regarding the level of warning she desired under different situations, the velocity ranges for the two warning modes were refined. YIPPEE 1 was configured to activate for approaching objects with velocities between 0.1 m/s and 2.2 m/s, representing lower-speed hazards such as pedestrians walking toward the cyclist. YIPPEE 2 was configured to activate for velocities greater than 2.2 m/s, representing faster-moving hazards that require increased awareness. The design goal was to detect approaching hazards at distances up to 8.92 m. During validation testing, objects approaching directly behind the bicycle were detected at nearly the full 8.92 m range. However, when objects approached from the left or right side of the radar field of view, detection typically occurred at approximately 6 m, which is consistent with the reduced sensitivity at the edges of the radar's detection zone. The haptic feedback system was also evaluated to verify proper integration with the IMU. When the bicycle was stationary, the system correctly activated the patterned warning vibration, indicating that the IMU detected a stable bicycle state. When the bicycle was in motion, the system successfully switched to the continuous long-buzz warning mode, demonstrating that the IMU correctly identified bicycle movement and altered the haptic feedback accordingly. This behavior confirms that the IMU-based state detection functioned as intended and provided context-appropriate warnings to the rider. As shown in Table X, YIPPEE 1 consistently activated when a person approached while walking, while YIPPEE 2 activated when a person approached on a skateboard. These results demonstrate that the system can distinguish between different levels of approaching hazards and provide an appropriate warning to the rider while minimizing false detections from surrounding environmental features.

                                                               Table 2. Testing Results
| Trial | Approach Direction | Distance (m) | Bike Stationary | YIPPEE 1 Activated | YIPPEE 2 Activated | Bike Moving | YIPPEE 1 Activated | YIPPEE 2 Activated |
|-------|--------------------|--------------|-----------------|-------------------|-------------------|-------------|-------------------|-------------------|
| 1 | Left | <8.92 | Yes | Yes | Yes | Yes | Yes | Yes |
| 2 | Center | <8.92 | Yes | Yes | Yes | Yes | Yes | Yes |
| 3 | Right | <8.92 | Yes | Yes | Yes | Yes | Yes | Yes |


# Lessons learned

## Radar Configuration and Software Integration

One of the most significant challenges involved the TI IWR6843 mmWave radar module. Unlike many sensors that function immediately after wiring and power-up, the radar required separate configuration through Texas Instruments software tools before it could begin transmitting useful data. This process was complicated by documentation inconsistencies and software version compatibility issues, as some referenced tools and resources were outdated. Significant time was spent identifying the correct software environment and configuration procedure before successful communication could be established. While the IWR6843 ultimately simplified system development by providing onboard target detection and processing, integrating the module required considerably more software configuration than initially anticipated. This highlighted the importance of evaluating not only sensor capabilities, but also the supporting software ecosystem, configuration procedures, and development tools associated with highly integrated hardware platforms.

## Radar Communication and Hardware Interface Challenges

Establishing communication with the radar module proved more difficult than initially anticipated. Early testing indicated that the radar was powered correctly but was not transmitting usable data to the microcontroller. After extensive debugging, the team determined that the radar's hardware configuration switches and communication routing were not configured correctly. The system only began functioning once the appropriate communication path was routed from the PC USB connection through the radar's onboard interface and into the UART channels used by the microcontroller.

Additional challenges arose from the radar connector itself. During the initial PCB design, it was assumed that the exposed header pins on the radar module provided access to the required communication signals. Later investigation revealed that the necessary interfaces were only available through the radar's J2 60-pin connector. Because this connector was incompatible with the existing PCB design and difficult to interface with directly, the team designed and manufactured a custom adapter PCB that broke out the required signals from the 60-pin connector to a JST-GH connector compatible with the main control board. This redesign required additional effort and cost but ultimately was able to cleanly interface between the two boards.

## Detection Threshold Tuning

While the radar was capable of reliably detecting objects, determining appropriate detection thresholds required substantial experimentation. Multiple rounds of testing were performed to tune velocity and distance thresholds for the intended bicycle environment. Thresholds that were too sensitive resulted in unnecessary detections, while overly restrictive settings reduced warning time and awareness for approaching vehicles. Through iterative testing and refinement, the team identified operating parameters that provided reliable detection performance while minimizing false alerts. This process demonstrated the importance of real-world testing when developing sensor-based systems.

## Power Management and Demonstration Readiness

A final challenge occurred immediately prior to the project demonstration when the primary battery pack had discharged to approximately 6.2 V. The reduced battery voltage resulted in unreliable system operation and required the team to quickly recharge the battery and prepare a backup power source shortly before the demonstration. Although the issue was resolved successfully, it emphasized the importance of power-management planning, battery monitoring, and pre-demonstration system checks.
