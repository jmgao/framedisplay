// touhou_framedisplay.cpp:
//
// Core Touhou fighting games manager.

#include "touhou_framedisplay.h"

#ifdef WIN32
#include <windows.h>
#endif

#include "render.h"
#include "clone.h"

#include <cstdio>
#include <cstring>

struct Touhou_Character_Info {
  const char* name;

  const char* long_name;
};

static const Touhou_Character_Info touhou_character_info[] = {
    // th075 chars
    {"reimu", "Reimu Hakurei"},
    {"marisa", "Marisa Kirisame"},
    {"sakuya", "Sakuya Izayoi"},
    {"alice", "Alice Margatroid"},
    {"patchouli", "Patchouli Knowledge"},
    {"youmu", "Youmu Konpaku"},
    {"remilia", "Remilia Scarlet"},
    {"yuyuko", "Yuyuko Saigyouji"},
    {"yukari", "Yukari Yakumo"},
    {"suika", "Suika Ibuki"},

    // th075 1.11
    {"meiling", "Hong Meiling"},

    // th105 chars
    {"aya", "Aya Shameimaru"},
    {"udonge", "Reisen Udongein Inaba"},
    {"komachi", "Komachi Onoduka"},
    {"iku", "Iku Nagae"},
    {"tenshi", "Tenshi Hinanai"},

    // th123 chars
    {"sanae", "Sanae Kotiya"},
    {"meirin", "Hong Meiling"},
    {"chirno", "Cirno"},
    {"suwako", "Suwako Moriya"},
    {"utsuho", "Utsuho Reiuji"},
};

const int touhou_ncharacter_info = sizeof(touhou_character_info) / sizeof(touhou_character_info[0]);

static int get_real_info_n(Touhou_GameID id, int n) {
  switch (id) {
    case TOUHOU_GAME_TH075:
    case TOUHOU_GAME_TH075_11:
      return n;
    case TOUHOU_GAME_TH105:
    case TOUHOU_GAME_TH123_105:
      if (n >= 10) {
        ++n;
      }
      return n;
    case TOUHOU_GAME_TH123: {
      int table[9] = {0, 1, 3, 4, 16, 17, 18, 19, 20};
      if (n < 0 || n >= 9) {
        return 0;
      }

      return table[n];
    }
    default:
      break;
  }

  return 0;
}

const char* Touhou_FrameDisplay::get_character_long_name(int n) {
  if (n >= 0 && n < m_max_character) {
    n = get_real_info_n(m_game_id, n);

    return touhou_character_info[n].long_name;
  }

  return FrameDisplay::get_character_long_name(n);
}

const char* Touhou_FrameDisplay::get_character_name(int n) {
  if (n >= 0 && n < m_max_character) {
    n = get_real_info_n(m_game_id, n);

    return touhou_character_info[n].name;
  }

  return FrameDisplay::get_character_name(n);
}

int Touhou_FrameDisplay::get_sequence_count() {
  if (!m_initialized) {
    return FrameDisplay::get_sequence_count();
  }

  return m_framedata.get_sequence_count();
}

bool Touhou_FrameDisplay::has_sequence(int n) {
  if (!m_initialized) {
    return 0;
  }

  return m_framedata.has_sequence(n);
}

const char* Touhou_FrameDisplay::get_sequence_name(int n) {
  if (m_initialized) {
    Touhou_Sequence* seq = m_framedata.get_sequence(n);
    if (seq) {
    }
  }
  return FrameDisplay::get_sequence_name(n);
}

