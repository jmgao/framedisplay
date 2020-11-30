#include "ougon_framedisplay.h"
#include "render.h"
#include "clone.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

struct Ougon_Character_Info {
  const char* name;
  const char* filename;
  const char* moveset_name;
};

static const Ougon_Character_Info ougon_character_info[] = {
    {"Battler", "FILES/CHAR/00.LZR", "FILES/CHAR/00.ANZ"},
    {"Ange", "FILES/CHAR/01.LZR", "FILES/CHAR/01.ANZ"},
    {"Shannon", "FILES/CHAR/02.LZR", "FILES/CHAR/02.ANZ"},
    {"Kanon", "FILES/CHAR/03.LZR", "FILES/CHAR/03.ANZ"},
    {"Lucifer", "FILES/CHAR/04.LZR", "FILES/CHAR/04.ANZ"},
    {"Chiester", "FILES/CHAR/05.LZR", "FILES/CHAR/05.ANZ"},
    {"Ronove", "FILES/CHAR/06.LZR", "FILES/CHAR/06.ANZ"},
    {"Eva-Beatrice", "FILES/CHAR/07.LZR","FILES/CHAR/07.ANZ"},
    {"Virgilia", "FILES/CHAR/08.LZR", "FILES/CHAR/08.ANZ"},
    {"Beatrice", "FILES/CHAR/09.LZR", "FILES/CHAR/09.ANZ"},
    {"George", "FILES/CHAR/10.LZR", "FILES/CHAR/10.ANZ"},
    {"Jessica", "FILES/CHAR/11.LZR", "FILES/CHAR/11.ANZ"},
    {"Rosa", "FILES/CHAR/12.LZR", "FILES/CHAR/12.ANZ"},
    {"Erika", "FILES/CHAR/13.LZR", "FILES/CHAR/13.ANZ"},
    {"Dlanor", "FILES/CHAR/14.LZR", "FILES/CHAR/14.ANZ"},
    {"Willard", "FILES/CHAR/15.LZR", "FILES/CHAR/15.ANZ"},
    {"Bernkastel", "FILES/CHAR/16.LZR", "FILES/CHAR/16.ANZ"},
    {"Lambda", "FILES/CHAR/17.LZR", "FILES/CHAR/17.ANZ"},
    {"B.Battler", "FILES/CHAR/18.LZR","FILES/CHAR/18.ANZ"},
};

static const int ougon_ncharacters = sizeof(ougon_character_info) / sizeof(ougon_character_info[0]);

const char* Ougon_FrameDisplay::get_character_name(int n) {
  if (n < 0 || n >= m_ncharacters) {
    return FrameDisplay::get_character_name(n);
  }

  return ougon_character_info[n].name;
}

const char* Ougon_FrameDisplay::get_character_long_name(int n) { return get_character_name(n); }

int Ougon_FrameDisplay::get_sequence_count() { return m_framedata.get_sequence_count(); }

bool Ougon_FrameDisplay::has_sequence(int n) { return m_framedata.has_sequence(n); }

const char* Ougon_FrameDisplay::get_sequence_name(int n) {
  if (m_initialized) {
    Ougon_Sequence* seq = m_framedata.get_sequence(n);
    if (seq) {
    }
  }
  return FrameDisplay::get_sequence_name(n);
}

