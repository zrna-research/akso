The API's pack contains examples that are written for documentation to take inspiration for use cases.

# Doxygen documentation that provides all necessary informations about the API functions, data structures and examples requirements.

# 8 examples are developed and compatible with Visual Studio and Qt creator IDEs:
	-	3 examples with ST-LINK debug interface (JTAG/SWD) with different scenarios.
	-	1 example with SPI bootloader via ST-LINK/V3 Bridge
	-	1 example with CAN bootloader via ST-LINK/V3 Bridge
	-	1 example with I2C bootloader via ST-LINK/V3 Bridge
	-	1 example with USB DFU bootloader
	-	1 example with UART bootloader

# If you compile the project from the install path, you should verify the correct permisions for Read/Write operations.

# The attached Visual Studio projects are written to be compiled both for x86 and x64 architectures.
    -	Go to \api\project\Visual Studio\x86 to build the x86 project.
    -	Go to \api\project\Visual Studio\x64 to build the x64 project.
    -	The working directory, the Output and the dependecies paths are the same : \api\lib
    - The x86 project links to \api\lib\x86\CubeProgrammer_API.lib to import and define correctly all API's functions.
    - The x64 project links to \api\lib\x64\CubeProgrammer_API.lib to import and define correctly all API's functions.
    - If you install a STM32CubeProgrammer with x86 architecture you should use only the x86 project to avoid any type of inconsistency 
      on libraries at the compile or run times (x86 DLLs on \api\lib).
    - If you install a STM32CubeProgrammer with x64 architecture you should use only the x64 project to avoid any type of inconsistency 
    on libraries at the compile or run times(x64 DLLS on \api\lib).

# refer to the help file (STM32CubeProgrammer_API.chm) in doc folder for more API informations.