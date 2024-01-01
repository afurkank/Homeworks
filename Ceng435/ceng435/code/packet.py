import struct
import hashlib
class Packet:
    def __init__(self, global_seq_no, file_seq_no, packet_type, checksum) -> None:
        self.global_seq_no = global_seq_no
        self.file_seq_no = file_seq_no
        self.packet_type = packet_type
        self.checksum = checksum


    def get_header():
        pass
    def get_payload():
        pass
    def create_packet(data, global_seq_no, file_seq_no, is_large):
        packet_type = 1 if is_large else 0
        checksum = hashlib.md5(data).hexdigest()
        format = 'IIB32s' # < u_int - u_int - u_char - 32 byte string > = 4+4+1+32 = 41 bytes
        header = struct.pack(format, global_seq_no, file_seq_no, packet_type, checksum.encode())
        return header + data