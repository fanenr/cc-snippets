import socket
import datetime

HOST = "127.0.0.1"
PORT = 13

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    sock.bind((HOST, PORT))
    print(f"Daytime server listening on {HOST}:{PORT}\n")

    while True:
        data, addr = sock.recvfrom(128)
        print(f"Received from {addr}: {data.decode().strip()}")

        now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        sock.sendto(now.encode(), addr)
        print(f"Sent to {addr}: {now}\n")
