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

def create_packet(ack_no):
    data = b'This is a dummy data for ACK packet purposes'
    checksum = hashlib.md5(data).hexdigest()
    format = 'I32s'
    header = struct.pack(format, ack_no, checksum.encode())
    return header + data

def client():
    host = 'server'
    port = 8000
    bufferSize = 2048

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))

        large_obj_no = 0
        small_obj_no = 0

        while large_obj_no < 9 or small_obj_no < 9:
            expected_seq_no = 0
            received_packets = {}

            large_chunks = {}
            small_chunks = {}

            while True:
                packet, addr = s.recvfrom(2048)
                header = packet[:42]
                data = packet[42:]
                format = 'IIBB32s'
                nextseqnum, file_seq_no, is_end, packet_type, checksum = struct.unpack(format, header)
                checksum = checksum.decode().strip('\x00')

                if verify_packet(data, checksum): # packet is not corrupt
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
                            
                            # reassemble the file
                            reassemble_file(chunks, file_name)
                            # send final ACK for the end packet
                            ack_packet = create_packet(expected_seq_no - 1)
                            s.sendto(ack_packet, addr)

                            if packet_type:
                                large_obj_no += 1
                            else:
                                small_obj_no += 1
                            break
                        else:
                            # send ACK for non-end packets
                            ack_packet = create_packet(expected_seq_no - 1)
                            s.sendto(ack_packet, addr)
                    else:
                        # do not register packet, send ACK again
                        ack_packet = create_packet(expected_seq_no - 1)
                        s.sendto(ack_packet, addr)
                else:
                    # do not register packet, send ACK again
                    ack_packet = create_packet(expected_seq_no - 1)
                    s.sendto(ack_packet, addr)
        s.close()

if __name__ == '__main__':
    client()