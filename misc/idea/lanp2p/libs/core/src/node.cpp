#include "lanp2p/core/node.hpp"

#include "lanp2p/common/defaults.hpp"
#include "lanp2p/common/identity.hpp"

namespace lanp2p::core {

Node::Node(AppConfig config)
    : config_(std::move(config)),
      work_(boost::asio::make_work_guard(io_)),
      index_(config_.share_dir),
      maintenance_timer_(io_) {
  if (config_.node_id.empty()) config_.node_id = default_node_id();
  if (config_.hostname.empty()) config_.hostname = default_hostname();
}

int Node::run_daemon() {
  discovery_only_ = false;
  discovery_ = std::make_unique<discovery::Service>(
      io_, registry_, config_.udp_port, config_.udp_port, config_.tcp_port,
      config_.node_id, config_.hostname);
  if (!transfer_) {
    transfer_ =
        std::make_unique<transfer::Server>(io_, config_.tcp_port, index_);
  }
  discovery_->start();
  transfer_->start();
  schedule_maintenance();
  io_thread_ = std::thread([this]() { io_.run(); });
  return 0;
}

int Node::run_discovery_only() {
  discovery_only_ = true;
  discovery_ = std::make_unique<discovery::Service>(
      io_, registry_, 0, config_.udp_port, config_.tcp_port, config_.node_id,
      config_.hostname);
  discovery_->start();
  schedule_maintenance();
  io_thread_ = std::thread([this]() { io_.run(); });
  return 0;
}

void Node::request_stop() {
  if (stopping_.exchange(true)) return;
  boost::system::error_code ec;
  (void)maintenance_timer_.cancel();
  if (discovery_) discovery_->stop();
  if (!discovery_only_ && transfer_) transfer_->stop();
  work_.reset();
  io_.stop();
  if (io_thread_.joinable()) io_thread_.join();
}

void Node::schedule_maintenance() {
  maintenance_timer_.expires_after(kMaintenanceInterval);
  maintenance_timer_.async_wait([this](boost::system::error_code ec) {
    if (stopping_ || ec) return;
    maintenance_tick();
    schedule_maintenance();
  });
}

void Node::maintenance_tick() {
  registry_.expire(kPeerTtl);
  index_.refresh();
}

peer::Registry& Node::peers() { return registry_; }
index::FileIndex& Node::files() { return index_; }

}  // namespace lanp2p::core
