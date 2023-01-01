1. Install 'AVRDUDESS-2.14-setup.exe'

2. Burn at target MCU the ATmega328P 'custom_bootloader.hex' (that changes
the clock source from external crystal to internal 8 MHz oscillator). 

I'm using an Arduino UNO as programmer, checks 'AVRDUDESS_setup.png' as
an example.

3. Copy 'breadboard.json' to
'C:\Users\<user>\.platformio\platforms\atmelavr\boards'

4. Upload de 'CS GO's C4 firmware' using VS Code + PlatformIO + some ISP programmer
(like a Arduino as ISP, AVR USBasp...)