#
PLATFORM_TARGET=avr-zigbit-edytee

start_usbs.sh

echo -e "Which /dev/ttyUSB to send? (Write only the number) \n>\c"
read

make TARGET=$PLATFORM_TARGET savetarget
make ${PWD##*/}
avr-objcopy --srec-len=128 -O srec ${PWD##*/}.$PLATFORM_TARGET ${PWD##*/}.srec
bootloader -f ${PWD##*/}.srec -p /dev/ttyUSB$REPLY

exit
