#
PLATFORM_TARGET=micaz-edytee

start_usbs.sh

echo -e "Which /dev/ttyUSB to send? (Write only the number) \n>\c"
read

make TARGET=$PLATFORM_TARGET savetarget
make ${PWD##*/}
avr-objcopy -O srec ${PWD##*/}.$PLATFORM_TARGET ${PWD##*/}.srec
sudo avrdude -cmib510 -P/dev/ttyUSB$REPLY -pm128 -U hfuse:w:0xd1:m -U efuse:w:0xff:m -e -v -U flash:w:${PWD##*/}.srec:a

exit
