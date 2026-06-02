#pragma once

/**
 * @file lanp2p.hpp
 * @brief Convenience umbrella header for embedding lanp2p as a library.
 *
 * Most applications will include specific module headers instead of this file.
 * This header is provided as a convenience for small tools that want to access
 * the common config/types and the high-level `core::Node`.
 */
// This file is intentionally an "umbrella" header; includes are exports.
#include "lanp2p/common/config.hpp"  // IWYU pragma: export
#include "lanp2p/common/types.hpp"   // IWYU pragma: export
#include "lanp2p/core/node.hpp"      // IWYU pragma: export
