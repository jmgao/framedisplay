
#include "touhou_framedisplay.h"

#include <cstdio>
#include <cstring>

static bool read_boxes(char*& data, unsigned int& n, int* nboxes, Touhou_Rect** boxes,
                       bool is_int) {
  if (is_int) {
    if (n < 4) {
      return 0;
    }

    *nboxes = *(int*)data;

    data += 4;
    n -= 4;
  } else {
    if (n < 1) {
      return 0;
    }

    *nboxes = *(unsigned char*)data;

    data += 1;
    n -= 1;
  }

  if (*nboxes > 0) {
    unsigned int c = (16 * *nboxes);
    if (n < c) {
      return 0;
    }

    *boxes = (Touhou_Rect*)(data);

    n -= c;
    data += c;
  } else {
    *nboxes = 0;
    *boxes = 0;
  }

  return 1;
}

static bool read_attack_boxes(char*& data, unsigned int& n, int* nboxes, Touhou_Rect** boxes,
                              bool is_int, int atkboxsize) {
  int max;
  if (is_int) {
    if (n < 4) {
      return 0;
    }

    max = *(int*)data;

    data += 4;
    n -= 4;
  } else {
    if (n < 1) {
      return 0;
    }

    max = *(unsigned char*)data;

    data += 1;
    n -= 1;
  }

  if (max > 0) {
    unsigned int c = (atkboxsize * max);
    if (n < c) {
      return 0;
    }

    if (max > 20) {
      max = 20;
    }

    *nboxes = max;
    char* d = data;
    for (int i = 0; i < max; ++i) {
      boxes[i] = (Touhou_Rect*)d;
      d += atkboxsize;
    }

    n -= c;
    data += c;
  } else {
    *nboxes = 0;
    *boxes = 0;
  }

  return 1;
}

