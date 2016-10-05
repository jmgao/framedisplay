#ifndef TOUHOU_FRAMEDISPLAY_H
#define TOUHOU_FRAMEDISPLAY_H

/*

Dear Tasogare Frontier:

Please make Touhou Suimusou 2. Or something similar.

Love, mauve.

 */

#include <list>

#include "framedisplay.h"
#include "texture.h"

enum Touhou_GameID {
	TOUHOU_GAME_NONE,
	TOUHOU_GAME_TH075,
	TOUHOU_GAME_TH075_11,
	TOUHOU_GAME_TH105,
	TOUHOU_GAME_TH123,
	TOUHOU_GAME_TH123_105
};

// ************************************************** touhou_packfiles.cpp

struct Touhou_Packfile_Data;

class Touhou_Packfiles {
private:
	std::list<Touhou_Packfile_Data *>	m_packfile_list;
public:
	bool		add_th075(const char *filename);
	bool		add_th105_123(const char *filename, bool back);
	
	bool		has_file(const char *filename);
	
	char *		read_file(const char *filename, unsigned int *dsize);
	
	void		free();
	
			Touhou_Packfiles();
			~Touhou_Packfiles();
};

// ************************************************** touhou_images.cpp

struct Touhou_Image;

class Touhou_Imagedata {
private:
	int		m_initialized;
	
	char		*m_th075_data;
	int		m_th075_size;
	
	bool		m_th075;
	
	Touhou_Image	*m_images;
	int		m_nimages;
	
	unsigned int	**m_palettes;
	int		m_npalettes;
	
	Touhou_Packfiles *m_packfile;
public:
	bool		load_th075(Touhou_Packfiles *packfile, int n);
	
	bool		load_th105_from_framedata(Touhou_Packfiles *packfile, const char *charname, char *data, int size);
	
	Texture *	get_texture(int n, int palette_no, bool palettized);
	
	int		get_image_count();
	
	const char *	get_filename(int n);
	
	int		get_palette_count();
	unsigned int *	get_palette(int n);
	
	void		free();
	
			Touhou_Imagedata();
			~Touhou_Imagedata();
};

// ************************************************** touhou_framedata.cpp

struct Touhou_Rect {
	int		x1, y1, x2, y2;
};

struct Touhou_Frame {
	struct {
		short render_frame;
		short tex_width;	// th105
		short tex_height;	// th105
		short x_offset;
		short y_offset;
		short duration;
		unsigned char identifier;
	} header;
	
	struct {
		short blend_mode;
		short scale;
		short angle;
	} type_2;
	
	struct {
		short damage;
		short proration;
		short chip;
		short untech;
		short limit;
		short unk9;
		short unk10;
		short velocity_x;
		short velocity_y;
		short unk15;
		short unk16;
		short attack_type;
		unsigned int fflags;
		unsigned int aflags;
	} props;
	
	int		n_boxes_collision;
	Touhou_Rect	*boxes_collision;
	
	int		n_boxes_hit;
	Touhou_Rect	*boxes_hit;
	
	int 		n_boxes_attack;
	Touhou_Rect	*boxes_attack[20];
	
	char		*raw_frame_ptr;
	int		raw_frame_size;
};

struct Touhou_Sequence {
	int		identifier;
	int		nframes;
	
	char		*raw_header_ptr;
	int		raw_header_size;
	
	Touhou_Frame	*frames;
};

class Touhou_Framedata {
private:
	int		m_initialized;
	
	Touhou_Sequence	*m_sequences;
	unsigned int	m_nsequences;
	
	char		*m_data;
	int		m_size;
public:
	bool		init_th075(Touhou_Packfiles *pack, const char *name);
	bool		init_th105_123(Touhou_Packfiles *pack, Touhou_Imagedata *imagedata, const char *name);
	
	unsigned int	get_sequence_count();
	bool		has_sequence(unsigned int n);
	Touhou_Sequence *get_sequence(unsigned int n);
	
	void		free();
	
			Touhou_Framedata();
			~Touhou_Framedata();
};

// ************************************************** touhou_framedisplay.cpp

class Touhou_FrameDisplay : public FrameDisplay {
protected:
	Touhou_GameID		m_game_id;
	
	Touhou_Packfiles	m_packs;
	
	Touhou_Framedata	m_framedata;
	Touhou_Imagedata	m_imagedata;
	
	int			m_sequence_id_lookup[1000];
	
	Texture *		m_texture;
	int			m_last_sprite;
	
	int			m_subframe_base;
	int			m_subframe;
	int			m_subframe_next;
	
	int			m_max_character;
	
	bool			init_internal();
	
	Touhou_Frame *		get_frame_data(int seq_id, int fr_id);
	
	const char *		get_sprite_filename(int n);
	
	void			set_character(int n);
	void			set_sequence(int n);
	void			set_frame(int n);
public:
	virtual const char *	get_character_long_name(int n);
	virtual const char *	get_character_name(int n);
	
	virtual int		get_sequence_count();
	virtual bool		has_sequence(int n);
	virtual const char *	get_sequence_name(int n);
	virtual const char *	get_sequence_move_name(int n, int *dmeter);
	
	virtual int		get_frame_count();
	virtual int		get_subframe();
	virtual int		get_subframe_count();
	
	virtual void		render(const RenderProperties *properties);
	virtual Clone *		make_clone();
	
	virtual void		flush_texture();
	
	virtual void		render_frame_properties(bool detailed, int scr_width, int scr_height);
	
	virtual void		command(FrameDisplayCommand command, void *param);
	
	virtual const char *	get_current_sprite_filename();
	virtual bool		save_current_sprite(const char *filename);
	virtual int		save_all_character_sprites(const char *directory);
	
	virtual bool		init();
	virtual bool		init(const char *filename);
	
	virtual void		free();
	
				Touhou_FrameDisplay();
	virtual			~Touhou_FrameDisplay();
};

#endif