static const char* get_th075_move_name(int character, int id) {
  switch (id) {
    case 0:
      return "5";
      break;
    case 1:
      return "5->2";
      break;
    case 2:
      return "2";
      break;
    case 3:
      return "2->5";
      break;
    case 4:
      return "6";
      break;
    case 5:
      return "4";
      break;
    case 6:
      return "8";
      break;
    case 7:
      return "9";
      break;
    case 8:
      return "7";
      break;
    case 9:
      return "air";
      break;
    case 10:
      return "land";
      break;
    case 40:
      return "name";
      break;
    case 41:
      return "portrait";
      break;
    case 42:
      return "spellcard 1";
      break;
    case 43:
      return "spellcard 2";
      break;
    case 44:
      return "spellcard 3";
      break;
    case 45:
      return "spellcard 4";
      break;
    case 46:
      return "spellcard 5";
      break;
    case 47:
      return "spellcard 6";
      break;
    case 50:
      return "5 hitstun light";
      break;
    case 51:
      return "5 hitstun medium";
      break;
    case 52:
      return "5 hitstun heavy";
      break;
    case 53:
      return "5 hitstun reel";
      break;
    case 62:
      return "2 hitstun light";
      break;
    case 63:
      return "2 hitstun medium";
      break;
    case 64:
      return "2 hitstun heavy";
      break;
    case 65:
      return "2 hitstun reel";
      break;
    case 93:
      return "wakeup roll";
      break;
    case 94:
      return "wakeup roll";
      break;
    case 95:
      return "wakeup 8";
      break;
    case 150:
      return "5 blockstun light";
      break;
    case 151:
      return "5 blockstun medium";
      break;
    case 152:
      return "5 blockstun heavy";
      break;
    case 154:
      return "2 blockstun light";
      break;
    case 155:
      return "2 blockstun medium";
      break;
    case 156:
      return "2 blockstun heavy";
      break;
    case 158:
      return "j.blockstun";
      break;
    case 200:
      return "6D";
      break;
    case 201:
      return "4D";
      break;
    case 202:
      return "j.6D";
      break;
    case 203:
      return "j.4D";
      break;
    case 204:
      return "8D";
      break;
    case 205:
      return "9D";
      break;
    case 206:
      return "7D";
      break;
    case 207:
      return "land";
      break;
    case 309:
      return "22B";
      break;
    case 310:
      return "22A";
      break;
    case 390:
      return "6DA";
      break;
    case 391:
      return "6DB";
      break;
    case 392:
      return "3DA";
      break;
    case 393:
      return "3DB";
      break;
  }

  switch (character) {
    case 0:   // Reimu
    case 10:  // Meiling
      switch (id) {
        case 300:
          return "5A";
          break;
        case 301:
          return "5B";
          break;
        // 302???
        case 303:
          return "2A";
          break;
        case 304:
          return "2B";
          break;
        case 305:
          return "j.5A";
          break;
        case 306:
          return "j.5B";
          break;
        case 307:
          return "6B";
          break;
        case 308:
          return "6A";
          break;
        case 320:
          return "5AA";
          break;
        case 321:
          return "5AB";
          break;
        case 322:
          return "5AAA";
          break;
        case 323:
          return "5AAB";
          break;
        case 324:
          return "5AAAA";
          break;
        case 325:
          return "5AAAB";
          break;
        case 326:
          return "5AAA2B";
          break;
      }
      break;
    case 1:  // Marisa
    case 2:  // Sakuya
    case 3:  // Alice
    case 4:  // Patchouli
    case 6:  // Remilia
    case 7:  // Yuyuko
      switch (id) {
        case 300:
          return "5A";
          break;
        case 301:
          return "5B";
          break;
        case 302:
          return "2A";
          break;
        case 303:
          return "2B";
          break;
        case 304:
          return "j.5A";
          break;
        case 305:
          return "j.5B";
          break;
        case 307:
          return "6A";
          break;
        case 308:
          return "6B";
          break;
      }
      break;
    case 5:  // Youmu
    case 9:  // Suika
      switch (id) {
        case 300:
          return "5A";
          break;
        case 301:
          return "5B";
          break;
        case 302:
          return "2A";
          break;
        case 303:
          return "2B";
          break;
        case 304:
          return "j.5A";
          break;
        case 305:
          return "j.5B";
          break;
        case 306:
          return "j.2B";
          break;
        case 307:
          return "6B";
          break;
        case 308:
          return "6A";
          break;
      }
      break;
    case 8:  // Yukari
      switch (id) {
        case 300:
          return "5A";
          break;
        case 301:
          return "5B";
          break;
        case 302:
          return "2A";
          break;
        case 303:
          return "2B";
          break;
        case 304:
          return "j.5A";
          break;
        case 305:
          return "j.5B";
          break;
        case 307:
          return "6A";
          break;
        case 308:
          return "6B";
          break;
        case 320:
          return "j.2A";
          break;
        case 321:
          return "j.2B";
          break;
      }
  }

  return 0;
}

