# Smart Monitor Project

## Introduction

This project is an embedded system created using an Arduino, designed to monitor and manage smart devices in a home environment. The system features a Finite State Machine (FSM) for handling various states of operation, device management, and a user interface via an LCD. The project was developed to explore concepts such as device synchronization, data storage in EEPROM, and efficient memory management.

## Features

### 1. Finite State Machine (FSM)
The system operates using an FSM with the following states:

- **SYNCHRONISATION State**: The system starts in this state, printing "Q" to the serial monitor at one-second intervals. It remains in this state until the user inputs "x" via the serial interface, at which point it transitions to the NORMAL_DISPLAY state.
  
- **NORMAL_DISPLAY State**: Displays all devices (whether on or off) on the smart monitor. The user can navigate through the devices using the up and down arrows without changing the state. The state can be changed by pressing:
  - **Left button**: Transitions to the ONLY_OFF state.
  - **Right button**: Transitions to the ONLY_ON state.
  - **Select button (held for 1 second)**: Transitions to the DISPLAY_STUDENT_ID state.

- **DISPLAY_STUDENT_ID State**: Displays the student's ID number when the select button is held for over one second. The system remains responsive to serial inputs but does not display device changes until the select button is released, returning to the previous state.

- **ONLY_OFF State**: Displays only devices that are currently off. It updates in real-time based on serial inputs. The state can be exited by pressing the left button (returns to NORMAL_DISPLAY) or by holding the select button (transitions to DISPLAY_STUDENT_ID).

- **ONLY_ON State**: Displays only devices that are currently on, with real-time updates. Exiting the state is similar to the ONLY_OFF state, but using the right button.

### 2. Data Structures

- **Device Class**: Manages all information about each smart device with the following attributes:
  - `ID`: A 4-character array storing the device ID.
  - `Location`: A 16-character array storing the device location.
  - `Type`: A character representing the device type ('S' for speaker, 'T' for thermistor, 'L' for light, 'C' for camera, 'O' for socket).
  - `State`: A 4-character array representing whether the device is ON or OFF.
  - `Power`: An integer representing the device's power output.
  - `Temperature`: An integer for the device's temperature (9-32°C).
  - `EPflag`: A Boolean indicating if the device was originally read from EEPROM.

- **Device Array**: 
  - `sDevices`: An array of `Device` objects representing the devices currently monitored.
  - `sDevicesCopy`: A copy of the `sDevices` array used in specific states like ONLY_ON and ONLY_OFF.

- **State Enumerator**: Defines the possible states of the smart monitor (e.g., SYNCHRONISATION, NORMAL_DISPLAY, ONLY_ON, ONLY_OFF, DISPLAY_STUDENT_ID).

### 3. EEPROM Integration

The project utilizes EEPROM for persistent storage of device data. It includes methods to read (`readEEPROM()`) and write (`writeToEEPROM()`) device information, with safeguards to identify if a device was originally saved to EEPROM.

### 4. Custom LCD Characters

The project defines custom characters for the LCD display (e.g., upArrow, downArrow) to enhance the user interface.

### 5. Memory Management and Debugging

- **Free RAM Display**: The system includes a function to display the available SRAM on the monitor alongside the student ID in the DISPLAY_STUDENT_ID state.
  
- **Debugging Approach**: The primary debugging method involved serial print statements to track the flow of execution and the status of variables, especially during function calls.

### 6. User Interface Extensions

- **SCROLL Feature**: For device locations longer than 11 characters, the location scrolls across the display. This feature is implemented in the `scrollLocation()` function, which shifts the displayed text every two seconds while checking for button presses or other interrupts.

### 7. Timers and Interrupt Handling

The project attempts to handle timers and interrupts effectively, ensuring that features such as button presses and serial inputs are responsive even when other tasks, like scrolling text, are running.

## Installation and Setup

1. **Clone the repository:**
    ```bash
    git clone https://github.com/EmmanuelAdio/Embedded_Prog.git
    cd smart-monitor
    ```

2. **Hardware Setup:**
   - **Arduino Board**: Ensure you have the necessary Arduino board and connect the LCD shield, buttons, and other peripherals as specified in your project setup.

3. **Upload Code:**
   - Use the Arduino IDE to upload the code to your Arduino board.

## Usage

- **State Transitions**: Use the buttons to navigate between different states as described in the FSM section.
- **Real-Time Monitoring**: Devices will update in real-time based on inputs from the serial interface.
- **Student ID Display**: Hold the select button for over one second to view the student ID and available SRAM.

## Troubleshooting

- **Memory Issues**: If the Arduino crashes or behaves unexpectedly, check for memory usage issues, especially related to string handling or large arrays.
- **Serial Monitor**: Use the serial monitor to debug and send commands (e.g., input "x" to transition from SYNCHRONISATION to NORMAL_DISPLAY).

## Reflection

The project achieved most of its goals, with functional FSM transitions and effective device management. However, improvements could be made in memory efficiency and more robust timer handling. Future iterations could explore alternative methods for string handling to reduce memory usage and enhance system stability.

## License

It is my University Coursework
