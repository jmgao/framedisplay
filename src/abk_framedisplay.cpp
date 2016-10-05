#include "abk_framedisplay.h"
#include "render.h"
#include "clone.h"

#include <cmath>
#include <cstdio>
#include <cstring>

struct ABK_Character_Info {
	const char *name;
	const char *int_name;
};

static const ABK_Character_Info abk_character_info[] = {
	{"Akatsuki",	"aka"	},
	{"Mycale",	"myc"	},
	{"Sai",		"sai"	},
	{"Kanae",	"kan"	},
	{"Fritz",	"fri"	},
	{"Marilyn",	"mar"	},
	{"Wei",		"pow"	},
	{"Anonym",	"ano"	},
	{"Soldat",	"sol"	},
	{"Adler",	"adl"	},
	{"Blitztank",	"tan"	},
	{"Murakumo",	"bos"	},
	{"Dice",	"dic"	}
};

static const int abk_ncharacters = sizeof(abk_character_info)/sizeof(abk_character_info[0]);

const char *ABK_FrameDisplay::get_character_name(int n) {
	if (n < 0 || n >= abk_ncharacters) {
		return FrameDisplay::get_character_name(n);
	}
	
	return abk_character_info[n].name;
}

const char *ABK_FrameDisplay::get_character_long_name(int n) {
	return get_character_name(n);
}

int ABK_FrameDisplay::get_sequence_count() {
	return m_framedata.get_sequence_count();
}

bool ABK_FrameDisplay::has_sequence(int n) {
	return m_framedata.has_sequence(n);
}

int ABK_FrameDisplay::get_frame_count() {
	return m_framedata.get_frame_count(m_sequence);
}

int ABK_FrameDisplay::get_subframe() {
	return m_subframe_base;
}

int ABK_FrameDisplay::get_subframe_count() {
	int fr_count = m_framedata.get_frame_count(m_sequence);
	
	int count = 0;
	
	for (int i = 0 ; i < fr_count; ++i) {
		ABK_Frame *frame = m_framedata.get_frame(m_sequence, i);
		if (frame) {
			count += frame->duration;
		}
	}
	
	return count;
}

static void copy_box_to_rect(rect_t *rect, ABK_Hitbox *hitbox) {
	rect->x1 = hitbox->x1;
	rect->y1 = hitbox->y1;
	rect->x2 = hitbox->x2;
	rect->y2 = hitbox->y2;
}

struct ABK_TempRenderData {
	int x;
	int y;
	int angle;
};

void ABK_FrameDisplay::calc_render(ABK_TempRenderData *rd, int seq_id, int ri_id, int count) {
	ABK_RenderInfo *ri = m_framedata.get_renderinfo(seq_id, ri_id);
	if (!ri) {
		return;
	}
	
	if (count > 0) {
		int n = ((ri->flags>>16)&0xff)-1;
		
		calc_render(rd, seq_id, n, count - 1);
	}
	
	if (rd->angle != 0) {
		//  cos(ang) -sin(ang)  tx
		//  sin(ang)  cos(ang)  ty
		
		double ang = (double)rd->angle * M_PI / 180.0f;
		double cang = cos(ang);
		double sang = sin(ang);

		double sx = ri->x;
		double sy = ri->y;
		double x = (sx * cang) - (sy * sang);
		double y = (sx * sang) + (sy * cang);
		
		rd->x += x;
		rd->y += y;
	} else {
		rd->x += ri->x;
		rd->y += ri->y;
	}
	
	rd->angle += ri->angle;
}

void ABK_FrameDisplay::set_render_properties(Texture *texture, int seq_id, int ri_id) {
	ABK_RenderInfo *ri = m_framedata.get_renderinfo(seq_id, ri_id);
	
	ABK_TempRenderData rd;
	
	int rcount = (ri->flags >> 24) & 0xff;
	if (rcount > 0 && rcount < 8) {
		rd.angle = 0;
		rd.x = 0;
		rd.y = 0;
		calc_render(&rd, seq_id, ri_id, rcount);
	} else {
		rd.angle = ri->angle;
		rd.x = ri->x;
		rd.y = ri->y;
	}
	
	texture->special_mode(1);

	// flip?
	if (ri->flags & 1) {
		rd.x += m_imagedata.get_width(ri->sprite);
		texture->special_mode(3);
	}
	
	texture->offset(rd.x, rd.y);
	
	if (rd.angle) {
		texture->rotate_z((float)rd.angle / 360.0);
	} else {
		texture->rotate_clear();
	}
}