const char* Touhou_FrameDisplay::get_sequence_move_name(int n, int* dmeter) {
  if (m_initialized) {
    Touhou_Sequence* seq = m_framedata.get_sequence(n);
    if (seq) {
      static char buf[50];
      int id = seq->identifier;

      if (seq->identifier >= 0) {
        sprintf(buf, "%3.3d", seq->identifier);
      } else {
        sprintf(buf, "%3d", seq->identifier);
      }

      if (id >= 0 && id < 999) {
        const char* cat = 0;
        if (m_sequence_id_lookup[id] != n) {
          cat = "unused";
        } else if (m_game_id == TOUHOU_GAME_TH075 || m_game_id == TOUHOU_GAME_TH075_11) {
          cat = get_th075_move_name(m_character, id);
        }

        if (cat) {
          strcat(buf, " - ");
          strcat(buf, cat);
        }
      }

      return buf;
    }
  }
  return FrameDisplay::get_sequence_move_name(n, dmeter);
}

int Touhou_FrameDisplay::get_frame_count() {
  if (!m_initialized) {
    return 0;
  }

  Touhou_Sequence* seq = m_framedata.get_sequence(m_sequence);

  if (!seq) {
    return 0;
  }

  return seq->nframes;
}

int Touhou_FrameDisplay::get_subframe() { return m_subframe_base + m_subframe; }

int Touhou_FrameDisplay::get_subframe_count() {
  Touhou_Sequence* seq = m_framedata.get_sequence(m_sequence);

  if (!seq) {
    return 0;
  }

  int count = 0;
  for (int i = 0; i < seq->nframes; ++i) {
    count += seq->frames[i].header.duration;
  }

  return count;
}

Touhou_Frame* Touhou_FrameDisplay::get_frame_data(int seq_id, int fr_id) {
  Touhou_Sequence* seq = m_framedata.get_sequence(m_sequence);

  if (!seq) {
    return 0;
  }

  if (fr_id < 0 || fr_id >= seq->nframes) {
    return 0;
  }

  return &seq->frames[fr_id];
}

void Touhou_FrameDisplay::render(const RenderProperties* properties) {
  if (!m_initialized) {
    return;
  }

  Touhou_Frame* frame = get_frame_data(m_sequence, m_frame);

  if (!frame) {
    return;
  }

  if (properties->display_sprite) {
    int id = frame->header.render_frame;
    if (id >= 0) {
      if (id != m_last_sprite || !m_texture) {
        if (m_texture) {
          delete m_texture;
        }

        m_texture = m_imagedata.get_texture(id, m_palette, 0);

        m_last_sprite = id;
      }

      if (m_texture) {
        if (frame->header.identifier == 2) {
          if (frame->type_2.blend_mode == 1) {
            m_texture->blend_mode(2);
          } else if (frame->type_2.blend_mode == 2) {
            m_texture->blend_mode(3);
          } else {
            m_texture->blend_mode(1);
          }

          if (frame->type_2.angle) {
            m_texture->rotate_z((float)frame->type_2.angle / 360.0);
          }
        }

        m_texture->draw(-frame->header.x_offset, -frame->header.y_offset, 2);
      }
    }
  }

  if (properties->display_collision_box) {
    rect_t rects[40];
    int n = frame->n_boxes_collision;
    if (n > 40) {
      n = 40;
    }
    for (int i = 0; i < n; ++i) {
      rects[i].x1 = frame->boxes_collision[i].x1;
      rects[i].y1 = frame->boxes_collision[i].y1;
      rects[i].x2 = frame->boxes_collision[i].x2;
      rects[i].y2 = frame->boxes_collision[i].y2;
    }

    render_boxes(BOX_COLLISION, rects, n, properties->display_solid_boxes);
  }

  if (properties->display_hit_box) {
    rect_t rects[40];
    int n = frame->n_boxes_hit;
    if (n > 40) {
      n = 40;
    }
    for (int i = 0; i < n; ++i) {
      rects[i].x1 = frame->boxes_hit[i].x1;
      rects[i].y1 = frame->boxes_hit[i].y1;
      rects[i].x2 = frame->boxes_hit[i].x2;
      rects[i].y2 = frame->boxes_hit[i].y2;
    }

    render_boxes(BOX_HIT, rects, n, properties->display_solid_boxes);
  }
  if (properties->display_attack_box) {
    rect_t rects[40];
    int n = frame->n_boxes_attack;
    if (n > 40) {
      n = 40;
    }
    for (int i = 0; i < n; ++i) {
      rects[i].x1 = frame->boxes_attack[i]->x1;
      rects[i].y1 = frame->boxes_attack[i]->y1;
      rects[i].x2 = frame->boxes_attack[i]->x2;
      rects[i].y2 = frame->boxes_attack[i]->y2;
    }

    render_boxes(BOX_ATTACK, rects, n, properties->display_solid_boxes);
  }
}

