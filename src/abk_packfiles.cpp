
#include "abk_framedisplay.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

struct ABK_Packfile {
  std::string name;

  FILE* file;

  std::list<ABK_File> file_list;
};

struct ABK_File {
  ABK_Packfile* packfile;

  std::string name;

  int position;
  int size;
};

bool ABK_Packfiles::open_pack(const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    return 0;
  }

  // read entries
  unsigned int nfiles;
  fread(&nfiles, 4, 1, file);

  if (nfiles == 0 || nfiles > 3000) {
    fclose(file);
    return 0;
  }

  unsigned int size = 0x108 * nfiles;
  unsigned char* data = new unsigned char[size];
  struct entry_t {
    char filename[0x100];
    unsigned int position;
    unsigned int size;
  }* entries = (entry_t*)data;

  if (!fread(data, size, 1, file)) {
    fclose(file);

    return 0;
  }

  // register packfile
  ABK_Packfile temp_pack;

  temp_pack.file = file;
  temp_pack.name = filename;

  m_packfile_list.push_front(temp_pack);

  ABK_Packfile* packfile = &(*m_packfile_list.begin());

  // register files
  for (unsigned int i = 0; i < nfiles; ++i) {
    entry_t* entry = &entries[i];

    // decrypt
    static const unsigned char key[16] = {0x10, 0x19, 0x27, 0x30, 0x33, 0x42, 0x48, 0x59,
                                          0x5b, 0x36, 0x2b, 0x20, 0x1d, 0x0b, 0x21, 0x1c};

    for (unsigned int j = 0; j < 256; ++j) {
      entry->filename[j] -= key[j & 0xf];
    }

    entry->filename[0xff] = '\0';

    ABK_File abk_file;

    abk_file.packfile = packfile;
    abk_file.position = entry->position;
    abk_file.size = entry->size;
    abk_file.name = entry->filename;

    m_map[entry->filename] = abk_file;
  }

  // cleanup and finish
  delete[] data;

  return 1;
}

bool ABK_Packfiles::register_rsp(const char* filename) {
  std::map<std::string, ABK_File>::iterator j = m_map.find(filename);

  if (j == m_map.end()) {
    return 0;
  }

  // read entries
  ABK_File* srcfile = &(*j).second;
  FILE* file = srcfile->packfile->file;

  fseek(file, srcfile->position, SEEK_SET);

  struct rsp_entry_t {
    char filename[0x100];
    unsigned int length;
  } * entries;
  unsigned int count = 0;

  if (!fread(&count, 4, 1, file)) {
    return 0;
  }

  if (count == 0 || count > 500) {
    return 0;
  }

  entries = new rsp_entry_t[count];
  fread(entries, sizeof(rsp_entry_t), count, file);

  // register packfile
  ABK_Packfile temp_pack;

  temp_pack.file = 0;
  temp_pack.name = filename;

  m_packfile_list.push_front(temp_pack);

  ABK_Packfile* packfile = &(*m_packfile_list.begin());

  // prepare filename prefix
  int fname_len = 0;
  const char* last_slash = strrchr(filename, '/');

  if (last_slash) {
    fname_len = (last_slash - filename) + 1;
  }

  // prepare size
  unsigned int start = srcfile->position + 4 + (sizeof(rsp_entry_t) * count);

  // read files
  for (unsigned int i = 0; i < count; ++i) {
    rsp_entry_t* rspentry = &entries[i];
    char entryfilename[512];

    // get 'real' filename
    rspentry->filename[0xff] = '\0';
    if (!memcmp(rspentry->filename, "./", 2)) {
      strcpy(entryfilename, rspentry->filename);
    } else {
      memcpy(entryfilename, filename, fname_len);
      strcpy(entryfilename + fname_len, rspentry->filename);
    }

    // replace only if doesn't already exist
    ABK_File abkfile;

    abkfile.packfile = srcfile->packfile;

    abkfile.position = start;
    abkfile.size = rspentry->length;

    start += rspentry->length;

    abkfile.name = entryfilename;

    std::string fname = entryfilename;
    if (m_map.find(fname) == m_map.end()) {
      m_map[fname] = abkfile;
    }

    packfile->file_list.push_back(abkfile);
  }

  delete[] entries;

  return 1;
}

