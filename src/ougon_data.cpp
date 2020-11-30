#include "ougon_framedisplay.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ************************************************ COMPRESSION ALGORITHM

static const unsigned int table[24] = {0x1,     0x3,     0x7,      0xf,      0x1f,     0x3f,
                                       0x7f,    0xff,    0x1ff,    0x3ff,    0x7ff,    0xfff,
                                       0x1fff,  0x3fff,  0x7fff,   0xffff,   0x1ffff,  0x3ffff,
                                       0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff};

static unsigned int bit_marker;
static unsigned int bit_data;
static unsigned int* bit_next;

static unsigned int do_401030() {
  unsigned int sv = 0x200;
  unsigned int retval = 0;

  do {
    unsigned int value = bit_marker & bit_data;
    if (value) {
      retval |= sv;
    }

    bit_marker >>= 1;
    if (bit_marker == 0) {
      bit_marker = 0x80000000;
      bit_data = *bit_next++;
    }

    sv >>= 1;
  } while (sv != 0);

  return retval;
}

static unsigned int do_401080() {
  unsigned int count = 0;
  unsigned int sv = 0;
  unsigned int ebx = 0;

  while (1) {
    unsigned int value = bit_marker & bit_data;

    bit_marker >>= 1;
    if (bit_marker == 0) {
      bit_marker = 0x80000000;
      bit_data = *bit_next++;
    }

    if (value == 0) {
      break;
    }

    ++count;
  }

  sv = 1 << count;
  ebx = 0;

  do {
    unsigned int value = bit_marker & bit_data;
    if (value) {
      ebx |= sv;
    }

    bit_marker >>= 1;
    if (bit_marker == 0) {
      bit_marker = 0x80000000;
      bit_data = *bit_next++;
    }

    sv >>= 1;
  } while (sv != 0);

  return table[count] + ebx;
}

bool Ougon_Data::decompress(unsigned char* data, unsigned int size, unsigned char** ddest,
                            unsigned int* dsize) {
  unsigned int v1;
  unsigned int v2;
  unsigned char* input;
  unsigned int read_data;
  unsigned char *dest, *dest_orig, *dest_end;

  if (memcmp(data, "LZLR", 4)) {
    return 0;
  }

  v1 = ((unsigned int*)data)[1];
  v2 = ((unsigned int*)data)[2];

  bit_marker = 0x80000000;
  bit_next = ((unsigned int*)data) + 3;
  bit_data = *bit_next++;

  input = data + v2;

  dest_orig = new unsigned char[v1 + 0x10000];
  dest = dest_orig;
  dest_end = dest + v1;

  read_data = 0;

  while (read_data < v1) {
    unsigned int value = bit_marker & bit_data;

    bit_marker >>= 1;
    if (bit_marker == 0) {
      bit_marker = 0x80000000;
      bit_data = *bit_next++;
    }

    if (value != 0) {
      unsigned int count1 = do_401080();

      while (count1-- > 0) {
        unsigned int count2 = do_401080();
        unsigned int count3 = do_401030();
        unsigned char* src = dest - (count3 * 4);

        for (unsigned int i = 0; i < count2; ++i) {
          *(unsigned int*)dest = *(unsigned int*)src;
          src += 4;
          dest += 4;
        }

        read_data += count2 * 4;
      }
    } else {
      unsigned int count = do_401080();

      memcpy(dest, input, count * 4);
      dest += count * 4;
      input += count * 4;

      read_data += count * 4;
    }
  }

  *ddest = dest_orig;
  *dsize = v1;

  return 1;
}

// ************************************************ DATA HANDLERS

void Ougon_Data::init_sprite_info() {
  unsigned int* ptrs = (unsigned int*)m_sprite_data;
  unsigned char* data = m_sprite_data + ptrs[0];
  unsigned int* tex_ptrs = (unsigned int*)data;
  unsigned int ntex = tex_ptrs[0] / 4;

  if (m_sprite_info) {
    delete[] m_sprite_info;
    m_sprite_info = 0;
  }

  if (ntex > 0) {
    m_sprite_info = new Ougon_SpriteInfo[ntex];
    for (unsigned int i = 0; i < ntex; ++i) {
      m_sprite_info[i].width = 0;
      m_sprite_info[i].height = 0;
    }
  }

  m_sprite_count = ntex / 2;
  while (m_sprite_count > 0 && tex_ptrs[((m_sprite_count - 1) * 2) + 1] == 0) {
    m_sprite_count -= 1;
  }
}

bool Ougon_Data::load_and_decomp(const char* filename, const char* base_path, unsigned char** ddata,
                                 unsigned int* dsize) {
  char my_fname[2048];
  FILE* file;
  unsigned char *data, *uncomp;
  unsigned int size, uncomp_size;
  bool retval = 0;

  sprintf(my_fname, "%s//%s", base_path, filename);

  file = fopen(my_fname, "rb");

  if (!file) {
    return 0;
  }

  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  data = new unsigned char[size];

  fread(data, size, 1, file);

  fclose(file);

  if (decompress(data, size, &uncomp, &uncomp_size)) {
    if (*ddata) {
      delete[] * ddata;
      *ddata = 0;
    }

    *ddata = uncomp;
    *dsize = uncomp_size;

    retval = 1;
  }

  delete[] data;

  return retval;
}

