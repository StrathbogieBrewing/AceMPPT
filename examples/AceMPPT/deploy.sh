#!/bin/sh -xe

set -e

REMOTEHOST=johny@192.168.8.204

arduino-cli compile --clean --fqbn arduino:avr:mega AceMPPT.ino

scp build/arduino.avr.mega/AceMPPT.ino.hex $REMOTEHOST:~

ssh $REMOTEHOST /bin/bash <<'EOT'
avrdude -v -patmega2560 -cwiring -P/dev/ttyACM0 -b115200 -D -Uflash:w:AceMPPT.ino.hex:i
EOT

# arduino-cli compile -e --clean --fqbn arduino:avr:mega 

# avrdude -v -patmega2560 -cwiring -P/dev/ttyACM0 -b115200 -D -Uflash:w:libraries/AceMPPT/examples/AceMPPT/build/arduino.avr.mega/AceMPPT.ino.hex:i

# arduino-cli compile -e --clean --fqbn arduino:avr:mega libraries/AceMPPT/examples/AceMPPT/AceMPPT.ino

arduino-cli compile -e --clean --fqbn arduino:avr:mega AceMPPT/examples/AceMPPT

arduino-cli upload --verify --port /dev/ttyACM0 --fqbn arduino:avr:mega AceMPPT/examples/AceMPPT
