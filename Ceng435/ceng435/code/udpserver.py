import socket
import hashlib
import struct
import itertools
import select
import time

def verify_packet(data, checksum):
    return hashlib.md5(data).hexdigest() == checksum

def file_chunks(file_path, is_large, chunk_size=1024):
    """
    read 1024 byte chunks out of a given file until 
    there is no more data left to read
    """
    with open(file_path, 'rb') as file:
        seq_no = 0 # this is to represent a chunks place in a file for reassembly
        chunk = file.read(chunk_size) # read first 1024 bytes
        while chunk: # keep reading until no more data left
            yield (chunk, seq_no, is_large) # yield tuple
            seq_no += 1 # increase sequence number
            chunk = file.read(chunk_size) # read 1024 bytes

def create_packet(data, nextseqnum, file_seq_no, is_large, is_end):
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
    client_address = (host, port)  # client address
    bufferSize = 1024
    window = 4
    timeout = 2

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.setblocking(0)

        for i in range(10):
            nextseqnum= 0
            base = 0
            packets_buffer = {}
            packet_send_times = {}

            large_file = f'large-{i}.obj'
            small_file = f'small-{i}.obj'

            large_chunks = file_chunks(large_file, True)  # read ith large object
            small_chunks = file_chunks(small_file, False) # read ith small object

            chunk_iterator = interleave_chunks(large_chunks, small_chunks)
            object_sent = False
            while not object_sent:
                while nextseqnum < base + window:
                    try:
                        large_chunk, small_chunk = next(chunk_iterator)
                    except StopIteration:
                        object_sent = True
                        break # no more chunks to send

                    if large_chunk:
                        data, file_seq_no, is_large = large_chunk

                        is_end = True if file_seq_no == len(large_chunks) - 1 else False
                        packet = create_packet(data, nextseqnum, file_seq_no, is_large=True, is_end=is_end)
                        
                        s.sendto(packet, client_address)

                        packets_buffer[nextseqnum] = packet
                        if base == nextseqnum:
                            packet_send_times[nextseqnum] = time.time()
                        nextseqnum += 1

                    if small_chunk:
                        data, file_seq_no, is_large = small_chunk

                        is_end = True if file_seq_no == len(small_chunks) - 1 else False
                        packet = create_packet(data, nextseqnum, file_seq_no, is_large=False, is_end=is_end)
                        
                        s.sendto(packet, client_address)

                        packets_buffer[nextseqnum] = packet
                        if base == nextseqnum:
                            packet_send_times[nextseqnum] = time.time()
                        nextseqnum += 1

                while True:
                    # If all packets have been sent and acknowledged, exit loop
                    if base == nextseqnum and not packets_buffer:
                        break

                    # Listen for ACKs or wait for the timeout
                    ready = select.select([s], [], [], timeout)
                    if ready[0]:
                        # ACK received
                        ack_packet, _ = s.recvfrom(bufferSize)

                        header = ack_packet[:36]
                        data = ack_packet[36:]
                        ack_no, checksum = struct.unpack('I32s', header)
                        checksum = checksum.decode().strip('\x00')

                        # if ack packet is corrupted, ignore it and wait for another
                        if not verify_packet(data, checksum):
                            continue

                        # Slide window
                        base = ack_no + 1

                        # Remove acknowledged packets and their send times from buffers
                        to_remove = [seq for seq in packets_buffer if seq < base]
                        for seq in to_remove:
                            del packets_buffer[seq]
                            del packet_send_times[seq]

                    elif base in packet_send_times and (time.time() - packet_send_times[base]) >= timeout:
                        # Timeout occurred, resend all packets in the window
                        for seq in range(base, nextseqnum):
                            s.sendto(packets_buffer[seq], client_address)
                            packet_send_times[seq] = time.time()  # Update send time
        s.close()
if __name__ == '__main__':
    server()