bool Ougon_Data::open_patch(const char* filename, const char* base_path) {
  return load_and_decomp(filename, base_path, &m_patch_data, &m_patch_size);
}

bool Ougon_Data::open_pack(const char* filename, const char* base_path, bool isSprite) {
  printf("filename : %s (%d)\n", filename, isSprite);

  bool retval;

  if (!isSprite) {
    retval = load_and_decomp(filename, base_path, &m_data, &m_size);
  }
  else {
    retval = load_and_decomp(filename, base_path, &m_sprite_data, &m_sprite_size);
  }

  if (retval && isSprite) {
    init_sprite_info();
  }

  return retval;
}

bool Ougon_Data::get_frame_data(unsigned char** ddata, unsigned int* dsize, int character_id) {
  if (m_patch_data) {
    unsigned int* ptrs = (unsigned int*)m_patch_data;

    if (character_id < 0) {
      character_id = 0;
    }

    ptrs += character_id * 4;

    *ddata = m_patch_data + ptrs[0];
    *dsize = ptrs[1];

    return 1;
  }

  if (!m_data) {
    return 0;
  }

  unsigned int* ptrs = (unsigned int*)m_data;

  *ddata = m_data + ptrs[0];
  *dsize = ptrs[1];

  return 1;
}

int Ougon_Data::get_sprite_count() {
  if (!m_sprite_data) {
    return 0;
  }

  return m_sprite_count;
}

int sprite_dummy = 6;

Texture* Ougon_Data::get_sprite(int id) {
  if (!m_sprite_data || id < 0 || id >= m_sprite_count) {
    return 0;
  }

  unsigned int* ptrs = (unsigned int*)m_sprite_data;

  unsigned char* data = m_sprite_data + ptrs[0];
  unsigned int size = ptrs[1];
  unsigned int* tex_ptrs = (unsigned int*)data;

  data += tex_ptrs[id * 2];
  size = tex_ptrs[(id * 2) + 1];

  if (size == 0) {
    return 0;
  }

  unsigned char* tex_data;
  unsigned int tex_size;
  Texture* texture = 0;

  if (!decompress(data, size, &tex_data, &tex_size)) {
    printf("failed to decompress get_sprite\n");
    return 0;
  }

  if (!memcmp(tex_data, "DDS ", 4) && tex_size > 0x100 &&
      ((unsigned int*)tex_data)[0x04 / 4] == 0x7c && ((unsigned int*)tex_data)[0x4c / 4] == 0x20) {
    struct header_1_t {
      unsigned int flags;
      unsigned int height;
      unsigned int width;
      unsigned int pitch;
      unsigned int depth;
      unsigned int mipmapcount;
    };
    struct header_2_t {  // pixelformat
      unsigned int flags;
      unsigned int fourcc;
      unsigned int rgb_bitcount;
      unsigned int r_bitmask;
      unsigned int g_bitmask;
      unsigned int b_bitmask;
      unsigned int alpha_bitmask;

      unsigned int ddscaps1;
      unsigned int ddscaps2;
    };
    header_1_t* header_1 = (header_1_t*)(tex_data + 8);
    header_2_t* header_2 = (header_2_t*)(tex_data + 0x50);
    unsigned int size = (header_1->width * header_1->height * 4);

    if (header_2->rgb_bitcount == 32 && tex_size >= size + 0x80) {
      unsigned char* pixels = new unsigned char[size];
      memcpy(pixels, tex_data + 0x80, size);

      if (header_2->r_bitmask == 0xff0000) {
        // bgr swap needed
        unsigned int* px = (unsigned int*)pixels;
        unsigned int count = size / 4;

        while (count--) {
          unsigned int p = *px;
          *px++ = (p & 0xff00ff00) | ((p & 0xff0000) >> 16) | ((p & 0xff) << 16);
        }
      }

      m_sprite_info[id].width = header_1->width;
      m_sprite_info[id].height = header_1->height;

      texture = new Texture();

      texture->init(pixels, header_1->width, header_1->height, 0);
    }
  }

  delete[] tex_data;

  return texture;
}

Ougon_SpriteInfo* Ougon_Data::get_sprite_info(int id) {
  if (!m_sprite_data || id < 0 || id >= m_sprite_count) {
    return 0;
  }

  return &m_sprite_info[id];
}

void Ougon_Data::free_pack() {
  if (m_data) {
    delete[] m_data;
    m_data = 0;
  }
  m_size = 0;
  if (m_sprite_info) {
    delete[] m_sprite_info;
    m_sprite_info = 0;
  }
}

void Ougon_Data::free_all() {
  free_pack();

  if (m_patch_data) {
    delete[] m_patch_data;
    m_patch_data = 0;

    m_patch_size = 0;
  }
}

bool Ougon_Data::loaded() { return m_data != 0; }

Ougon_Data::Ougon_Data() {
  m_data = 0;
  m_size = 0;

  m_sprite_data = 0;
  m_sprite_size = 0;

  m_patch_data = 0;
  m_patch_size = 0;
  m_sprite_info = 0;
}

Ougon_Data::~Ougon_Data() { free_all(); }
