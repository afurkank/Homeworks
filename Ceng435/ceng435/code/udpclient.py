import socket
import struct
import hashlib

def verify_packet(data, checksum):
    return hashlib.md5(data).hexdigest() == checksum

def reassemble_file(received_chunks: dict, file_name: str):
    sorted_chunks = sorted(received_chunks.items())
    file_data = b''.join(data for _, data in sorted_chunks)

    with open(file_name, 'wb') as file:
        file.write(file_data)

def client():
    host = 'server'
    port = 8000
    bufferSize = 2048

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))

        expected_seq_no = 0
        received_packets = {}
        large_chunks = {} # big boi chonkers OwO
        small_chunks = {} # cute little chunks UwU
        large_obj_no = 0
        small_obj_no = 0

        while True:
            packet, addr = s.recvfrom(2048)   
            header = packet[:42]  # header was created as 42 bytes on client side
            data = packet[42:] # the rest of the packet is payload
            format = 'IIBB32s' # < u_int - u_int - u_char - u_char - 32 byte string > = 4+4+1+1+32 = 42 bytes
            nextseqnum, file_seq_no, is_end, packet_type, checksum = struct.unpack(format, header) # unpack
            checksum = checksum.decode().strip('\x00')

            if verify_packet(data, checksum):
                if nextseqnum == expected_seq_no: # expected packet received
                    received_packets[nextseqnum] = (data, packet_type)
                    expected_seq_no += 1

                    if packet_type: 
                        large_chunks[file_seq_no] = data
                    else: 
                        small_chunks[file_seq_no] = data

                    if is_end:
                        file_name = f"received_large-{large_obj_no}.obj" if packet_type else f"received_small-{small_obj_no}.obj"
                        chunks = large_chunks if packet_type else small_chunks
                        
                        if packet_type:
                            large_obj_no += 1
                        else:
                            small_obj_no += 1
                        
                        reassemble_file(chunks, file_name)
                        # Send final ACK for the end packet
                        s.sendto(struct.pack('I', expected_seq_no - 1), addr)
                        
                        if large_obj_no == 9 and small_obj_no == 9:
                            # all objects received
                            break
                    else:
                        # send ACK for non-end packets
                        s.sendto(struct.pack('I', expected_seq_no - 1), addr)
                else:
                    # do not register packet, send ACK again
                    s.sendto(struct.pack('I', expected_seq_no - 1), addr)
            else:
                # do not register packet, send ACK again
                s.sendto(struct.pack('I', expected_seq_no - 1), addr)
        s.close()

if __name__ == '__main__':
    client()
