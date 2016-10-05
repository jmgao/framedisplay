// touhou_packfiles:
//
// Handles TH075 and TH105/TH123 packfiles.

#include <cstdio>
#include <cstring>
#include <string>
#include <map>

#include "touhou_framedisplay.h"
#include "prng_mt.h"

struct File_Index {
  unsigned int position;
  unsigned int size;

  unsigned char xor_key;
};

struct Touhou_Packfile_Data {
  FILE* file;

  std::map<std::string, File_Index> map;
};

static void dexorcrypt(char* data, int size, unsigned char v1, unsigned char v2, unsigned char v3) {
  for (int i = 0; i < size; ++i) {
    data[i] ^= v1;
    v1 += v2;
    v2 += v3;
  }
}

static void dexor(char* data, int size, unsigned char v) {
  for (int i = 0; i < size; ++i) {
    data[i] ^= v;
  }
}

bool Touhou_Packfiles::add_th075(const char* filename) {
  FILE* file = fopen(filename, "rb");

  if (!file) {
    return 0;
  }

  unsigned short fcount;
  fread(&fcount, 2, 1, file);

  if (fcount > 2000) {
    fclose(file);
    return 0;
  }

  int size = fcount * 108;
  char* index_buf = new char[size];
  struct file_entry_t {
    char name[100];
    unsigned int size;
    unsigned int position;
  }* entries = (file_entry_t*)index_buf;

  if (fread(index_buf, size, 1, file) <= 0) {
    delete[] index_buf;
    fclose(file);

    return 0;
  }

  dexorcrypt(index_buf, size, 100, 100, 77);

  Touhou_Packfile_Data* packfile = new Touhou_Packfile_Data;
  packfile->file = file;

  for (int i = 0; i < fcount; ++i) {
    File_Index index;

    index.position = entries[i].position;
    index.size = entries[i].size;
    index.xor_key = 0;  // no xorcrypt going on here

    entries[i].name[99] = '\0';

    packfile->map[entries[i].name] = index;
  }

  delete[] index_buf;

  m_packfile_list.push_front(packfile);

  return 1;
}

bool Touhou_Packfiles::add_th105_123(const char* filename, bool back) {
  FILE* file = fopen(filename, "rb");

  if (!file) {
    return 0;
  }

  unsigned short fcount = 0;
  unsigned long size = 0;

  fread(&fcount, 2, 1, file);
  fread(&size, 4, 1, file);

  if (fcount == 0 || fcount > 12000 || size == 0 || size > (1024 * 1024)) {
    fclose(file);
    return 0;
  }

  char* index_buf = new char[size];
  if (fread(index_buf, size, 1, file) <= 0) {
    delete[] index_buf;
    return 0;
  }

  // decrypt
  PRNG_MT mt(size + 6);
  for (unsigned int i = 0; i < size; ++i) {
    unsigned char a = mt.rand_long();
    index_buf[i] ^= a;
  }

  dexorcrypt(index_buf, size, 0xc5, 0x83, 0x53);

  // read in filenames
  char* index_end = index_buf + size;
  Touhou_Packfile_Data* packfile = new Touhou_Packfile_Data;
  packfile->file = file;

  char* p = index_buf;

  for (unsigned int i = 0; i < fcount; ++i) {
    int position, size;
    unsigned char string_len;

    if ((index_end - p) < 9) {
      break;
    }

    memcpy(&position, p, 4);
    memcpy(&size, p + 4, 4);
    memcpy(&string_len, p + 8, 1);
    p += 9;

    if ((index_end - p) < string_len) {
      break;
    }
    char string[256];

    memcpy(string, p, string_len);
    string[string_len] = '\0';

    p += string_len;

    File_Index index;

    index.position = position;
    index.size = size;
    index.xor_key = ((position >> 1) | 0x23) & 0xff;

    packfile->map[string] = index;
  }

  delete[] index_buf;

  if (back) {
    m_packfile_list.push_back(packfile);
  } else {
    m_packfile_list.push_front(packfile);
  }

  return 1;
}

bool Touhou_Packfiles::has_file(const char* filename) {
  std::string fname = filename;

  std::list<Touhou_Packfile_Data*>::iterator i = m_packfile_list.begin();
  for (; i != m_packfile_list.end(); ++i) {
    std::map<std::string, File_Index>::iterator j = (*i)->map.find(fname);

    if (j != (*i)->map.end()) {
      return 1;
    }
  }

  return 0;
}

char* Touhou_Packfiles::read_file(const char* filename, unsigned int* dsize) {
  std::string fname = filename;
  std::list<Touhou_Packfile_Data*>::iterator i = m_packfile_list.begin();
  File_Index* index = 0;
  for (; i != m_packfile_list.end(); ++i) {
    std::map<std::string, File_Index>::iterator j = (*i)->map.find(fname);

    if (j != (*i)->map.end()) {
      index = &(*j).second;

      break;
    }
  }

  if (!index) {
    return 0;
  }

  if (fseek((*i)->file, index->position, SEEK_SET) != 0) {
    return 0;
  }

  char* data = new char[index->size];
  unsigned int size = index->size;

  if (fread(data, size, 1, (*i)->file) <= 0) {
    delete[] data;
    return 0;
  }

  *dsize = size;

  if (index->xor_key != 0) {
    dexor(data, size, index->xor_key);
  }

  // th075.musicroom: dexorcrypt(d, s, 0x5c, 0x5a, 0x3d);
  // th075.cardlist: dexorcrypt(d, s, 0x60, 0x61, 0x41);
  // th075.scenario: dexorcrypt(d, s, 0x63, 0x62, 0x42);
  // th105.cv0/.cv1: dexorcrypt(d, s, 0x7b, 0x71, 0x95);

  return data;
}

void Touhou_Packfiles::free() {
  std::list<Touhou_Packfile_Data*>::iterator i;

  while ((i = m_packfile_list.begin()) != m_packfile_list.end()) {
    fclose((*i)->file);

    delete *i;

    m_packfile_list.erase(i);
  }
}

Touhou_Packfiles::Touhou_Packfiles() {}

Touhou_Packfiles::~Touhou_Packfiles() { free(); }