Clone* Touhou_FrameDisplay::make_clone() {
  if (!m_initialized) {
    return 0;
  }

  Touhou_Frame* frame = get_frame_data(m_sequence, m_frame);

  if (!frame) {
    return 0;
  }

  Clone* clone = new Clone;

  if (frame->header.render_frame >= 0) {
    Texture* texture;

    texture = m_imagedata.get_texture(frame->header.render_frame, m_palette, 0);

    if (texture) {
      clone->init_texture(texture, -frame->header.x_offset, -frame->header.y_offset, 2);
    }
  }

  int n = frame->n_boxes_collision + frame->n_boxes_hit + frame->n_boxes_attack;
  if (n > 0) {
    CloneHitbox hitboxes[n];
    CloneHitbox* box = hitboxes;

    Touhou_Rect* rects = 0;
    int count = 0;
    BoxType render_box = BOX_DEFAULT;

    for (int i = 0; i < 2; ++i) {
      switch (i) {
        case 0:
          count = frame->n_boxes_collision;
          rects = frame->boxes_collision;
          render_box = BOX_COLLISION;
          break;
        case 1:
          count = frame->n_boxes_hit;
          rects = frame->boxes_hit;
          render_box = BOX_HIT;
          break;
      }

      for (int j = 0; j < count; ++j) {
        box->type = render_box;
        box->rect.x1 = rects[j].x1;
        box->rect.y1 = rects[j].y1;
        box->rect.x2 = rects[j].x2;
        box->rect.y2 = rects[j].y2;

        ++box;
      }
    }

    for (int j = 0; j < frame->n_boxes_attack; ++j) {
      box->type = BOX_ATTACK;
      box->rect.x1 = frame->boxes_attack[j]->x1;
      box->rect.y1 = frame->boxes_attack[j]->y1;
      box->rect.x2 = frame->boxes_attack[j]->x2;
      box->rect.y2 = frame->boxes_attack[j]->y2;

      ++box;
    }

    clone->init_hitboxes(hitboxes, n);
  }

  return clone;
}

void Touhou_FrameDisplay::flush_texture() {
  if (m_texture) {
    delete m_texture;
    m_texture = 0;
  }
  m_last_sprite = -1;
}

