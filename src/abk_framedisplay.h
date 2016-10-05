#ifndef ABK_FRAMEDISPLAY_H
#define ABK_FRAMEDISPLAY_H

#include "framedisplay.h"
#include "texture.h"

#include <string>
#include <map>
#include <list>

// ************************************************** abk_packfiles.cpp

struct ABK_Packfile;
struct ABK_File;

class ABK_Packfiles {
private:
	std::map< std::string, ABK_File >	m_map;
	
	std::list< ABK_Packfile >		m_packfile_list;
	
	char *		unpack_zcp(char *data, unsigned int size, unsigned int *dsize);
	
	char *		read_file_int(ABK_File *file, unsigned int *dsize);

	ABK_File *	get_pack_entry(const char *packname, int n);
public:
	bool		open_pack(const char *filename);
	
	bool		register_rsp(const char *filename);
	
	char *		read_file(const char *filename, unsigned int *dsize);
	char *		read_pack_entry(const char *packname, int n, unsigned int *dsize);
	
	int		get_pack_file_count(const char *packname);
	const char *	get_pack_file_name(const char *packname, int n);
	
	void		free();
	
			ABK_Packfiles();
			~ABK_Packfiles();
};

// ************************************************** abk_images.cpp

struct ABK_Image;

class ABK_Images {
private:
	bool		m_initialized;
	
	char		m_packpath[64];
	
	unsigned int	m_npalettes;
	unsigned int	**m_palettes;
	
	unsigned int	m_nimages;
	ABK_Image	*m_images;
	
	ABK_Packfiles	*m_packs;
public:
	bool		load_images(ABK_Packfiles *packs, const char *charname);

	Texture *	get_texture(int n, int pal_no, bool force_8bpp);
	
	int		get_width(int n);
	
	int		get_palette_count();
	unsigned int *	get_palette(int n);
	
	int		get_image_count();
	const char *	get_image_filename(int n);
	
	void		free();
	
			ABK_Images();
			~ABK_Images();
};

// ************************************************** abk_framedata.cpp

struct ABK_RenderInfo {
	int		x;
	int		y;
	int		flags;
	int		sprite;
	int		angle;
};

struct ABK_Frame {
	int		id;
	int		duration;
};

struct ABK_Hitbox {
	int		x1;
	int		y1;
	int		x2;
	int		y2;
	int		type;
};

class ABK_Framedata { 
private:
	bool		m_initialized;
	
	char		*m_data;
	int		m_size;
	
	int		**m_renderinfo;
	int		m_nrenderinfo;
	
	int		**m_sequences;
	int		m_nsequences;
	
	int		**m_boxdata;
	int		m_nboxdata;
public:
	bool		load(ABK_Packfiles *packs, const char *char_name);

	int		get_sequence_count();
	bool		has_sequence(int seq_id);
	
	int		get_renderinfo_count(int seq_id);
	ABK_RenderInfo *get_renderinfo(int seq_id, int ri_id);
	int		get_frame_count(int seq_id);
	ABK_Frame *	get_frame(int seq_id, int fr_id);
	int		get_hitbox_count(int seq_id);
	ABK_Hitbox *	get_hitbox(int seq_id, int box_id);
	
	void		free();
	
			ABK_Framedata();
			~ABK_Framedata();
};

// ************************************************** abk_framedisplay.cpp

struct ABK_TempRenderData;

class ABK_FrameDisplay : public FrameDisplay {
protected:
	bool			m_initialized;
	
	ABK_Packfiles		m_packs;
	ABK_Images		m_imagedata;
	ABK_Framedata		m_framedata;
	
	int			m_subframe_base;
	int			m_subframe_next;
	int			m_subframe;
	
	Texture			*m_textures[16];
	int			m_texture_id[16];
	
	void			calc_render(ABK_TempRenderData *rd, int seq_id, int ri_id, int count);
	
	void			set_render_properties(Texture *texture, int seq_id, int ri_id);
	
	void			set_character(int n);
	void			set_sequence(int n, bool prev_dir = 0);
	void			set_frame(int n);
public:
	virtual const char *	get_character_name(int n);
	virtual const char *	get_character_long_name(int n);
	
	virtual int		get_sequence_count();
	virtual bool		has_sequence(int n);
	//virtual const char *	get_sequence_name(int n);
	//virtual const char *	get_sequence_move_name(int n, int *dmeter);
	
	virtual int		get_frame_count();
	virtual int		get_subframe();
	virtual int		get_subframe_count();
	
	virtual void		render(const RenderProperties *properties);
	virtual Clone *		make_clone();
	
	virtual void		flush_texture();
	
	// ABK framedisp doesn't know what the properties are
	//virtual void		render_frame_properties(bool detailed, int scr_width, int scr_height);
	
	virtual void		command(FrameDisplayCommand command, void *param);
	
	virtual const char *	get_current_sprite_filename();
	virtual bool		save_current_sprite(const char *filename);
	virtual int		save_all_character_sprites(const char *directory);
	
	virtual bool		init();
	virtual bool		init(const char *filename);
	
	virtual void		free();
	
				ABK_FrameDisplay();
	virtual			~ABK_FrameDisplay();
};

#endif
