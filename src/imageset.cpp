#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "mbaa_framedisplay.h"

static MBAA_FrameDisplay fdisp;

int main(int argc, char **argv) {
	if (!fdisp.init("gant-mbaa.iso")) {
		return 0;
	}
	
	int value;
	
	value = 21;
	if (argc > 1) {
		value = atoi(argv[1]);
	}
	fdisp.command(COMMAND_CHARACTER_SET, &value);
	
	/*
	value = 248;
	fdisp.command(COMMAND_SEQUENCE_SET, &value);
	
	value = 2;
	fdisp.command(COMMAND_FRAME_SET, &value);
	 */
	
	value = 0;
	fdisp.command(COMMAND_PALETTE_SET, &value);
	
	for (int i = 0; i < 36; ++i) {
		char filename[256];
		sprintf(filename, "out/%2.2d.png", i);
		
		fdisp.save_current_sprite(filename);
		
		fdisp.command(COMMAND_PALETTE_NEXT, 0);
	}
	
	fdisp.free();
	
	return 0;
}
