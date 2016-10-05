// MBAACC Frame Display:
//
// Core manager.

#include "mbaacc_framedisplay.h"

#include <cstdio>
#include <cstring>

struct MBAACC_Character_Info {
  const char* long_name;
  const char* short_name;
  const char* file_name;
  int moon;
};

static const MBAACC_Character_Info mbaacc_character_info[] = {
    {"C Akiha", "C-AKIHA", "AKIHA", 0},
    {"F Akiha", "F-AKIHA", "AKIHA", 1},
    {"H Akiha", "H-AKIHA", "AKIHA", 2},
    {"C V.Akiha", "C-AKAAKIHA", "AKAAKIHA", 0},
    {"F V.Akiha", "F-AKAAKIHA", "AKAAKIHA", 1},
    {"H V.Akiha", "H-AKAAKIHA", "AKAAKIHA", 2},
    {"C S.Akiha", "C-S_AKIHA", "S_AKIHA", 0},
    {"F S.Akiha", "F-S_AKIHA", "S_AKIHA", 1},
    {"H S.Akiha", "H-S_AKIHA", "S_AKIHA", 2},
    {"C Aoko", "C-AOKO", "AOKO", 0},
    {"F Aoko", "F-AOKO", "AOKO", 1},
    {"H Aoko", "H-AOKO", "AOKO", 2},
    {"C Arcueid", "C-ARC", "ARC", 0},
    {"F Arcueid", "F-ARC", "ARC", 1},
    {"H Arcueid", "H-ARC", "ARC", 2},
    {"C Warcueid", "C-WARC", "WARC", 0},
    {"F Warcueid", "F-WARC", "WARC", 1},
    {"H Warcueid", "H-WARC", "WARC", 2},
    {"C Archetype", "C-P_ARC", "P_ARC", 0},
    {"F Archetype", "F-P_ARC", "P_ARC", 1},
    {"H Archetype", "H-P_ARC", "P_ARC", 2},
    {"C Ciel", "C-CIEL", "CIEL", 0},
    {"F Ciel", "F-CIEL", "CIEL", 1},
    {"H Ciel", "H-CIEL", "CIEL", 2},
    {"C P.Ciel", "C-P_CIEL", "P_CIEL", 0},
    {"F P.Ciel", "F-P_CIEL", "P_CIEL", 1},
    {"H P.Ciel", "H-P_CIEL", "P_CIEL", 2},
    {"C Hisui", "C-HISUI", "HISUI", 0},
    {"F Hisui", "F-HISUI", "HISUI", 1},
    {"H Hisui", "H-HISUI", "HISUI", 2},
    {"C Kouma", "C-KISHIMA", "KISHIMA", 0},
    {"F Kouma", "F-KISHIMA", "KISHIMA", 1},
    {"H Kouma", "H-KISHIMA", "KISHIMA", 2},
    {"C Kohaku", "C-KOHAKU", "KOHAKU", 0},
    {"F Kohaku", "F-KOHAKU", "KOHAKU", 1},
    {"H Kohaku", "H-KOHAKU", "KOHAKU", 2},
    {"C Kohaku M", "C-KOHAKU_M", "KOHAKU_M", 0},
    {"F Kohaku M", "F-KOHAKU_M", "KOHAKU_M", 1},
    {"H Kohaku M", "H-KOHAKU_M", "KOHAKU_M", 2},
    {"C Len", "C-LEN", "LEN", 0},
    {"F Len", "F-LEN", "LEN", 1},
    {"H Len", "H-LEN", "LEN", 2},
    {"C W.Len", "C-WLEN", "WLEN", 0},
    {"F W.Len", "F-WLEN", "WLEN", 1},
    {"H W.Len", "H-WLEN", "WLEN", 2},
    {"C M.Hisui", "C-M_HISUI", "M_HISUI", 0},
    {"F M.Hisui", "F-M_HISUI", "M_HISUI", 1},
    {"H M.Hisui", "H-M_HISUI", "M_HISUI", 2},
    {"C M.Hisui M", "C-M_HISUI_M", "M_HISUI_M", 0},
    {"F M.Hisui M", "F-M_HISUI_M", "M_HISUI_M", 1},
    {"H M.Hisui M", "H-M_HISUI_M", "M_HISUI_M", 2},
    {"C P.Hisui P", "C-M_HISUI_P", "M_HISUI_P", 0},
    {"F P.Hisui P", "F-M_HISUI_P", "M_HISUI_P", 1},
    {"H P.Hisui P", "H-M_HISUI_P", "M_HISUI_P", 2},
    {"C Miyako", "C-MIYAKO", "MIYAKO", 0},
    {"F Miyako", "F-MIYAKO", "MIYAKO", 1},
    {"H Miyako", "H-MIYAKO", "MIYAKO", 2},
    {"C Nanaya", "C-NANAYA", "NANAYA", 0},
    {"F Nanaya", "F-NANAYA", "NANAYA", 1},
    {"H Nanaya", "H-NANAYA", "NANAYA", 2},
    {"C Neco Arc", "C-NECO", "NECO", 0},
    {"F Neco Arc", "F-NECO", "NECO", 1},
    {"H Neco Arc", "H-NECO", "NECO", 2},
    {"C Neco Arc P", "C-NECO_P", "NECO_P", 0},
    {"F Neco Arc P", "F-NECO_P", "NECO_P", 1},
    {"H Neco Arc P", "H-NECO_P", "NECO_P", 2},
    {"C Neco Chaos", "C-NECHAOS", "NECHAOS", 0},
    {"F Neco Chaos", "F-NECHAOS", "NECHAOS", 1},
    {"H Neco Chaos", "H-NECHAOS", "NECHAOS", 2},
    {"C Nero", "C-NERO", "NERO", 0},
    {"F Nero", "F-NERO", "NERO", 1},
    {"H Nero", "H-NERO", "NERO", 2},
    {"C Riesbyfe", "C-RIES", "RIES", 0},
    {"F Riesbyfe", "F-RIES", "RIES", 1},
    {"H Riesbyfe", "H-RIES", "RIES", 2},
    {"C Roa", "C-ROA", "ROA", 0},
    {"F Roa", "F-ROA", "ROA", 1},
    {"H Roa", "H-ROA", "ROA", 2},
    {"C Ryougi", "C-RYOUGI", "RYOUGI", 0},
    {"F Ryougi", "F-RYOUGI", "RYOUGI", 1},
    {"H Ryougi", "H-RYOUGI", "RYOUGI", 2},
    {"C Satsuki", "C-SATSUKI", "SATSUKI", 0},
    {"F Satsuki", "F-SATSUKI", "SATSUKI", 1},
    {"H Satsuki", "H-SATSUKI", "SATSUKI", 2},
    {"C Shiki", "C-SHIKI", "SHIKI", 0},
    {"F Shiki", "F-SHIKI", "SHIKI", 1},
    {"H Shiki", "H-SHIKI", "SHIKI", 2},
    {"C Sion", "C-SION", "SION", 0},
    {"F Sion", "F-SION", "SION", 1},
    {"H Sion", "H-SION", "SION", 2},
    {"C V.Sion", "C-V_SION", "V_SION", 0},
    {"F V.Sion", "F-V_SION", "V_SION", 1},
    {"H V.Sion", "H-V_SION", "V_SION", 2},
    {"C Warakia", "C-WARAKIA", "WARAKIA", 0},
    {"F Warakia", "F-WARAKIA", "WARAKIA", 1},
    {"H Warakia", "H-WARAKIA", "WARAKIA", 2},
    {"Effect", "EFFECT", "EFFECT", -1},
};

