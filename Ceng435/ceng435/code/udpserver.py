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

            large_file = f'large-{i}.obj'
            small_file = f'small-{i}.obj'

            large_chunks = file_chunks(large_file, True)  # read ith large object
            small_chunks = file_chunks(small_file, False) # read ith small object

            chunk_iterator = interleave_chunks(large_chunks, small_chunks)
            for large_chunk, small_chunk in interleave_chunks(large_chunks, small_chunks):
                while nextseqnum < base + window:
                    if large_chunk:
                        data, file_seq_no, is_large = large_chunk

                        packet = None
                        if file_seq_no == len(large_chunks) - 1:
                            # end packet
                            packet = create_packet(b'', nextseqnum, 0, is_large=True, is_end=True)
                        else:
                            packet = create_packet(data, nextseqnum, file_seq_no, is_large=True, is_end=False)
                        
                        s.sendto(packet, client_address)

                        packets_buffer[nextseqnum] = packet
                        nextseqnum += 1

                        large_chunk = next(large_chunks, None)

                    if small_chunk:
                        data, file_seq_no, is_large = small_chunk

                        packet = None
                        if file_seq_no == len(small_chunks) - 1:
                            # end packet
                            packet = create_packet(b'', nextseqnum, 0, is_large=False, is_end=True)
                        else:
                            packet = create_packet(data, nextseqnum, file_seq_no, is_large=False, is_end=False)
                        
                        s.sendto(packet, client_address)

                        packets_buffer[nextseqnum] = packet
                        nextseqnum += 1

                        small_chunk = next(small_chunks, None)

                # listen for ACKS
                ready = select.select([s], [], [], timeout)
                # if there is ACK, remove packets up to ACK number from buffer
                if ready[0]:
                    ack_packet, _ = s.recvfrom(2048)
                    ack_no = struct.unpack('I', ack_packet)[0]
                    base = ack_no + 1

                    for seq in range(ack_no + 1):
                        packets_buffer.pop(seq, None)
                
                # if timeout occurs, retransmit all packets in buffer
                if not ready[0]:
                    for seq in range(base, nextseqnum):
                        s.sendto(packets_buffer[seq], client_address)

if __name__ == '__main__':
    server()
