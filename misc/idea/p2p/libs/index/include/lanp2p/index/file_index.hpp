#pragma once

#include "lanp2p/common/types.hpp"

#include <filesystem>
#include <optional>
#include <vector>

namespace lanp2p::index {

class FileIndex {
public:
  explicit FileIndex(std::filesystem::path root);

  void set_root(std::filesystem::path root);
  void refresh();
  std::vector<FileEntry> entries() const;
  std::optional<std::filesystem::path> resolve(const std::string& relative_path) const;

private:
  std::filesystem::path root_;
  std::vector<FileEntry> entries_;
};

} // namespace lanp2p::index
