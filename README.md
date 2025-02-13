# Fall detector project for 4B25
Ruben Ruiz-Mateos Serrano, CRSid = rr632, Hughes Hall
---

*This repository is based on the `Warp-firmware` repository provided by Prof. Stanley-Marbell [1].* 

The code presented in this repository implements the firmware required to generate an embedded system for fall recognition.

Two principal changes have been made to the original `Warp-firmware` in this project:

- The file `script.c` has been created to replace the original `boot.c` file in the `/home/students/rr632/Warp-firmware/src/boot/ksdk1.1.0/` directory. `script.c` is an exact replica of `boot.c` which excludes the Warp boot menu for storage space purposes. The `main` function in `script.c` includes a call to the `falldetectorMMA8451Q` function.
Since the file `script.c` is being used to boot the `Warp-firmware`, the files `Makefile` and `/home/students/rr632/Warp-firmware/src/boot/ksdk1.1.0/CMakeLists-Warp.txt` had to be modified to include `script.c` in the boot process.

- The files `devMMA8451Q.c` and `devMMA8451Q.h` have been modified in two ways:
	-  The function `configureSensorMMA8451Q` has been expanded to take as input 4 unsigned integer values, which are sent as calibration values for the **F_SETUP**, **CRTL_REG1**, **XYZ_DATA_CFG** and **CRTL_REG2** registers, respectively.
	-  The function `falldetectorMMA8451Q` has been created to read x, y and z axis acceleration data from the FRDMKL03Z board in-built MMA8451Q three-axis accelerometer, convert it into x, y and z difference in acceleration, average it over 20 samples, obtain the magnitude of this difference and shine an intermittent red LED light when the value crosses a threshold of 2000 (i.e. approx 1g, a high difference in acceleration that is only achieved during a fall). Furthermore, the function forces the device to shine an intermittent green light if the configuration of the MMA8451Q accelerometer is not successful.  

*Notes: - the `.gitignore` file has been modified to allow the storage of all files generated during the build process. This has been done simply to keep record of different s-record files. - further descriptions of commands are given inside the various files mentioned above.*

[1] Phillip Stanley-Marbell and Martin Rinard. “Warp: A Hardware Platform for Efficient Multi-Modal Sensing with Adaptive Approximation”. IEEE Micro, Volume 40 , Issue 1 , Jan.-Feb. 2020.
