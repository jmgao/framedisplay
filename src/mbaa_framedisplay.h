#ifndef MBAA_FRAMEDISPLAY_H
#define MBAA_FRAMEDISPLAY_H

#include <string>
#include <list>
#include <map>

#include "framedisplay.h"

#include "texture.h"

#include "mbaa_iso.h"

// ************************************************** mbaa_tileimage.cpp
class MBAA_TileImage {
protected:
	bool		m_loaded;
	
	unsigned char	*m_tile_data;
	bool		m_8bpp;
	
	unsigned int	*m_stored_palette;
public:
	void		copy_region_to(unsigned char *pixels,
				int dx, int dy, int dw, int dh,
				int sx, int sy, int sw, int sh,
				unsigned int *palette,
				bool is_8bpp);
	
	void		free();

	bool		load_enc(const unsigned char *data, int size);
	bool		load_pvr(const unsigned char *data, int size);
	bool		load_env(const unsigned char *data, int size);
	
	bool		store_pal(const unsigned char *data, int size);
	
	bool		is_8bpp();
	
			MBAA_TileImage();
			~MBAA_TileImage();
};

// ************************************************** mbaa_arx.cpp

class MBAA_ARX {
protected:
	bool		m_loaded;
	
	std::map < std::string, MBAA_TileImage * >	m_map;
public:
	bool		load(MBAA_ISO *iso, const char *name);
	
	MBAA_TileImage *get_tileimage(const char *name);
	
	void		free();
	
			MBAA_ARX();
			~MBAA_ARX();
};

// ************************************************** mbaa_cg2.cpp

struct MBAA_CG2_Image;
struct MBAA_CG2_Alignment;

class MBAA_CG2 {
protected:
	bool				m_loaded;
	
	char				*m_data;
	unsigned int			m_data_size;
	
	const unsigned int		*m_indices;
	
	unsigned int			m_nimages;
	
	const MBAA_CG2_Alignment	*m_align;
	unsigned int			m_nalign;
	
	const MBAA_CG2_Image	*get_image(unsigned int n);
public:
	bool			load(MBAA_ISO *iso, const char *name);
	
	void			free();
	
	const char		*get_filename(unsigned int n);
	
	Texture			*draw_texture(unsigned int n, MBAA_ARX *arx,
					unsigned int *palette, bool to_pow2,
					bool draw_8bpp = 0);
	
	int			get_image_count();
	
				MBAA_CG2();
				~MBAA_CG2();
};

// ************************************************** mbaa_framedata.cpp

// cleverly organized to be similar to MBAC's frame data.

struct MBAA_Hitbox {
	short x1, y1, x2, y2;
};

struct MBAA_Frame_AF {
	// rendering data
	bool		active;
	
	int		frame;
	int		frame_unk;
	
	int		offset_y;
	int		offset_x;
	
	int		duration;
	int		AFF;
	
	int		blend_mode;
	
	unsigned char	alpha;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
	
	float		z_rotation;
	float		y_rotation;
	float		x_rotation;
	
	bool		has_zoom;
	float		zoom_x;
	float		zoom_y;

	int		AFJP;
};

struct MBAA_Frame_AS {
	// state data
	int		speed_flags;
	int		speed_horz;
	int		speed_vert;
	int		accel_horz;
	int		accel_vert;
	
	int		ASMV;
	
	int		stand_state;
	
	int		cancel_flags;
};

struct MBAA_Frame_AT {
	bool		active;
	
	int		guard_flags;
	
	int		proration;
	
	int		damage;
	int		red_damage;
	int		dmg_unknown;
	int		circuit_gain;
};

struct MBAA_Frame_EF {
	int		command;
	int		parameter;
	int		values[12];
};

struct MBAA_Frame_IF {
	int		command;
	int		values[12];
};

struct MBAA_Frame {
	MBAA_Frame_AF	AF;
	
	MBAA_Frame_AS	*AS;
	MBAA_Frame_AT	*AT;
	
	MBAA_Frame_EF	*EF[8];
	MBAA_Frame_IF	*IF[8];
	
	MBAA_Hitbox	*hitboxes[33];
};

struct MBAA_Sequence {
	// sequence property data
	std::string	name;
	
	bool		is_move;
	std::string	move_name;
	int		move_meter;
	
	int		subframe_count;
	
	bool		initialized;
	
	char		*data;
	
