#include "lanp2p/index/file_index.hpp"

#include <system_error>

namespace lanp2p::index {

FileIndex::FileIndex(std::filesystem::path root) : root_(std::move(root)) {
  refresh();
}

void FileIndex::set_root(std::filesystem::path root) {
  root_ = std::move(root);
  refresh();
}

void FileIndex::refresh() {
  entries_.clear();
  std::error_code ec;
  if (!std::filesystem::exists(root_, ec)) {
    std::filesystem::create_directories(root_, ec);
  }
  if (!std::filesystem::is_directory(root_, ec)) {
    return;
  }

  for (const auto& entry : std::filesystem::recursive_directory_iterator(
           root_, std::filesystem::directory_options::skip_permission_denied,
           ec)) {
    if (ec) break;
    if (!entry.is_regular_file(ec) || ec) continue;
    const auto rel = std::filesystem::relative(entry.path(), root_, ec);
    if (ec) continue;
    const auto path_str = rel.generic_string();
    if (path_str.empty() || path_str.front() == '.') continue;
    entries_.push_back(FileEntry{path_str, entry.file_size(ec)});
  }
}

std::vector<FileEntry> FileIndex::entries() const { return entries_; }

std::optional<std::filesystem::path> FileIndex::resolve(
    const std::string& relative_path) const {
  if (relative_path.empty() || relative_path.find("..") != std::string::npos) {
    return std::nullopt;
  }
  std::filesystem::path candidate = root_ / relative_path;
  std::error_code ec;
  candidate = std::filesystem::weakly_canonical(candidate, ec);
  if (ec) return std::nullopt;
  const auto root_canon = std::filesystem::weakly_canonical(root_, ec);
  if (ec) return std::nullopt;
  auto mismatch =
      std::mismatch(root_canon.begin(), root_canon.end(), candidate.begin());
  if (mismatch.first != root_canon.end()) return std::nullopt;
  if (!std::filesystem::is_regular_file(candidate, ec) || ec)
    return std::nullopt;
  return candidate;
}

}  // namespace lanp2p::index
