import serial
import time
import argparse
import sys

# Call this program with
# python coa202ser.py portdevice
# e.g. on windows:
#      python coa202ser.py COM2
# or macos/linux:
#      python coa202ser.py /dev/ttyACM0
#
# Remember you cannot leave this running while uploading
# 

# The code below parses the command line arguments and leaves the
# variable args.port containing the device name.

parser = argparse.ArgumentParser(
    description='Talk to Ardunio over serial')
parser.add_argument(
    'port',
    metavar='device',
    type=str,
    nargs='?',
    help='The port to connect to')
args = parser.parse_args()

# Open the serial port (this will reset your Arduino)
print('connecting to port', args.port)
ser = serial.Serial(args.port, 9600, timeout=5,
                    rtscts=False, xonxoff=False, dsrdtr=False)

print("waiting for sync")
going = True
while going:
    s = ser.read(1)  # Read just one byte
    # print(s)       # Print it for debugging
    if s == b'Q':
        going = False
ser.write(b'X')
print("Sync")

line = ser.readline()
print(line)  # This should print BASIC or your extension list


# Build a list of messages to send
# the b'' notation creates byte arrays suitable for 
# passing to ser.write().  ser.write() will not accept
# str variables.

msgs = [b'A-PZW-S-LivingRoom', b'A-BDA-T-Entrance', b'A-HJL-C-Garden', b'S-PZW-ON', b'S-BDA-ON', b'P-PZW-100', b'P-BDA-20', b'S-HJL-1', b'P-BDA-35', b'A-PZW-S-OutsideGardenShed', b'P-HJL-50', b'P-BDA', b'A-FDG-O-Kitchen', b'A-FDG-C-', b'A-DFD-O-', b'A-DFD-Z-Kitchen', b'A-DFD-Kitchen', b'ADFD-C-LivingRoom']

# Simply write these messages out once per second
# Customise above and below as you see fit.

for x in msgs:
    print("Write:", str(x))
    ser.write(x+b'\n')

    # Check for message back.  This will timeout after a second
    line = ser.readline()
    print("Read: ", line)
