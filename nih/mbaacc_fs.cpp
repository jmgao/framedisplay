#include "mbaacc_fs.h"

#include <assert.h>

#include <algorithm>
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

  auto previsit = [&output_root](const std::string& path, Directory*) {
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

// Map of directory name -> file name -> vector<char>
using ChangedFiles = std::map<std::string, std::map<std::string, std::vector<char>>>;

static ChangedFiles find_changed_files(FS* fs, const std::string& diff_path) {
  // TODO: This only checks for changed files, not new ones. At least check and warn for new files?
  ChangedFiles changed_files;
  auto visit_directory = [](const std::string&, Directory*) { return true; };
  auto visit_file = [&diff_path, &changed_files](const std::string& path, File* file) {
    auto it = std::find(path.rbegin(), path.rend(), *PATH_SEPARATOR);
    std::string::const_iterator last_separator;
    if (it == path.rend()) {
      last_separator = path.begin();
    } else {
      last_separator = (it + 1).base();
    }
    std::string directory_name(path.begin(), last_separator);
    std::string file_name = file->name;
    std::string file_path = diff_path + PATH_SEPARATOR + path;

    // TODO: Do this with mmap.
    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(file_path.c_str(), "rb"), fclose);
    if (!fp) {
      fatal_errno("failed to open file '%s'", file_path.c_str());
    }

    if (fseek(fp.get(), 0L, SEEK_END) != 0) {
      fatal_errno("fseek failed");
    }

    off_t file_size = ftell(fp.get());
    if (file_size == -1) {
      fatal_errno("ftell failed");
    }

    rewind(fp.get());

    // TODO: Provide a way to get the file size without decrypting the entire file.
    gsl::span<char> canonical_data = file->Data();
    std::vector<char> buf;

    buf.resize(file_size);
    if (fread(buf.data(), file_size, 1, fp.get()) != 1) {
      fatal_errno("fread failed when trying to read %s", file_path.c_str());
    }

    if (canonical_data.size() != size_t(file_size)) {
      goto different;
    }

    if (memcmp(buf.data(), canonical_data.data(), file_size) != 0) {
      goto different;
    }
    return true;

  different:
    fprintf(stderr, "found changed file: %s\n", path.c_str());
    changed_files[directory_name][file_name] = std::move(buf);
    return true;
  };

  if (!fs->Walk(visit_directory, visit_file, visit_directory)) {
    fatal("failed to walk MBACC filesystem");
  }

  return changed_files;
}

bool FS::GenerateUpdatePack(FILE* output, const std::string& diff_path) {
  auto changed_files = find_changed_files(this, diff_path);
  if (changed_files.empty()) {
    fatal("no changed files detected, aborting");
  }

  // Reserve spots/a byte for PackDataVersion/pack_0008_version.txt.
  size_t folders = changed_files.size() + 1;
  size_t files = 1;
  size_t data_size = 1;
  for (const auto& folder_it : changed_files) {
    for (const auto& file_it : folder_it.second) {
      ++files;
      data_size += file_it.second.size();
    }
  }

  size_t hdr_size = sizeof(PackHeader) + folders * sizeof(FolderIndex) + files * sizeof(FileIndex);
  size_t pack_size = hdr_size + data_size;
  Pack pack = Pack::Create(pack_size);
  PackHeader& header = pack.header();

  snprintf(header.name, sizeof(header.name), "FilePacHeaderA");
  header.flag = 1;
  header.xor_key = 0x4f0c30f8; // ???
  header.data_offset = hdr_size;
  header.data_size = data_size;
  printf("data_size = %zd\n", data_size);
  header.folder_count = folders;
  header.file_count = files;
  header.unknown[0] = 1;
  header.unknown[1] = 3;
  header.encrypted_length = 0x1000;

  pack.AssumeDecrypted(files);

  {
    // Create PackDataVersion/pack_0008_version.txt at the beginning.
    FolderIndex& folder = pack.folders()[0];
    snprintf(folder.filename, sizeof(folder.filename), ".\\PackDataVersion");
    folder.offset = 0;
    folder.file_start_id = 0;
    folder.size = 1;

    FileIndex& file = pack.files()[0];
    snprintf(file.filename, sizeof(file.filename), "pack_0008_version.txt");
    file.offset = 0;
    file.folder_id = 0;
    file.size = 1;
    pack.file_data(0)[0] = '3';
  }

  ssize_t rc;
  uint32_t folder_index = 1;
  uint32_t file_index = 1;
  size_t total_offset = 1;
  for (const auto& folder_it : changed_files) {
    uint32_t current_folder = folder_index++;
    FolderIndex& folder = pack.folders()[current_folder];

    // Properly mangle the directory name.
    if (folder_it.first.empty()) {
      folder.filename[0] = '.';
    } else {
      rc = snprintf(folder.filename, sizeof(folder.filename), ".\\%s", folder_it.first.c_str());
      if (rc >= sizeof(folder.filename)) {
        fatal("FolderIndex name overflow: %s", folder_it.first.c_str());
      }

      for (ssize_t i = 0; i < rc; ++i) {
        if (folder.filename[i] == '/') {
          folder.filename[i] = '\\';
        }
      }
    }

    folder.offset = total_offset;
    folder.file_start_id = file_index;
    folder.size = 0;
    for (const auto& file_it : folder_it.second) {
      uint32_t current_file = file_index++;
      FileIndex& file = pack.files()[current_file];
      rc = snprintf(file.filename, sizeof(file.filename), "%s", file_it.first.c_str());
      if (rc >= sizeof(file.filename)) {
        fatal("FileIndex name overflow: %s", file_it.first.c_str());
      }

      file.offset = total_offset;
      file.folder_id = current_folder;
      file.size = file_it.second.size();
      folder.size += file_it.second.size();
      total_offset += file_it.second.size();

      printf("adding file %s\\%s, range = [%#x-%#x]\n", folder.filename, file.filename, file.offset, file.offset + file.size);

      auto data = pack.file_data(current_file);
      assert(data.size() == file_it.second.size());
      memcpy(data.data(), file_it.second.data(), data.size());
    }
  }

  return pack.write(output);
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
