#!/usr/bin/env python3

"""Script for running latency tests managed through the serial console
"""

import glob
import os
import serial
import subprocess
import sys
import time

def main():
    # Connect to serial ports
    s1 = serial.Serial(
            port='/dev/ttyACM0', 
            baudrate=115200, 
            parity=serial.PARITY_NONE, 
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=None)
    
    l = s1.readline()
    while b'STARTING' not in l:
        l = s1.readline()

    while True:
        l = s1.readline()
        if len(l) == 0:
            print('Timed out')
            break
        if b'DONE' in l:
            break
        else:
            print(l.decode('UTF-8').strip('\r\n'))

if __name__ == '__main__':
    main()
