version = 3.0.1

firmware:
	cd axoloti/firmware && ./compile_firmware_linux.sh

gui: firmware
	cd axoloti && ant

package-linux: gui
	mkdir -p package
	java -jar external/packr-all-2.7.0.jar --verbose packr-linux-x64.json
	rm -rf package/linux/akso-${version}/external/jdks
	rm -rf package/linux/akso-${version}/external/gcc-arm/win
	rm -rf package/linux/akso-${version}/external/gcc-arm/mac
	rm -rf package/linux/akso-${version}/external/STM32CubeProgrammer/win
	rm -rf package/linux/akso-${version}/external/STM32CubeProgrammer/mac
	rm -rf package/linux/akso-${version}/firmware/build/lst
	rm -rf package/linux/akso-${version}/firmware/build/obj
	rm -rf package/linux/akso-${version}/firmware/flasher/flasher_build/lst
	rm -rf package/linux/akso-${version}/firmware/flasher/flasher_build/obj
	rm -rf package/linux/akso-${version}/firmware/mounter/flasher_build/lst
	rm -rf package/linux/akso-${version}/firmware/mounter/flasher_build/obj
	cd package/linux && tar -czvf ../akso-linux-${version}.tar.gz *

package-mac: gui
	mkdir -p package
	java -jar external/packr-all-2.7.0.jar --verbose packr-mac-x64.json
	rm -rf package/mac/Akso.app/Contents/Resources/external/jdks
	rm -rf package/mac/Akso.app/Contents/Resources/external/gcc-arm/linux
	rm -rf package/mac/Akso.app/Contents/Resources/external/gcc-arm/win
	rm -rf package/mac/Akso.app/Contents/Resources/external/STM32CubeProgrammer/linux
	rm -rf package/mac/Akso.app/Contents/Resources/external/STM32CubeProgrammer/win
	rm -rf package/mac/Akso.app/Contents/Resources/firmware/build/lst
	rm -rf package/mac/Akso.app/Contents/Resources/firmware/build/obj
	rm -rf package/mac/Akso.app/Contents/Resources/firmware/flasher/flasher_build/lst
	rm -rf package/mac/Akso.app/Contents/Resources/firmware/flasher/flasher_build/obj
	rm -rf package/mac/Akso.app/Contents/Resources/firmware/mounter/flasher_build/lst
	rm -rf package/mac/Akso.app/Contents/Resources/firmware/mounter/flasher_build/obj
	chmod +x package/mac/Akso.app/Contents/Resources/jre/lib/jspawnhelper
	chmod +x package/mac/Akso.app/Contents/Resources/external/STM32CubeProgrammer/mac/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI
	cd package/mac && zip -r ../akso-mac-${version}.zip *

package-win: gui
	mkdir -p package
	java -jar external/packr-all-2.7.0.jar --verbose packr-win-x64.json
	rm -rf package/win/akso-${version}/external/jdks
	rm -rf package/win/akso-${version}/external/gcc-arm/linux
	rm -rf package/win/akso-${version}/external/gcc-arm/mac
	rm -rf package/win/akso-${version}/external/STM32CubeProgrammer/linux
	rm -rf package/win/akso-${version}/external/STM32CubeProgrammer/mac
	rm -rf package/win/akso-${version}/firmware/build/lst
	rm -rf package/win/akso-${version}/firmware/build/obj
	rm -rf package/win/akso-${version}/firmware/flasher/flasher_build/lst
	rm -rf package/win/akso-${version}/firmware/flasher/flasher_build/obj
	rm -rf package/win/akso-${version}/firmware/mounter/flasher_build/lst
	rm -rf package/win/akso-${version}/firmware/mounter/flasher_build/obj
	cd package/win && zip -r ../akso-win-${version}.zip *

package: package-linux package-mac package-win

clean:
	cd axoloti/firmware && make clean
	cd axoloti && ant clean
	rm -rf package

.PHONY: firmware gui linux mac win package clean
