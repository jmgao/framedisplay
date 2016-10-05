// MBAA Frame Display:
//
// Core manager.

#include "mbaa_framedisplay.h"

#include <cstdio>
#include <cstring>

struct MBAA_Character_Info {
  const char* long_name;
  const char* short_name;
  const char* file_name;
  int moon;
};

static const MBAA_Character_Info mbaa_character_info[] = {
    {"Console C V.Akiha", "C-AKAAKIHA", "AKAAKIHA", 0},
    {"Console F V.Akiha", "F-AKAAKIHA", "AKAAKIHA", 1},
    {"Console H V.Akiha", "H-AKAAKIHA", "AKAAKIHA", 2},
    {"Console C Akiha", "C-AKIHA", "AKIHA", 0},
    {"Console F Akiha", "F-AKIHA", "AKIHA", 1},
    {"Console H Akiha", "H-AKIHA", "AKIHA", 2},
    {"Console C Aoko", "C-AOKO", "AOKO", 0},
    {"Console F Aoko", "F-AOKO", "AOKO", 1},
    {"Console H Aoko", "H-AOKO", "AOKO", 2},
    {"Console C Arcueid", "C-ARC", "ARC", 0},
    {"Console F Arcueid", "F-ARC", "ARC", 1},
    {"Console H Arcueid", "H-ARC", "ARC", 2},
    {"Console C Ciel", "C-CIEL", "CIEL", 0},
    {"Console F Ciel", "F-CIEL", "CIEL", 1},
    {"Console H Ciel", "H-CIEL", "CIEL", 2},
    {"Console C Hisui", "C-HISUI", "HISUI", 0},
    {"Console F Hisui", "F-HISUI", "HISUI", 1},
    {"Console H Hisui", "H-HISUI", "HISUI", 2},
    {"Console C Kouma", "C-KISHIMA", "KISHIMA", 0},
    {"Console F Kouma", "F-KISHIMA", "KISHIMA", 1},
    {"Console H Kouma", "H-KISHIMA", "KISHIMA", 2},
    {"Console C Kohaku", "C-KOHAKU", "KOHAKU", 0},
    {"Console F Kohaku", "F-KOHAKU", "KOHAKU", 1},
    {"Console H Kohaku", "H-KOHAKU", "KOHAKU", 2},
    {"Console C Kohaku M", "C-KOHAKU_M", "KOHAKU_M", 0},
    {"Console F Kohaku M", "F-KOHAKU_M", "KOHAKU_M", 1},
    {"Console H Kohaku M", "H-KOHAKU_M", "KOHAKU_M", 2},
    {"Console C Len", "C-LEN", "LEN", 0},
    {"Console F Len", "F-LEN", "LEN", 1},
    {"Console H Len", "H-LEN", "LEN", 2},
    {"Console C M.Hisui", "C-M_HISUI", "M_HISUI", 0},
    {"Console F M.Hisui", "F-M_HISUI", "M_HISUI", 1},
    {"Console H M.Hisui", "H-M_HISUI", "M_HISUI", 2},
    {"Console C M.Hisui M", "C-M_HISUI_M", "M_HISUI_M", 0},
    {"Console F M.Hisui M", "F-M_HISUI_M", "M_HISUI_M", 1},
    {"Console H M.Hisui M", "H-M_HISUI_M", "M_HISUI_M", 2},
    {"Console C M.Hisui P", "C-M_HISUI_P", "M_HISUI_P", 0},
    {"Console F M.Hisui P", "F-M_HISUI_P", "M_HISUI_P", 1},
    {"Console H M.Hisui P", "H-M_HISUI_P", "M_HISUI_P", 2},
    {"Console C Miyako", "C-MIYAKO", "MIYAKO", 0},
    {"Console F Miyako", "F-MIYAKO", "MIYAKO", 1},
    {"Console H Miyako", "H-MIYAKO", "MIYAKO", 2},
    {"Console C Nanaya", "C-NANAYA", "NANAYA", 0},
    {"Console F Nanaya", "F-NANAYA", "NANAYA", 1},
    {"Console H Nanaya", "H-NANAYA", "NANAYA", 2},
    {"Console C Neco Chaos", "C-NECHAOS", "NECHAOS", 0},
    {"Console F Neco Chaos", "F-NECHAOS", "NECHAOS", 1},
    {"Console H Neco Chaos", "H-NECHAOS", "NECHAOS", 2},
    {"Console C Neco Arc", "C-NECO", "NECO", 0},
    {"Console F Neco Arc", "F-NECO", "NECO", 1},
    {"Console H Neco Arc", "H-NECO", "NECO", 2},
    {"Console C Neco Arc P", "C-NECO_P", "NECO_P", 0},
    {"Console F Neco Arc P", "F-NECO_P", "NECO_P", 1},
    {"Console H Neco Arc P", "H-NECO_P", "NECO_P", 2},
    {"Console C Nero", "C-NERO", "NERO", 0},
    {"Console F Nero", "F-NERO", "NERO", 1},
    {"Console H Nero", "H-NERO", "NERO", 2},
    {"Console C Riesbyfe", "C-RIES", "RIES", 0},
    {"Console F Riesbyfe", "F-RIES", "RIES", 1},
    {"Console H Riesbyfe", "H-RIES", "RIES", 2},
    {"Console C Roa", "C-ROA", "ROA", 0},
    {"Console F Roa", "F-ROA", "ROA", 1},
    {"Console H Roa", "H-ROA", "ROA", 2},
    {"Console C Ryougi", "C-RYOUGI", "RYOUGI", 0},
    {"Console F Ryougi", "F-RYOUGI", "RYOUGI", 1},
    {"Console H Ryougi", "H-RYOUGI", "RYOUGI", 2},
    {"Console C S.Akiha", "C-S_AKIHA", "S_AKIHA", 0},
    {"Console F S.Akiha", "F-S_AKIHA", "S_AKIHA", 1},
    {"Console H S.Akiha", "H-S_AKIHA", "S_AKIHA", 2},
    {"Console C Satsuki", "C-SATSUKI", "SATSUKI", 0},
    {"Console F Satsuki", "F-SATSUKI", "SATSUKI", 1},
    {"Console H Satsuki", "H-SATSUKI", "SATSUKI", 2},
    {"Console C Shiki", "C-SHIKI", "SHIKI", 0},
    {"Console F Shiki", "F-SHIKI", "SHIKI", 1},
    {"Console H Shiki", "H-SHIKI", "SHIKI", 2},
    {"Console C Sion", "C-SION", "SION", 0},
    {"Console F Sion", "F-SION", "SION", 1},
    {"Console H Sion", "H-SION", "SION", 2},
    {"Console C V.Sion", "C-V_SION", "V_SION", 0},
    {"Console F V.Sion", "F-V_SION", "V_SION", 1},
    {"Console H V.Sion", "H-V_SION", "V_SION", 2},
    {"Console C Warakia", "C-WARAKIA", "WARAKIA", 0},
    {"Console F Warakia", "F-WARAKIA", "WARAKIA", 1},
    {"Console H Warakia", "H-WARAKIA", "WARAKIA", 2},
    {"Console C Warcueid", "C-WARC", "WARC", 0},
    {"Console F Warcueid", "F-WARC", "WARC", 1},
    {"Console H Warcueid", "H-WARC", "WARC", 2},
    {"Console C W.Len", "C-WLEN", "WLEN", 0},
    {"Console F W.Len", "F-WLEN", "WLEN", 1},
    {"Console H W.Len", "H-WLEN", "WLEN", 2},
    {"Console E Archetype: Earth", "E-B_ARC", "B_ARC", 9},
    {"Console Effects", "EFFECT", "EFFECT", -1},
    {"Arcade C V.Akiha", "C-ARCADE_AKAAKIHA", "ARCADE_AKAAKIHA", 0},
    {"Arcade F V.Akiha", "F-ARCADE_AKAAKIHA", "ARCADE_AKAAKIHA", 1},
    {"Arcade H V.Akiha", "H-ARCADE_AKAAKIHA", "ARCADE_AKAAKIHA", 2},
    {"Arcade C Akiha", "C-ARCADE_AKIHA", "ARCADE_AKIHA", 0},
    {"Arcade F Akiha", "F-ARCADE_AKIHA", "ARCADE_AKIHA", 1},
    {"Arcade H Akiha", "H-ARCADE_AKIHA", "ARCADE_AKIHA", 2},
    {"Arcade C Aoko", "C-ARCADE_AOKO", "ARCADE_AOKO", 0},
    {"Arcade F Aoko", "F-ARCADE_AOKO", "ARCADE_AOKO", 1},
    {"Arcade H Aoko", "H-ARCADE_AOKO", "ARCADE_AOKO", 2},
    {"Arcade C Arcueid", "C-ARCADE_ARC", "ARCADE_ARC", 0},
    {"Arcade F Arcueid", "F-ARCADE_ARC", "ARCADE_ARC", 1},
    {"Arcade H Arcueid", "H-ARCADE_ARC", "ARCADE_ARC", 2},
    {"Arcade C Ciel", "C-ARCADE_CIEL", "ARCADE_CIEL", 0},
    {"Arcade F Ciel", "F-ARCADE_CIEL", "ARCADE_CIEL", 1},
    {"Arcade H Ciel", "H-ARCADE_CIEL", "ARCADE_CIEL", 2},
    {"Arcade C Hisui", "C-ARCADE_HISUI", "ARCADE_HISUI", 0},
    {"Arcade F Hisui", "F-ARCADE_HISUI", "ARCADE_HISUI", 1},
    {"Arcade H Hisui", "H-ARCADE_HISUI", "ARCADE_HISUI", 2},
    {"Arcade C Kouma", "C-ARCADE_KISHIMA", "ARCADE_KISHIMA", 0},
    {"Arcade F Kouma", "F-ARCADE_KISHIMA", "ARCADE_KISHIMA", 1},
    {"Arcade H Kouma", "H-ARCADE_KISHIMA", "ARCADE_KISHIMA", 2},
    {"Arcade C Kohaku", "C-ARCADE_KOHAKU", "ARCADE_KOHAKU", 0},
    {"Arcade F Kohaku", "F-ARCADE_KOHAKU", "ARCADE_KOHAKU", 1},
    {"Arcade H Kohaku", "H-ARCADE_KOHAKU", "ARCADE_KOHAKU", 2},
    {"Arcade C Kohaku M", "C-ARCADE_KOHAKU_M", "ARCADE_KOHAKU_M", 0},
    {"Arcade F Kohaku M", "F-ARCADE_KOHAKU_M", "ARCADE_KOHAKU_M", 1},
    {"Arcade H Kohaku M", "H-ARCADE_KOHAKU_M", "ARCADE_KOHAKU_M", 2},
    {"Arcade C Len", "C-ARCADE_LEN", "ARCADE_LEN", 0},
    {"Arcade F Len", "F-ARCADE_LEN", "ARCADE_LEN", 1},
    {"Arcade H Len", "H-ARCADE_LEN", "ARCADE_LEN", 2},
    {"Arcade C M.Hisui", "C-ARCADE_M_HISUI", "ARCADE_M_HISUI", 0},
    {"Arcade F M.Hisui", "F-ARCADE_M_HISUI", "ARCADE_M_HISUI", 1},
    {"Arcade H M.Hisui", "H-ARCADE_M_HISUI", "ARCADE_M_HISUI", 2},
    {"Arcade C M.Hisui M", "C-ARCADE_M_HISUI_M", "ARCADE_M_HISUI_M", 0},
    {"Arcade F M.Hisui M", "F-ARCADE_M_HISUI_M", "ARCADE_M_HISUI_M", 1},
    {"Arcade H M.Hisui M", "H-ARCADE_M_HISUI_M", "ARCADE_M_HISUI_M", 2},
    {"Arcade C M.Hisui P", "C-ARCADE_M_HISUI_P", "ARCADE_M_HISUI_P", 0},
    {"Arcade F M.Hisui P", "F-ARCADE_M_HISUI_P", "ARCADE_M_HISUI_P", 1},
    {"Arcade H M.Hisui P", "H-ARCADE_M_HISUI_P", "ARCADE_M_HISUI_P", 2},
    {"Arcade C Miyako", "C-ARCADE_MIYAKO", "ARCADE_MIYAKO", 0},
    {"Arcade F Miyako", "F-ARCADE_MIYAKO", "ARCADE_MIYAKO", 1},
    {"Arcade H Miyako", "H-ARCADE_MIYAKO", "ARCADE_MIYAKO", 2},
    {"Arcade C Nanaya", "C-ARCADE_NANAYA", "ARCADE_NANAYA", 0},
    {"Arcade F Nanaya", "F-ARCADE_NANAYA", "ARCADE_NANAYA", 1},
    {"Arcade H Nanaya", "H-ARCADE_NANAYA", "ARCADE_NANAYA", 2},
    {"Arcade C Neco Chaos", "C-ARCADE_NECHAOS", "ARCADE_NECHAOS", 0},
    {"Arcade F Neco Chaos", "F-ARCADE_NECHAOS", "ARCADE_NECHAOS", 1},
    {"Arcade H Neco Chaos", "H-ARCADE_NECHAOS", "ARCADE_NECHAOS", 2},
    {"Arcade C Neco Arc", "C-ARCADE_NECO", "ARCADE_NECO", 0},
    {"Arcade F Neco Arc", "F-ARCADE_NECO", "ARCADE_NECO", 1},
    {"Arcade H Neco Arc", "H-ARCADE_NECO", "ARCADE_NECO", 2},
    {"Arcade C Neco Arc P", "C-ARCADE_NECO_P", "ARCADE_NECO_P", 0},
    {"Arcade F Neco Arc P", "F-ARCADE_NECO_P", "ARCADE_NECO_P", 1},
    {"Arcade H Neco Arc P", "H-ARCADE_NECO_P", "ARCADE_NECO_P", 2},
    {"Arcade C Nero", "C-ARCADE_NERO", "ARCADE_NERO", 0},
    {"Arcade F Nero", "F-ARCADE_NERO", "ARCADE_NERO", 1},
    {"Arcade H Nero", "H-ARCADE_NERO", "ARCADE_NERO", 2},
    {"Arcade C Riesbyfe", "C-ARCADE_RIES", "ARCADE_RIES", 0},
    {"Arcade F Riesbyfe", "F-ARCADE_RIES", "ARCADE_RIES", 1},
    {"Arcade H Riesbyfe", "H-ARCADE_RIES", "ARCADE_RIES", 2},
    {"Arcade C Roa", "C-ARCADE_ROA", "ARCADE_ROA", 0},
    {"Arcade F Roa", "F-ARCADE_ROA", "ARCADE_ROA", 1},
    {"Arcade H Roa", "H-ARCADE_ROA", "ARCADE_ROA", 2},
    {"Arcade C Ryougi", "C-ARCADE_RYOUGI", "ARCADE_RYOUGI", 0},
    {"Arcade F Ryougi", "F-ARCADE_RYOUGI", "ARCADE_RYOUGI", 1},
    {"Arcade H Ryougi", "H-ARCADE_RYOUGI", "ARCADE_RYOUGI", 2},
    {"Arcade C S.Akiha", "C-ARCADE_S_AKIHA", "ARCADE_S_AKIHA", 0},
    {"Arcade F S.Akiha", "F-ARCADE_S_AKIHA", "ARCADE_S_AKIHA", 1},
    {"Arcade H S.Akiha", "H-ARCADE_S_AKIHA", "ARCADE_S_AKIHA", 2},
    {"Arcade C Satsuki", "C-ARCADE_SATSUKI", "ARCADE_SATSUKI", 0},
    {"Arcade F Satsuki", "F-ARCADE_SATSUKI", "ARCADE_SATSUKI", 1},
    {"Arcade H Satsuki", "H-ARCADE_SATSUKI", "ARCADE_SATSUKI", 2},
    {"Arcade C Shiki", "C-ARCADE_SHIKI", "ARCADE_SHIKI", 0},
    {"Arcade F Shiki", "F-ARCADE_SHIKI", "ARCADE_SHIKI", 1},
    {"Arcade H Shiki", "H-ARCADE_SHIKI", "ARCADE_SHIKI", 2},
    {"Arcade C Sion", "C-ARCADE_SION", "ARCADE_SION", 0},
    {"Arcade F Sion", "F-ARCADE_SION", "ARCADE_SION", 1},
    {"Arcade H Sion", "H-ARCADE_SION", "ARCADE_SION", 2},
    {"Arcade C V.Sion", "C-ARCADE_V_SION", "ARCADE_V_SION", 0},
    {"Arcade F V.Sion", "F-ARCADE_V_SION", "ARCADE_V_SION", 1},
    {"Arcade H V.Sion", "H-ARCADE_V_SION", "ARCADE_V_SION", 2},
    {"Arcade C Warakia", "C-ARCADE_WARAKIA", "ARCADE_WARAKIA", 0},
    {"Arcade F Warakia", "F-ARCADE_WARAKIA", "ARCADE_WARAKIA", 1},
    {"Arcade H Warakia", "H-ARCADE_WARAKIA", "ARCADE_WARAKIA", 2},
    {"Arcade C Warcueid", "C-ARCADE_WARC", "ARCADE_WARC", 0},
    {"Arcade F Warcueid", "F-ARCADE_WARC", "ARCADE_WARC", 1},
    {"Arcade H Warcueid", "H-ARCADE_WARC", "ARCADE_WARC", 2},
    {"Arcade C W.Len", "C-ARCADE_WLEN", "ARCADE_WLEN", 0},
    {"Arcade F W.Len", "F-ARCADE_WLEN", "ARCADE_WLEN", 1},
    {"Arcade H W.Len", "H-ARCADE_WLEN", "ARCADE_WLEN", 2},
    {"Arcade E Archetype: Earth", "E-ARCADE_B_ARC", "ARCADE_B_ARC", 9},
    {"Arcade Effects", "ARCADE_EFFECT", "ARCADE_EFFECT", -1}};

