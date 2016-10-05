// .ENC and .PVR files:
//
// Compressed image formats for sprites and image tilemaps.

#include "mbaa_framedisplay.h"

#include <cstring>

bool MBAA_TileImage::is_8bpp() { return m_8bpp && !m_stored_palette; }

void MBAA_TileImage::copy_region_to(unsigned char* pixels, int dx, int dy, int dw, int dh, int sx,
                                    int sy, int sw, int sh, unsigned int* palette, bool is_8bpp) {
  if (m_8bpp) {
    if (m_stored_palette) {
      palette = m_stored_palette;
    } else if (!palette) {
      return;
    }
  }

  if (sx >= 256 || sy >= 256) {
    return;
  }

  if (dx < 0 || dy < 0) {
    return;
  }

  if ((sx + sw) > 256) {
    sw = 256 - sx;
  }
  if ((sy + sh) > 256) {
    sh = 256 - sy;
  }

  if ((dx + sw) > dw) {
    sw = dw - dx;
  }
  if ((dy + sh) > dh) {
    sh = dh - dy;
  }

  if (sx < 0 || sy < 0 || sw < 0 || sh < 0) {
    return;
  }

  int tile_pos = (sy * 256) + sx;

  if (is_8bpp) {
    if (!m_8bpp) {
      return;
    }

    unsigned char* dest = (unsigned char*)pixels;

    dest += (dy * dw) + dx;

    unsigned char* src = m_tile_data + tile_pos;

    int spitch = 256 - sw;
    int dpitch = dw - sw;

    for (int i = 0; i < sh; ++i) {
      for (int j = 0; j < sw; ++j) {
        *dest++ = *src++;
      }

      src += spitch;
      dest += dpitch;
    }

    return;
  }

  unsigned int* dest = (unsigned int*)pixels;

  dest += (dy * dw) + dx;

  if (m_8bpp) {
    unsigned char* src = m_tile_data + tile_pos;

    int spitch = 256 - sw;
    int dpitch = dw - sw;

    for (int i = 0; i < sh; ++i) {
      for (int j = 0; j < sw; ++j) {
        *dest++ = palette[*src++];
      }

      src += spitch;
      dest += dpitch;
    }
  } else {
    unsigned int* src = (unsigned int*)m_tile_data;
    src += tile_pos;

    int spitch = 256 - sw;
    int dpitch = dw - sw;

    for (int i = 0; i < sh; ++i) {
      for (int j = 0; j < sw; ++j) {
        *dest++ = *src++;
      }

      src += spitch;
      dest += dpitch;
    }
  }
}

bool MBAA_TileImage::load_enc(const unsigned char* data, int size) {
  if (m_loaded) {
    return 0;
  }

  // this is the stupidest RLE compression format i have ever seen.
  // and i've seen some idiotic ones.

  if (memcmp(data, "COMPRESSED", 10) || size < 0x300) {
    return 0;
  }

  const unsigned short* tile_pointers = (const unsigned short*)(data + 0x10);
  const unsigned char* tile_info = data + 0x210;
  const unsigned char* tile_pal_lookup = data + 0x310;

  if (tile_pointers[255] > size) {
    return 0;
  }

  unsigned char* tile_data = new unsigned char[256 * 256];

  for (int i = 0; i < 256; ++i) {
    unsigned char pixels[256];

    unsigned char* px = pixels;
    unsigned char* px_end = px + (16 * 16);

    const unsigned char* s = data + tile_pointers[i];

    const unsigned char* s_end;
    if (i < 255) {
      s_end = data + tile_pointers[i + 1];
    } else {
      s_end = data + size;
    }

    if (tile_info[i] == 0x80) {
      // 8bpp
      while (s < s_end) {
        unsigned char count = *s++;

        if (count > 0x80) {
          count -= 0x80;

          while (count-- > 0 && s < s_end && px < px_end) {
            *px++ = *s++;
          }
        } else {
          if (s >= s_end) {
            break;
          }

          unsigned char value = *s++;

          while (count-- > 0 && px < px_end) {
            *px++ = value;
          }
        }
      }
    } else {
      // 4bpp, lookup table
      const unsigned char* lookup = tile_pal_lookup + (tile_info[i] * 16);

      --px_end;  // optimization: to prevent (px+1) < px_end

      while (s < s_end) {
        unsigned char count = *s++;

        if (count > 0x80) {
          count -= 0x80;

          while (count-- > 0 && s < s_end && px < px_end) {
            unsigned char v1 = lookup[*s & 0xf];
            unsigned char v2 = lookup[*s >> 4];
            ++s;

            *px++ = v1;
            *px++ = v2;
          }
        } else {
          if (s >= s_end) {
            break;
          }

          unsigned char v1 = lookup[*s & 0xf];
          unsigned char v2 = lookup[*s >> 4];

          ++s;

          while (count-- > 0 && px < px_end) {
            *px++ = v1;
            *px++ = v2;
          }
        }
      }
    }

    // copy pixels to tile data
    unsigned char* dpx = tile_data + ((i / 16) * 16 * 256) + ((i % 16) * 16);
    px = pixels;

    for (int j = 0; j < 16; ++j) {
      for (int k = 0; k < 16; ++k) {
        *dpx++ = *px++;
      }
      dpx += 256 - 16;
    }
  }

  m_tile_data = tile_data;

  m_8bpp = 1;

  m_loaded = 1;

  return 1;
}

