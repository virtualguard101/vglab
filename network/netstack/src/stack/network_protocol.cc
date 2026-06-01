/**
 * @file network_protocol.cc
 */

#include "netstack/stack/network_protocol.hh"

namespace netstack::stack {

void RecordingTransportDispatcher::DeliverTransportPacket(
    Route* route, TransportProtocolNumber protocol, PacketBuffer pkt) {
  (void)route;
  entries_.push_back(Entry{protocol, pkt.Data().size()});
}

}  // namespace netstack::stack