static const int mbaa_ncharacter_info =
    sizeof(mbaa_character_info) / sizeof(mbaa_character_info[0]);

const char* MBAA_FrameDisplay::get_character_long_name(int n) {
  if (n < 0 || n >= mbaa_ncharacter_info) {
    return FrameDisplay::get_character_long_name(n);
  }
  return mbaa_character_info[n].long_name;
}

const char* MBAA_FrameDisplay::get_character_name(int n) {
  if (n < 0 || n >= mbaa_ncharacter_info) {
    return FrameDisplay::get_character_name(n);
  }
  return mbaa_character_info[n].short_name;
}

int MBAA_FrameDisplay::get_sequence_count() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_sequence_count();
}

bool MBAA_FrameDisplay::has_sequence(int n) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.has_sequence(n);
}
const char* MBAA_FrameDisplay::get_sequence_name(int n) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_sequence_name(n);
}

const char* MBAA_FrameDisplay::get_sequence_move_name(int n, int* dmeter) {
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

int MBAA_FrameDisplay::get_frame_count() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_frame_count(m_sequence);
}

int MBAA_FrameDisplay::get_subframe() { return m_subframe + m_subframe_base; }

int MBAA_FrameDisplay::get_subframe_count() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_subframe_count(m_sequence);
}

void MBAA_FrameDisplay::set_frame(int n) {
  if (!m_initialized) {
    return;
  }

  m_frame = m_character_data.find_frame(m_sequence, n);

  m_subframe = 0;

  m_subframe_base = m_character_data.count_subframes(m_sequence, m_frame);
  m_subframe_next = m_character_data.get_subframe_length(m_sequence, m_frame);
}

