import socket
import os
import hashlib
import struct
import itertools

def file_chunks(file_path, is_large, chunk_size=1024):
    with open(file_path, 'rb') as file:
        seq_no = 0
        chunk = file.read(chunk_size)
        while chunk:
            yield (chunk, seq_no, is_large)
            seq_no += 1
            chunk = file.read(chunk_size)

def create_packet(data, global_seq_no, file_seq_no, is_large):
    packet_type = 1 if is_large else 0
    checksum = hashlib.md5(data).hexdigest()
    format = 'IIB32s' # < u_int - u_int - u_char - 32 byte string > = 4+4+1+32 = 41 bytes
    header = struct.pack(format, global_seq_no, file_seq_no, packet_type, checksum.encode())
    return header + data

def interleave_chunks(large_chunks, small_chunks):
    return itertools.zip_longest(large_chunks, small_chunks)

def server():
    host = "server"
    port = 8000
    client_address = (host, port)  # Client address

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        global_seq_no = 0

        for i in range(10):
            large_file = f'large-{i}.obj'
            small_file = f'small-{i}.obj'

            large_chunks = file_chunks(large_file, True)
            small_chunks = file_chunks(small_file, False)

            for large_chunk, small_chunk in interleave_chunks(large_chunks, small_chunks):
                if large_chunk:
                    data, file_seq_no, is_large = large_chunk
                    packet = create_packet(data, global_seq_no, file_seq_no, is_large)
                    s.sendto(packet, client_address)
                    global_seq_no += 1

                if small_chunk:
                    data, file_seq_no, is_large = small_chunk
                    packet = create_packet(data, global_seq_no, file_seq_no, is_large)
                    s.sendto(packet, client_address)
                    global_seq_no += 1

                # Implement ACK handling and retransmission logic here

if __name__ == '__main__':
    server()
