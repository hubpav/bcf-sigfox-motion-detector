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
    if length < 10:
        raise Exception("Bad data length, min 10 characters expected")

    header = int(data[0], 16)

    return {
        "header": header_lut[header],
        "position": int(data[1], 16),
        "voltage": int(data[2], 16) / 4.0 + 2,
        "temperature": temperature_decode(data[4:6]),
        "count": int(data[6:10], 16),
        "temperature_alert": temperature_decode(data[6:8]) if header == SIGFOX_HEADER_ALERT_TEMPERATURE else None,
        "temperature_previous": temperature_decode(data[8:10]) if header == SIGFOX_HEADER_ALERT_TEMPERATURE else None,
        "humidity":  int(data[10:12], 16) / 2.0 if len(data) >= 12 else None,
        "illuminance":  int(data[12:16], 16) * 2.0 if len(data) >= 16 else None,
        "pressure":  int(data[16:20], 16) * 2.0 if len(data) >= 20 else None
    }


def pprint(data):
    print('Message :', data['header'])
    print('Position :', data['position'])
    print('Voltage :', data['voltage'], 'V')
    print('Temperature :', data['temperature'], '°C')
    if data['count'] is not None:
        print('Event count :', data['count'])
    if data['temperature_alert'] is not None:
        print('Temperature alert :', data['temperature_alert'], '°C')
    if data['temperature_previous'] is not None:
        print('Temperature previous :', data['temperature_previous'], '°C')

    print("Humidity :", data["humidity"], '%')
    print("Illuminance :", data["illuminance"], 'lux')
    print("Pressure :", data["pressure"], 'Pa')


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example ALERT MOTION paket: python3 decode.py 21e06b0001650003c1c8")
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)
