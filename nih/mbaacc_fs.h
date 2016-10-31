#pragma once

#include <stdio.h>

#include <functional>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include <gsl.h>

#include "mbaacc_pack.h"
#include "util/unique_fd.h"

namespace mbaacc {

enum class NodeType {
  Directory,
  File,
};

class Directory;
class File;

// TODO: Eventually replace with std::variant<Directory, File>?
struct Node {
  Node();
  Node(std::string name);

  virtual ~Node() = default;

  std::string name;
};

class Directory : public Node {
 public:
  Directory(std::string name);
  virtual ~Directory() = default;

  Node* GetChild(const std::string& name);

  File* AddFile(const std::string& name, Pack* pack, uint32_t file_id);
  Directory* AddSubdirectory(const std::string& name);

 private:
  std::map<std::string, std::unique_ptr<Node>> children_;

 public:
  const decltype(children_)& children() const {
    return children_;
  }
};

class File : public Node {
 public:
  File(std::string name);
  virtual ~File() = default;

  gsl::span<char> Data();
  void UpdateFile(Pack* pack, uint32_t file_id);

 private:
  Pack* pack_;
  uint32_t file_id_;
};

// Each MBAACC pack file is an overlay on top of the previous one.
class FS {
 public:
  FS();
  void AddPack(std::unique_ptr<Pack> pack);

  bool Extract(const std::string& output_path);
  bool GenerateUpdatePack(FILE* output, const std::string& diff_path);

  // Callbacks for FS::Walk. Return false to end the traversal.
  // PreVisitDirectory is called before entering a directory,
  // then PreVisitDirectory/VisitFile for each child in that directory,
  // and then finally PostVisitDirectory.
  using PreVisitDirectory = std::function<bool(const std::string&, Directory*)>;
  using VisitFile = std::function<bool(std::string, File*)>;
  using PostVisitDirectory = std::function<bool(const std::string&, Directory*)>;

  bool Walk(PreVisitDirectory pre, VisitFile visit, PostVisitDirectory post);

 private:
  Directory root_;
  std::vector<std::unique_ptr<Pack>> pack_files_;
};

} /* namespace mbaacc */