static const int mbaacc_ncharacter_info =
    sizeof(mbaacc_character_info) / sizeof(mbaacc_character_info[0]);

const char* MBAACC_FrameDisplay::get_character_long_name(int n) {
  if (n < 0 || n >= mbaacc_ncharacter_info) {
    return FrameDisplay::get_character_long_name(n);
  }
  return mbaacc_character_info[n].long_name;
}

const char* MBAACC_FrameDisplay::get_character_name(int n) {
  if (n < 0 || n >= mbaacc_ncharacter_info) {
    return FrameDisplay::get_character_name(n);
  }
  return mbaacc_character_info[n].short_name;
}

int MBAACC_FrameDisplay::get_sequence_count() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_sequence_count();
}

bool MBAACC_FrameDisplay::has_sequence(int n) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.has_sequence(n);
}
const char* MBAACC_FrameDisplay::get_sequence_name(int n) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_sequence_name(n);
}

const char* MBAACC_FrameDisplay::get_sequence_move_name(int n, int* dmeter) {
  if (!m_initialized) {
    return 0;
  }

  const char* str = m_character_data.get_sequence_move_name(n, dmeter);

  if (!str) {
    const char* ch_name = get_character_name(m_character);

    if (ch_name && !strstr(ch_name, "EFFECT")) {
      switch (n) {
        case 0:
          str = "5";
          break;
        case 1:
          str = "5A";
          break;
        case 2:
          str = "5B";
          break;
        case 3:
          str = "5C";
          break;
        case 4:
          str = "2A";
          break;
        case 5:
          str = "2B";
          break;
        case 6:
          str = "2C";
          break;
        case 7:
          str = "j.A";
          break;
        case 8:
          str = "j.B";
          break;
        case 9:
          str = "j.C";
          break;
        case 10:
          str = "6";
          break;
        case 11:
          str = "4";
          break;
        case 12:
          str = "5->2";
          break;
        case 13:
          str = "2";
          break;
        case 14:
          str = "2->5";
          break;
        case 17:
          str = "4 Guard";
          break;
        case 18:
          str = "3 Guard";
          break;
        case 19:
          str = "j.4 Guard";
        case 35:
          str = "9";
          break;
        case 36:
          str = "8";
          break;
        case 37:
          str = "7";
          break;
        case 38:
          str = "j.9";
          break;
        case 39:
          str = "j.8";
          break;
        case 40:
          str = "j.7";
          break;
        case 50:
          str = "intro";
          break;
        case 52:
          str = "win pose";
          break;
        case 250:
          str = "heat";
          break;
        case 255:
          str = "circuit spark";
          break;
      }
    }
  }

  return str;
}

