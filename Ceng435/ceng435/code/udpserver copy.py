import socket
import hashlib
import struct
import itertools
import select
import time

def file_chunks(file_path, is_large, chunk_size=1024):
    """
    read 1024 byte chunks out of a given file until 
    there is no more data left to read.
    return (chunk, seq_no, is_large)
    """
    with open(file_path, 'rb') as file:
        seq_no = 0 # this is to represent a chunks place in a file for reassembly
        chunk = file.read(chunk_size) # read first 1024 bytes
        while chunk: # keep reading until no more data left
            yield (chunk, seq_no, is_large) # yield tuple
            seq_no += 1 # increase sequence number
            chunk = file.read(chunk_size) # read 1024 bytes

def create_packet(data, nextseqnum, file_seq_no, is_large, is_end):
    """
    this func creates a packet

    here is the process:
    - read checksum from .md5 file
    - create format
    - create header
    - merge header and data
    """
    packet_type = 1 if is_large else 0
    end_packet = 1 if is_end else 0

    checksum = hashlib.md5(data).hexdigest() # compute checksum from .md5 file

    format = 'IIBB32s' # < u_int - u_int - u_char - u_char - 32 byte string > = 4+4+1+1+32 = 42 bytes

    #TODO: do we know that the size of checksum of data is always 32 bytes(32s)?

    header = struct.pack(format, nextseqnum, file_seq_no, end_packet, packet_type, checksum.encode()) # create packet with struct library
    
    return header + data

def interleave_chunks(large_chunks, small_chunks):
    """
    This returns an iterable that contains tuples of large and 
    small chunks until one of them is exhausted. Then, the 
    exhausted chunk is returned as None until the longer one 
    is exhausted as well
    """
    return itertools.zip_longest(large_chunks, small_chunks)

def server():
    host = "server"
    port = 8000
    client_address = (host, port)
    bufferSize = 1024
    window = 4
    timeout = 2

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.setblocking(0)

            for i in range(10):
                nextseqnum = 0
                base = 0
                packets_buffer = {}
                packet_send_times = {}

                large_file = f'large-{i}.obj'
                small_file = f'small-{i}.obj'

                large_chunks = file_chunks(large_file, True)
                small_chunks = file_chunks(small_file, False)

                chunk_iterator = interleave_chunks(large_chunks, small_chunks)
                object_sent = False

                while not object_sent:
                    while nextseqnum < base + window:
                        try:
                            large_chunk, small_chunk = next(chunk_iterator)
                        except StopIteration:
                            object_sent = True
                            break

                        if large_chunk:
                            packet = process_chunk(large_chunk, nextseqnum, s, client_address, packets_buffer, packet_send_times)
                            nextseqnum += 1

                        if small_chunk:
                            packet = process_chunk(small_chunk, nextseqnum, s, client_address, packets_buffer, packet_send_times)
                            nextseqnum += 1

                    if not handle_acknowledgements(s, base, nextseqnum, packets_buffer, packet_send_times, bufferSize, timeout):
                        # Resend packets if timeout occurred
                        resend_packets(base, nextseqnum, s, client_address, packets_buffer, packet_send_times)

    except socket.error as e:
        print(f"Socket error occurred: {e}")

def process_chunk(chunk, nextseqnum, socket, address, packets_buffer, packet_send_times):
    data, file_seq_no, is_large = chunk
    is_end = True if file_seq_no == len(chunks) - 1 else False
    packet = create_packet(data, nextseqnum, file_seq_no, is_large, is_end)

    socket.sendto(packet, address)
    packets_buffer[nextseqnum] = packet
    if base == nextseqnum:
        packet_send_times[nextseqnum] = time.time()

def handle_acknowledgements(socket, base, nextseqnum, packets_buffer, packet_send_times, bufferSize, timeout):
    current_time = time.time()
    if base in packet_send_times and current_time - packet_send_times[base] > timeout:
        return False

    ready = select.select([socket], [], [], timeout - (current_time - packet_send_times.get(base, current_time)))
    if ready[0]:
        ack_packet, _ = socket.recvfrom(bufferSize)
        ack_no = struct.unpack('I', ack_packet)[0]

        base = ack_no + 1
        remove_acknowledged_packets(base, packets_buffer, packet_send_times)
    return True

def resend_packets(base, nextseqnum, socket, address, packets_buffer, packet_send_times):
    current_time = time.time()
    for seq in range(base, nextseqnum):
        socket.sendto(packets_buffer[seq], address)
        packet_send_times[seq] = current_time

def remove_acknowledged_packets(base, packets_buffer, packet_send_times):
    to_remove = [seq for seq in packets_buffer if seq < base]
    for seq in to_remove:
        del packets_buffer[seq]
        del packet_send_times[seq]

if __name__ == '__main__':
    server()