static char* read_frame(char* data, char* end, Touhou_Frame* frame, int version) {
  unsigned int n = end - data;
  if (version == 1) {
    struct header_t {
      short render_frame;
      short x_offset;
      short y_offset;
      short duration;
      unsigned char identifier;
    }* header = (header_t*)data;

    if (n < 9) {
      return 0;
    }

    n -= 9;
    data += 9;

    memset(&frame->header, 0, sizeof(frame->header));

    frame->header.render_frame = header->render_frame;
    frame->header.tex_width = -1;
    frame->header.tex_height = -1;
    frame->header.x_offset = header->x_offset;
    frame->header.y_offset = header->y_offset;
    frame->header.duration = header->duration;
    frame->header.identifier = header->identifier;

    if (frame->header.identifier == 2) {
      // read type_2
      struct type_2_t {
        short blend_mode;
        unsigned char unk2;
        unsigned char unk3;
        unsigned char unk4;
        unsigned char unk5;
        short scale;
        short unk7;
        short unk8;
        short unk9;
        short unk10;
        short unk11;
        short angle;
      } __attribute__((packed))* type_2 = (type_2_t*)data;

      if (n < sizeof(type_2_t)) {
        return 0;
      }
      data += sizeof(type_2_t);
      n -= sizeof(type_2_t);

      frame->type_2.blend_mode = type_2->blend_mode;
      frame->type_2.scale = type_2->scale;
      frame->type_2.angle = type_2->angle;
    }

    struct props_t {
      short damage;
      short proration;
      short chip;
      short untech;
      short unk5;
      short unk6;
      short unk7;
      short unk8;
      short unk9;
      short unk10;
      short unk11;
      short unk12;
      short velocity_x;
      short velocity_y;
      short unk15;
      short unk16;
      short attack_type;
      unsigned int fflags;
      unsigned int aflags;

      Touhou_Rect rect;
    } __attribute__((packed))* props = (struct props_t*)data;

    if (n < sizeof(props_t)) {
      return 0;
    }
    data += sizeof(props_t);
    n -= sizeof(props_t);

    frame->props.damage = props->damage;
    frame->props.proration = props->proration;
    frame->props.chip = props->chip;
    frame->props.untech = props->untech;
    frame->props.velocity_x = props->velocity_x;
    frame->props.velocity_y = props->velocity_y;
    frame->props.attack_type = props->attack_type;
    frame->props.fflags = props->fflags;
    frame->props.aflags = props->aflags;

    frame->props.unk9 = props->unk9;
    frame->props.unk10 = props->unk10;
    frame->props.unk15 = props->unk15;
    frame->props.unk16 = props->unk16;

    frame->n_boxes_collision = 1;
    frame->boxes_collision = &props->rect;

    if (!read_boxes(data, n, &frame->n_boxes_hit, &frame->boxes_hit, 1)) {
      return 0;
    }
    if (!read_attack_boxes(data, n, &frame->n_boxes_attack, frame->boxes_attack, 1, 16)) {
      return 0;
    }
  } else {
    if (n < sizeof(frame->header)) {
      return 0;
    }

    struct header_t {
      short render_frame;
      short unk2;
      short unk3;
      short unk4;
      short tex_width;
      short tex_height;
      short x_offset;
      short y_offset;
      short duration;
      unsigned char identifier;
    } __attribute__((packed))* header = (header_t*)data;

    if (n < sizeof(header_t)) {
      return 0;
    }

    n -= sizeof(header_t);
    data += sizeof(header_t);

    memset(&frame->header, 0, sizeof(frame->header));

    frame->header.render_frame = header->render_frame;
    frame->header.tex_width = header->tex_width;
    frame->header.tex_height = header->tex_height;
    frame->header.x_offset = header->x_offset;
    frame->header.y_offset = header->y_offset;
    frame->header.duration = header->duration;
    frame->header.identifier = header->identifier;

    if (frame->header.identifier == 2) {
      if (version == 3) {
        // th105 type_2
        struct type_2_t {
          short blend_mode;
          short unk2;
          short unk3;
          short scale;
          short unk5;
          short unk6;
          short angle;
        } __attribute__((packed))* type_2 = (type_2_t*)data;

        if (n < sizeof(type_2_t)) {
          return 0;
        }
        data += sizeof(type_2_t);
        n -= sizeof(type_2_t);

        frame->type_2.blend_mode = type_2->blend_mode;
        frame->type_2.scale = type_2->scale;
        frame->type_2.angle = type_2->angle;
      } else {
        // th123 type_2
        struct type_2_t {
          short blend_mode;
          short unk2;
          short unk3;
          short unk4;
          short scale;
          short unk6;
          short unk7;
          short angle;
        } __attribute__((packed))* type_2 = (type_2_t*)data;

        if (n < sizeof(type_2_t)) {
          return 0;
        }
        data += sizeof(type_2_t);
        n -= sizeof(type_2_t);

        frame->type_2.blend_mode = type_2->blend_mode;
        frame->type_2.scale = type_2->scale;
        frame->type_2.angle = type_2->angle;
      }
    }

    struct props_t {
      short damage;
      short proration;
      short unk3;
      short unk4;
      short untech;
      short newunk5;
      short limit;
      short unk5;
      short unk6;
      short unk7;
      short unk8;
      short unk9;
      short unk10;
      short unk11;
      short unk12;
      short velocity_x;
      short velocity_y;
      short unk15;
      short unk16;
      short attack_type;
      unsigned char unk18;
      unsigned int fflags;
      unsigned int aflags;
    } __attribute__((packed))* props = (struct props_t*)data;

    if (n < sizeof(props_t)) {
      return 0;
    }
    data += sizeof(props_t);
    n -= sizeof(props_t);

    frame->props.damage = props->damage;
    frame->props.proration = props->proration;
    frame->props.limit = props->limit;
    frame->props.untech = props->untech;
    frame->props.velocity_x = props->velocity_x;
    frame->props.velocity_y = props->velocity_y;
    frame->props.attack_type = props->attack_type;
    frame->props.fflags = props->fflags;
    frame->props.aflags = props->aflags;

    frame->props.unk9 = props->unk9;
    frame->props.unk10 = props->unk10;
    frame->props.unk15 = props->unk15;
    frame->props.unk16 = props->unk16;

    if (!read_boxes(data, n, &frame->n_boxes_collision, &frame->boxes_collision, 0)) {
      return 0;
    }
    if (!read_boxes(data, n, &frame->n_boxes_hit, &frame->boxes_hit, 0)) {
      return 0;
    }
    if (!read_attack_boxes(data, n, &frame->n_boxes_attack, frame->boxes_attack, 0, 17)) {
      return 0;
    }

    // read unknown th123 box
    if (version == 5) {
      if (n < 0x1e) {
        return 0;
      }

      data += 0x1e;
      n -= 0x1e;
    }
  }

  return data;
}