int MBAACC_FrameDisplay::get_frame_count() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_frame_count(m_sequence);
}

int MBAACC_FrameDisplay::get_subframe() { return m_subframe + m_subframe_base; }

int MBAACC_FrameDisplay::get_subframe_count() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_subframe_count(m_sequence);
}

void MBAACC_FrameDisplay::set_frame(int n) {
  if (!m_initialized) {
    return;
  }

  m_frame = m_character_data.find_frame(m_sequence, n);

  m_subframe = 0;

  m_subframe_base = m_character_data.count_subframes(m_sequence, m_frame);
  m_subframe_next = m_character_data.get_subframe_length(m_sequence, m_frame);
}

void MBAACC_FrameDisplay::set_sequence(int n) {
  if (!m_initialized) {
    return;
  }

  m_sequence = m_character_data.find_sequence(n, n < m_sequence ? -1 : 1);

  m_subframe_base = 0;
  m_subframe = 0;

  set_frame(0);
}

void MBAACC_FrameDisplay::set_palette(int n) {
  if (!m_initialized) {
    return;
  }

  m_palette = n % 36;

  m_character_data.set_palette(n);
}

void MBAACC_FrameDisplay::set_active_character(int n) {
  if (!m_initialized) {
    return;
  }

  if (n == m_character) {
    return;
  }

  if (n < 0 || n >= (mbaacc_ncharacter_info)) {
    return;
  }

  bool need_gfx = 1;

  if (m_character >= 0 && m_character < mbaacc_ncharacter_info) {
    if (!strcmp(mbaacc_character_info[m_character].file_name, mbaacc_character_info[n].file_name)) {
      need_gfx = 0;
    }
  }

  if (need_gfx) {
    m_character_data.free();
  } else {
    m_character_data.free_frame_data();
  }

  if (m_character_data.load(&m_pack, mbaacc_character_info[n].file_name,
                            mbaacc_character_info[n].moon)) {
    if (need_gfx) {
      m_character_data.load_graphics(&m_pack);
    }
  }
  m_character = n;

  set_palette(m_palette);
  set_sequence(0);
  set_frame(0);
}

bool MBAACC_FrameDisplay::init(const char* filename) {
  if (m_initialized) {
    return 1;
  }

  if (!m_pack.open_pack(filename)) {
    return 0;
  }

  // finish up
  if (!FrameDisplay::init()) {
    free();

    return 0;
  }

  // set defaults
  m_character = -1;
  set_active_character(0);

  return 1;
}

