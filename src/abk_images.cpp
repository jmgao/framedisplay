
#include "abk_framedisplay.h"
#include "misc.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

struct ABK_Image {
	unsigned char	*data;
	
	bool		unloadable;
	
	int		size;
	
	int		width;
	int		height;
};

bool ABK_Images::load_images(ABK_Packfiles *packs, const char *charname) {
	if (m_initialized) {
		return 0;
	}
	
	sprintf(m_packpath, "./data/chr/%s/0000.rsp", charname);
	
	int count = packs->get_pack_file_count(m_packpath);
	
	if (count < 2) {
		return 0;
	}
	
	// initialize image block
	ABK_Image *images = new ABK_Image[count - 1];
	
	m_images = images;
	m_nimages = count - 1;
	
	for (unsigned int i = 0; i < m_nimages; ++i) {
		m_images[i].data = 0;
		m_images[i].unloadable = 0;
	}
	
	// read palettes: always the last entry
	unsigned int size;
	char *paldata = packs->read_pack_entry(m_packpath, 0, &size);
	
	if (paldata) {
		unsigned int *p = (unsigned int *)paldata;
		unsigned int *p_end = (unsigned int *)(paldata + size);
		unsigned int pcount = 0;
		
		if (size > 4) {
			pcount = *p;
		}
		
		if (pcount > 0 && pcount < 100) {
			++p;
			
			m_npalettes = pcount;
			m_palettes = new unsigned int*[pcount];
			for (unsigned int i = 0; i < pcount; ++i) {
				m_palettes[i] = 0;
			}
			
			for (unsigned int i = 0; i < pcount; ++i) {
				if (p_end - p < 257) {
					m_npalettes = i;
					
					break;
				}
				
				unsigned int *pal = new unsigned int[256];
				m_palettes[i] = pal;
				
				for (int j = 0; j < 256; ++j) {
					unsigned int npixel;
					
					npixel = (*p & 0xff0000) >> 16;
					npixel |= (*p & 0x0000ff) << 16;
					npixel |= (*p & 0xff00);
					npixel |= 0xff000000;

					++p;
					
					pal[j] = npixel;
				}
				pal[0] &= ~ 0xff000000;
			}
		}
		delete[] paldata;
	}
	
	// finish up
	m_packs = packs;
	
	m_initialized = 1;
	
	return 1;
}

Texture *ABK_Images::get_texture(int n, int pal_no, bool force_8bpp) {
	if (!m_initialized) {
		return 0;
	}
	
	if (n < 0 || (unsigned int)n >= m_nimages) {
		return 0;
	}
	
	// get palette
	if (pal_no < 0 || (unsigned int)pal_no >= m_npalettes) {
		if (m_npalettes == 0) {
			return 0;
		}
		
		pal_no = 0;
	}
	unsigned int *pal = m_palettes[pal_no];
	if (!pal && !force_8bpp) {
		return 0;
	}
	
	// if image isn't loaded, load it
	if (!m_images[n].data) {
		if (m_images[n].unloadable) {
			return 0;
		}
		
		unsigned int size;
		char *data = m_packs->read_pack_entry(m_packpath, n + 1, &size);
		
		if (!data) {
			m_images[n].unloadable = 1;
			return 0;
		}
		
		if (size < 0x410) {
			m_images[n].unloadable = 1;
			delete[] data;
			return 0;
		}
		
		int *header = (int *)data;
		int width = header[1];
		int height = header[2];
		unsigned int psize = (width * height) + 0x410;
		
		if (size < psize || width <= 0 || height <= 0 || width > 1024 || height > 1024) {
			m_images[n].unloadable = 1;
			delete[] data;
			return 0;
		}
		
		m_images[n].data = (unsigned char *)data;
		m_images[n].size = size;
		m_images[n].width = width;
		m_images[n].height = height;
	}
	
	// convert data to texture
	Texture *texture = new Texture;
	
	bool loaded = 0;
	int w = m_images[n].width;
	int h = m_images[n].height;
	
	if (force_8bpp) {
		int size = w * h;
		unsigned char *pixels = new unsigned char[size];
		unsigned char *s = m_images[n].data + 0x410;
		unsigned char *d = pixels;
		
		memcpy(d, s, size);
		
		loaded = texture->init(pixels, w, h, 1);
	} else {
		int rw = to_pow2(w);
		int rh = to_pow2(h);
		int inc = rw - w;
		
		unsigned int *pixels = new unsigned int[rw*rh];
		unsigned char *s = (unsigned char *)m_images[n].data + 0x410;
		unsigned int *d = pixels;
		
		int y;
		for (y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				*d++ = pal[*s++];
			}
		
			memset(d, 0, 4*inc);
			
			d += inc;
		}
		
		if (y < rh) {
			memset(d, 0, 4 * rw * (rh - y));
		}
		
		loaded = texture->init((unsigned char *)pixels, rw, rh, 0);
	}
	
	if (!loaded) {
		delete texture;
		texture = 0;
	}
	
	return texture;
}

int ABK_Images::get_width(int n) {
	if (!m_initialized) {
		return 0;
	}
	
	if (n < 0 || (unsigned int)n >= m_nimages) {
		return 0;
	}

	if (!m_images[n].data) {
		return 0;
	}
	
	return m_images[n].width;
}

int ABK_Images::get_palette_count() {
	return m_npalettes;
}

unsigned int *ABK_Images::get_palette(int n) {
	if (n < 0 || (unsigned int)n >= m_npalettes) {
		return 0;
	}
	
	return m_palettes[n];
}

int ABK_Images::get_image_count() {
	return m_nimages;
}

const char *ABK_Images::get_image_filename(int n) {
	if (!m_initialized) {
		return 0;
	}
	
	if (n < 0 || (unsigned int)n >= m_nimages) {
		return 0;
	}
	
	const char *name = m_packs->get_pack_file_name(m_packpath, (m_nimages - 1) - n);
	if (!name) {
		return 0;
	}
	
	static char namebuf[128];
	int l = strlen(name);
	if (l < 7) {
		return 0;
	}

	l -= 2;
	
	if (l > 127) {
		l = 127;
	}
	
	memcpy(namebuf, name+2, l);
	namebuf[l] = '\0';
	
	char *s = namebuf;
	
	while ((s = strchr(s, '/')) != 0) {
		*s = '_';
	}
	
	strcpy(namebuf+l-4, ".png");
	
	return namebuf;
}

void ABK_Images::free() {
	if (m_images) {
		for (unsigned int i = 0; i < m_nimages; ++i) {
			if (m_images[i].data) {
				delete[] m_images[i].data;
			}
		}
		
		delete[] m_images;
		m_images = 0;
	}
	m_nimages = 0;
	
	if (m_palettes) {
		for (unsigned int i = 0; i < m_npalettes; ++i) {
			if (m_palettes[i]) {
				delete[] m_palettes[i];
			}
		}
		delete[] m_palettes;
		m_palettes = 0;
	}
	m_npalettes = 0;
	
	m_packpath[0] = '\0';
	
	m_packs = 0;
	
	m_initialized = 0;
}

ABK_Images::ABK_Images() {
	m_packpath[0] = '\0';
	
	m_packs = 0;
	
	m_images = 0;
	m_nimages = 0;
	
	m_palettes = 0;
	m_npalettes = 0;
	
	m_initialized = 0;
}

ABK_Images::~ABK_Images() {
	free();
}