bool MBAA_TileImage::load_pvr(const unsigned char* data, int size) {
  if (memcmp(data, "GBIX", 4) || memcmp(data + 0x10, "PVRT", 4)) {
    return 0;
  }

  if (size < (0x820 + (128 * 128))) {
    return 0;
  }

  unsigned short width = *(unsigned short*)(data + 0x1c);
  unsigned short height = *(unsigned short*)(data + 0x1e);
  if (width != 256 || height != 256) {
    return 0;
  }
  unsigned char mode = *(data + 0x18);

  // initialize palette
  // image contains 1024 16 bit palette colors
  unsigned int palette[256 * 4];
  unsigned short* pal_src = (unsigned short*)(data + 0x20);

  // FIXME mode == 2 is not handled right now
  if (mode == 2) {
    // 4444
    for (int i = 0; i < 256 * 4; ++i) {
      unsigned short v = *pal_src++;
      unsigned int v2;

      v2 = ((v & 0xf000) >> 8) << 24;
      v2 |= ((v & 0x0f00) >> 4) << 16;
      v2 |= ((v & 0x00f0)) << 8;
      v2 |= ((v & 0x000f) << 4);

      v2 |= (v2 >> 4);

      palette[i] = v2;
    }
  } else if (mode == 1) {
    // 5650
    for (int i = 0; i < 256 * 4; ++i) {
      unsigned short v = *pal_src++;
      unsigned int v2;

      v2 = 0xff000000;
      v2 |= ((v & 0xf800) >> 8) << 16;
      v2 |= ((v & 0x07e0) >> 3) << 8;
      v2 |= ((v & 0x001f) << 3);

      palette[i] = v2;
    }
  } else {
    // 5551
    for (int i = 0; i < 256 * 4; ++i) {
      unsigned short v = *pal_src++;
      unsigned int v2;

      v2 = (v & 0x8000) ? 0xff000000 : 0;
      v2 |= ((v & 0x7c00) >> 7) << 16;
      v2 |= ((v & 0x0ee0) >> 2) << 8;
      v2 |= ((v & 0x001f) << 3);

      palette[i] = v2;
    }
  }

  // allocate image data
  unsigned char* tile_data = new unsigned char[256 * 256 * 4];

  // uncompress image
  unsigned char* src = (unsigned char*)(data + 0x820);
  unsigned int* px = (unsigned int*)tile_data;
  for (int i = 0; i < 128; ++i) {
    unsigned int* px2 = px + 256;

    for (int j = 0; j < 128; ++j) {
      unsigned int* p = palette + ((*src++) * 4);

      *px++ = *p++;
      *px2++ = *p++;
      *px++ = *p++;
      *px2++ = *p++;
    }

    px += 256;
  }

  m_tile_data = tile_data;

  m_8bpp = 0;

  m_loaded = 1;

  return 1;
}

bool MBAA_TileImage::load_env(const unsigned char* data, int size) {
  if (memcmp(data, "CVQ 256", 7)) {
    return 0;
  }

  if (size < 0x820) {
    return 0;
  }

  int* lookup = (int*)(data + 0x20);

  int count = 0;
  for (int i = 0; i < 256; ++i) {
    if (lookup[i] != -1) {
      ++count;
    }
  }

  if (size < 0x820 + (count * 8 * 8)) {
    return 0;
  }

  unsigned char* p_lookup = (unsigned char*)(data + 0x420);

  if (!m_stored_palette) {
    unsigned int* palette = new unsigned int[256];
    for (int i = 0; i < 256; ++i) {
      palette[i] = i | (i << 8) | (i << 16) | 0xff000000;
    }

    m_stored_palette = palette;
  }

  // allocate image data
  unsigned char* tile_data = new unsigned char[256 * 256];
  memset(tile_data, 0, 256 * 256);

  // uncompress image
  for (int i = 0; i < 256; ++i) {
    if (lookup[i] < 0) {
      continue;
    }

    unsigned char* src = (unsigned char*)(data + 0x820);
    src += lookup[i] * 8 * 8;

    unsigned char* px = tile_data;
    px += ((i / 16) * 256 * 16) + ((i % 16) * 16);

    for (int j = 0; j < 8; ++j) {
      unsigned char* px2 = px + 256;

      for (int k = 0; k < 8; ++k) {
        unsigned char* p = p_lookup + ((*src++) * 4);

        *px++ = *p++;
        *px2++ = *p++;
        *px++ = *p++;
        *px2++ = *p++;
      }

      px += 512 - 16;
    }
  }

  m_tile_data = tile_data;

  m_8bpp = 1;

  m_loaded = 1;

  return 1;
}

bool MBAA_TileImage::store_pal(const unsigned char* data, int size) {
  if (size < 1024) {
    return 0;
  }

  if (!m_stored_palette) {
    m_stored_palette = new unsigned int[256];
  }

  memcpy(m_stored_palette, data, 256 * 4);

  for (int i = 0; i < 256; ++i) {
    unsigned int v = m_stored_palette[i];
    unsigned int v2;
    unsigned int alpha = v >> 24;

    alpha *= 2;
    if (alpha > 255) {
      alpha = 255;
    }

    v2 = (alpha << 24) | (v & 0xffffff);

    m_stored_palette[i] = v2;
  }

  // this bug is ridiculous.
  unsigned int* p = m_stored_palette + 8;
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      unsigned int v = p[8];
      p[8] = p[0];
      p[0] = v;

      ++p;
    }

    p += 24;
  }

  return 1;
}

void MBAA_TileImage::free() {
  if (m_tile_data) {
    delete[] m_tile_data;
    m_tile_data = 0;
  }
  m_8bpp = 0;

  if (m_stored_palette) {
    delete[] m_stored_palette;
    m_stored_palette = 0;
  }

  m_loaded = 0;
}

MBAA_TileImage::MBAA_TileImage() {
  m_loaded = 0;

  m_tile_data = 0;

  m_stored_palette = 0;

  m_8bpp = 0;
}

MBAA_TileImage::~MBAA_TileImage() { free(); }
