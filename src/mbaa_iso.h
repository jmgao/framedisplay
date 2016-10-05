#ifndef MBAA_ISO_H
#define MBAA_ISO_H

#include <cstdio>

class MBAA_ISO {
private:
	FILE		*m_file;
	
	unsigned int	m_start_pos;
	
public:
	bool		open_iso(const char *filename);
	void		close_iso();
	
	bool		read_file(const char *filename, char **dest, unsigned int *size);
	
			MBAA_ISO();
			~MBAA_ISO();
};

#endif