void MBAA_FrameDisplay::set_sequence(int n) {
  if (!m_initialized) {
    return;
  }

  m_sequence = m_character_data.find_sequence(n, n < m_sequence ? -1 : 1);

  m_subframe_base = 0;
  m_subframe = 0;

  set_frame(0);
}

void MBAA_FrameDisplay::set_palette(int n) {
  if (!m_initialized) {
    return;
  }

  m_palette = n % 36;

  m_character_data.set_palette(n);
}

void MBAA_FrameDisplay::set_active_character(int n) {
  if (!m_initialized) {
    return;
  }

  if (n == m_character) {
    return;
  }

  if (n < 0 || n >= (mbaa_ncharacter_info)) {
    return;
  }

  bool need_gfx = 1;

  if (m_character >= 0 && m_character < mbaa_ncharacter_info) {
    if (!strcmp(mbaa_character_info[m_character].file_name, mbaa_character_info[n].file_name)) {
      need_gfx = 0;
    }
  }

  if (need_gfx) {
    m_character_data.free();
  } else {
    m_character_data.free_frame_data();
  }

  if (m_character_data.load(&m_iso, mbaa_character_info[n].file_name,
                            mbaa_character_info[n].moon)) {
    if (need_gfx) {
      m_character_data.load_graphics(&m_iso);
    }
  }
  m_character = n;

  set_palette(m_palette);
  set_sequence(0);
  set_frame(0);
}

