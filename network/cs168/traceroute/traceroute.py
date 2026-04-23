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
        self.type = buffer[0]
        '''Error type, 1 byte.
        '''
        self.code = buffer[1]
        '''Error code, 1 byte.
        '''
        self.cksum = int.from_bytes(buffer[2:4], byteorder='big')
        '''Checksum, 2 bytes.
        Convert bytes 2-3 of the buffer to an integer in network byte order.
        '''

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
        self.src_port = int.from_bytes(buffer[0:2], byteorder='big')
        '''Source port, 2 bytes.
        Convert bytes 0-1 of the buffer to an integer in network byte order.
        '''
        self.dst_port = int.from_bytes(buffer[2:4], byteorder='big')
        '''Destination port, 2 bytes.
        Convert bytes 2-3 of the buffer to an integer in network byte order.
        '''
        self.len = int.from_bytes(buffer[4:6], byteorder='big')
        '''Length, 2 bytes.
        Convert bytes 4-5 of the buffer to an integer in network byte order.
        '''
        self.cksum = int.from_bytes(buffer[6:8], byteorder='big')
        '''Checksum, 2 bytes.
        Convert bytes 6-7 of the buffer to an integer in network byte order.
        '''

    def __str__(self) -> str:
        return f"UDP (src_port {self.src_port}, dst_port {self.dst_port}, " + \
            f"len {self.len}, cksum 0x{self.cksum:x})"

def check_ttl_expired(icmp: ICMP) -> bool:
    """Check if the TTL has expired.
    In ICMP packets, if the error type is 11 and the error code is 0, it means the TTL has expired.

    Args:
        icmp (ICMP): The ICMP packet.

    Returns:
        bool: True if the TTL has expired, False otherwise.
    """
    return icmp.type == 11 and icmp.code == 0

def check_unreachable(icmp: ICMP) -> bool:
    """Check if the packet is unreachable.
    In ICMP packets, if the error type is 3 and the error code is 3,
    it means the packet is unreachable (Destination Port Unreachable).

    Args:
        icmp (ICMP): The ICMP packet.

    Returns:
        bool: True if the packet is unreachable, False otherwise.
    """
    return icmp.type == 3 and icmp.code == 3

def probe_response(recvsock: util.Socket, ttl: int) -> util.typing.Optional[str]:
    """Consume ICMP responses and return the router IP for a valid probe reply.

    Packet layout used by this parser (ICMP error message):

        [Outer IPv4 header][ICMP header + 4-byte field][Quoted original packet...]
                         ^                              ^
                         |                              |
                         |-- ICMP starts at offset: outer_ipv4.header_len * 4

        Quoted original packet starts after the first 8 bytes of ICMP payload:
        [Quoted original IPv4 header][Quoted original UDP header...]
                                     ^
                                     |
                                     |-- UDP starts at:
                                         outer_ipv4.header_len * 4 + 8
                                         + quoted_ipv4.header_len * 4

    Args:
        recvsock (util.Socket): ICMP receive socket used to read incoming packets.
        ttl (int): TTL used by the probe attempt (currently kept for call-site context).

    Returns:
        Optional[str]: Source IPv4 address of the router that generated a valid
        ICMP Time Exceeded or Destination Unreachable response for this traceroute
        probe, or None if no matching response is available.
    """
    # Keep draining readable ICMP packets to avoid stale replies affecting later probes.
    while recvsock.recv_select():
        # Read one full packet; sender_addr[0] is the source IP of this ICMP reply.
        packet, sender_addr = recvsock.recvfrom()
        # Parse the outer IPv4 header (the IPv4 header of the ICMP error packet itself).
        ipv4 = IPv4(packet)

        # Only process ICMP (proto=1); other protocols are unrelated to traceroute feedback.
        if ipv4.proto != 1:
            continue
        # ICMP begins immediately after the outer IPv4 header.
        icmp = ICMP(packet[ipv4.header_len*4:])

        # Accept only the two ICMP error classes relevant to UDP traceroute:
        # - TTL expired in transit: type=11, code=0
        # - Destination port unreachable: type=3, code=3
        if check_ttl_expired(icmp=icmp) or check_unreachable(icmp=icmp):
            # ICMP error payload includes the original packet that triggered the error.
            # After the outer IPv4 header, the first 8 ICMP bytes are fixed fields;
            # the quoted original IPv4 header starts right after those 8 bytes.
            ipv4_sender = IPv4(packet[:ipv4.header_len*4 + 8:])
            # Quoted UDP header starts after: outer IPv4 + ICMP fixed 8 bytes + quoted IPv4.
            udp = UDP(packet[ipv4.header_len*4 + 8 + ipv4_sender.header_len*4:])
            # Keep only replies that quote our traceroute destination port.
            if udp.dst_port == TRACEROUTE_PORT_NUMBER:
                # Return the current hop IP (router or final destination).
                return sender_addr[0]
    return None

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

    # Accumulate one router list per TTL hop.
    discovered_routers = []
    # Probe TTL values from 1 up to the configured maximum.
    for ttl in range(1, TRACEROUTE_MAX_TTL+1):
        # Configure outgoing probe packets to expire after `ttl` hops.
        sendsock.set_ttl(ttl)
        # Store unique router IPs discovered for this specific TTL.
        routers = []
        # Send multiple probes for this TTL to tolerate packet loss.
        for _ in range(PROBE_ATTEMPT_COUNT):
            # Send a UDP traceroute probe toward the target host and destination port.
            sendsock.sendto(b"traceroute probe", (ip, TRACEROUTE_PORT_NUMBER))
            # Try to read one valid ICMP response for this probe.
            addr = probe_response(recvsock=recvsock, ttl=ttl)
            # Keep only non-empty and non-duplicate hop addresses.
            if addr and addr not in routers:
                routers.append(addr)
        # Print intermediate traceroute output for this TTL.
        util.print_result(routers=routers, ttl=ttl)
        # Persist this TTL result into the overall traceroute path.
        discovered_routers.append(routers)
        # Stop early once the destination host is reached.
        if ip in routers:
            break
    return discovered_routers


if __name__ == '__main__':
    args = util.parse_args()
    ip_addr = util.gethostbyname(args.host)
    print(f"traceroute to {args.host} ({ip_addr})")
    traceroute(util.Socket.make_udp(), util.Socket.make_icmp(), ip_addr)
