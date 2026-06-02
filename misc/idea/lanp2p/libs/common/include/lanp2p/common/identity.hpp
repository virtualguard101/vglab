#pragma once

#include <string>

/**
 * @file identity.hpp
 * @brief Helpers for generating default node identity information.
 *
 * These functions provide best-effort defaults used by the CLI when the user
 * does not explicitly configure `AppConfig::node_id` or `AppConfig::hostname`.
 *
 * The resulting strings are intended for display and basic identification on a
 * trusted LAN; they are not cryptographic identities.
 */
namespace lanp2p {

/**
 * @brief Determine a best-effort hostname for the current machine.
 * @return Hostname string (may fall back to a generic placeholder).
 */
std::string default_hostname();

/**
 * @brief Generate a default node id for the current machine/user session.
 * @return Node id string.
 */
std::string default_node_id();

}  // namespace lanp2p
