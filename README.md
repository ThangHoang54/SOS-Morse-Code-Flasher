# SOS-Morse-Code-Flasher
This repository contains the source code for our group project in EEET2505 Introduction to Embedded System

## 1. Problem Introduction 
### 1.1. Introduction about Morse code 
Morse code is a method used in telecommunication to encode text characters as standardized sequences 
of two different signal durations, called dots and dashes, or dits and dahs. Morse code is named after 
Samuel Morse, one of the inventors of the telegraph. Morse code is a character-encoding scheme that 
allows operators to send messages using a series of electrical pulses represented as short or long pulses, 
dots, and dashes.
However, ATTENTION, in this assessment, students are not coding by Morse code, for sure, they will code 
in C language. 
### 1.2. Using Morse code 
International Morse code is composed of five elements:
•   short mark, dot or dit (  ▄ ): "dit duration" is one time unit long 
•   long mark, dash or dah (  ▄▄▄ ): three time units long 
• inter-element gap between the dits and dahs within a character: one dot duration or one unit 
long 
•   short gap (between letters): three time units long 
•   medium gap (between words): seven time units long 
International Morse code encodes the 26 basic Latin letters a through z, one accented Latin letter (é), the 
Arabic numerals, and a small set of punctuation and procedural signals (prosigns). There is no distinction 
between upper and lower case letters. Those Morse-code 26 letters and 10 numerals are shown in Figure 
1 below.

![image](https://github.com/user-attachments/assets/d5103997-a9db-42cf-bef3-9925cd074679)

Once people would like to send a Morse-code message, they will need to make a START signal (START 
prosign), then the message that they would like to transmit, and then the STOP signal (STOP prosign).  

## 2. Assessment Key Requirements 
### 2.1. Hardware
a) A Morse-code SOS flasher hardware should be built with a BUTTON, at which a 220-Ohm resistor and an 
LED were used.  
b) A red color LED to indicate the main message, turning on during dit or dah, and turning off during gaps. 
c) A blue color LED to indicate the dit only, it means only turning on during dit. 
d) A green color LED to indicate the dah only, it means only turning on during dah. 
e) A 7segment LED to count the number of button press. 

<img width="307" alt="image" src="https://github.com/user-attachments/assets/6e6bf5fa-c054-4259-85d4-323901daf6c6" />

### 2.2 Software
Use Arduino IDE with C language to program the Morse-code SOS flasher software. Your software must 
control the hardware to show the signal as required: 
EEET2505 – Introduction to Embedded Systems  
a) For each message, need to make a START signal (START prosign), then the message that they would like 
to transmit, and then the STOP signal (STOP prosign).  
b) When you upload the code onto your Arduino, there will be nothing happened. 
c) You need to use Timer 1 to setup one time unit to be 0.5s. And use this time unit to keep track of your 
five elements of Morse code in section 2.2. If you use delay function to keep track of the Morse code 
instead, there will be mark deduction. 
d) When button is pressed two times, the system will display the message including the first names of 
each group member, once. The 7segment LED should show ‘2’ at this stage. 
e) The dih and dah signal must be displayed (through the blinking LED). For example: dih signal for 0.5 
second ON and 0.5 second OFF, and your dah signal must be 1.5 seconds ON and 0.5 second OFF. 
f) 
All gaps between letters and between words need to be obeyed to section 2.2. 
g) After finishing group members’ name transmission, the message SOS (including the START prosign, SOS 
message, and STOP prosign) display on the LED only when you press the button four times (two extra 
times compared to step d), and the message SOS is sent once. The 7segment LED should show ‘4’ at 
this stage. 
h) If you press the button six times (two extra times compared to step g), the system should stop sending 
Morse code, and return to step (b) above, and repeat the behavior like in step (c) and (f). The 7segment 
LED should show ‘6’ at this stage. If you press the button one more time, the 7segment LED should go 
back to ‘1’. 

## 3. Compile & Upload
```sh
arduino-cli compile --fqbn arduino:avr:uno --port /dev/ACM0 --upload morse-code-flasher.ino
```

## 4. Demo
[Project_Demo](https://www.youtube.com/watch?v=v4beXHAHlrU)