void ABK_FrameDisplay::render(const RenderProperties *properties) {
	ABK_Frame *frame = m_framedata.get_frame(m_sequence, m_frame);
	if (!frame) {
		return;
	}
	
	int render_count = m_framedata.get_renderinfo_count(frame->id);
	if (render_count > 0 && properties->display_sprite) {
		if (render_count > 16) {
			render_count = 16;
		}
		for (int i = render_count - 1; i >= 0; --i) {
			ABK_RenderInfo *ri = m_framedata.get_renderinfo(frame->id, i);
			
			Texture *texture = m_textures[i];
			
			if (!texture || m_texture_id[i] != ri->sprite) {
				if (texture) {
					delete texture;
				}
				
				texture = m_imagedata.get_texture(ri->sprite, m_palette, 0);
				
				m_textures[i] = texture;
				m_texture_id[i] = ri->sprite;
				
				if (!texture) {
					continue;
				}
			}
			
			set_render_properties(texture, frame->id, i);
			
			texture->draw(0, 0, 1);
		}
	}
	
	// render the boxes
	int box_count = m_framedata.get_hitbox_count(frame->id);
	if (box_count > 0) {
		rect_t rects[box_count];
		
		for (int j = 0; j < 3; ++j) {
			bool render;
			BoxType box_type;
			int andmask;
			
			switch(j) {
			case 0:
				render = properties->display_collision_box;
				box_type = BOX_COLLISION;
				andmask = 1;
				break;
			case 1:
				render = properties->display_hit_box;
				box_type = BOX_HIT;
				andmask = 4;
				break;
			case 2:
				render = properties->display_attack_box;
				box_type = BOX_ATTACK;
				andmask = 2;
				break;
			default: // never happens
				render = 0;
				box_type = BOX_DEFAULT;
				andmask = 0;
				break;
			}
			
			if (!render) {
				continue;
			}
			
			int count = 0;
			for (int i = 0; i < box_count; ++i) {
				ABK_Hitbox *hitbox = m_framedata.get_hitbox(frame->id, i);
				if (hitbox) {
					if (hitbox->type & andmask) {
						copy_box_to_rect(&rects[count], hitbox);
						++count;
					}
				}
			}
			
			if (count > 0) {
				render_boxes(box_type, rects, count, properties->display_solid_boxes);
			}
		}
	}
}

Clone *ABK_FrameDisplay::make_clone() {
	ABK_Frame *frame = m_framedata.get_frame(m_sequence, m_frame);
	if (!frame) {
		return 0;
	}
	
	Clone *clone = new Clone;
	
	// only do one sprite. sorry, blitztank
	int render_count = m_framedata.get_renderinfo_count(frame->id);
	if (render_count == 1) {
		ABK_RenderInfo *ri = m_framedata.get_renderinfo(frame->id, 0);
		
		Texture *texture = m_imagedata.get_texture(ri->sprite, m_palette, 0);
		
		if (texture) {
			set_render_properties(texture, frame->id, 0);
			
			clone->init_texture(texture, 0, 0, 1);
		}
	}
	
	// count hitboxes
	int box_count = m_framedata.get_hitbox_count(frame->id);
	int nboxes = 0;
	for (int i = 0; i < box_count; ++i) {
		ABK_Hitbox *box = m_framedata.get_hitbox(frame->id, i);
		if (box) {
			int t = box->type;
			
			for (int j = 0; j < 3; ++j) {
				if (t & 1) {
					++nboxes;
				}
				
				t >>= 1;
			}
		}
	}
	
	if (nboxes > 0) {
		CloneHitbox boxes[nboxes];
		CloneHitbox *dbox = &boxes[0];
		
		for (int i = 0; i < 3; ++i) {
			BoxType box_type;
			int andmask;
			
			switch (i) {
			case 0:
				box_type = BOX_COLLISION;
				andmask = 1;
				break;
			case 1:
				box_type = BOX_HIT;
				andmask = 4;
				break;
			case 2:
				box_type = BOX_ATTACK;
				andmask = 2;
				break;
			default:
				box_type = BOX_DEFAULT;
				andmask = 0;
				break;
			}
			
			for (int j = 0; j < box_count; ++j) {
				ABK_Hitbox *box = m_framedata.get_hitbox(frame->id, j);
				if (box && (box->type & andmask)) {
					dbox->type = box_type;
					dbox->rect.x1 = box->x1;
					dbox->rect.y1 = box->y1;
					dbox->rect.x2 = box->x2;
					dbox->rect.y2 = box->y2;
					
					++dbox;
				}
			}
		}
		
		nboxes = dbox - boxes;
		
		clone->init_hitboxes(boxes, nboxes);
	}
	
	return clone;
}

void ABK_FrameDisplay::flush_texture() {
	for (int i = 0; i < 16; ++i) {
		if (m_textures[i]) {
			delete m_textures[i];
			m_textures[i] = 0;
		}
		
		m_texture_id[i] = -1;
	}
}

const char *ABK_FrameDisplay::get_current_sprite_filename() {
	ABK_Frame *frame = m_framedata.get_frame(m_sequence, m_frame);
	if (!frame) {
		return 0;
	}
	
	int render_count = m_framedata.get_renderinfo_count(frame->id);
	if (render_count != 1) {
		return 0;
	}
	
	ABK_RenderInfo *ri = m_framedata.get_renderinfo(frame->id, 0);
	
	return m_imagedata.get_image_filename(ri->sprite);
}

