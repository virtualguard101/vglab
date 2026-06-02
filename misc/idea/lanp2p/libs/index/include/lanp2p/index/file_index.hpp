#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "lanp2p/common/types.hpp"

/**
 * @file file_index.hpp
 * @brief Local share directory index and safe path resolution.
 *
 * The file index is responsible for:
 * - Scanning the configured share root directory and producing `FileEntry`
 *   records for listing to peers.
 * - Resolving a protocol/CLI relative path to an absolute filesystem path
 *   within the share root while preventing path traversal.
 */
namespace lanp2p::index {

/**
 * @brief Maintains a snapshot of files under a share root directory.
 *
 * This is a lightweight component used by the transfer server to answer LIST
 * requests and to locate files during downloads. The implementation may choose
 * to rescan the filesystem on `refresh()` or to apply incremental updates.
 */
class FileIndex {
 public:
  /**
   * @brief Construct an index rooted at `root`.
   * @param root Share root directory.
   */
  explicit FileIndex(std::filesystem::path root);

  /**
   * @brief Update the share root directory.
   * @param root New share root.
   */
  void set_root(std::filesystem::path root);

  /**
   * @brief Refresh the in-memory snapshot from the filesystem.
   */
  void refresh();

  /**
   * @brief Return a snapshot of the indexed entries.
   * @return Vector of file entries.
   */
  std::vector<FileEntry> entries() const;

  /**
   * @brief Resolve a relative path into an absolute path within the share root.
   *
   * Security:
   * - The function must prevent path traversal (e.g. `../secret`).
   * - Implementations should reject absolute paths and normalize separators.
   *
   * @param relative_path Relative file path from the protocol/CLI.
   * @return Absolute filesystem path when resolvable and allowed.
   */
  std::optional<std::filesystem::path> resolve(
      const std::string& relative_path) const;

 private:
  std::filesystem::path root_;
  std::vector<FileEntry> entries_;
};

}  // namespace lanp2p::index