const char* Ougon_FrameDisplay::get_sequence_move_name(int n, int* dmeter) {
  if (m_initialized) {
    *dmeter = 0;
    n += 1;

    switch (n) {
      case 1:
        return "5";
        break;
      case 2:
        return "6->4";
        break;
      case 3:
        return "5->2";
        break;
      case 5:
        return "2";
        break;
      case 6:
        return "3->1";
        break;
      case 7:
        return "2->5";
        break;
      case 9:
        return "6";
        break;
      case 10:
        return "4";
        break;
      case 11:
        return "8";
        break;
      case 12:
        return "9";
        break;
      case 13:
        return "7";
        break;

      case 20:
        return "66";
        break;
      case 21:
        return "44";
        break;

      case 61:
        return "5A";
        break;
      case 62:
        return "n.5A";
        break;
      case 63:
        return "6A";
        break;  // untested

      case 64:
        return "5B";
        break;
      case 65:
        return "n.5B";
        break;
      case 66:
        return "6B";
        break;

      case 67:
        return "5C";
        break;
      case 68:
        return "n.5C";
        break;
      case 69:
        return "6C";
        break;

      case 70:
        return "2A";
        break;
      case 71:
        return "n.2A";
        break;

      case 72:
        return "2B";
        break;
      case 73:
        return "n.2B";
        break;

      case 74:
        return "2C";
        break;
      case 75:
        return "3C";
        break;

      case 76:
        return "j8.A";
        break;
      case 77:
        return "j8.B";
        break;
      case 78:
        return "j8.C";
        break;
      case 79:
        return "j79.A";
        break;
      case 80:
        return "j79.B";
        break;
      case 81:
        return "j79.C";
        break;

      case 83:
        return "j.2C";
        break;

      case 118:
        return "BC";
        break;
      case 119:
        return "BC whiff";
        break;

      case 135:
        return "Dizzy";
        break;
      case 136:
        return "Appeal";
        break;

      case 154:
        return "Attack touch";
        break;

      default:
        break;
    }
  }
  return FrameDisplay::get_sequence_move_name(n, dmeter);
}

int Ougon_FrameDisplay::get_frame_count() { return m_framedata.get_frame_count(m_sequence); }

int Ougon_FrameDisplay::get_subframe() { return m_subframe_base; }

int Ougon_FrameDisplay::get_subframe_count() {
  int fr_count = m_framedata.get_frame_count(m_sequence);

  int count = 0;

  for (int i = 0; i < fr_count; ++i) {
    Ougon_Frame* frame = m_framedata.get_frame(m_sequence, i);
    if (frame) {
      count += frame->duration;
    }
  }

  return count;
}

static void copy_box_to_rect(rect_t* rect, Ougon_Hitbox* hitbox, Ougon_Frame* frame,
                             Ougon_SpriteInfo* info) {
  rect->x1 = hitbox->x + frame->tex_x;
  rect->y1 = hitbox->y + frame->tex_y;
  rect->x2 = rect->x1 - hitbox->w;
  rect->y2 = rect->y1 - hitbox->h;

  rect->x1 += hitbox->w / 2;
  rect->x2 += hitbox->w / 2;
  rect->y1 += hitbox->h / 2;
  rect->y2 += hitbox->h / 2;

  rect->x1 = -rect->x1;
  rect->x2 = -rect->x2;

  if (rect->x1 > rect->x2) {
    short t = rect->x1;
    rect->x1 = rect->x2;
    rect->x2 = t;
  }

  if (rect->y1 > rect->y2) {
    short t = rect->y1;
    rect->y1 = rect->y2;
    rect->y2 = t;
  }
}

void Ougon_FrameDisplay::set_render_properties(Texture* texture, int seq_id, int fr_id) {
  Ougon_Frame* frame = m_framedata.get_frame(seq_id, fr_id);

  if (!frame) {
    return;
  }

  Ougon_SpriteInfo* info = m_sprite_data.get_sprite_info(frame->sprite_id);
  if (!info) {
    return;
  }

  texture->special_mode(3);

  texture->offset(-frame->tex_x + (info->width / 2), frame->tex_y - (info->height / 2));
}

void Ougon_FrameDisplay::render(const RenderProperties* properties) {
  Ougon_Frame* frame = m_framedata.get_frame(m_sequence, m_frame);
  if (!frame) {
    printf("failed to get frame display::render\n");
    return;
  }

  Ougon_SpriteInfo* info = m_sprite_data.get_sprite_info(frame->sprite_id);
  if (!info) {
    printf("failed to get sprite info display::render\n");
    return;
  }

  if (properties->display_sprite) {
    if (!m_texture) {
      m_texture = m_sprite_data.get_sprite(frame->sprite_id);
    }
    if (m_texture) {
      set_render_properties(m_texture, m_sequence, m_frame);
      m_texture->draw(0, 0, 1);
    }
  }

  // render the boxes
  if (properties->display_hit_box) {
    rect_t rects[3];
    int box_count = 0;
    for (int i = 0; i < 3; ++i) {
      Ougon_Hitbox* box = &frame->hitboxes[i];
      if (box->w == 0 || box->h == 0) {
        continue;
      }

      copy_box_to_rect(&rects[box_count], box, frame, info);

      box_count++;
    }
    if (box_count > 0) {
      render_boxes(BOX_HIT, rects, box_count, properties->display_solid_boxes);
    }
  }

  if (properties->display_attack_box && frame->attackbox.w != 0 && frame->attackbox.h != 0 &&
      (frame->attack.prop1 != 0 || frame->attack.damage != 0)) {
    rect_t rect;
    copy_box_to_rect(&rect, &frame->attackbox, frame, info);
    render_boxes(BOX_ATTACK, &rect, 1, properties->display_solid_boxes);
  }
}

