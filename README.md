# Bachelor Thesis *(Summary)*
Technische Universität Wien, 2013.


My bachelor thesis was a project at the Institute of Electrodynamics, Microwave and Circuit Engineering of Technical University of Vienna. I had the opportunity to work on a practical project that required manufacturing of a PCB and purchasing of electronic components. The project was to build a power meter for optical signals to monitor optical power by withdrawing a small percentage of the power for the measurement and keeping the majority of the power for the actual application. Therefore, the power meter had to be jointly calibrated with a power splitter for different wavelengths.

**The requirements for the power meter were:**
- Wide dynamic range of input power to the photo diode from 100 pW to 10 mW,
- for wavelengths between 320nm and 1000nm,
- 10:90 splitter, 10% for the measurement, 90% for the application,
- different calibrations for different wavelengths should be possible,
- display of the measured power,
- remote control via USB interface,
- power supply via USB interface (preferred) or batteries (if no USB cable connected).

**The project consisted of the following tasks:**
- Learning theoretical basics of optical power measurement and the related challenges (i.e. for low power levels),
- identifying suitable electronic components (i.e. the photo diode and the analog/digital converter),
- designing a printed circuit board with the photo diode, the A/D converter, a  microcontroller, a display,... ,
- assembling the printed circuit board,
- programming the microcontroller,
- developing a user interface,
- calibrating and characterizing the power meter,
- writing a bachelor thesis about the project.



### 1) The Photo Diode and A/D conversion
The logarithmic converter AD6304 from Analog Devices with 160 dB range (100 pA – 10 mA) and adjustable slope and intercept was selected for this application. The photo diode was surrounded by  a guard ring to prevent leakage currents. 
The input range was divided into two blocks with different conversion slope and intercept values. Three small analogue switches were used to switch between the external configurations of the AD6304 depending on the input power.

<img width="500" alt="ADC gain switching" src="https://github.com/juliankozak/InlinePowerMeter/assets/82274251/c43fdef2-7003-43b5-85bc-cda1534dd563">

The two slopes and intercepts were calculated for the two ranges (cf. thesis pdf). The configured input/output characteristic of the A/D converter is shown in the following figure:

<img width="400" alt="Characterization input power and output voltage" src="https://github.com/juliankozak/InlinePowerMeter/assets/82274251/51b8bf28-3534-4bdb-bd92-383d06171ef4">

### 2) The assembled device looked like this:
<img width="500" alt="Final PCB, Display, USB Interface and Keys" src="https://github.com/juliankozak/InlinePowerMeter/assets/82274251/94244a48-553b-4521-935d-aaacf1837a46">

The analogue circuitry is on the left top part of the PCB and the photo diode is located within the FC/PC connector. The four buttons can be used to navigate through the menu and to change the settings:

<img width="400" alt="menu" src="https://github.com/juliankozak/InlinePowerMeter/assets/82274251/b4c2f36d-b0eb-4e65-a566-217f4870ecb1">

The device could also be controlled remotely via USB interface. The device was recognized as a virtual COM port and could be controlled with simple commands. The device could be used as a stand-alone device or as a remote controlled device. The following figure shows the remote control console interface:

<img width="400" alt="remote control console interface" src="https://github.com/juliankozak/InlinePowerMeter/assets/82274251/b219a31c-d5f9-426d-9101-c1ba52938346">


Finally, the achieved accuracy for single mode laser at 635nm wavelength:

<img width="400" alt="Accuracy single mode laser" src="https://github.com/juliankozak/InlinePowerMeter/assets/82274251/8f1651b4-12fc-42cd-9ead-0f694d2e71dd">