	MBAA_Frame	*frames;
	unsigned int	nframes;
	
	MBAA_Hitbox	*hitboxes;
	unsigned int	nhitboxes;
	
	MBAA_Frame_AT	*AT;
	unsigned int	nAT;
	
	MBAA_Frame_AS	*AS;
	unsigned int	nAS;
	
	MBAA_Frame_EF	*EF;
	unsigned int	nEF;
	
	MBAA_Frame_IF	*IF;
	unsigned int	nIF;
};

class MBAA_FrameData {
private:
	MBAA_Sequence	**m_sequences;
	unsigned int	m_nsequences;
	
	bool		m_loaded;
public:
	bool		load(MBAA_ISO *iso, const char *filename);

	bool		load_move_list(MBAA_ISO *iso, const char *filename);
	
	int		get_sequence_count();
	
	MBAA_Sequence	*get_sequence(int n);
	
	void		free();
	
		MBAA_FrameData();
		~MBAA_FrameData();
};


// ************************************************** mbaa_character.cpp

class MBAA_Character {
protected:
	bool		m_loaded;
	
	MBAA_FrameData	m_framedata;
	
	char		*m_name;
	
	MBAA_ARX	m_arx;
	
	MBAA_CG2	m_cg2;
	
	unsigned int	**m_palettes;
	
	int		m_active_palette;
	
	Texture		*m_texture;
	int		m_last_sprite_id;
	
	bool		do_sprite_save(int id, const char *filename);
	
	MBAA_Frame *	get_frame(int seq_id, int fr_id);
	
	void		set_render_properties(const MBAA_Frame *frame, Texture *texture);
public:
	bool		load(MBAA_ISO *iso, const char *name, int sub_type);
	
	void		load_graphics(MBAA_ISO *iso);
	void		unload_graphics();
	
	void		render(const RenderProperties *properties, int seq, int frame);
	Clone		*make_clone(int seq_id, int fr_id);
	
	void		flush_texture();

	void		render_frame_properties(bool detailed, int scr_width, int scr_height, int seq, int frame);
	
	void		set_palette(int n);
	
	int		get_sequence_count();
	bool		has_sequence(int n);
	const char *	get_sequence_name(int n);
	const char *	get_sequence_move_name(int n, int *meter);
	
	int		get_frame_count(int seq_id);
	int		get_subframe_count(int seq_id);
	int		get_subframe_length(int seq_id, int fr_id);
	int		count_subframes(int seq_id, int fr_id);
	
	int		find_sequence(int seq_id, int direction);
	int		find_frame(int seq_id, int fr_id);
	
	const char *	get_current_sprite_filename(int seq_id, int fr_id);
	bool		save_current_sprite(const char *filename, int seq_id, int fr_id);
	int		save_all_character_sprites(const char *directory);
	
	void		free_frame_data();
	void		free_graphics();
	void		free();
	
			MBAA_Character();
			~MBAA_Character();
};

// ************************************************** mbaa_framedisplay.cpp

class MBAA_FrameDisplay : public FrameDisplay {
protected:
	MBAA_ISO	m_iso;
	
	MBAA_Character	m_character_data;
	
	int		m_subframe_base;
	int		m_subframe_next;
	int		m_subframe;
	
	void		set_active_character(int n);
	void		set_palette(int n);
	void		set_sequence(int n);
	void		set_frame(int n);
public:
	virtual const char *get_character_long_name(int n);
	virtual const char *get_character_name(int n);
	virtual int	get_sequence_count();
	virtual bool	has_sequence(int n);
	virtual const char *get_sequence_name(int n);
	virtual const char *get_sequence_move_name(int n, int *meter);
	virtual int	get_frame_count();
	virtual int	get_subframe();
	virtual int	get_subframe_count();
	
	virtual void	render(const RenderProperties *properties);
	virtual Clone	*make_clone();
	
	virtual void	flush_texture();

	virtual void	render_frame_properties(bool detailed, int scr_width, int scr_height);
	
	virtual void	command(FrameDisplayCommand command, void *param);
	
	virtual const char *get_current_sprite_filename();
	virtual bool	save_current_sprite(const char *filename);
	virtual int	save_all_character_sprites(const char *directory);
	
	virtual bool	init();
	virtual bool	init(const char *filename);
	
	virtual	void	free();
	
			MBAA_FrameDisplay();
	virtual		~MBAA_FrameDisplay();
};

#endif
