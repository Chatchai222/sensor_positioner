import socket 

UDP_IP = "0.0.0.0"
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))


print("Start UDP consumer")

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024
    print("received message: %s" % data)
    print("from address", addr)


