#!/usr/bin/env python3

"""Script for running RTT tests managed through the serial console
"""

import argparse
import glob
import os
import serial
import subprocess
import sys
import time

parser = argparse.ArgumentParser(description='RTT tests')
parser.add_argument('--save', dest='save', action='store_true')
parser.add_argument('--no-save', dest='save', action='store_false')
parser.set_defaults(save=True)
parser.add_argument('--packetsize', type=int, default=1, help='Size in bytes of each packet')
parser.add_argument('-n', type=int, default=10, help='Number of packets to send')
parser.add_argument('--interval', type=int, default=500, help='Millisecond wait time between packets')
parser.add_argument('dst', help='The address of the receiver node')

def get_tty():
    tty = '/dev/ttyACM2'

    dev = '/dev/ttyACM*'
    scan = glob.glob(dev)
    if tty not in scan:
        return None

    return tty

def main():
    args = parser.parse_args()
    print(args.dst)
    # Check arguments
    if args.n <= 0:
        print('-n must be > 0')
        return

    if args.packetsize < 1 or args.packetsize > 100:
        print('--packetsize must be between 1 and 100 bytes')
        return

    if args.interval < 0:
        print('--interval must be non-negative')
        return

    # Find serial ports
    tty = get_tty()
    if tty == None:
        print('Could not find expected serial port.\
                Make sure the board is flashed and connected')
        return

    # Connect to serial ports
    s1 = serial.Serial(
            port=tty, 
            baudrate=115200, 
            parity=serial.PARITY_NONE, 
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=2)

    results = []
    # Make sure everything is working
    s1.write(f'send {args.dst} 1\n'.encode('UTF-8'))
    s1.readline()
    l = s1.readline()
    if b'rtt:' not in l:
        print(l)
        print(f'Latency not printed at receiver. Check GPIO connections, flashed firmware, and serial connections.')
        return
    time.sleep(1)

    # Run tests
    for i in range(args.n):
        s1.write(f'send {args.dst} {args.packetsize}\n'.encode('UTF-8'))
        s1.readline()
        try:
            l = s1.readline()
            if len(l) == 0:
                continue
            l = l.decode('UTF-8')
            print(f'{i}: \t{l.strip()}')
            latency = int(l.split()[1])
            results.append(latency)
        except Exception as e:
            print(e)
            pass
        time.sleep(args.interval / 1000)


    print(f'Tests completed.')

    # Get current commit
    try:
        git_hash = subprocess.check_output(["git", "describe", "--always"]).strip().decode()
    except:
        git_hash = 'Unknown'

    date = time.strftime('%Y-%m-%d')
    curr_time = time.strftime('%H:%M:%S')
    timestamp = int(time.time())

    # Print summary
    print(f'------ RTT Test {date} {curr_time} ------\n') 
    print(f'Current git hash: {git_hash}')
    print(f'Parameters:')
    print(f'    n:           {args.n}')
    print(f'    packetsize:  {args.packetsize}')
    print(f'    interval:    {args.interval} ms\n')
    print(f'Average RTT: {int(sum(results) / len(results))}')

    if args.save:
        # Create output dir
        dirname = f'{date}_{args.packetsize}bytes_{timestamp}'
        abs_dir = os.path.dirname(os.path.abspath(__file__))
        output_dir = os.path.join(abs_dir, 'results', dirname)
        os.makedirs(output_dir)

        print(f'Saving results to {output_dir}')

        with open(os.path.join(output_dir, 'summary.txt'), 'w') as summary:
            summary.write(f'------ RTT Test {date} {curr_time} ------\n\n') 
            summary.write(f'Current git hash: {git_hash}\n')
            summary.write(f'Parameters:\n')
            summary.write(f'    n:           {args.n}\n')
            summary.write(f'    packetsize:  {args.packetsize}\n\n')
            summary.write(f'    interval:    {args.interval} ms\n\n')
            summary.write(f'Average RTT: {int(sum(results) / len(results))}\n')
            summary.write(f'Full results stored in rtt.csv\n')

        # Write resuts to csv
        with open(os.path.join(output_dir, 'rtt.csv'), 'w') as out:
            out.writelines([str(i)+'\n' for i in results])

if __name__ == '__main__':
    main()