Clone* Ougon_FrameDisplay::make_clone() {
  Ougon_Frame* frame = m_framedata.get_frame(m_sequence, m_frame);
  if (!frame) {
    return 0;
  }

  Ougon_SpriteInfo* info = m_sprite_data.get_sprite_info(frame->sprite_id);
  if (!info) {
    return 0;
  }

  Clone* clone = new Clone;

  // do the texture
  Texture* texture = m_sprite_data.get_sprite(frame->sprite_id);
  if (texture) {
    set_render_properties(texture, m_sequence, m_frame);

    clone->init_texture(texture, 0, 0, 1);
  }

  // do hittable boxes
  CloneHitbox cloneboxes[4];
  rect_t rects[4];
  int box_count = 0;
  for (int i = 0; i < 3; ++i) {
    Ougon_Hitbox* box = &frame->hitboxes[i];
    if (box->w == 0 || box->h == 0) {
      continue;
    }

    copy_box_to_rect(&rects[box_count], box, frame, info);

    box_count++;
  }

  for (int i = 0; i < box_count; ++i) {
    cloneboxes[i].type = BOX_HIT;
  }

  if (frame->attackbox.w != 0 && frame->attackbox.h != 0 &&
      (frame->attack.prop1 != 0 || frame->attack.damage != 0)) {
    copy_box_to_rect(&rects[box_count], &frame->attackbox, frame, info);
    cloneboxes[box_count].type = BOX_ATTACK;
    box_count++;
  }

  if (box_count > 0) {
    for (int i = 0; i < box_count; ++i) {
      cloneboxes[i].rect.x1 = rects[i].x1;
      cloneboxes[i].rect.y1 = rects[i].y1;
      cloneboxes[i].rect.x2 = rects[i].x2;
      cloneboxes[i].rect.y2 = rects[i].y2;
    }

    clone->init_hitboxes(cloneboxes, box_count);
  }

  return clone;
}

void Ougon_FrameDisplay::flush_texture() {
  if (m_texture) {
    delete m_texture;
    m_texture = 0;
  }
}

const char* Ougon_FrameDisplay::get_current_sprite_filename() {
  Ougon_Frame* frame = m_framedata.get_frame(m_sequence, m_frame);
  if (!frame) {
    return 0;
  }

  const char* name = get_character_name(m_character);
  if (!name) {
    return 0;
  }

  static char namebuf[256];

  sprintf(namebuf, "%s-%4.4d.png", name, frame->sprite_id);

  return namebuf;
}

bool Ougon_FrameDisplay::save_current_sprite(const char* filename) {
  Ougon_Frame* frame = m_framedata.get_frame(m_sequence, m_frame);
  if (!frame) {
    return 0;
  }

  Texture* texture = m_sprite_data.get_sprite(frame->sprite_id);
  bool retval = 0;

  if (texture) {
    retval = texture->save_to_png(filename, 0);

    delete texture;
  }

  return retval;
}

int Ougon_FrameDisplay::save_all_character_sprites(const char* directory) {
  if (!m_initialized) {
    return 0;
  }

  int n = m_sprite_data.get_sprite_count();
  int count = 0;

  const char* name = get_character_name(m_character);
  if (!name) {
    return 0;
  }

  for (int i = 0; i < n; ++i) {
    Texture* texture = m_sprite_data.get_sprite(i);

    if (texture) {
      char filename[2048];

      sprintf(filename, "%s%s-%4.4d.png", directory, name, i);

      bool ok = texture->save_to_png(filename, 0);
      if (ok) {
        ++count;
      }

      delete texture;
    }
  }

  return count;
}