static char* read_sequence(char* data, char* end, Touhou_Sequence* seq, int version) {
  int n = end - data;

  if (version == 1) {
    struct header_t {
      int identifier;
      short unk1;
      short unk2;

      unsigned int nframes;
    } __attribute__((packed))* header = (struct header_t*)data;

    if (n < 12) {
      return 0;
    }

    seq->raw_header_ptr = data;
    seq->raw_header_size = 12;

    seq->identifier = header->identifier;
    seq->nframes = header->nframes;

    data += 12;
  } else {
    // identifier has already been read
    struct header_t {
      short unk1;
      short unk2;
      unsigned char unk3;

      unsigned int nframes;
    } __attribute__((packed))* header = (struct header_t*)data;

    if (n < 9) {
      return 0;
    }

    seq->raw_header_ptr = data - 4;
    seq->raw_header_size = 13;

    seq->nframes = header->nframes;

    data += 9;
    n -= 9;
  }

  if (seq->nframes == 0) {
    seq->frames = 0;
  } else if (seq->nframes > 1024) {
    // yeah, no.
    return 0;
  } else {
    seq->frames = new Touhou_Frame[seq->nframes];

    for (int i = 0; i < seq->nframes && data; ++i) {
      seq->frames[i].raw_frame_ptr = data;

      data = read_frame(data, end, &seq->frames[i], version);

      if (data) {
        seq->frames[i].raw_frame_size = data - seq->frames[i].raw_frame_ptr;
      }
    }
  }

  return data;
}

bool Touhou_Framedata::init_th075(Touhou_Packfiles* pack, const char* name) {
  if (m_initialized) {
    return 0;
  }

  char path[256];

  sprintf(path, "data\\character\\%s\\%s.pat", name, name);

  char* data;
  unsigned int size;

  data = pack->read_file(path, &size);
  if (!data) {
    return 0;
  }

  if (size < 8) {
    delete[] data;
    return 0;
  }

  unsigned int seq_count;
  memcpy(&seq_count, data + 4, 4);

  if (seq_count < 0 || seq_count > 4096) {
    delete[] data;
    return 0;
  }

  char* d = data + 8;
  char* end = data + size;

  Touhou_Sequence* sequences = new Touhou_Sequence[seq_count];

  unsigned int i;
  for (i = 0; i < seq_count && d; ++i) {
    Touhou_Sequence* seq = &sequences[i];
    d = read_sequence(d, end, seq, 1);
  }

  if (!d) {
    delete[] data;
    delete[] sequences;
    return 0;
  }

  m_data = data;
  m_size = size;

  m_sequences = sequences;
  m_nsequences = seq_count;

  return 1;
}

bool Touhou_Framedata::init_th105_123(Touhou_Packfiles* pack, Touhou_Imagedata* imagedata,
                                      const char* name) {
  if (m_initialized) {
    return 0;
  }

  char filename[128];
  sprintf(filename, "data/character/%s/%s.pat", name, name);

  unsigned int size;
  char *data, *d, *end;

  data = pack->read_file(filename, &size);

  if (!data) {
    return 0;
  }

  end = data + size;

  char version = *data;

  if (imagedata) {
    imagedata->load_th105_from_framedata(pack, name, data + 1, size - 1);
  }

  d = data;

  int count = *(unsigned short*)(d + 1);

  d = d + 3 + (count * 128);

  if (end - d < 4) {
    return 0;
  }

  unsigned int seq_count = *(unsigned int*)d;

  d += 4;

  Touhou_Sequence* sequences = new Touhou_Sequence[seq_count];

  unsigned int i;
  for (i = 0; i < seq_count && d; ++i) {
    Touhou_Sequence* seq = &sequences[i];
    if (end - d < 4) {
      break;
    }

    memcpy(&seq->identifier, d, sizeof(seq->identifier));
    d += 4;

    if (seq->identifier == -1) {
      d += 8;  // no idea what this is.
      --i;
      continue;
    }

    d = read_sequence(d, end, seq, version);
  }

  if (!d) {
    delete[] data;
    delete[] sequences;
    return 0;
  }

  m_data = data;
  m_size = size;

  m_sequences = sequences;
  m_nsequences = i;  // not seq_count!

  return 0;
}

unsigned int Touhou_Framedata::get_sequence_count() { return m_nsequences; }

bool Touhou_Framedata::has_sequence(unsigned int n) { return (n >= 0 && n < m_nsequences); }

Touhou_Sequence* Touhou_Framedata::get_sequence(unsigned int n) {
  return has_sequence(n) ? &m_sequences[n] : 0;
}

void Touhou_Framedata::free() {
  if (m_sequences) {
    delete[] m_sequences;
    m_sequences = 0;
  }
  m_nsequences = 0;

  if (m_data) {
    delete[] m_data;
    m_data = 0;
  }
  m_size = 0;

  m_initialized = 0;
}

Touhou_Framedata::Touhou_Framedata() {
  m_initialized = 0;

  m_sequences = 0;
  m_nsequences = 0;

  m_data = 0;
  m_size = 0;
}

Touhou_Framedata::~Touhou_Framedata() {}
