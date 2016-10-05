#include "ougon_framedisplay.h"

#include <cstring>
#include <cstdio>

bool Ougon_Framedata::load(Ougon_Data *o_data, int character_id) {
	if (!o_data) {
		return 0;
	}
	
	unsigned char *data;
	unsigned int size;
	
	if (o_data->get_frame_data(&data, &size, character_id) == 0) {
		return 0;
	}
	
	if (memcmp(data, "ANM100", 6)) {
		return 0;
	}
	
	unsigned int *anmptrs = (unsigned int *)(data + 8);
	
	for (int i = 0; i < 250; ++i) {
		m_sequences[i].header = (Ougon_SequenceHeader *)(data + anmptrs[i]);
		m_sequences[i].frames = (Ougon_Frame *)(data + anmptrs[i] + 0x44);
		m_sequences[i].frame_count = m_sequences[i].header->frame_count;
		
		int count = m_sequences[i].frame_count;
		
		// process out junk frames
		for (int j = 0; j < count; ++j) {
			if (m_sequences[i].frames[j].duration == 0) {
				int amount = count-j;
				if (amount > 0) {
					memmove(&m_sequences[i].frames[j], &m_sequences[i].frames[j+1],
						0x60 * amount);
				}
				--count;
				--j;
			}
		}
		
		m_sequences[i].frame_count = count;
	}
	
	m_data = data;
	m_size = size;
	m_initialized = 1;
	
	return 1;
}

int Ougon_Framedata::get_sequence_count() {
	if (!m_initialized) {
		return 0;
	}
	return 250;
}

bool Ougon_Framedata::has_sequence(int seq_id) {
	if (!m_initialized || seq_id < 0 || seq_id >= 250) {
		return 0;
	}
	
	return m_sequences[seq_id].frame_count != 0;
}

Ougon_Sequence *Ougon_Framedata::get_sequence(int seq_id) {
	if (!has_sequence(seq_id)) {
		return 0;
	}
	
	return &m_sequences[seq_id];
}

int Ougon_Framedata::get_frame_count(int seq_id) {
	Ougon_Sequence *sequence = get_sequence(seq_id);
	if (!sequence) {
		return 0;
	}
	
	return sequence->frame_count;
}

Ougon_Frame *Ougon_Framedata::get_frame(int seq_id, int fr_id) {
	Ougon_Sequence *sequence = get_sequence(seq_id);
	if (!sequence) {
		return 0;
	}
	
	if (fr_id < 0 || (unsigned int)fr_id >= sequence->frame_count) {
		return 0;
	}
	
	return &sequence->frames[fr_id];
}

void Ougon_Framedata::free() {
	m_data = 0;
	m_size = 0;
	
	m_initialized = 0;
}

bool Ougon_Framedata::loaded() {
	return m_initialized;
}

Ougon_Framedata::Ougon_Framedata() {
	m_initialized = 0;
}

Ougon_Framedata::~Ougon_Framedata() {
	free();
}