void Ougon_FrameDisplay::set_character(int n) {
  if (n < 0) {
    n = m_ncharacters - 1;
    if (n < 0) {
      return;
    }
  }
  if (n >= m_ncharacters) {
    n = 0;
  }

  if (n == m_character) {
    return;
  }

  // unload images, frame data
  m_sprite_data.free_pack();
  m_data.free_pack();
  m_framedata.free();

  // register new images, frame data
  m_sprite_data.open_pack(ougon_character_info[n].filename, m_base_path, true);
  m_data.open_pack(ougon_character_info[n].moveset_name, m_base_path, false);
  m_framedata.load(&m_data, n);

  printf("frameload complete\n");

  // finish up
  m_character = n;

  set_sequence(0);
}

void Ougon_FrameDisplay::set_sequence(int n, bool prev_dir) {
  if (!m_framedata.has_sequence(n)) {
    int count = m_framedata.get_sequence_count();
    if (!count) {
      // fail
      m_sequence = 0;
      m_frame = 0;
      return;
    }

    if (prev_dir) {
      for (int i = 0; i < count; ++i) {
        --n;
        if (n < 0) {
          n = count - 1;
        }

        if (m_framedata.has_sequence(n)) {
          break;
        }
      }
    } else {
      for (int i = 0; i < count; ++i) {
        ++n;
        if (n >= count) {
          n = 0;
        }

        if (m_framedata.has_sequence(n)) {
          break;
        }
      }
    }
  }

  m_sequence = n;

  set_frame(0);
}

void Ougon_FrameDisplay::set_frame(int n) {
  int count = m_framedata.get_frame_count(m_sequence);

  if (n < 0) {
    if (count == 0) {
      n = 0;
    } else {
      n = count - 1;
    }
  }

  if (n >= count) {
    n = 0;
  }

  m_frame = n;

  m_subframe_base = 0;
  for (int i = 0; i < n; ++i) {
    Ougon_Frame* frame = m_framedata.get_frame(m_sequence, i);

    if (frame) {
      m_subframe_base += frame->duration;
    }
  }

  Ougon_Frame* frame = m_framedata.get_frame(m_sequence, n);
  if (frame) {
    m_subframe_next = frame->duration;
  } else {
    m_subframe_next = 0;
  }

  m_subframe = 0;

  flush_texture();
}

void Ougon_FrameDisplay::render_frame_properties(bool detailed, int scr_width, int scr_height) {
  if (!m_initialized) {
    return;
  }

  Ougon_Frame* frame = m_framedata.get_frame(m_sequence, m_frame);

  if (!frame) {
    return;
  }

  char strbuf[1024];
  char* blah = strbuf;
  *blah = '\0';

  Ougon_Sequence *seq = m_framedata.get_sequence(m_sequence);

//  unsigned short *data = (unsigned short *)seq->header;
//  for (int i = 0; i < 0x44/16; ++i) {
//          sprintf(blah, "%4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x\n",
//                  data[0], data[1],
//                  data[2], data[3],
//                  data[4], data[5],
//                  data[6], data[7]);
//          blah += strlen(blah);
//          data += 8;
//  }
//
//  sprintf(blah, "%4.4x %4.4x\n\n",
//          data[0], data[1]);
//  blah += strlen(blah);
//
//  data = (unsigned short *)frame;
//  for (int i = 0; i < 0x60/16; ++i) {
//          sprintf(blah, "%4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x\n",
//                  data[0], data[1],
//                  data[2], data[3],
//                  data[4], data[5],
//                  data[6], data[7]);
//          blah += strlen(blah);
//          data += 8;
//  }

//  for (int i = 0; i < 0x12; ++i) {
//          sprintf(blah, "%d\n", frame->unknown3[i]);
//          blah += strlen(blah);
//  }


  sprintf(blah, "Duration %d\n\n", frame->duration);
  blah += strlen(blah);

  if (frame->attack.prop1 != 0 || frame->attack.damage != 0) {
    //sprintf(blah, "Attack\n  Damage %d\n\n  Props\n  %2.2x %4.4x %4.4x\n\n",
    sprintf(blah, "Attack\n  Damage %d\n\n  Props\n  %x %x %x %x\n\n",
            frame->attack.damage * 3, frame->attack.prop1, frame->attack.prop2, frame->attack.prop3,
            frame->attack.prop4);
    blah += strlen(blah);
  }

  unsigned short* d = &frame->unknown05;
  for (int i = 0x05; i < 0x0e; ++i) {
    if (*d != 0 && *d != 0x64) {
      sprintf(blah, "Unk %2.2x - %4.4x\n", i, *d);
      blah += strlen(blah);
    }
    ++d;
  }

  d = &frame->unknown1e;
  for (int i = 0x1e; i < 0x22; ++i) {
    if (*d != 0) {
      sprintf(blah, "Unk %2.2x - %4.4x\n", i, *d);
      blah += strlen(blah);
    }
    ++d;
  }

  d = &frame->unknown26;
  for (int i = 0x26; i < 0x2f; ++i) {
    if (*d != 0) {
      sprintf(blah, "Unk %2.2x - %4.4x\n", i, *d);
      blah += strlen(blah);
    }
    ++d;
  }

  render_shaded_text(scr_width - 150, 15, strbuf);
}

