# Fall detector project for 4B25
Ruben Ruiz-Mateos Serrano, CRSid = rr632
---

*This repository is based on the `Warp-firmware` repository provided by Prof. Stanley-Marbell.* 

Two principal changes have been made to the original `Warp-firmware` in this project:

- The file `script.c` has been created to replace the original `boot.c` file in the `/home/students/rr632/Warp-firmware/src/boot/ksdk1.1.0/` directory. `script.c` is an exact replica of `boot.c` which excludes the Warp boot menu for storage space purposes. The `main` function in `script.c` includes a call to the `falldetectorMMA8451Q` function.
Since the file `script.c` is being used to boot the `Warp-firmware`, the files `Makefile` and `/home/students/rr632/Warp-firmware/src/boot/ksdk1.1.0/CMakeLists-Warp.txt` had to be modified to include `script.c` in the boot process.

- The files `devMMA8451Q.c` and `devMMA8451Q.h` have been modified in two ways:
	-  The function `configureSensorMMA8451Q` has been expanded to take as input 4 unsigned integer values, which are sent as calibration values for the **F_SETUP**, **CRTL_REG1**, **XYZ_DATA_CFG** and **CRTL_REG2** registers, respectively.
	-  The function `falldetectorMMA8451Q` has been created to read x, y and z axis acceleration data from the FRDMKL03Z board in-built MMA8451Q three-axis accelerometer, convert it into x, y and z velocity, average it over 20 samples, obtain the magnitude of the velocity and shine an intermittent red LED light when the velocity magnitude exceeds 30 m/s (as this high velocity is only achieved during a fall). Furthermore, the function forces the device to shine an intermittent green light if the configuration of the MMA8451Q accelerometer is not successful.  

*Note: the `.gitognore` file has been modified to allow the storage of all files generated during the build process. This has been done simply to keep record of different s-record files.*
