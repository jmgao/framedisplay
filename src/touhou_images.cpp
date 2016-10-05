// touhou_images:
//
// handles storage of texture/sprite data

#include "touhou_framedisplay.h"

#include "misc.h"
#include "texture.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

struct Touhou_Image {
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int bpp;
  unsigned int size;

  bool th075;

  char* filename;

  unsigned char* data;
};

bool Touhou_Imagedata::load_th075(Touhou_Packfiles* packfile, int n) {
  if (m_initialized) {
    return 0;
  }

  // initialize and verify header
  char* data;
  unsigned int size;

  char filename[256];
  sprintf(filename, "data\\character\\%2.2d.dat", n);

  data = packfile->read_file(filename, &size);
  if (!data) {
    return 0;
  }

  if (data[0] != 2 || size < 1025) {
    delete[] data;
    return 0;
  }

  // load palettes

  m_palettes = new unsigned int*[2];

  for (int i = 0; i < 2; ++i) {
    unsigned short* s = (unsigned short*)(data + 1 + (i * 512));

    unsigned int* p = new unsigned int[256];

    m_palettes[i] = p;

    for (int j = 0; j < 256; ++j) {
      unsigned long v = *s++;
      unsigned long o;

      o = (v & 0x8000) ? 0xff000000 : 0;
      o |= ((v & 0x7c00) << 1) >> 8;
      o |= ((v & 0x03e0) << 6);
      o |= ((v & 0x001f) << 3) << 16;

      *p++ = o;
    }
  }

  m_npalettes = 2;

  // index all images for fast lookup
  char* end = data + size;
  char* d = data + 1025;
  struct header_t {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned char bpp;
    unsigned int size;
  } __attribute__((packed)) * header;

  // - first, get file count
  int count = 0;
  while ((end - d) >= 17) {
    header = (header_t*)d;

    d += 17;
    d += header->size;

    ++count;
  }

  // - read in all image data
  Touhou_Image* images = new Touhou_Image[count];
  memset(images, 0, sizeof(Touhou_Image) * count);

  Touhou_Image* image = images;

  d = data + 1025;

  for (int i = 0; i < count; ++i) {
    header = (header_t*)d;

    d += 17;

    image->width = header->width;
    image->pitch = header->width;
    image->height = header->height;
    image->bpp = (header->bpp & 0x30) ? 32 : 8;
    image->size = header->size;

    image->th075 = 1;

    image->filename = 0;
    image->data = (unsigned char*)d;

    d += image->size;

    ++image;
  }

  // finish up
  m_images = images;
  m_nimages = count;

  m_th075_data = data;
  m_th075_size = size;

  m_th075 = 1;

  m_initialized = 1;

  return 1;
}

bool Touhou_Imagedata::load_th105_from_framedata(Touhou_Packfiles* packfile, const char* charname,
                                                 char* data, int size) {
  if (m_initialized || size < 2) {
    return 0;
  }

  unsigned short count = *(unsigned short*)data;

  if (count > 2048 || size < (2 + (count * 128))) {
    return 0;
  }

  char filename[256];

  // read palettes, either 2 or 8, based on the game.
  m_palettes = new unsigned int*[8];
  m_npalettes = 8;

  for (int i = 0; i < 8; ++i) {
    sprintf(filename, "data/character/%s/palette%3.3d.pal", charname, i);

    unsigned int psize;
    char* pdata = packfile->read_file(filename, &psize);
    if (!pdata || psize < 512) {
      m_npalettes = i;
      break;
    }

    unsigned short* pal_src = (unsigned short*)(pdata + 1);
    unsigned int* palette = new unsigned int[256];

    m_palettes[i] = palette;

    for (int j = 0; j < 256; ++j) {
      unsigned long v = *pal_src++;
      unsigned long o;

      o = (v & 0x8000) ? 0xff000000 : 00;
      o |= ((v & 0x7c00) << 1) >> 8;
      o |= ((v & 0x03e0) << 6);
      o |= ((v & 0x001f) << 3) << 16;

      palette[j] = o;
    }

    delete[] pdata;

    // utsuho's cape is normally purple. fix that
    if (!strcmp(charname, "utsuho")) {
      if (palette[31] == 0xfff800f8) {
        palette[31] = 0xff201010;
      }
    }
  }

  Touhou_Image* images = new Touhou_Image[count];
  memset(images, 0, sizeof(Touhou_Image) * count);

  Touhou_Image* image = images;

  data += 2;

  sprintf(filename, "data/character/%s/", charname);
  char* fn = filename + strlen(filename);

  for (int i = 0; i < count; ++i) {
    memcpy(fn, data, 128);
    fn[128] = '\0';
    data += 128;

    char* s = strstr(fn, ".bmp");
    if (s) {
      strcpy(s, ".cv2");
    }

    image->filename = strdup(filename);
    image->data = 0;

    ++image;
  }

  // finish up

  m_images = images;
  m_nimages = count;

  m_th075 = 0;

  m_packfile = packfile;

  m_initialized = 1;

  return 1;
}