void Touhou_FrameDisplay::render_frame_properties(bool detailed, int scr_width, int scr_height) {
  if (!m_initialized) {
    return;
  }

  Touhou_Frame* frame = get_frame_data(m_sequence, m_frame);

  if (!frame) {
    return;
  }

  char strbuf[1024];
  sprintf(strbuf, "duration %d\n\n fflags %8.8x\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
          frame->header.duration, frame->props.fflags, (frame->props.fflags & 1) ? "Standing" : "-",
          (frame->props.fflags & 2) ? "Crouching" : "-",
          (frame->props.fflags & 4) ? "Airborne" : "-",
          (frame->props.fflags & 16) ? "Guard cancel" : "-",
          (frame->props.fflags & 32) ? "Cancellable" : "-",
          (frame->props.fflags & 64) ? "CH when hit" : "-",
          (frame->props.fflags & 128) ? "Superarmor" : "-",
          (frame->props.fflags & 8192) ? "Melee Inv" : "-",
          (frame->props.fflags & 16384) ? "Graze" : "-");
  if (m_game_id != TOUHOU_GAME_TH075 && m_game_id != TOUHOU_GAME_TH075_11) {
    sprintf(strbuf + strlen(strbuf), "%s\n", (frame->props.fflags & 0x200000) ? "HJC" : "-");
  }

  strcat(strbuf, "\n\n");

  if (frame->n_boxes_attack > 0) {
    sprintf(strbuf + strlen(strbuf), " aflags %8.8x\n block: %s %s %s %s\n%s\n%s\n%s\n%s\n\n",
            frame->props.aflags, (frame->props.aflags & 2) ? "H" : "-",
            (frame->props.aflags & 4) ? "L" : "-", (frame->props.aflags & 8) ? "A" : "-",
            (frame->props.aflags & 16) ? "U" : "-",
            (frame->props.aflags & 1024) ? "Induces CH" : "-",
            (frame->props.aflags & 262144) ? "Ungrazeable" : "-",
            (frame->props.aflags & 524288) ? "Guard crush" : "-",
            (frame->props.aflags & 2097152) ? "Stagger" : "-");
    if (m_game_id == TOUHOU_GAME_TH075 || m_game_id == TOUHOU_GAME_TH075_11) {
      sprintf(strbuf + strlen(strbuf),
              "   Damage: %d\nSpiritDmg: %d, %d\n     Chip: %d, %d\nProration: "
              "%d\n   Untech: %d\nMeterGain: %d %d\nVelocityX: %d\nVelocityY: "
              "%d\n     Type: %d %d %d\n",
              frame->props.damage, (frame->props.damage) / 2, (frame->props.damage) / 4,
              frame->props.chip + (frame->props.damage / 10), frame->props.chip,
              frame->props.proration, frame->props.untech, frame->props.unk9, frame->props.unk10,
              frame->props.velocity_x, frame->props.velocity_y, frame->props.unk15,
              frame->props.unk16, frame->props.attack_type);
    } else {
      sprintf(strbuf + strlen(strbuf),
              "   Damage: %d\nProration: %d\n    "
              "Limit: %d\n   Untech: %d\nCard Gain: "
              "%d %d\nVelocityX: %d\nVelocityY: %d\n  "
              "   Type: %d %d %d\n",
              frame->props.damage, frame->props.proration, frame->props.limit, frame->props.untech,
              frame->props.unk9, frame->props.unk10, frame->props.velocity_x,
              frame->props.velocity_y, frame->props.unk15, frame->props.unk16,
              frame->props.attack_type);
    }
  }

  render_shaded_text(scr_width - 180, 15, strbuf);
}

void Touhou_FrameDisplay::command(FrameDisplayCommand command, void* param) {
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
      set_sequence(m_sequence - 1);
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
      if (m_imagedata.get_palette_count() > 0) {
        m_palette = (m_palette + 1) % m_imagedata.get_palette_count();
        flush_texture();
      }
      break;
    case COMMAND_PALETTE_PREV:
      if (m_imagedata.get_palette_count() > 0) {
        m_palette =
            (m_palette - 1 + m_imagedata.get_palette_count()) % m_imagedata.get_palette_count();
        flush_texture();
      }
      break;
    case COMMAND_PALETTE_SET:
      if (m_imagedata.get_palette_count() > 0) {
        m_palette = *(int*)param % m_imagedata.get_palette_count();

        flush_texture();
      }
      break;
  }
}

const char* Touhou_FrameDisplay::get_sprite_filename(int n) {
  static char buf[256];
  if (m_game_id == TOUHOU_GAME_TH075 || m_game_id == TOUHOU_GAME_TH075_11) {
    sprintf(buf, "data_character_%2.2d-%3.3d.png", m_character, n);
  } else {
    const char* img_name = m_imagedata.get_filename(n);
    if (!img_name) {
      return 0;
    }

    strcpy(buf, img_name);

    char* s = buf;

    while ((s = strchr(s, '/')) != 0) {
      *s = '_';
    }

    int len = strlen(buf);
    if (buf[len - 4] == '.') {
      strcpy(buf + len - 4, ".png");
    } else {
      strcpy(buf + len, ".png");
    }
  }

  return buf;
}

