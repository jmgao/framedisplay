// .ARX file loader:
//
// Handles unpacking of .ARX image files and storage of its tile images.

#include "mbaa_framedisplay.h"

#include <cstring>

bool MBAA_ARX::load(MBAA_ISO* iso, const char* name) {
  char* data;
  unsigned int size;

  // load, get basic info, check validity
  if (!iso->read_file(name, &data, &size)) {
    return 0;
  }

  unsigned int count = *(int*)(data + 0x20);
  unsigned int base_pos = *(int*)(data + 0x24);

  if (((count * 0x28) + 0x28) > size) {
    delete[] data;

    // not a valid arx file.

    return 0;
  }

  // read files
  struct file_entry_t {
    char filename[32];
    unsigned int size;
    unsigned int pos;
  }* files = (file_entry_t*)(data + 0x28);

  for (unsigned int i = 0; i < count; ++i) {
    file_entry_t* file = &files[i];

    int len = strlen(file->filename);

    if (len < 4) {
      continue;
    }

    // calculate data position
    unsigned int i_pos = base_pos + file->pos;
    unsigned int i_len = file->size;
    unsigned char* i_data = (unsigned char*)(data + i_pos);

    if ((i_len + i_pos) > size) {
      continue;
    }

    // load image
    MBAA_TileImage* tileimage = 0;

    char suffix[5];
    strcpy(suffix, file->filename + len - 4);

    file->filename[len - 4] = '\0';

    MBAA_TileImage* ti = m_map[file->filename];

    if (!strcmp(suffix, ".ENC")) {
      // load enc
      MBAA_TileImage* enc = new MBAA_TileImage();
      if (enc->load_enc(i_data, i_len)) {
        tileimage = enc;
      } else {
        delete enc;
      }
    } else if (!strcmp(suffix, ".ENV")) {
      // load enc
      MBAA_TileImage* env;
      if (ti) {
        env = ti;
        ti = 0;
      } else {
        env = new MBAA_TileImage();
      }

      if (env->load_env(i_data, i_len)) {
        tileimage = env;
      } else {
        delete env;
      }
    } else if (!strcmp(suffix, ".PVR")) {
      // load pvr
      MBAA_TileImage* pvr = new MBAA_TileImage();
      if (pvr->load_pvr(i_data, i_len)) {
        tileimage = pvr;
      } else {
        delete pvr;
      }
    } else if (!strcmp(suffix, ".AGI")) {
      // load agi
    } else if (!strcmp(suffix, ".PAL")) {
      MBAA_TileImage* pal = new MBAA_TileImage();
      if (pal->store_pal(i_data, i_len)) {
        tileimage = pal;
      } else {
        delete pal;
      }
    }

    // did we load?
    if (!tileimage) {
      // nothing loaded, skip
      continue;
    }

    if (ti) {
      delete ti;
    }

    m_map[file->filename] = tileimage;
  }

  // cleanup
  delete[] data;

  return 1;
}

MBAA_TileImage* MBAA_ARX::get_tileimage(const char* filename) {
  MBAA_TileImage* image = m_map[filename];

  return image;
}

void MBAA_ARX::free() {
  for (std::map<std::string, MBAA_TileImage*>::iterator i = m_map.begin(); i != m_map.end(); ++i) {
    MBAA_TileImage* image = (*i).second;

    if (image) {
      delete image;

      (*i).second = 0;
    }
  }
  m_map.empty();

  m_loaded = 0;
}

MBAA_ARX::MBAA_ARX() { m_loaded = 0; }

MBAA_ARX::~MBAA_ARX() { free(); }
