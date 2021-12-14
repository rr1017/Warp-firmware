# Fall detector project for 4B25
Ruben Ruiz-Mateos Serrano
CRSid = rr632
---

*This repository is based on the `Warp-firmware` repository provided by Prof. Stanley-Marbell.* 

Two principal changes have been made to the original `Warp-firmware` in this project:

- The file `script.c` has been created to replace the original `boot.c` file in the `/home/students/rr632/Warp-firmware/src/boot/ksdk1.1.0/` directory. `script.c` is an exact replica of `boot.c` which excludes the Warp boot menu for storage space purposes. The `main` function in `script.c` includes a call to the `falldetectorMMA8451Q` function.
Since the file `script.c` is being used to boot the `Warp-firmware`, it was neccesary to modify the files `Makefile`, `/home/students/rr632/Warp-firmware/src/boot/ksdk1.1.0/CMakeLists-Warp.txt` and 
- The files `devMMA8451Q.c` and `devMMA8451Q.h` have been modified in two ways:
	-  kjsdncksdc
