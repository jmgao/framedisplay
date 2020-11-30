#ifndef OUGON_FRAMEDISPLAY_H
#define OUGON_FRAMEDISPLAY_H

#include "framedisplay.h"
#include "texture.h"

#include <string>
#include <map>
#include <list>

// ************************************************** ougon_data.cpp

struct Ougon_SpriteInfo {
  int width;
  int height;
};

class Ougon_Data {
 private:
  unsigned char* m_data;
  unsigned int m_size;

  unsigned char* m_sprite_data;
  unsigned int m_sprite_size;

  unsigned char* m_patch_data;
  unsigned int m_patch_size;

  Ougon_SpriteInfo* m_sprite_info;
  int m_sprite_count;

  bool decompress(unsigned char* data, unsigned int size, unsigned char** ddest,
                  unsigned int* dsize);

  void init_sprite_info();

  bool load_and_decomp(const char* filename, const char* base_path, unsigned char** ddata,
                       unsigned int* dsize);

 public:
  bool open_patch(const char* filename, const char* base_path);

  bool open_pack(const char* filename, const char* base_path, bool isSprite);

  bool get_frame_data(unsigned char** ddata, unsigned int* dsize, int character_id);

  int get_sprite_count();
  Texture* get_sprite(int id);
  Ougon_SpriteInfo* get_sprite_info(int id);

  void free_pack();

  void free_all();

  bool loaded();

  Ougon_Data();
  ~Ougon_Data();
};

// ************************************************** ougon_framedata.cpp

struct Ougon_SequenceHeader {
  unsigned int unknown[0x40 / 4];
  unsigned int frame_count;
};

struct Ougon_Hitbox {
  short x;
  short y;
  short w;
  short h;
};

struct Ougon_Frame {
  unsigned short sprite_id;
  unsigned short duration;
  unsigned short unknown02;
  short tex_x;
  short tex_y;
  unsigned short unknown05;
  unsigned short unknown06;
  unsigned short unknown07;
  unsigned short unknown08;
  unsigned short unknown09;
  unsigned short unknown0a;
  unsigned short unknown0b;
  unsigned short unknown0c;
  unsigned short unknown0d;

  Ougon_Hitbox attackbox;
  Ougon_Hitbox hitboxes[3];

  unsigned short unknown1e;
  unsigned short unknown1f;
  unsigned short unknown20;
  unsigned short unknown21;

  struct {
    unsigned char damage;  // *3
    unsigned char prop1;
    unsigned short prop2;
    unsigned short prop3;
    unsigned short prop4;
  } attack;

  unsigned short unknown26;
  unsigned short unknown27;
  unsigned short unknown28;
  unsigned short unknown29;
  unsigned short unknown2a;
  unsigned short unknown2b;
  unsigned short unknown2c;
  unsigned short unknown2d;
  unsigned short unknown2e;
  unsigned short unknown2f;
};

struct Ougon_Sequence {
  Ougon_SequenceHeader* header;
  Ougon_Frame* frames;

  unsigned int frame_count;
};

class Ougon_Framedata {
 private:
  bool m_initialized;

  unsigned char* m_data;
  int m_size;

  Ougon_Sequence m_sequences[250];

 public:
  bool load(Ougon_Data* data, int character_id);

  int get_sequence_count();
  bool has_sequence(int seq_id);
  Ougon_Sequence* get_sequence(int seq_id);

  int get_frame_count(int seq_id);
  Ougon_Frame* get_frame(int seq_id, int fr_id);

  void free();

  bool loaded();

  Ougon_Framedata();
  ~Ougon_Framedata();
};

// ************************************************** abk_framedisplay.cpp

class Ougon_FrameDisplay : public FrameDisplay {
 protected:
  bool m_initialized;

  Ougon_Data m_data;
  Ougon_Data m_sprite_data;

  Ougon_Framedata m_framedata;

  int m_subframe_base;
  int m_subframe_next;
  int m_subframe;

  Texture* m_texture;
  int m_last_sprite;

  char* m_base_path;

  int m_ncharacters;

  void set_render_properties(Texture* texture, int seq_id, int fr_id);

  void set_character(int n);
  void set_sequence(int n, bool prev_dir = 0);
  void set_frame(int n);

 public:
  virtual const char* get_character_name(int n);
  virtual const char* get_character_long_name(int n);

  virtual int get_sequence_count();
  virtual bool has_sequence(int n);
  virtual const char* get_sequence_name(int n);
  virtual const char* get_sequence_move_name(int n, int* dmeter);

  virtual int get_frame_count();
  virtual int get_subframe();
  virtual int get_subframe_count();

  virtual void render(const RenderProperties* properties);
  virtual Clone* make_clone();

  virtual void flush_texture();

  virtual void render_frame_properties(bool detailed, int scr_width, int scr_height);

  virtual void command(FrameDisplayCommand command, void* param);

  virtual const char* get_current_sprite_filename();
  virtual bool save_current_sprite(const char* filename);
  virtual int save_all_character_sprites(const char* directory);

  virtual bool init();
  virtual bool init(const char* filename);

  virtual void free();

  Ougon_FrameDisplay();
  virtual ~Ougon_FrameDisplay();
};

#endif
