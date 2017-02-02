import socket
import time
socket.setdefaulttimeout = 5.3

def GET(HOST, PORT=80):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5.30)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # s.setblocking(0)
    s.connect((HOST, PORT))
    s.send(b"GET / HTTP/1.0\r\n\r\n")
    data = (s.recv(1000000))
    # https://docs.python.org/2/howto/sockets.html#disconnecting
    s.shutdown(1)
    s.close()
    return data

while True:
    print(time.asctime(), GET('10.0.100.155'))
    time.sleep(1)