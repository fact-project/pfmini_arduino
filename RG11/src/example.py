import socket
import time
import struct
from collections import namedtuple

Report = namedtuple('Report', 'dropCounter, pulse_length_in_ms, time, checksum')

def request(host, port=80):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5.)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.connect((host, port))
    s.send(b"GET / HTTP/1.0\r\n\r\n")
    data = s.recv(1000000)
    s.shutdown(1)
    s.close()
    return data

while True:
    try:
        msg = struct.unpack("IIIH", request('10.0.100.155'))
        report = Report(
            dropCounter=msg[0],
            pulse_length_in_ms=msg[1] / 1e3,  # 1e3 convert us to ms.
            time=msg[2] / 1e3,  # 1e3 convert ms to seconds.
            checksum=msg[3]
        )

        print(time.asctime(), report)
    except Exception as e:
        print(time.asctime(), e)

    time.sleep(1)