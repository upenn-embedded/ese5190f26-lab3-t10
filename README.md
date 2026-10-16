# Lab 3 Theremin

**Team Name/Number: Lab 10**

**GitHub Repository URL: https://github.com/upenn-embedded/ese5190f26-lab3-t10.git**

| Team Member Name | Email Address       | GitHub Handle       |
|------------------|---------------------|---------------------|
| [Yaohu Gu]         | [guy7@engineering.upenn.edu]           | [Guangguy7]          |
| [Nikhil Sheth]         | [nsheth1@wharton.upenn.edu]           | [nsheth19]          |


I1
![B1 Screenshot](./B1.png)

I2
![B2 Screenshot](./B2.png)

I3
![B3 Screenshot](./B3.png)

I4
![B4 Screenshot](./B4.png)

I5

![C1 Screenshot](./C1.png)

I6
![C2 Screenshot](./C2.png)

R1
| Note | Frequency (Hz) | OCR0A |
|------|---------------:|------:|
| C6   | 1046           | 60    |
| D6   | 1174           | 53    |
| E6   | 1318           | 47    |
| F6   | 1397           | 45    |
| G6   | 1568           | 40    |
| A6   | 1760           | 36    |
| B6   | 1975           | 32    |
| C7   | 2093           | 30    |

R2
OCR0A = 0.002709 * SENSOR_VALUE + 29.664

R3
| ADC Range | Duty Cycle |
|---|---:|
| 24 - 120 | 5% |
| 121 - 217 | 10% |
| 218 - 314 | 15% |
| 315 - 410 | 20% |
| 411 - 507 | 25% |
| 508 - 604 | 30% |
| 605 - 700 | 35% |
| 701 - 797 | 40% |
| 798 - 894 | 45% |
| 895 - 991 | 50% |

S1
![S1 Screenshot](./E.png)


S2
![S2 Screenshot](./F.png)


I7
![I7 Screenshot](./G1.png)

I8
![I8 Screenshot](./G2.png)

I9
![I9 Screenshot](./G3.png)
Brief Description:
SDA: Carries the I^2 C address and data bits
SCL: Provides the clock used to synchronize communication
Slave address + Write: Selects the IMU at address 0x6B for writing
Register address: 0x28 selects the X-axis acceleration low byte register
Slave address + Read: Selects the same IMU for reading
Data bytes: 0x3A and 0xBE are the low and high bytes of the X axis acceleration reading

I10
![I10 Screenshot](./G4.png)