void Ougon_FrameDisplay::command(FrameDisplayCommand command, void* param) {
  switch (command) {
    case COMMAND_CHARACTER_NEXT:
      set_character(m_character + 1);
      break;
    case COMMAND_CHARACTER_PREV:
      set_character(m_character - 1);
      break;
    case COMMAND_CHARACTER_SET:
      if (param) {
        set_character(*(int*)param);
      }
      break;
    case COMMAND_SEQUENCE_NEXT:
      set_sequence(m_sequence + 1);
      break;
    case COMMAND_SEQUENCE_PREV:
      set_sequence(m_sequence - 1, 1);
      break;
    case COMMAND_SEQUENCE_SET:
      if (param) {
        set_sequence(*(int*)param);
      }
      break;
    case COMMAND_FRAME_NEXT:
      set_frame(m_frame + 1);
      break;
    case COMMAND_FRAME_PREV:
      set_frame(m_frame - 1);
      break;
    case COMMAND_FRAME_SET:
      if (param) {
        set_frame(*(int*)param);
      }
      break;
    case COMMAND_SUBFRAME_NEXT:
      ++m_subframe;
      if (m_subframe >= m_subframe_next) {
        set_frame(m_frame + 1);
      }
      break;
    case COMMAND_SUBFRAME_PREV:
      --m_subframe;
      if (m_subframe < 0) {
        set_frame(m_frame - 1);
        m_subframe = m_subframe_next - 1;
      }
      break;
    case COMMAND_SUBFRAME_SET:
      // eh.
      break;
    case COMMAND_PALETTE_NEXT:
    case COMMAND_PALETTE_PREV:
    case COMMAND_PALETTE_SET:
      break;
  }
}

bool Ougon_FrameDisplay::init(const char* base_path) {
  if (m_initialized) {
    return 0;
  }

  printf("Init!\n");

  // hope for the best!
  m_base_path = strdup(base_path);

//  m_data.open_patch("FILES/CHAR/DIFF.LZR", m_base_path);

  m_ncharacters = ougon_ncharacters;

  // test for jessica while we're here.
//  {
//    char lzr_filename[2048];
//    sprintf(lzr_filename, "%s%s", m_base_path, ougon_character_info[10].filename);
//
//    FILE* file = fopen(lzr_filename, "rb");
//    if (file) {
//      fclose(file);
//      m_ncharacters = 11;
//    }
//  }

  m_character = -1;

  set_character(0);

  printf("m_data.loaded() : %d\n", m_data.loaded());
  printf("m_framedata.loaded() : %d\n", m_framedata.loaded());

  m_initialized = m_data.loaded() && m_framedata.loaded();

  if (!m_initialized) {
    ::free(m_base_path);
    m_base_path = 0;
  }

  return m_initialized;
}

bool Ougon_FrameDisplay::init() { return init("./"); }

void Ougon_FrameDisplay::free() {
  m_data.free_all();
  m_framedata.free();

  flush_texture();

  m_subframe_base = 0;
  m_subframe_next = 0;
  m_subframe = 0;

  m_initialized = 0;

  if (m_base_path) {
    ::free(m_base_path);
    m_base_path = 0;
  }
}

Ougon_FrameDisplay::Ougon_FrameDisplay() {
  m_texture = 0;

  m_initialized = 0;
  m_base_path = 0;
}

Ougon_FrameDisplay::~Ougon_FrameDisplay() { free(); }