bool MBAA_FrameDisplay::init(const char* filename) {
  if (m_initialized) {
    return 1;
  }

  if (!m_iso.open_iso(filename)) {
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

bool MBAA_FrameDisplay::init() {
  if (m_initialized) {
    return 1;
  }

  if (!init("mbaa.iso")) {
    if (!init("gant-mbaa.iso")) {
      if (!init("mepRofs.cvm")) {
        return 0;
      }
    }
  }

  return 1;
}

void MBAA_FrameDisplay::free() {
  m_iso.close_iso();

  m_character_data.free();

  m_character = -1;

  m_subframe = 0;
  m_subframe_base = 0;
  m_subframe_next = 0;

  FrameDisplay::free();
}

void MBAA_FrameDisplay::render(const RenderProperties* properties) {
  if (!m_initialized) {
    return;
  }

  m_character_data.render(properties, m_sequence, m_frame);
}

Clone* MBAA_FrameDisplay::make_clone() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.make_clone(m_sequence, m_frame);
}

void MBAA_FrameDisplay::flush_texture() {
  if (!m_initialized) {
    return;
  }

  m_character_data.flush_texture();
}

void MBAA_FrameDisplay::render_frame_properties(bool detailed, int scr_width, int scr_height) {
  if (!m_initialized) {
    return;
  }

  m_character_data.render_frame_properties(detailed, scr_width, scr_height, m_sequence, m_frame);
}

void MBAA_FrameDisplay::command(FrameDisplayCommand command, void* param) {
  if (!m_initialized) {
    return;
  }

  switch (command) {
    case COMMAND_CHARACTER_NEXT:
      if (m_character == -1) {
        set_active_character(0);
      } else {
        set_active_character((m_character + 1) % (mbaa_ncharacter_info));
      }
      break;
    case COMMAND_CHARACTER_PREV:
      if (m_character == -1) {
        set_active_character(0);
      } else {
        set_active_character((m_character + (mbaa_ncharacter_info)-1) % mbaa_ncharacter_info);
      }
      break;
    case COMMAND_CHARACTER_SET:
      if (!param) {
        break;
      }
      set_active_character((int)(*(unsigned int*)param) % (mbaa_ncharacter_info));
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

const char* MBAA_FrameDisplay::get_current_sprite_filename() {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.get_current_sprite_filename(m_sequence, m_frame);
}

bool MBAA_FrameDisplay::save_current_sprite(const char* filename) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.save_current_sprite(filename, m_sequence, m_frame);
}

int MBAA_FrameDisplay::save_all_character_sprites(const char* directory) {
  if (!m_initialized) {
    return 0;
  }

  return m_character_data.save_all_character_sprites(directory);
}

MBAA_FrameDisplay::MBAA_FrameDisplay() {
  m_character = -1;
  m_subframe = 0;
  m_subframe_base = 0;
  m_subframe_next = 0;
}

MBAA_FrameDisplay::~MBAA_FrameDisplay() {
  // cleanup will do the work
}