char* ABK_Packfiles::unpack_zcp(char* sdata, unsigned int size, unsigned int* dsize) {
  unsigned char* data = (unsigned char*)sdata;
  unsigned char* data_end;
  int length;

  data_end = data + size;
  data += 5;

  if ((data + 4) > data_end) {
    return 0;
  }

  length = *((unsigned int*)data);

  if (length < 0 || length > 1000000) {
    return 0;
  }

  data += 4;

  unsigned char buffer[0x1000];
  unsigned int control = 0;
  unsigned int index = 0xfee;
  unsigned char v, v2;
  unsigned char* out = new unsigned char[length];
  unsigned char* p = out;

  *dsize = length;

  memset(buffer, 0, 0x1000);

  while (length > 0 && (data < data_end)) {
    control >>= 1;
    if (!(control & 0x100)) {
      v = *data++;
      if (data == data_end) {
        break;
      }

      control = v | 0xff00;
    }

    if (control & 1) {
      v = *data++;

      *p++ = v;

      buffer[index] = v;

      index = (index + 1) & 0xfff;

      --length;
    } else {
      v = *data++;

      if (data == data_end) {
        break;
      }

      v2 = *data++;

      int value = v | ((v2 & 0xf0) << 4);
      int count = (v2 & 0x0f) + 3;

      if (count > length) {
        count = length;
      }

      length -= count;

      while (count > 0) {
        v = buffer[value];

        *p++ = v;
        buffer[index] = v;

        value = (value + 1) & 0xfff;
        index = (index + 1) & 0xfff;

        --count;
      }
    }
  }

  *dsize -= length;

  return (char*)out;
}

char* ABK_Packfiles::read_file_int(ABK_File* abkfile, unsigned int* dsize) {
  char* data = new char[abkfile->size];
  unsigned int size;

  fseek(abkfile->packfile->file, abkfile->position, SEEK_SET);
  fread(data, abkfile->size, 1, abkfile->packfile->file);

  size = abkfile->size;

  const char* filename = abkfile->name.c_str();

  int l = strlen(filename);
  if (l > 4 && !memcmp(filename + l - 4, ".zcp", 4)) {
    char* unpacked;
    unsigned int zsize;

    unpacked = unpack_zcp(data, size, &zsize);

    if (unpacked) {
      delete[] data;

      data = unpacked;
      size = zsize;
    }
  }

  *dsize = size;

  return data;
}

char* ABK_Packfiles::read_file(const char* filename, unsigned int* dsize) {
  std::map<std::string, ABK_File>::iterator j = m_map.find(filename);

  if (j == m_map.end()) {
    return 0;
  }

  return read_file_int(&(*j).second, dsize);
}

ABK_File* ABK_Packfiles::get_pack_entry(const char* packname, int n) {
  std::string name = packname;

  std::list<ABK_Packfile>::iterator i = m_packfile_list.begin();

  for (; i != m_packfile_list.end(); ++i) {
    if (i->name != name) {
      continue;
    }

    std::list<ABK_File>::iterator j = i->file_list.begin();
    for (; j != i->file_list.end(); ++j) {
      if (n == 0) {
        return &(*j);
      }
      --n;
    }
    break;
  }

  return 0;
}

char* ABK_Packfiles::read_pack_entry(const char* packname, int n, unsigned int* dsize) {
  ABK_File* file = get_pack_entry(packname, n);

  if (file) {
    return read_file_int(file, dsize);
  }

  return 0;
}

int ABK_Packfiles::get_pack_file_count(const char* packname) {
  std::string name = packname;

  std::list<ABK_Packfile>::iterator i = m_packfile_list.begin();

  for (; i != m_packfile_list.end(); ++i) {
    if (i->name != name) {
      continue;
    }

    unsigned int count = 0;

    std::list<ABK_File>::iterator j = i->file_list.begin();
    for (; j != i->file_list.end(); ++j) {
      ++count;
    }

    return count;
  }

  return 0;
}

const char* ABK_Packfiles::get_pack_file_name(const char* packname, int n) {
  ABK_File* file = get_pack_entry(packname, n);

  if (file) {
    return file->name.c_str();
  }

  return 0;
}

void ABK_Packfiles::free() {
  std::list<ABK_Packfile>::iterator i;

  while ((i = m_packfile_list.begin()) != m_packfile_list.end()) {
    if (i->file) {
      fclose(i->file);
    }

    m_packfile_list.erase(i);
  }

  m_map.empty();
}

ABK_Packfiles::ABK_Packfiles() {}

ABK_Packfiles::~ABK_Packfiles() { free(); }