static void read_th105_image(Touhou_Packfiles* packfile, Touhou_Image* image) {
  if (!packfile || !image || !image->filename || image->data) {
    return;
  }

  char* idata;
  unsigned int isize;
  idata = packfile->read_file(image->filename, &isize);

  if (idata) {
    struct header_t {
      unsigned char type;
      unsigned int width;
      unsigned int height;
      unsigned int pitch;
      unsigned char unknown;
      unsigned short unknown2;
      unsigned char unknown3;
    } __attribute__((packed))* header = (header_t*)idata;

    image->width = header->width;
    image->pitch = header->pitch;
    image->height = header->height;
    image->bpp = header->type == 0x8 ? 8 : 32;

    image->th075 = 0;
    image->data = (unsigned char*)idata;
    image->size = isize;
  }
}

Texture* Touhou_Imagedata::get_texture(int n, int palette_no, bool palettized) {
  if (!m_initialized) {
    return 0;
  }

  if (n < 0 || n >= m_nimages) {
    return 0;
  }

  if (palette_no < 0 || palette_no >= m_npalettes) {
    return 0;
  }

  unsigned int* palette = m_palettes[palette_no];
  Touhou_Image* image = &m_images[n];
  Texture* texture = new Texture;
  bool loaded = 0;

  if (!image->th075 && !image->data && image->filename) {
    read_th105_image(m_packfile, image);
  }

  if (image->th075) {
    // RLE
    int w = image->width;
    int x = 0;
    unsigned int count = image->width * image->height;
    int nw = to_pow2(image->width);
    int nh = to_pow2(image->height);
    int inc = nw - w;
    int nsize = nw * nh;

    if (palettized && image->bpp == 8) {
      unsigned char* pixels = new unsigned char[nsize];
      memset(pixels, 0, nsize);

      unsigned char* src = image->data;
      unsigned char* dest = pixels;

      while (count > 0) {
        unsigned char c = *src++;
        unsigned int v = *src++;

        if (c > count) {
          c = count;
        }
        count -= c;

        while (c--) {
          *dest++ = v;

          ++x;
          if (x == w) {
            dest += inc;
            x = 0;
          }
        }
      }

      loaded = texture->init(pixels, nw, nh, 1);
    } else if (image->bpp == 8) {
      unsigned char* pixels = new unsigned char[nsize * 4];
      memset(pixels, 0, nsize * 4);

      unsigned char* src = image->data;
      unsigned int* dest = (unsigned int*)pixels;

      while (count > 0) {
        unsigned char c = *src++;
        unsigned int v = palette[*src++];

        if (c > count) {
          c = count;
        }
        count -= c;

        while (c--) {
          *dest++ = v;

          ++x;
          if (x == w) {
            dest += inc;
            x = 0;
          }
        }
      }

      loaded = texture->init(pixels, nw, nh, 0);
    } else {
      // so stupid
      unsigned char* pixels = new unsigned char[nsize * 4];
      memset(pixels, 0, nsize * 4);

      unsigned int* src = (unsigned int*)image->data;
      unsigned int* dest = (unsigned int*)pixels;

      while (count > 0) {
        unsigned int c = *src++;
        unsigned int v = *src++;

        if (c > count) {
          c = count;
        }
        count -= c;

        // bgr swap
        v = (v & 0xff00ff00) | ((v & 0xff0000) >> 16) | ((v & 0x0000ff) << 16);

        while (c--) {
          *dest++ = v;

          ++x;
          if (x == w) {
            dest += inc;
            x = 0;
          }
        }
      }

      loaded = texture->init(pixels, nw, nh, 0);
    }
  } else if (image->data) {
    unsigned int w = image->width;
    unsigned int h = image->height;
    unsigned int p = image->pitch;
    int nw = to_pow2(image->width);
    int nh = to_pow2(image->height);
    int nsize = nw * nh;

    if (palettized && image->bpp == 8) {
      unsigned char* pixels = new unsigned char[nsize];
      memset(pixels, 0, nsize);

      unsigned char* src = image->data + 17;
      unsigned char* dest = pixels;

      for (unsigned int y = 0; y < h; ++y) {
        memcpy(dest, src, w);

        src += p;
        dest += nw;
      }

      loaded = texture->init(pixels, nw, nh, 1);
    } else if (image->bpp == 8) {
      int inc_s = image->pitch - w;
      int inc = nw - w;

      unsigned char* pixels = new unsigned char[nsize * 4];
      memset(pixels, 0, nsize * 4);

      unsigned char* src = image->data + 17;
      unsigned int* dest = (unsigned int*)pixels;

      for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
          *dest++ = palette[*src++];
        }

        dest += inc;
        src += inc_s;
      }

      loaded = texture->init(pixels, nw, nh, 0);
    } else {
      unsigned char* pixels = new unsigned char[nsize * 4];
      memset(pixels, 0, nsize * 4);

      unsigned char* src = image->data + 17;
      unsigned char* dest = pixels;

      for (unsigned int y = 0; y < h; ++y) {
        memcpy(dest, src, w * 4);

        src += p * 4;
        dest += nw * 4;
      }

      loaded = texture->init(pixels, nw, nh, 0);
    }
  }

  if (!loaded) {
    delete texture;
    texture = 0;
  }

  return texture;
}

