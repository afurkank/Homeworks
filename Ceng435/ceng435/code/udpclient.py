import socket
import struct
import hashlib

def verify_packet(data, checksum):
    return hashlib.md5(data).hexdigest() == checksum

def client():
    host = 'server'
    port = 8000
    bufferSize = 1024

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))

        received_packets = {}

        while True:
            packet, addr = s.recvfrom(bufferSize)
            header = packet[:41]  # TODO: header size
            data = packet[41:]

            seq_no, packet_type, checksum = struct.unpack('I?32s', header)
            checksum = checksum.decode().strip('\x00')

            if verify_packet(data, checksum):
                # Send ACK
                s.sendto(struct.pack('I', seq_no), addr)

                received_packets[seq_no] = (data, packet_type)

                # Check if all packets received and reassemble files
                # Implement file reassembly logic

if __name__ == '__main__':
    client()
