#!/usr/bin/env python3
import sys
import __future__

SIGFOX_HEADER_BOOT =  0x00
SIGFOX_HEADER_BEACON = 0x01
SIGFOX_HEADER_ALERT_MOTION = 0x02
SIGFOX_HEADER_ALERT_TEMPERATURE = 0x03

header_lut = {
    SIGFOX_HEADER_BOOT: 'BOOT',
    SIGFOX_HEADER_BEACON: 'BEACON',
    SIGFOX_HEADER_ALERT_MOTION: 'ALERT_MOTION',
    SIGFOX_HEADER_ALERT_TEMPERATURE: 'ALERT_TEMPERATURE'
}


def temperature_decode(txt):
    if txt == 'ff':
        return None
    return int(txt, 16) / 2.0 - 28


def decode(data):
    length = len(data)
    if length != 6 and length != 10:
        raise Exception("Bad data length, 6 or 10 characters expected")

    header = int(data[0], 16)

    return {
        "header": header_lut[header],
        "position": int(data[1], 16),
        "voltage": int(data[2], 16) / 8.0 + 1.8,
        "temperature": temperature_decode(data[4:6]),
        "count": int(data[6:10], 16) if header == SIGFOX_HEADER_ALERT_MOTION else None,
        "temperature_alert": temperature_decode(data[6:8]) if header == SIGFOX_HEADER_ALERT_TEMPERATURE else None,
        "temperature_previous": temperature_decode(data[8:10]) if header == SIGFOX_HEADER_ALERT_TEMPERATURE else None
    }


def pprint(data):
    print('Message :', data['header'])
    print('Position :', data['position'])
    print('Voltage :', data['voltage'])
    print('Temperature :', data['temperature'])
    if data['count'] is not None:
        print('Event count :', data['count'])
    if data['temperature_alert'] is not None:
        print('Temperature alert :', data['temperature_alert'])
    if data['temperature_previous'] is not None:
        print('Temperature previous :', data['temperature_previous'])


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example BOOT paket: python3 decode.py 014062")
        print("example BEACON paket: python3 decode.py 168064")
        print("example ALERT MOTION paket: python3 decode.py 2150620001")
        print("example ALERT MOTION paket: python3 decode.py 2150620000")
        print('example ALERT TEMPERATURE paket: python3 decode.py 3350667262')
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)