const char* Touhou_FrameDisplay::get_current_sprite_filename() {
  if (!m_initialized) {
    return 0;
  }

  Touhou_Frame* frame = get_frame_data(m_sequence, m_frame);

  if (!frame || frame->header.render_frame < 0) {
    return 0;
  }

  return get_sprite_filename(frame->header.render_frame);
}

bool Touhou_FrameDisplay::save_current_sprite(const char* filename) {
  if (!m_initialized) {
    return 0;
  }

  Touhou_Frame* frame = get_frame_data(m_sequence, m_frame);

  if (!frame || frame->header.render_frame < 0) {
    return 0;
  }

  bool retval = 0;
  Texture* texture;

  texture = m_imagedata.get_texture(frame->header.render_frame, m_palette, 1);

  if (texture) {
    retval = texture->save_to_png(filename, m_imagedata.get_palette(m_palette));
  }

  delete texture;

  return retval;
}

int Touhou_FrameDisplay::save_all_character_sprites(const char* directory) {
  if (!m_initialized) {
    return 0;
  }

  int n = m_imagedata.get_image_count();
  int count = 0;

  for (int i = 0; i < n; ++i) {
    const char* image_filename = get_sprite_filename(i);
    if (!image_filename) {
      continue;
    }

    Texture* texture = m_imagedata.get_texture(i, m_palette, 1);

    if (texture) {
      char filename[2048];

      sprintf(filename, "%s%s", directory, image_filename);

      bool ok = texture->save_to_png(filename, m_imagedata.get_palette(m_palette));
      if (ok) {
        ++count;
      }

      delete texture;
    }
  }

  return count;
}

void Touhou_FrameDisplay::set_character(int n) {
  if (n == m_character) {
    return;
  }

  if (n < 0) {
    n = m_max_character - 1;
  }
  if (n >= m_max_character) {
    n = 0;
  }

  flush_texture();
  m_framedata.free();
  m_imagedata.free();

  m_character = n;

  switch (m_game_id) {
    case TOUHOU_GAME_TH075:
    case TOUHOU_GAME_TH075_11:
      m_framedata.init_th075(&m_packs, get_character_name(n));
      m_imagedata.load_th075(&m_packs, n);

      set_sequence(0);
      break;
    case TOUHOU_GAME_TH105:
    case TOUHOU_GAME_TH123:
    case TOUHOU_GAME_TH123_105:
      m_framedata.init_th105_123(&m_packs, &m_imagedata, get_character_name(n));

      set_sequence(0);
      break;
    default:
      break;
  }

  // initialize sequence ID lookup table
  for (int i = 0; i < 1000; ++i) {
    m_sequence_id_lookup[i] = -1;
  }

  int max = m_framedata.get_sequence_count();
  if (max > 0) {
    for (int i = 0; i < max; ++i) {
      Touhou_Sequence* seq = m_framedata.get_sequence(i);

      if (seq->identifier < 0 || seq->identifier > 999) {
        continue;
      }

      m_sequence_id_lookup[seq->identifier] = i;
    }
  }

  // fix palette
  if (m_palette >= m_imagedata.get_palette_count()) {
    m_palette = 0;
  }
}

void Touhou_FrameDisplay::set_sequence(int n) {
  if (!has_sequence(n)) {
    if (!m_initialized) {
      return;
    }
    if (n < 0) {
      n = m_framedata.get_sequence_count() - 1;
    } else {
      n = 0;
    }
    if (!has_sequence(n)) {
      return;
    }
  }

  m_sequence = n;

  set_frame(0);
}

void Touhou_FrameDisplay::set_frame(int n) {
  Touhou_Sequence* seq = m_framedata.get_sequence(m_sequence);

  if (!seq || !seq->nframes) {
    return;
  }

  if (n < 0) {
    n = seq->nframes - 1;
  }
  if (n >= seq->nframes) {
    n = 0;
  }

  m_frame = n;

  int count = 0;
  for (int i = 0; i < n; ++i) {
    count += seq->frames[i].header.duration;
  }
  m_subframe_base = count;
  m_subframe = 0;
  m_subframe_next = seq->frames[n].header.duration;
}

