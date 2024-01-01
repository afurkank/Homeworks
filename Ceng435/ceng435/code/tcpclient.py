import socket

def receive_file(s, file_path):
    with open(file_path, 'wb') as f:
        while True:
            data = s.recv(1024)
            if not data:
                break
            f.write(data)

def client():
    host = '172.17.0.2'
    port = 8000

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))

        for i in range(10):
            # receiving large object
            large_file = f'received_large-{i}.obj'
            print(f"Receiving {large_file}")
            receive_file(s, large_file)

            # receiving small object
            small_file = f'received_small-{i}.obj'
            print(f"Receiving {small_file}")
            receive_file(s, small_file)

        print("All files received successfully")

if __name__ == '__main__':
    client()
