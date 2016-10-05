// .CG2 loader
//
// .CG2 contains information about sprite mappings from the ENC and PVR tiles.

#include "mbaa_framedisplay.h"
#include "misc.h"

#include <cstdlib>
#include <cstring>

struct MBAA_CG2_Alignment {
	int	x;
	int	y;
	int	width;
	int	height;
	short	source_x;
	short	source_y;
	short	source_image;
	short	unk8;		// no clue on this one: only 1 or 0. flip flag?
};

struct MBAA_CG2_Image {
	char		filename[32];
	int		type_id;	// i think this is render mode
	unsigned int	width;
	unsigned int	height;
	unsigned int	bpp;
	int		bounds_x1;
	int		bounds_y1;
	int		bounds_x2;
	int		bounds_y2;
	unsigned int	align_start;
	unsigned int	align_len;
};

const MBAA_CG2_Image *MBAA_CG2::get_image(unsigned int n) {
	if (n >= m_nimages) {
		return 0;
	}
	
	unsigned int index = m_indices[n];
	if (index < 0) {
		return 0;
	}
	
	if (index + sizeof(MBAA_CG2_Image) > m_data_size) {
		return 0;
	}
	
	return (const MBAA_CG2_Image *)(m_data + index);
}

const char *MBAA_CG2::get_filename(unsigned int n) {
	if (!m_loaded) {
		return 0;
	}
	
	const MBAA_CG2_Image *image = get_image(n);
	if (!image) {
		return 0;
	}
	
	return image->filename;
}

int MBAA_CG2::get_image_count() {
	return m_nimages;
}

Texture *MBAA_CG2::draw_texture(unsigned int n, MBAA_ARX *arx, unsigned int *palette, bool to_pow2_flg, bool draw_8bpp) {
	if (!arx) {
		return 0;
	}
	
	const MBAA_CG2_Image *image = get_image(n);
	if (!image) {
		return 0;
	}

	if ((image->align_start + image->align_len) > m_nalign) {
		return 0;
	}
	
	// initialize texture and boundaries
	int x1 = 0;
	int y1 = 0;
	
	if (!draw_8bpp) {
		x1 = image->bounds_x1;
		y1 = image->bounds_y1;
	}
	
	int width = image->bounds_x2 - x1;
	int height = image->bounds_y2 - y1;
	
	if (width == 0 || height == 0) {
		return 0;
	}
	
	if (to_pow2_flg) {
		width = to_pow2(width);
		height = to_pow2(height);
	}
	
	unsigned char *pixels = new unsigned char[width*height*4];
	memset(pixels, 0, width*height*4);
	
	// run through all tile region data
	const MBAA_CG2_Alignment *align;
	int last_image = -32769;
	MBAA_TileImage *tile_image = 0;
	
	bool is_8bpp;
	
	if (draw_8bpp) {
		is_8bpp = 1;
		align = &m_align[image->align_start];
		last_image = -32769;
		for (unsigned int i = 0; i < image->align_len; ++i) {
			if (align->source_image != last_image) {
				char filename[5];
				sprintf(filename, "%4.4d", align->source_image);
			
				tile_image = arx->get_tileimage(filename);
			
				last_image = align->source_image;
				
				is_8bpp = is_8bpp && tile_image->is_8bpp();
			}
		}
	} else {
		is_8bpp = 0;
	}
	
	align = &m_align[image->align_start];
	last_image = -32769;
	for (unsigned int i = 0; i < image->align_len; ++i) {
		if (align->source_image != last_image) {
			char filename[5];
			sprintf(filename, "%4.4d", align->source_image);
			
			tile_image = arx->get_tileimage(filename);
			
			last_image = align->source_image;
		}
		
		if (!tile_image) {
			continue;
		}

		tile_image->copy_region_to(pixels,
			align->x - x1,
			align->y - y1,
			width,
			height,
			align->source_x,
			align->source_y,
			align->width,
			align->height,
			palette,
			is_8bpp);
		++align;
	}
	
	// finalize in texture
	Texture *texture = new Texture();
	
	if (!texture->init(pixels, width, height, is_8bpp)) {
		delete texture;
		delete[] pixels;
		texture = 0;
	} else {
		texture->offset(image->bounds_x1*2, image->bounds_y1*2);
	}
		
	return texture;
}

bool MBAA_CG2::load(MBAA_ISO *iso, const char *name) {
	if (m_loaded) {
		return 0;
	}
	
	char *data;
	unsigned int size;
	
	if (!iso->read_file(name, &data, &size)) {
		return 0;
	}
	
	// verify size and header
	if (size < 0x4f30 || memcmp(data, "BMP Cutter3", 11)) {
		delete[] data;
		
		return 0;
	}
	
	// palette data.
	// we always use external palettes, so skip this.
	unsigned int *d = (unsigned int *)(data + 0x10);
	d += 1;		// has palette data?
	d += 0x800;	// palette data - always included.
	
	// 'parse' header
	unsigned int *indices = d + 12;
	unsigned int image_count = d[3];
	
	if (image_count >= 2999) {
		delete[] data;
		
		return 0;
	}
	
	// alignment data
	int align_count = (size - indices[3000]) / sizeof(MBAA_CG2_Alignment);
	
	if (align_count <= 0) {
		delete[] data;
		
		return 0;
	}
	
	// store everything for lookup later
	m_align = (MBAA_CG2_Alignment *)(data + indices[3000]);
	m_nalign = align_count;
	
	m_data = data;
	m_data_size = size;
	
	m_indices = indices;
	
	m_nimages = image_count;
	
	m_loaded = 1;
	
	return 1;
}

void MBAA_CG2::free() {
	if (m_data) {
		delete[] m_data;
	}
	m_data = 0;
	m_data_size = 0;
	
	m_indices = 0;
	
	m_nimages = 0;
	
	m_align = 0;
	m_nalign = 0;
	
	m_loaded = 0;
}

MBAA_CG2::MBAA_CG2() {
	m_data = 0;
	m_data_size = 0;
	
	m_indices = 0;
	
	m_nimages = 0;
	
	m_align = 0;
	m_nalign = 0;
	
	m_loaded = 0;
}

MBAA_CG2::~MBAA_CG2() {
	free();
}

