import util

# Your program should send TTLs in the range [1, TRACEROUTE_MAX_TTL] inclusive.
# Technically IPv4 supports TTLs up to 255, but in practice this is excessive.
# Most traceroute implementations cap at approximately 30.  The unit tests
# assume you don't change this number.
TRACEROUTE_MAX_TTL = 30

# Cisco seems to have standardized on UDP ports [33434, 33464] for traceroute.
# While not a formal standard, it appears that some routers on the internet
# will only respond with time exceeeded ICMP messages to UDP packets send to
# those ports.  Ultimately, you can choose whatever port you like, but that
# range seems to give more interesting results.
TRACEROUTE_PORT_NUMBER = 33434  # Cisco traceroute port number.

# Sometimes packets on the internet get dropped.  PROBE_ATTEMPT_COUNT is the
# maximum number of times your traceroute function should attempt to probe a
# single router before giving up and moving on.
PROBE_ATTEMPT_COUNT = 3

class IPv4:
    # Each member below is a field from the IPv4 packet header.  They are
    # listed below in the order they appear in the packet.  All fields should
    # be stored in host byte order.
    #
    # You should only modify the __init__() method of this class.
    version: int
    header_len: int  # Note length in bytes, not the value in the packet.
    tos: int         # Also called DSCP and ECN bits (i.e. on wikipedia).
    length: int      # Total length of the packet.
    id: int
    flags: int
    frag_offset: int
    ttl: int
    proto: int
    cksum: int
    src: str
    dst: str

    def __init__(self, buffer: bytes):
        bitstring = ''.join(f'{byte:08b}' for byte in buffer)
        '''bitstring -> str: 
        Transfer the buffer(ipv4 header) into a continuous bitstring.
        It will be extracted to corresponding fields in the ipv4 header below.
        '''
        self.version = int(bitstring[:4], base=2)
        '''Extract the version field from the bitstring.
        Convert the first 4 bits of the bitstring to an integer.
        '''
        self.header_len = int(bitstring[4:8], base=2)
        '''Extract the header length field from the bitstring.
        Convert the next 4 bits of the bitstring to an integer.
        '''
        # self.tos = int(bitstring[16:24], base=2) # inefficient, cost twice time of type conversion
        self.tos = buffer[1] 
        '''Evaluate the type of service field from the second byte of the buffer(TOS/DSCP+ECN) directly.
        '''
        self.length = int.from_bytes(buffer[2:4], byteorder='big') 
        '''Extract the total length of the packet field from the buffer.
        Convert the next 2 bytes of the buffer to an integer.

        Notice: `byteorder='big'` is necessary to convert the bytes to an integer in network byte order(big-end).
        '''
        # self.flags = int(bitstring[48:48+3], base=2)
        # '''Extract the flags field from the bitstring.
        # Convert the next 3 bits of the bitstring to an integer.
        # '''
        # self.frag_offset = int(bitstring[51:64], base=2)
        # '''Extract the fragment offset field from the bitstring.
        # Convert the next 13 bits of the bitstring to an integer.
        # '''
        flags_frag = int.from_bytes(buffer[6:8], byteorder='big')
        '''Extract the combined flags + fragment offset field from the buffer.
        Convert bytes 7-8 of the IPv4 header to a 16-bit integer in network byte order.
        '''
        self.flags = (flags_frag >> 13) & 0x7
        '''Extract the flags field from the high 3 bits of the combined 16-bit value.
        Shift right by 13 bits, then keep only the lowest 3 bits by masking with 0x7.
        '''
        self.frag_offset = flags_frag & 0x1FFF
        '''Extract the fragment offset field from the low 13 bits of the combined value.
        Mask with 0x1FFF to keep only bits [12:0] of the original 16-bit field.
        '''
        self.ttl = buffer[8]
        '''Extract the TTL field from the buffer.
        The TTL field is the 9th byte of the buffer.
        '''
        self.proto = buffer[9]
        '''Extract the protocol field from the buffer.
        The protocol field is the 10th byte of the buffer.
        '''
        self.cksum = int.from_bytes(buffer[10:12], byteorder='big')
        '''Extract the checksum field from the buffer.
        Convert bytes 11-12 of the IPv4 header to a 16-bit integer in network byte order.
        '''
        self.src = util.inet_ntoa(buffer[12:16])
        '''Extract the source IP address field from the buffer.
        Use `util.inet_ntoa()` to convert an IP address from 32-bit packed binary format to string format.
        '''
        self.dst = util.inet_ntoa(buffer[16:20])
        '''Extract the destination IP address field from the buffer.
        The same way to above.
        '''

    def __str__(self) -> str:
        return f"IPv{self.version} (tos 0x{self.tos:x}, ttl {self.ttl}, " + \
            f"id {self.id}, flags 0x{self.flags:x}, " + \
            f"ofsset {self.frag_offset}, " + \
            f"proto {self.proto}, header_len {self.header_len}, " + \
            f"len {self.length}, cksum 0x{self.cksum:x}) " + \
            f"{self.src} > {self.dst}"


class ICMP:
    # Each member below is a field from the ICMP header.  They are listed below
    # in the order they appear in the packet.  All fields should be stored in
    # host byte order.
    #
    # You should only modify the __init__() function of this class.
    type: int
    code: int
    cksum: int

    def __init__(self, buffer: bytes):
        pass  # TODO

    def __str__(self) -> str:
        return f"ICMP (type {self.type}, code {self.code}, " + \
            f"cksum 0x{self.cksum:x})"


class UDP:
    # Each member below is a field from the UDP header.  They are listed below
    # in the order they appear in the packet.  All fields should be stored in
    # host byte order.
    #
    # You should only modify the __init__() function of this class.
    src_port: int
    dst_port: int
    len: int
    cksum: int

    def __init__(self, buffer: bytes):
        pass  # TODO

    def __str__(self) -> str:
        return f"UDP (src_port {self.src_port}, dst_port {self.dst_port}, " + \
            f"len {self.len}, cksum 0x{self.cksum:x})"

# TODO feel free to add helper functions if you'd like

def traceroute(sendsock: util.Socket, recvsock: util.Socket, ip: str) \
        -> list[list[str]]:
    """ Run traceroute and returns the discovered path.

    Calls util.print_result() on the result of each TTL's probes to show
    progress.

    Arguments:
    sendsock -- This is a UDP socket you will use to send traceroute probes.
    recvsock -- This is the socket on which you will receive ICMP responses.
    ip -- This is the IP address of the end host you will be tracerouting.

    Returns:
    A list of lists representing the routers discovered for each ttl that was
    probed.  The ith list contains all of the routers found with TTL probe of
    i+1.   The routers discovered in the ith list can be in any order.  If no
    routers were found, the ith list can be empty.  If `ip` is discovered, it
    should be included as the final element in the list.
    """

    # TODO Add your implementation
    for ttl in range(1, TRACEROUTE_MAX_TTL+1):
        util.print_result([], ttl)
    return []


if __name__ == '__main__':
    args = util.parse_args()
    ip_addr = util.gethostbyname(args.host)
    print(f"traceroute to {args.host} ({ip_addr})")
    traceroute(util.Socket.make_udp(), util.Socket.make_icmp(), ip_addr)
