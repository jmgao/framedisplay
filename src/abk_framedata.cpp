
#include "abk_framedisplay.h"

#include <cstdio>

bool ABK_Framedata::load(ABK_Packfiles* packs, const char* pack_name) {
  char path[64];

  sprintf(path, "./data/chr/%s/1000.zcp", pack_name);

  unsigned int size;
  char* data = packs->read_file(path, &size);
  if (!data) {
    return 0;
  }

  if (size < 4) {
    delete[] data;
    return 0;
  }

  int* s = (int*)data;
  int* s_end = (int*)(data + size);

  bool success = 0;

  // allocate a table of pointers to the data, no need to
  // rearrange the data
  do {
    unsigned int count = *s++;
    if (count > 0x1000) {
      break;
    }

    // data block 1: rendering info
    m_nrenderinfo = count;
    m_renderinfo = new int*[count];

    bool fail = 0;
    for (unsigned int i = 0; i < count; ++i) {
      m_renderinfo[i] = 0;

      if ((s_end - s) < 1) {
        fail = 1;
        break;
      }

      int n = *s;

      if (n < 0 || n > 500) {
        fail = 1;
        break;
      }
      if (n == 0) {
        ++s;
        continue;
      }

      m_renderinfo[i] = s;
      ++s;

      s += 5 * n;
    }

    if (fail) {
      break;
    }

    // data block 2: sequence data
    if (s_end - s < 1) {
      break;
    }

    count = *s++;

    if (count < 0 || count > 0x1000) {
      break;
    }

    m_nsequences = count;
    m_sequences = new int*[count];

    for (unsigned int i = 0; i < count; ++i) {
      m_sequences[i] = 0;

      if ((s_end - s) < 1) {
        fail = 1;
        break;
      }

      int n = *s;

      if (n < 0 || n > 500) {
        fail = 1;
        break;
      }
      if (n == 0) {
        ++s;
        continue;
      }

      m_sequences[i] = s;
      ++s;

      s += 2 * n;
    }

    if (fail) {
      break;
    }

    // data block 3: hitbox data
    if (s_end - s < 1) {
      break;
    }

    count = *s++;

    if (count < 0 || count > 0x1000) {
      break;
    }

    m_nboxdata = count;
    m_boxdata = new int*[count];

    for (unsigned int i = 0; i < count; ++i) {
      m_boxdata[i] = 0;

      if ((s_end - s) < 1) {
        fail = 1;
        break;
      }

      int n = *s;

      if (n < 0 || n > 500) {
        fail = 1;
        break;
      }
      if (n == 0) {
        ++s;
        continue;
      }

      m_boxdata[i] = s;
      ++s;

      s += 5 * n;
    }

    if (fail) {
      break;
    }

    success = 1;
  } while (0);

  m_data = data;
  m_size = size;

  if (!success) {
    free();

    return 0;
  }

  m_initialized = 1;

  return 1;
}

int ABK_Framedata::get_sequence_count() { return m_nsequences; }

bool ABK_Framedata::has_sequence(int n) {
  return (n >= 0) && (n < m_nsequences) && m_sequences[n] != 0;
}

int ABK_Framedata::get_renderinfo_count(int seq_id) {
  if (seq_id < 0 || seq_id >= m_nrenderinfo || !m_renderinfo[seq_id]) {
    return 0;
  }
  return *m_renderinfo[seq_id];
}

ABK_RenderInfo* ABK_Framedata::get_renderinfo(int seq_id, int ri_id) {
  int count = get_renderinfo_count(seq_id);

  if (ri_id >= count || ri_id < 0) {
    return 0;
  }

  return (ABK_RenderInfo*)(m_renderinfo[seq_id] + 1 + (5 * ri_id));
}

int ABK_Framedata::get_frame_count(int seq_id) {
  if (seq_id < 0 || seq_id >= m_nsequences || !m_sequences[seq_id]) {
    return 0;
  }
  return *m_sequences[seq_id];
}

ABK_Frame* ABK_Framedata::get_frame(int seq_id, int fr_id) {
  int count = get_frame_count(seq_id);

  if (fr_id >= count || fr_id < 0) {
    return 0;
  }

  return (ABK_Frame*)(m_sequences[seq_id] + 1 + (2 * fr_id));
}

int ABK_Framedata::get_hitbox_count(int seq_id) {
  if (seq_id < 0 || seq_id >= m_nboxdata || !m_boxdata[seq_id]) {
    return 0;
  }
  return *m_boxdata[seq_id];
}

ABK_Hitbox* ABK_Framedata::get_hitbox(int seq_id, int box_id) {
  int count = get_hitbox_count(seq_id);

  if (box_id >= count || box_id < 0) {
    return 0;
  }

  return (ABK_Hitbox*)(m_boxdata[seq_id] + 1 + (5 * box_id));
}

void ABK_Framedata::free() {
  if (m_data) {
    delete[] m_data;
    m_data = 0;
  }
  m_size = 0;

  if (m_renderinfo) {
    delete[] m_renderinfo;
    m_renderinfo = 0;
  }
  m_nrenderinfo = 0;

  if (m_sequences) {
    delete[] m_sequences;
    m_sequences = 0;
  }
  m_nsequences = 0;

  if (m_boxdata) {
    delete[] m_boxdata;
    m_boxdata = 0;
  }
  m_nboxdata = 0;

  m_initialized = 0;
}

ABK_Framedata::ABK_Framedata() {
  m_data = 0;
  m_size = 0;

  m_renderinfo = 0;
  m_nrenderinfo = 0;
  m_sequences = 0;
  m_nsequences = 0;
  m_boxdata = 0;
  m_nboxdata = 0;

  m_initialized = 0;
}

ABK_Framedata::~ABK_Framedata() { free(); }
