#!/bin/sh -xe

set -e

REMOTEHOST=pi@raspberrypi.local

arduino-cli compile --fqbn arduino:avr:mega AceMPPT.ino

scp build/arduino.avr.mega/AceMPPT.ino.hex $REMOTEHOST:~

ssh $REMOTEHOST /bin/bash <<'EOT'
avrdude -v -patmega2560 -cwiring -P/dev/ttyACM0 -b115200 -D -Uflash:w:AceMPPT.ino.hex:i
EOT