bool MBAACC_FrameDisplay::init() {
  if (m_initialized) {
    return 1;
  }

  if (!init("0002.p")) {
    return 0;
  }

  return 1;
}

void MBAACC_FrameDisplay::free() {
  m_pack.close_pack();

  m_character_data.free();

  m_character = -1;

  m_subframe = 0;
  m_subframe_base = 0;
  m_subframe_next = 0;

  FrameDisplay::free();
}

void MBAACC_FrameDisplay::render(const RenderProperties* properties) {
  if (!m_initialized) {
    return;
  }

  m_character_data.render(properties, m_sequence, m_frame);
}

Clone* MBAACC_FrameDisplay::make_clone() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.make_clone(m_sequence, m_frame);
}

void MBAACC_FrameDisplay::flush_texture() {
  if (!m_initialized) {
    return;
  }

  m_character_data.flush_texture();
}

void MBAACC_FrameDisplay::render_frame_properties(bool detailed, int scr_width, int scr_height) {
  if (!m_initialized) {
    return;
  }

  m_character_data.render_frame_properties(detailed, scr_width, scr_height, m_sequence, m_frame);
}

void MBAACC_FrameDisplay::command(FrameDisplayCommand command, void* param) {
  if (!m_initialized) {
    return;
  }

  switch (command) {
    case COMMAND_CHARACTER_NEXT:
      if (m_character == -1) {
        set_active_character(0);
      } else {
        set_active_character((m_character + 1) % (mbaacc_ncharacter_info));
      }
      break;
    case COMMAND_CHARACTER_PREV:
      if (m_character == -1) {
        set_active_character(0);
      } else {
        set_active_character((m_character + (mbaacc_ncharacter_info)-1) % mbaacc_ncharacter_info);
      }
      break;
    case COMMAND_CHARACTER_SET:
      if (!param) {
        break;
      }
      set_active_character((int)(*(unsigned int*)param) % (mbaacc_ncharacter_info));
      break;
    case COMMAND_PALETTE_NEXT:
      set_palette((m_palette + 1) % 36);
      break;
    case COMMAND_PALETTE_PREV:
      set_palette((m_palette + 35) % 36);
      break;
    case COMMAND_PALETTE_SET:
      if (!param) {
        break;
      }
      set_palette((int)(*(int*)param) % 36);
      break;
    case COMMAND_SEQUENCE_NEXT:
      set_sequence(m_sequence + 1);
      break;
    case COMMAND_SEQUENCE_PREV:
      set_sequence(m_sequence - 1);
      break;
    case COMMAND_SEQUENCE_SET:
      if (!param) {
        break;
      }
      set_sequence(*(int*)param);
      break;
    case COMMAND_FRAME_NEXT:
      set_frame(m_frame + 1);
      break;
    case COMMAND_FRAME_PREV:
      set_frame(m_frame - 1);
      break;
    case COMMAND_FRAME_SET:
      if (!param) {
        break;
      }
      set_frame(*(int*)param);
      break;
    case COMMAND_SUBFRAME_NEXT:
      m_subframe++;
      if (m_subframe >= m_subframe_next) {
        set_frame(m_frame + 1);
      }
      break;
    case COMMAND_SUBFRAME_PREV:
      --m_subframe;
      if (m_subframe < 0) {
        set_frame(m_frame + 1);
        m_subframe = m_subframe_next - 1;
      }
      break;
    case COMMAND_SUBFRAME_SET:
      break;
  }
}

const char* MBAACC_FrameDisplay::get_current_sprite_filename() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_current_sprite_filename(m_sequence, m_frame);
}

bool MBAACC_FrameDisplay::save_current_sprite(const char* filename) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.save_current_sprite(filename, m_sequence, m_frame);
}

int MBAACC_FrameDisplay::save_all_character_sprites(const char* directory) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.save_all_character_sprites(directory);
}

MBAACC_FrameDisplay::MBAACC_FrameDisplay() {
  m_character = -1;
  m_subframe = 0;
  m_subframe_base = 0;
  m_subframe_next = 0;
}

MBAACC_FrameDisplay::~MBAACC_FrameDisplay() {
  // cleanup will do the work
}