int Touhou_Imagedata::get_image_count() { return m_nimages; }

int Touhou_Imagedata::get_palette_count() { return m_npalettes; }

const char* Touhou_Imagedata::get_filename(int n) {
  if (!m_initialized) {
    return 0;
  }

  if (n < 0 || n >= m_nimages) {
    return 0;
  }

  return m_images[n].filename;
}

unsigned int* Touhou_Imagedata::get_palette(int n) {
  if (n < 0 || n >= m_npalettes) {
    return 0;
  }

  return m_palettes[n];
}

void Touhou_Imagedata::free() {
  if (m_palettes) {
    for (int i = 0; i < m_npalettes; ++i) {
      delete[] m_palettes[i];
    }
    delete[] m_palettes;
    m_palettes = 0;
  }
  m_npalettes = 0;

  if (m_images) {
    if (!m_th075) {
      for (int i = 0; i < m_nimages; ++i) {
        if (m_images[i].filename) {
          ::free(m_images[i].filename);
        }
        if (m_images[i].data) {
          delete[](char*)m_images[i].data;
        }
      }
    }
    delete[] m_images;
    m_images = 0;
  }
  m_nimages = 0;

  if (m_th075_data) {
    delete[] m_th075_data;
    m_th075_data = 0;
  }
  m_th075_size = 0;

  m_packfile = 0;

  m_initialized = 0;
}

Touhou_Imagedata::Touhou_Imagedata() {
  m_palettes = 0;
  m_npalettes = 0;

  m_images = 0;
  m_nimages = 0;

  m_th075_data = 0;
  m_th075_size = 0;

  m_th075 = 0;

  m_packfile = 0;

  m_initialized = 0;
}

Touhou_Imagedata::~Touhou_Imagedata() { free(); }
