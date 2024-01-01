import socket
import os

def send_file(conn, file_path):
    with open(file_path, 'rb') as f:
        while True:
            bytes_read = f.read(1024)
            if not bytes_read:
                break
            conn.sendall(bytes_read)

def server():
    host = '172.17.0.2'
    port = 8000

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()

        print("Server listening on port", port)
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)

            for i in range(10):
                # sending large object
                large_file = f'large-{i}.obj'
                print(f"Sending {large_file}")
                send_file(conn, large_file)

                # sending small object
                small_file = f'small-{i}.obj'
                print(f"Sending {small_file}")
                send_file(conn, small_file)

            print("All files sent successfully")

if __name__ == '__main__':
    server()
