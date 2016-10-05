// mbaa.iso:
//
// Core file loader.

#include "mbaa_iso.h"
#include "mbaa_iso_index.h"

#include <cstring>

bool MBAA_ISO::open_iso(const char *filename) {
	if (m_file) {
		return 0;
	}
	
	FILE *f;
	
	f = fopen(filename, "rb");
	if (!f) {
		return 0;
	}
	
	char name[14];
	// test if cvm file
	fseek(f, 0x34, SEEK_SET);
	fread(name, 12, 1, f);
	
	if (!memcmp(name, "ROFSROFSBLD ", 12)) {
		m_start_pos = mbaa_iso_data_start;
		
		m_file = f;
		
		return 1;
	}
	
	// test if ISO file
	fseek(f, 0x10875, SEEK_SET);
	
	
	fread(name, 14, 1, f);
	
	if (!memcmp(name, "MELTY_BLOOD_AA",14)) {
		m_start_pos = mbaa_iso_data_start + mbaa_iso_cvmfile_start;
		
		m_file = f;
		
		return 1;
	}
	
	return 0;
}

void MBAA_ISO::close_iso() {
	if (!m_file) {
		return;
	}
	
	fclose(m_file);
	m_file = 0;
}

bool MBAA_ISO::read_file(const char *filename, char **dest, unsigned int *dsize) {
	for (int i = mbaa_iso_index_count-1 ; i >= 0; --i) {
		if (!strcmp(filename, mbaa_iso_index[i].filename)) {
			unsigned int size = mbaa_iso_index[i].size;
			char *data = new char[size + 1];
			data[size] = '\0';
			
			fseek(m_file, m_start_pos + (mbaa_iso_index[i].sector * 0x800), SEEK_SET);
			
			int count = fread(data, size, 1, m_file);
			
			if (count > 0) {
				*dest = data;
				*dsize = size;
				
				return 1;
			}
			
			delete[] data;
		}
	}
	
	return 0;
}

MBAA_ISO::MBAA_ISO() {
	m_file = 0;
}

MBAA_ISO::~MBAA_ISO() {
	close_iso();
}

