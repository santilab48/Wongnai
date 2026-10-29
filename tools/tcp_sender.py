#!/usr/bin/env python3
"""Send an authorized test receipt directly to an ESP32 capture listener."""
import argparse
import socket
import time

def payload(kind):
    return {'english': b'Table 4\nNoodles x2\n',
            'thai': 'โต๊ะ 4\nข้าวผัด 2\n'.encode(),
            'escpos': b'\x1b@\x1ba\x01Table 4\n\x1dV\x00',
            'binary': bytes(range(256)),
            'invalid': b'\xff\xfe\x80'}[kind]

def send(host, port, data, chunk=1024, pause=0):
    with socket.create_connection((host, port), timeout=10) as conn:
        for i in range(0, len(data), chunk):
            conn.sendall(data[i:i+chunk])
            if pause: time.sleep(pause)

if __name__ == '__main__':
    ap=argparse.ArgumentParser()
    ap.add_argument('host')
    ap.add_argument('--port',type=int,default=9100)
    ap.add_argument('--kind',choices=['english','thai','escpos','binary','invalid'],default='english')
    ap.add_argument('--chunk',type=int,default=1024)
    ap.add_argument('--pause',type=float,default=0)
    args=ap.parse_args()
    send(args.host,args.port,payload(args.kind),args.chunk,args.pause)
