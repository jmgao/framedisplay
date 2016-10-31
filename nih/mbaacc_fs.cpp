#include "mbaacc_fs.h"

#include <functional>
#include <string>
#include <memory>
#include <vector>

#include <gsl.h>

#include "mbaacc_pack.h"
#include "util/fatal.h"
#include "util/mkdirs.h"
#include "util/split.h"

namespace mbaacc {

Node::Node() : Node(std::string()) {
}

Node::Node(std::string name) : name(std::move(name)) {
}

Directory::Directory(std::string name) : Node(std::move(name)) {
}

Node* Directory::GetChild(const std::string& name) {
  auto it = children_.find(name);
  if (it != children_.end()) {
    return it->second.get();
  }
  return nullptr;
}

File* Directory::AddFile(const std::string& name, Pack* pack, uint32_t file_id) {
  Node* node = GetChild(name);
  File* file = dynamic_cast<File*>(node);

  if (!node) {
    std::unique_ptr<File> ptr = std::make_unique<File>(name);
    file = ptr.get();
    children_.emplace(name, std::move(ptr));
  }

  if (!file) {
    fatal("non-file '%s' already exists", name.c_str());
  }

  file->UpdateFile(pack, file_id);
  return file;
}

Directory* Directory::AddSubdirectory(const std::string& name) {
  Node* node = GetChild(name);
  Directory* dir = dynamic_cast<Directory*>(node);

  if (!node) {
    std::unique_ptr<Directory> ptr = std::make_unique<Directory>(name);
    dir = ptr.get();
    children_.emplace(name, std::move(ptr));
  }

  if (!dir) {
    fatal("non-directory '%s' already exists", name.c_str());
  }

  return dir;
}

File::File(std::string name) : Node(std::move(name)) {
}

gsl::span<char> File::Data() {
  return pack_->file_data(file_id_);
}

void File::UpdateFile(Pack* pack, uint32_t file_id) {
  pack_ = pack;
  file_id_ = file_id;
}

FS::FS() : root_(std::string()) {
}

void FS::AddPack(std::unique_ptr<Pack> pack) {
  gsl::span<FolderIndex> folder_entries = pack->folders();
  std::vector<Directory*> directories;
  for (const FolderIndex& folder : folder_entries) {
    Directory* dir = &root_;
    std::vector<gsl::cstring_span> components = Split(folder.filename, "\\");
    for (gsl::cstring_span component : components) {
      if (strncmp(".", component.data(), component.size()) == 0) {
        // For whatever reason, leading .'s are in some of the packfiles.
        continue;
      } else if (strncmp("..", component.data(), component.size()) == 0) {
        fatal("directory has .. component: %.*s", int(sizeof(folder.filename)), folder.filename);
      }

      if (!component.empty()) {
        dir = dir->AddSubdirectory(to_string(component));
      }
    }

    directories.push_back(dir);
  }

  gsl::span<FileIndex> file_entries = pack->files();

  for (size_t i = 0; i < file_entries.size(); ++i) {
    const FileIndex& file = file_entries[i];
    if (file.folder_id >= directories.size()) {
      fatal("folder id out of range");
    }

    Directory* dir = directories[file.folder_id];
    dir->AddFile(file.filename, pack.get(), i);
  }

  pack_files_.push_back(std::move(pack));
}

bool FS::Extract(const std::string& out) {
  std::string output_root = out;
  if (output_root.back() != *PATH_SEPARATOR) {
    output_root += PATH_SEPARATOR;
  }

  auto previsit = [&output_root](const std::string& path, Directory* dir) {
    std::string dir_path = output_root + path;
    if (mkdir(dir_path.c_str(), 0700) != 0 && errno != EEXIST) {
      fatal("failed to create directory '%s'", output_root.c_str());
    }
    return true;
  };
  auto postvisit = [](const std::string&, Directory*) { return true; };
  auto visit = [&output_root](const std::string &path, File* file) {
    std::string file_output_path = output_root + path;
    FILE* out = fopen(file_output_path.c_str(), "wb");
    if (!out) {
      fatal_errno("failed to create file '%s'", file_output_path.c_str());
    }

    auto data = file->Data();
    if (fwrite(data.data(), data.size(), 1, out) != 1) {
      fatal_errno("write failed");
    }
    fclose(out);
    return true;
  };

  return Walk(previsit, visit, postvisit);
}

bool FS::GenerateUpdatePack(FILE* output, const std::string& diff_path) {
  // TODO: This only checks for changed files, not new ones. At least check and warn for new files?

  fatal("GenerateUpdatePack unimplemented");
}

static bool recursive_walk(Directory* current, const std::string& path, FS::PreVisitDirectory pre,
                           FS::VisitFile visit, FS::PostVisitDirectory post) {
  if (!pre(path, current)) {
    return false;
  }

  for (const auto& it : current->children()) {
    Node* node = it.second.get();
    std::string node_path = path;
    if (!node->name.empty()) {
      if (!node_path.empty() && node_path.back() != *PATH_SEPARATOR) {
        node_path += PATH_SEPARATOR;
      }
      node_path += node->name;
    }

    if (File* file = dynamic_cast<File*>(node)) {
      if (!visit(node_path, file)) {
        return false;
      }
    } else if (Directory* dir = dynamic_cast<Directory*>(node)) {
      if (!recursive_walk(dir, node_path, pre, visit, post)) {
        return false;
      }
    } else {
      fatal("unknown Node type");
    }
  }

  return post(path, current);
}

bool FS::Walk(FS::PreVisitDirectory pre, FS::VisitFile visit, FS::PostVisitDirectory post) {
  return recursive_walk(&root_, "", pre, visit, post);
}

} /* namespace mbaacc */