bool Touhou_FrameDisplay::init_internal() {
  switch (m_game_id) {
    case TOUHOU_GAME_TH075:
      m_max_character = 10;
      break;
    case TOUHOU_GAME_TH075_11:
      m_max_character = 11;
      break;
    case TOUHOU_GAME_TH105:
      m_max_character = 15;
      break;
    case TOUHOU_GAME_TH123:
      m_max_character = 9;
      break;
    case TOUHOU_GAME_TH123_105:
      m_max_character = 20;
      break;
    default:
      return 0;
  }

  // initialize character etc
  m_character = -1;
  set_character(0);

  return FrameDisplay::init();
}

bool Touhou_FrameDisplay::init(const char* base_path) {
  if (m_initialized) {
    return 0;
  }

  int n = strlen(base_path);

  char path[n + 20];

  m_game_id = TOUHOU_GAME_NONE;

  // check for th123a
  sprintf(path, "%sth123a.dat", base_path);
  if (m_packs.add_th105_123(path, 0)) {
    sprintf(path, "%sth123c.dat", base_path);

    m_packs.add_th105_123(path, 0);

    m_game_id = TOUHOU_GAME_TH123;

    // get th105 path from .ini, if any.
    const char* th105_path = base_path;
#ifdef WIN32
    char ini_filename[1024];
    sprintf(path, "%sconfigex123.ini", base_path);

    ini_filename[1023] = '\0';
    GetPrivateProfileString("th105path", "path", "", ini_filename, 1023, path);
    if (ini_filename[0] != '\0') {
      strcat(ini_filename, "\\");

      th105_path = ini_filename;
    }
#endif

    // th105c is not used here.
    sprintf(path, "%sth105a.dat", th105_path);
    if (m_packs.add_th105_123(path, 1)) {
      m_game_id = TOUHOU_GAME_TH123_105;
    } else {
      sprintf(path, "%sth105a.dat", base_path);
      if (m_packs.add_th105_123(path, 1)) {
        m_game_id = TOUHOU_GAME_TH123_105;
      }
    }

    m_initialized = 1;

    return init_internal();
  }

  // check for th105a
  sprintf(path, "%sth105a.dat", base_path);
  if (m_packs.add_th105_123(path, 0)) {
    m_game_id = TOUHOU_GAME_TH105;

    sprintf(path, "%sth105c.dat", base_path);
    m_packs.add_th105_123(path, 0);

    m_initialized = 1;

    return init_internal();
  }

  // check for th075
  sprintf(path, "%sth075.dat", base_path);

  if (m_packs.add_th075(path)) {
    m_game_id = TOUHOU_GAME_TH075;

    sprintf(path, "%sth075b.dat", base_path);

    if (m_packs.add_th075(path)) {
      if (m_packs.has_file("data\\character\\10.dat")) {
        // has meiling
        m_game_id = TOUHOU_GAME_TH075_11;
      }
    }

    return init_internal();
  }

  return 0;
}

bool Touhou_FrameDisplay::init() { return init(""); }

void Touhou_FrameDisplay::free() {
  m_packs.free();
  m_imagedata.free();

  m_game_id = TOUHOU_GAME_NONE;

  if (m_texture) {
    delete m_texture;
    m_texture = 0;
  }
  m_last_sprite = -1;

  m_subframe_base = 0;
  m_subframe = 0;
  m_subframe_next = 0;

  m_max_character = 0;

  m_initialized = 0;
}

Touhou_FrameDisplay::Touhou_FrameDisplay() {
  m_game_id = TOUHOU_GAME_NONE;

  m_texture = 0;
  m_last_sprite = -1;

  m_subframe_base = 0;
  m_subframe = 0;
  m_subframe_next = 0;

  m_max_character = 0;

  m_initialized = 0;
}

Touhou_FrameDisplay::~Touhou_FrameDisplay() {}