bool ABK_FrameDisplay::save_current_sprite(const char *filename) {
	ABK_Frame *frame = m_framedata.get_frame(m_sequence, m_frame);
	if (!frame) {
		return 0;
	}
	
	int render_count = m_framedata.get_renderinfo_count(frame->id);
	if (render_count != 1) {
		return 0;
	}
	
	ABK_RenderInfo *ri = m_framedata.get_renderinfo(frame->id, 0);
	
	Texture *texture = m_imagedata.get_texture(ri->sprite, m_palette, 1);
	bool retval = 0;
	
	if (texture) {
		retval = texture->save_to_png(filename, m_imagedata.get_palette(m_palette));
		
		delete texture;
	}
	
	return retval;
}

int ABK_FrameDisplay::save_all_character_sprites(const char *directory) {
	if (!m_initialized) {
		return 0;
	}
	
	int n = m_imagedata.get_image_count();
	int count = 0;
	
	for (int i = 0; i < n; ++i) {
		const char *image_filename = m_imagedata.get_image_filename(i);
		if (!image_filename) {
			continue;
		}
		
		Texture *texture = m_imagedata.get_texture(i, m_palette, 1);
		
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

void ABK_FrameDisplay::set_character(int n) {
	if (n < 0) {
		n = abk_ncharacters - 1;
		if (n < 0) {
			return;
		}
	}
	if (n >= abk_ncharacters) {
		n = 0;
	}
	
	if (n == m_character) {
		return;
	}
	
	// unload images, frame data
	m_imagedata.free();
	m_framedata.free();
	
	// register new images, frame data
	m_imagedata.load_images(&m_packs, abk_character_info[n].int_name);
	m_framedata.load(&m_packs, abk_character_info[n].int_name);
	
	// finish up
	m_character = n;
	
	if (m_palette >= m_imagedata.get_palette_count()) {
		m_palette = 0;
	}
	
	set_sequence(0);
}

void ABK_FrameDisplay::set_sequence(int n, bool prev_dir) {
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

void ABK_FrameDisplay::set_frame(int n) {
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
		ABK_Frame *frame = m_framedata.get_frame(m_sequence, i);
		
		if (frame) {
			m_subframe_base += frame->duration;
		}
	}
	
	ABK_Frame *frame = m_framedata.get_frame(m_sequence, n);
	if (frame) {
		m_subframe_next = frame->duration;
	} else {
		m_subframe_next = 0;
	}
	
	m_subframe = 0;
	
	flush_texture();
}

void ABK_FrameDisplay::command(FrameDisplayCommand command, void *param) {
	switch(command) {
	case COMMAND_CHARACTER_NEXT:
		set_character(m_character + 1);
		break;
	case COMMAND_CHARACTER_PREV:
		set_character(m_character - 1);
		break;
	case COMMAND_CHARACTER_SET:
		if (param) {
			set_character(*(int *)param);
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
			set_sequence(*(int *)param);
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
			set_frame(*(int *)param);
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
			m_palette = (m_palette - 1 + m_imagedata.get_palette_count()) % m_imagedata.get_palette_count();
			flush_texture();
		}
		break;
	case COMMAND_PALETTE_SET:
		if (m_imagedata.get_palette_count() > 0) {
			m_palette = *(int *)param % m_imagedata.get_palette_count();
			
			flush_texture();
		}
		break;
	}
}

bool ABK_FrameDisplay::init(const char *base_path) {
	if (m_initialized) {
		return 0;
	}
	
	// open the two base packfiles
	int len = strlen(base_path);
	char path[len + 64];
	
	sprintf(path, "%sdata.pak", base_path);
	
	if (!m_packs.open_pack(path)) {
		return 0;
	}
	
	sprintf(path, "%spatch.pak", base_path);
	
	if (!m_packs.open_pack(path)) {
		m_packs.free();
		return 0;
	}
	
	// we're good. register the rsps
	for (int i = 0; i < abk_ncharacters; ++i) {
		// frame data
		sprintf(path, "./data/chr/%s/9000.rsp", abk_character_info[i].int_name);
		if (!m_packs.register_rsp(path)) {
			m_packs.free();
			return 0;
		}

		// images
		sprintf(path, "./data/chr/%s/0000.rsp", abk_character_info[i].int_name);
		if (!m_packs.register_rsp(path)) {
			m_packs.free();
			return 0;
		}
	}
	
	// finish up
	m_initialized = 1;
	
	m_character = -1;
	
	set_character(0);
	
	return 1;
}

bool ABK_FrameDisplay::init() {
	return init("");
}

void ABK_FrameDisplay::free() {
	m_packs.free();
	m_imagedata.free();
	m_framedata.free();
	
	flush_texture();
	
	m_subframe_base = 0;
	m_subframe_next = 0;
	m_subframe = 0;
	
	m_initialized = 0;
}

ABK_FrameDisplay::ABK_FrameDisplay() {
	for (int i = 0; i < 16; ++i) {
		m_textures[i] = 0;
		m_texture_id[i] = -1;
	}
	
	m_initialized = 0;
}

ABK_FrameDisplay::~ABK_FrameDisplay() {
}

