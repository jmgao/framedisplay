// Frame Display:
//
// Windows interface.

// I fucking hate Windows.

#define WINVER 0x0501
#define _WIN32_IE 0x0501

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <winnt.h>

#include <wingdi.h>

#include <commdlg.h>
#include <commctrl.h>
#include <shlobj.h>

#include <gl.h>

#include "framedisplay.h"
#include "clone.h"
#include "render.h"

#include "mbaa_framedisplay.h"
#include "mbaacc_framedisplay.h"
#include "touhou_framedisplay.h"
#include "abk_framedisplay.h"
#include "ougon_framedisplay.h"

#define IDM_FILE_OPEN_TOUHOU		1001
#define IDM_FILE_OPEN_ABK		1002
#define IDM_FILE_OPEN_OUGON		1003
#define IDM_FILE_OPEN_MBAA		1004
#define IDM_FILE_OPEN_MBAACC		1005
#define IDM_FILE_CLOSE			1098
#define IDM_FILE_EXIT			1099
#define IDM_VIEW_ANIMATE		2010
#define IDM_VIEW_FLIP_HORIZONTALLY	2011
#define IDM_VIEW_DISPLAY_GRADIENT	2020
#define IDM_VIEW_DISPLAY_AXIS		2021
#define IDM_VIEW_DISPLAY_FRAME_INFO	2022
#define IDM_VIEW_DISPLAY_FRAME_PROPS	2023
#define IDM_VIEW_DISPLAY_SPRITE		2030
#define IDM_VIEW_DISPLAY_SOLID_BOXES	2031
#define IDM_VIEW_DISPLAY_COLLISION_BOX	2032
#define IDM_VIEW_DISPLAY_HIT_BOX	2033
#define IDM_VIEW_DISPLAY_ATTACK_BOX	2034
#define IDM_VIEW_DISPLAY_CLASH_BOX	2035
#define IDM_VIEW_NEXT_PALETTE		2040
#define IDM_VIEW_PREV_PALETTE		2041
#define IDM_VIEW_DEFAULT_PALETTE	2042
#define IDM_FRAME_PREV_CHARACTER	3001
#define IDM_FRAME_NEXT_CHARACTER	3002
#define IDM_FRAME_PREV_SEQUENCE		3003
#define IDM_FRAME_NEXT_SEQUENCE		3004
#define IDM_FRAME_PREV_STATE		3005
#define IDM_FRAME_NEXT_STATE		3006
#define IDM_FRAME_DETAILED_PROPERTIES	3010
#define IDM_FRAME_VIEW_SEQUENCE_TEXT	3020
#define IDM_FRAME_VIEW_FRAME_TEXT	3021
#define IDM_CLONE_CREATE_CLONE		4001
#define IDM_CLONE_DESTROY_CLONE		4002
#define IDM_CLONE_DESTROY_ALL_CLONES	4003
#define IDM_CLONE_FLIP_CLONE		4005
#define IDM_MISC_SAVE_SPRITE		5001
#define IDM_MISC_SAVE_ALL_SPRITES	5002

static FrameDisplay *framedisplay = 0;
static HINSTANCE hAppInst = 0;

static HWND hWnd = 0;
static HWND hWndToolbar = 0;
static HWND hWndOGL = 0;

static HWND hWndCharacter = 0;
static HWND hWndSequence = 0;
static HWND hWndState = 0;

static HMENU hMenubar = 0;

static HDC hDC = 0;
static HGLRC hGLRC = 0;

static const TCHAR *class_name = "FrameDisplay2";
static const TCHAR *opengl_class_name = "FrameDisplay2OpenGL";
static const TCHAR *program_name = "FrameDisplay v2.3";

static int toolbar_height = 0;

static HMENU hMenuFile = 0;
static HMENU hMenuView = 0;
static HMENU hMenuFrame = 0;
static HMENU hMenuClone = 0;
static HMENU hMenuMisc = 0;

// ****************************************************** TOGGLES
class Toggle {
private:
	bool		m_value;
	HMENU		*m_menu;
	int		m_menuID;
public:
	void set(bool value) {
		m_value = value;
		
		if (m_menu && *m_menu) {
			CheckMenuItem(*m_menu, m_menuID, MF_BYCOMMAND|(m_value?MF_CHECKED:MF_UNCHECKED));
		}
	}
	
	void toggle() { set(!m_value); }
	
	void update() { set(m_value); }
	
	bool get() { return m_value; };
	
	Toggle(bool value, HMENU *menu, int menuID) {
		m_value = value;
		m_menu = menu;
		m_menuID = menuID;
	}
};

Toggle toggle_view_animate(0, &hMenuView, IDM_VIEW_ANIMATE);
Toggle toggle_view_flip(0, &hMenuView, IDM_VIEW_FLIP_HORIZONTALLY);
Toggle toggle_view_draw_gradient(1, &hMenuView, IDM_VIEW_DISPLAY_GRADIENT);
Toggle toggle_view_draw_axis(1, &hMenuView, IDM_VIEW_DISPLAY_AXIS);
Toggle toggle_view_display_frame_info(1, &hMenuView, IDM_VIEW_DISPLAY_FRAME_INFO);
Toggle toggle_view_display_frame_props(1, &hMenuView, IDM_VIEW_DISPLAY_FRAME_PROPS);
Toggle toggle_view_display_sprite(1, &hMenuView, IDM_VIEW_DISPLAY_SPRITE);
Toggle toggle_view_display_solid_boxes(1, &hMenuView, IDM_VIEW_DISPLAY_SOLID_BOXES);
Toggle toggle_view_display_collision_box(1, &hMenuView, IDM_VIEW_DISPLAY_COLLISION_BOX);
Toggle toggle_view_display_hit_box(1, &hMenuView, IDM_VIEW_DISPLAY_HIT_BOX);
Toggle toggle_view_display_attack_box(1, &hMenuView, IDM_VIEW_DISPLAY_ATTACK_BOX);
Toggle toggle_view_display_clash_box(1, &hMenuView, IDM_VIEW_DISPLAY_CLASH_BOX);
Toggle toggle_frame_detailed_properties(0, &hMenuFrame, IDM_FRAME_DETAILED_PROPERTIES);

// ****************************************************** OPENGL CODE
static int window_height = 480;
static int window_width = 640;
static float position_x = 180.0/640.0;
static float position_y = 400.0/480.0;

static bool render = 0;

struct CloneData {
	float		x, y;
	
	bool		flip;
	
	Clone *		clone;
};

static std::list<CloneData> clone_list;

static void display_bg() {
	glBegin(GL_QUADS);
	glColor4f(0.0, 0.075, 0.075, 1.0);
	glVertex2f(0.0, 0.0);
	glVertex2f(window_width, 0.0);
	if (toggle_view_draw_gradient.get()) {
		glColor4f(0.0, 0.15, 0.15, 1.0);
	}
	glVertex2f(window_width, window_height);
	glVertex2f(0.0, window_height);
	glEnd();
}

static void display_axis(float x, float y, bool clone_color) {
	if (clone_color) {
		glColor4f(0.4, 0.2, 0.8, 1.0);
	} else {
		glColor4f(0.0, 0.8, 0.8, 1.0);
	}
	
	glBegin(GL_LINES);
	glVertex2f(0.0, y*window_height);
	glVertex2f(window_width, y*window_height);
	glVertex2f(x*window_width, 0.0);
	glVertex2f(x*window_width, window_height);
	glEnd();
}

static void draw_fdisp(const RenderProperties *properties) {
	glPushMatrix();
	glTranslatef((int)(position_x*window_width), (int)(position_y*window_height), 0.0);
	
	if (toggle_view_flip.get()) {
		glScalef(-1.0, 1.0, 1.0);
	}
	
	framedisplay->render(properties);
	
	glPopMatrix();
}

static void draw_opengl() {
	display_bg();
	
	if (toggle_view_draw_axis.get()) {
		std::list<CloneData>::iterator j = clone_list.begin();
		for (; j != clone_list.end(); ++j) {
			display_axis(j->x, j->y, 1);
		}
		
		if (framedisplay) {
			display_axis(position_x, position_y, 0);
		}
	}
	
	RenderProperties properties;
	properties.display_sprite = toggle_view_display_sprite.get();
	properties.display_solid_boxes = toggle_view_display_solid_boxes.get();
	properties.display_collision_box = toggle_view_display_collision_box.get();
	properties.display_hit_box = toggle_view_display_hit_box.get();
	properties.display_attack_box = toggle_view_display_attack_box.get();
	properties.display_clash_box = toggle_view_display_clash_box.get();
	
	if (clone_list.begin() != clone_list.end()) {
		RenderProperties clone_props;
		clone_props.display_sprite = 0;
		clone_props.display_solid_boxes = properties.display_solid_boxes;
		clone_props.display_collision_box = 0;
		clone_props.display_hit_box = 0;
		clone_props.display_attack_box = 0;
		clone_props.display_clash_box = 0;
		
		for (int i = 0; i < 5; ++i) {
			bool *dest_flag;
			bool value;
			
			switch (i) {
			case 0:
				value = properties.display_sprite;
				dest_flag = &clone_props.display_sprite;
				break;
			case 1:
				value = properties.display_collision_box;
				dest_flag = &clone_props.display_collision_box;
				break;
			case 2:
				value = properties.display_hit_box;
				dest_flag = &clone_props.display_hit_box;
				break;
			case 3:
				value = properties.display_attack_box;
				dest_flag = &clone_props.display_attack_box;
				break;
			case 4:
				value = properties.display_clash_box;
				dest_flag = &clone_props.display_clash_box;
				break;
			default:
				// never reached.
				value = 0;
				dest_flag = 0;
				break;
			}
			
			if (!value) {
				continue;
			}
			
			*dest_flag = 1;
			
			std::list<CloneData>::iterator j = clone_list.begin();
			for (; j != clone_list.end(); ++j) {
				glPushMatrix();
				glTranslatef((int)(j->x*window_width), (int)(j->y*window_height), 0.0);

				if (j->flip) {
					glScalef(-1.0, 1.0, 1.0);
				}
				
				j->clone->render(&clone_props);
				
				glPopMatrix();
			}
			
			if (framedisplay) {
				draw_fdisp(&clone_props);
			}
			
			*dest_flag = 0;
		}
	} else {
		if (framedisplay) {
			draw_fdisp(&properties);
		}
	}

	if (framedisplay) {
		if (toggle_view_display_frame_info.get()) {
			int ch_id = framedisplay->get_character();
			int seq_id = framedisplay->get_sequence();
			int seq_count = framedisplay->get_sequence_count();
			int subfr_id = framedisplay->get_subframe();
			int subfr_count = framedisplay->get_subframe_count();
			int fr_id = framedisplay->get_frame();
			int fr_count = framedisplay->get_frame_count();
			int pal_id = framedisplay->get_palette();
			const char *ch_name = framedisplay->get_character_name(ch_id);
			int move_meter = -1;
			const char *move_name = framedisplay->get_sequence_move_name(seq_id, &move_meter);
	
			char buf[256];
			
			if (move_name) {
				sprintf(buf, "cmd: %s", move_name);
				if (move_meter > 0) {
					sprintf(buf+strlen(buf), " (%3.1f%%)",
						(float)move_meter / 100.0f);
				}

				render_shaded_text(window_width - 550, window_height - 35 - 13, buf);
			}
			
			sprintf(buf, "$.%-20s   $yseq:$.%5d  $yfr:$.%5d  $yst:$.%5d\n"
				     "$W%2.2d                       $y/$. %5d   $y/$. %5d   $y/$. %5d",
				     ch_name,
				     seq_id,
				     subfr_id+1,
				     fr_id+1,
				     pal_id,
				     seq_count-1,
				     subfr_count,
				     fr_count);

			/*
			sprintf(buf, "%3d %-20s seq %5d fr %5d st %5d\n                           - %5d  - %5d  - %5d",
				pal_id,
				ch_name,
				seq_id,
				subfr_id+1,
				fr_id+1,
				seq_count-1,
				subfr_count,
				fr_count);
			 */
		
			render_shaded_text(window_width - 550, window_height - 35, buf);
		}
		
		if (toggle_view_display_frame_props.get()) {
			framedisplay->render_frame_properties(
				toggle_frame_detailed_properties.get(),
				window_width,
				window_height);
		}
	}
	
	SwapBuffers(hDC);
}

static void setup_opengl(int width, int height) {
	window_width = width;
	window_height = height;
	glViewport(0, 0, window_width, window_height);
	
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, -2048, 2048);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void close_opengl() {
	wglMakeCurrent(0, 0);
	
	if (hGLRC) {
		wglDeleteContext(hGLRC);
		hGLRC = 0;
	}
	
	if (hDC) {
		ReleaseDC(hWnd, hDC);
		hDC = 0;
	}
}

static LRESULT CALLBACK opengl_window_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	static bool window_drag = 0;
	static int mouse_x1 = 0;
	static int mouse_y1 = 0;
	static int drag_clone = -1;

	switch (Msg) {
		case WM_PAINT:
			draw_opengl();
			break;
		case WM_SIZE:
			setup_opengl(LOWORD(lParam), HIWORD(lParam));
			draw_opengl();
			//render = 1;
			break;
		case WM_LBUTTONDBLCLK:
			if (framedisplay) {
				toggle_view_flip.toggle();
				render = 1;
			}
			break;
		case WM_LBUTTONDOWN: {
				SetFocus(::hWnd);
				// are we dragging a clone or not
				mouse_x1 = GET_X_LPARAM(lParam);
				mouse_y1 = GET_Y_LPARAM(lParam);
				drag_clone = 0;
				std::list<CloneData>::iterator j = clone_list.end();
				--j;
				for (; j != clone_list.end(); --j) {
					int x = (int)(j->x * window_width);
					int y = (int)(j->y * window_height);
					
					x = mouse_x1 - x;
					y = mouse_y1 - y;
					
					if (j->flip) {
						x = -x;
					}
					
					if ((abs(x) < 5 && abs(y) < 5)
					    || (j->clone->in_box(x, y))) {
						CloneData temp = *j;
						
						clone_list.erase(j);
						
						clone_list.push_back(temp);
						
						drag_clone = 1;
						break;
					}
				}
				
				if (!drag_clone && !framedisplay) {
					break;
				}
				
				window_drag = 1;
				SetCapture(hWnd);
				
				render = 1;
			}
			break;
		case WM_LBUTTONUP:
			if (window_drag) {
				window_drag = 0;
				ReleaseCapture();
				
				render = 1;
			}
			break;
		case WM_MOUSEMOVE:
			if (window_drag) {
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);
				
				int dx = x - mouse_x1;
				int dy = y - mouse_y1;
				
				float fdx = (float)dx / window_width;
				float fdy = (float)dy / window_height;
				
				if (drag_clone == 1) {
					std::list<CloneData>::iterator j = clone_list.end();
					--j;
					if (j != clone_list.end()) {
						j->x += fdx;
						j->y += fdy;
					}
				} else {
					position_x += (float)dx / window_width;
					position_y += (float)dy / window_height;
				}
				
				mouse_x1 = x;
				mouse_y1 = y;
				
				draw_opengl();
			}
			break;
	}
	
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}


static bool register_opengl_window() {
	WNDCLASSEX WndClassEx;
	
	memset(&WndClassEx, 0, sizeof(WndClassEx));
	WndClassEx.cbSize = sizeof(WndClassEx);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WndClassEx.lpfnWndProc = opengl_window_proc;
	WndClassEx.hInstance = hAppInst;
	WndClassEx.hCursor = LoadCursor(0, IDC_ARROW);
	WndClassEx.hbrBackground = 0; // CreateSolidBrush(0);
	WndClassEx.lpszClassName = opengl_class_name;
	
	if (!RegisterClassEx(&WndClassEx)) {
		return 0;
	}
	
	return 1;
}

static bool init_opengl(HWND hWnd) {
	PIXELFORMATDESCRIPTOR pfd;
	
	hDC = GetDC(hWnd);
	if (!hDC) {
		return 0;
	}
	
	memset(&pfd, 0, sizeof(pfd));
	
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	
	int format = ChoosePixelFormat( hDC, &pfd );
	
	SetPixelFormat(hDC, format, &pfd);
	
	hGLRC = wglCreateContext(hDC);
	
	if (!hGLRC) {
		close_opengl();
		return 0;
	}
	
	if (!wglMakeCurrent(hDC, hGLRC)) {
		close_opengl();
		return 0;
	}
	
	// setup default window stuff
	render_font_init();
	
	position_x = 180.0/640.0;
	position_y = 400.0/480.0;
	
	RECT rect;
	GetClientRect(hWnd, &rect);
	
	setup_opengl(rect.right, rect.bottom);
	
	return 1;
}

// ****************************************************** FRAMEDISPLAY MANAGEMENT

static void convert_sjis_to_unicode(WCHAR *out, char *in) {
	unsigned char c;
	
	while ((c = (unsigned char)*in++) != 0) {
		if (c <= 0x80) {
			*out++ = c;
		} else if (c >= 0xa1 && c < 0xe0) {
			*out++ = c;
		} else if ((c >= 0x81 && c < 0xa0) || (c >= 0xe0 && c < 0xf0)) {
			unsigned char c2 = (unsigned char)*in;
			
			if (c2 >= 0x40 && c2 < 0xfd && c != 0x7f) {
				// sjis char
				union {
					struct {
						char sjis[2];
						char zero;
					};
					char	str[3];
				} sjis_str;
				
				sjis_str.sjis[0] = c;
				sjis_str.sjis[1] = c2;
				sjis_str.zero = '\0';
				
				int count = MultiByteToWideChar(932, 0, sjis_str.str, 2, out, 2);
				if (count > 0) {
					out += count;
				}
				in += 1;
			}
		}
	}
	
	*out++ = '\0';
}

static int add_box_item(HWND hWndBox, int n, char *str) {
	// - Convert string format.
	int len = strlen(str);
	WCHAR *wstr = new WCHAR[len + 50];
	
	convert_sjis_to_unicode(wstr, str);
	
	// - Send unicode string.
	COMBOBOXEXITEMW cbexi;
	memset(&cbexi, 0, sizeof(cbexi));
	
	cbexi.mask = CBEIF_TEXT;
	cbexi.iItem = n;
	cbexi.pszText = wstr;
	
	LRESULT res = SendMessageW(hWndBox, CBEM_INSERTITEMW, 0, (LPARAM)&cbexi);
	
	// - Finish up.
	delete[] wstr;
	
	return res;
}

static void update_state_box() {
	if (framedisplay) {
		int n = framedisplay->get_frame();
		
		SendMessage(hWndState, CB_SETCURSEL, (WPARAM)n, 0);
	}
}

static void fill_state_box() {
	SendMessage(hWndState, CB_RESETCONTENT, 0, 0);
	
	if (framedisplay) {
		int n = framedisplay->get_frame_count();
		int i = 0;
		
		while (i < n) {
			char str[20];
			if (n >= 100) {
				sprintf(str, "State %3.3d / %3.3d", i + 1, n);
			} else {
				sprintf(str, "State %2.2d / %2.2d", i + 1, n);
			}
			
			add_box_item(hWndState, i, str);
			
			++i;
		}
	}
		
	update_state_box();
}

static int *lookup_table_seq_to_box;
static int *lookup_table_box_to_seq;

static void update_sequence_box() {
	if (lookup_table_seq_to_box && framedisplay) {
		int n = framedisplay->get_sequence();
		
		if (n >= 0 && n < framedisplay->get_sequence_count()) {
			n = lookup_table_seq_to_box[n];
			if (n >= 0) {
				SendMessage(hWndSequence, CB_SETCURSEL, (WPARAM)n, 0);
			}
		}
	}
}

static void fill_sequence_box() {
	SendMessage(hWndSequence, CB_RESETCONTENT, 0, 0);
	if (lookup_table_seq_to_box) {
		delete[] lookup_table_seq_to_box;
		
		lookup_table_seq_to_box = 0;
	}
	if (lookup_table_box_to_seq) {
		delete[] lookup_table_box_to_seq;
		
		lookup_table_box_to_seq = 0;
	}

	if (framedisplay) {
		int n = framedisplay->get_sequence_count();
		
		if (n > 0) {
			lookup_table_seq_to_box = new int[n];
			lookup_table_box_to_seq = new int[n];
			
			for (int i = 0; i < n; ++i) {
				lookup_table_seq_to_box[i] = -1;
				lookup_table_box_to_seq[i] = -1;
			}
			
			int c = 0;
			for (int i = 0; i < n; ++i) {
				if (framedisplay->has_sequence(i)) {
					char buf[256];
					const char *name = framedisplay->get_sequence_name(i);
					int move_meter = -1;
					const char *move_name = framedisplay->get_sequence_move_name(i, &move_meter);
					
					sprintf(buf, "Seq %4.4d", i);
					
					if (move_name) {
						sprintf(buf + strlen(buf), " : %s", move_name);
						
						if (move_meter > 0) {
							sprintf(buf + strlen(buf), " (%3.1f%%)",
								(float)move_meter/100.0);
						}
					} else {
						strcat(buf, " : ------");
					}
					
					if (name) {
						sprintf(buf + strlen(buf), " - \"%s\"", name);
					}
					
					add_box_item(hWndSequence, c, buf);
					
					lookup_table_seq_to_box[i] = c;
					lookup_table_box_to_seq[c] = i;
					
					++c;
				}
			}
		}
	}
	
	update_sequence_box();
}

static void update_comboboxes() {
	if (framedisplay) {
		int n = framedisplay->get_character();
		
		SendMessage(hWndCharacter, CB_SETCURSEL, (WPARAM)n, 0);
	}
	
	update_sequence_box();
	update_state_box();
}

static void free_framedisplay() {
	if (framedisplay) {
		delete framedisplay;
	}
	
	framedisplay = 0;
	
	EnableMenuItem(hMenuView, IDM_VIEW_NEXT_PALETTE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuView, IDM_VIEW_PREV_PALETTE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuView, IDM_VIEW_DEFAULT_PALETTE, MF_BYCOMMAND|MF_GRAYED);
	
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_CHARACTER, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_CHARACTER, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_SEQUENCE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_SEQUENCE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_STATE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_STATE, MF_BYCOMMAND|MF_GRAYED);

	EnableMenuItem(hMenuFrame, IDM_FRAME_VIEW_SEQUENCE_TEXT, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_VIEW_FRAME_TEXT, MF_BYCOMMAND|MF_GRAYED);

	EnableMenuItem(hMenuFile, IDM_FILE_CLOSE, MF_BYCOMMAND|MF_GRAYED);

	EnableMenuItem(hMenuMisc, IDM_MISC_SAVE_SPRITE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuMisc, IDM_MISC_SAVE_ALL_SPRITES, MF_BYCOMMAND|MF_GRAYED);

	EnableMenuItem(hMenuClone, IDM_CLONE_CREATE_CLONE, MF_BYCOMMAND|MF_GRAYED);
	
	SendMessage(hWndCharacter, CB_RESETCONTENT, 0, 0);
	SendMessage(hWndSequence, CB_RESETCONTENT, 0, 0);
	SendMessage(hWndState, CB_RESETCONTENT, 0, 0);
}

static void set_framedisplay(FrameDisplay *fdisp) {
	free_framedisplay();
	
	framedisplay = fdisp;
	
	render = 1;
	
	// reset defaults
	position_x = 180.0/640.0;
	position_y = 400.0/480.0;
	
	toggle_view_animate.set(0);
	toggle_view_flip.set(0);

	EnableMenuItem(hMenuView, IDM_VIEW_NEXT_PALETTE, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuView, IDM_VIEW_PREV_PALETTE, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuView, IDM_VIEW_DEFAULT_PALETTE, MF_BYCOMMAND|MF_ENABLED);
	
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_CHARACTER, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_CHARACTER, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_SEQUENCE, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_SEQUENCE, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_STATE, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_STATE, MF_BYCOMMAND|MF_ENABLED);

	EnableMenuItem(hMenuFrame, IDM_FRAME_VIEW_SEQUENCE_TEXT, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_VIEW_FRAME_TEXT, MF_BYCOMMAND|MF_ENABLED);

	EnableMenuItem(hMenuFile, IDM_FILE_CLOSE, MF_BYCOMMAND|MF_ENABLED);

	EnableMenuItem(hMenuClone, IDM_CLONE_CREATE_CLONE, MF_BYCOMMAND|MF_ENABLED);

	EnableMenuItem(hMenuMisc, IDM_MISC_SAVE_SPRITE, MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(hMenuMisc, IDM_MISC_SAVE_ALL_SPRITES, MF_BYCOMMAND|MF_ENABLED);
	
	int i = 0;
	while (1) {
		const char *str = framedisplay->get_character_long_name(i);
		
		if (!str) {
			break;
		}
		
		COMBOBOXEXITEM cbexi;
		memset(&cbexi, 0, sizeof(cbexi));
		
		cbexi.mask = CBEIF_TEXT;
		cbexi.iItem = i;
		cbexi.pszText = (char *)str;
		
		SendMessage(hWndCharacter, CBEM_INSERTITEM, 0, (LPARAM)&cbexi);
		
		++i;
	}
	
	fill_sequence_box();
	fill_state_box();
	update_comboboxes();
}

static void callback_mbaa(const char *filename) {
	FrameDisplay *fdisp = new MBAA_FrameDisplay();
	if (!fdisp->init(filename)) {
		delete fdisp;
		return;
	}
	
	set_framedisplay(fdisp);
}

static void callback_mbaacc(const char *filename) {
	FrameDisplay *fdisp = new MBAACC_FrameDisplay();
	if (!fdisp->init(filename)) {
		delete fdisp;
		return;
	}
	
	set_framedisplay(fdisp);
}

static void callback_touhou(const char *filename) {
	FrameDisplay *fdisp = new Touhou_FrameDisplay();
	if (!fdisp->init(filename)) {
		delete fdisp;
		return;
	}
	
	set_framedisplay(fdisp);
}

static void callback_abk(const char *filename) {
	FrameDisplay *fdisp = new ABK_FrameDisplay();
	if (!fdisp->init(filename)) {
		delete fdisp;
		return;
	}
	
	set_framedisplay(fdisp);
}

static void callback_ougon(const char *filename) {
	FrameDisplay *fdisp = new Ougon_FrameDisplay();
	if (!fdisp->init(filename)) {
		delete fdisp;
		return;
	}
	
	set_framedisplay(fdisp);
}

// ****************************************************** FILE CODE
typedef void (*file_callback_t)(const char *filename);

static void init_ofn(OPENFILENAME *ofn, const char *formats, file_callback_t callback, char *filebuf) {
	memset(ofn, 0, sizeof(OPENFILENAME));
	
	ofn->lStructSize = sizeof(OPENFILENAME);
	
	ofn->hwndOwner = hWnd;
	ofn->hInstance = hAppInst;
	
	ofn->lpstrFile = filebuf;
	filebuf[0] = '\0';
	ofn->nMaxFile = 2048;
	
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	strcpy(buf, formats);
	
	char *b = buf-1;
	while ((b = strchr(buf+1, '%')) != 0) {
		*b = '\0';
	}
	
	ofn->lpstrFilter = buf;
	ofn->nFilterIndex = 1;
}

static void do_open_file(const char *formats, file_callback_t callback) {
	OPENFILENAME ofn;
	char filebuf[2048];
	
	init_ofn(&ofn, formats, callback, filebuf);
	
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY | OFN_HIDEREADONLY;
	
	int ok = GetOpenFileName(&ofn);
	
	if (ok) {
		callback(filebuf);
	}
}

static void callback_save_sprite(const char *filename) {
	if (!framedisplay || !filename) {
		return;
	}
	
	bool saved = framedisplay->save_current_sprite(filename);
	
	if (!saved) {
		MessageBox(hWnd, "Could not save image.", "Error", MB_OK);
	}
}

static void do_save_file(const char *filename, const char *formats, file_callback_t callback) {
	OPENFILENAME ofn;
	char filebuf[2048];

	init_ofn(&ofn, formats, callback, filebuf);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	
	if (filename) {
		strcpy(filebuf, filename);
	}
	
	int ok = GetSaveFileName(&ofn);
	
	if (ok) {
		callback(filebuf);
	}
}

static void callback_save_all_sprites(const char *folder) {
	int count;
	
	count = framedisplay->save_all_character_sprites(folder);
	
	char str[MAX_PATH + 80];
	sprintf(str, "Saved %d images to destination folder\n%s", count, folder);
	
	MessageBox(hWnd, str, "Save all images", MB_OK);
}

static void do_open_folder(const char *title, file_callback_t callback) {
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	TCHAR folder[MAX_PATH+1];
	folder[0] = '\0';
	
	memset(&bi, 0, sizeof(bi));
	
	bi.hwndOwner = hWnd;
	bi.pszDisplayName = folder;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_USENEWUI;
	
	pidl = SHBrowseForFolder(&bi);
	if (pidl) {
		bool result = SHGetPathFromIDList(pidl, folder);
		CoTaskMemFree(pidl);
		
		if (result) {
			strcat(folder, "\\");
			
			callback(folder);
		}
	}
}

// ****************************************************** TEXTVIEW CODE
static const char *textview_class_name = "FrameDisplayTextViewClass";

struct TextView {
	HWND	hWnd;
	
	HWND	hWndEdit;
	
	HFONT	hFont;
};

static std::list<TextView> textview_list;

static LRESULT CALLBACK textview_window_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	for (std::list<TextView>::iterator i = textview_list.begin(); i != textview_list.end(); ++i) {
		if (i->hWnd == hWnd) {
			switch(Msg) {
			case WM_DESTROY: {
					DeleteObject(i->hFont);
					
					textview_list.erase(i);
					break;
				}
			case WM_SIZE: {
					int width = LOWORD(lParam);
					int height = HIWORD(lParam);
				
					MoveWindow(i->hWndEdit, 0, 0, width, height, 1);
					break;
				}
			}
			
			break;
		}
	}
	
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

static bool register_textview() {
	WNDCLASSEX WndClassEx;
	
	memset(&WndClassEx, 0, sizeof(WndClassEx));
	WndClassEx.cbSize = sizeof(WndClassEx);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WndClassEx.lpfnWndProc = textview_window_proc;
	WndClassEx.hInstance = hAppInst;
	WndClassEx.hCursor = LoadCursor(0, IDC_ARROW);
	WndClassEx.hbrBackground = CreateSolidBrush(WHITE_BRUSH);
	WndClassEx.lpszClassName = textview_class_name;
	
	if (!RegisterClassEx(&WndClassEx)) {
		return 0;
	}
	
	return 1;
}

static void textview(const char *title, const char *text) {
	static bool registered = 0;
	if (!registered) {
		if (!register_textview()) {
			return;
		}
		
		registered = 1;
	}
	
	HWND hWndBase;
	
	hWndBase = CreateWindow(textview_class_name, title,
			WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			CW_USEDEFAULT, CW_USEDEFAULT,
			500, 400,
			hWnd, 0, hAppInst, 0);
	
	HWND hWndEdit;
	hWndEdit = CreateWindow("EDIT", 0,
			WS_CHILD|WS_VISIBLE|WS_VSCROLL|
			ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY,
			0, 0, 0, 0,
			hWndBase, 0, hAppInst, 0);
	
	HFONT font;
	font = CreateFont(16, 0, 0, 0, FW_DONTCARE,
			0,0,0,0,0,0,0,0,
			"Courier New");
			
	SendMessage(hWndEdit, WM_SETFONT, (WPARAM)font, (LPARAM)0);
	
	SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM)text);
	
	ShowWindow(hWndBase, SW_NORMAL);
	
	RECT rect;
	GetWindowRect(hWndBase,&rect);
	
	// i dunno.
	MoveWindow(hWndBase, rect.left, rect.top, rect.left+550, rect.top+400, 1);
	
	TextView textview;
	textview.hWnd = hWndBase;
	textview.hWndEdit = hWndEdit;
	textview.hFont = font;
	
	textview_list.push_front(textview);
}

// ****************************************************** MENU CODE
static void callback_command(HWND hWnd, int id, HWND src, UINT codeNotify) {
	// handle combobox selections
	if (id == 0) {
		if (!framedisplay) {
			return;
		}
		
		if (src == hWndCharacter) {
			if (codeNotify == 1) {
				int n = SendMessage(hWndCharacter, CB_GETCURSEL, 0, 0);
				if (n != framedisplay->get_character()) {
					framedisplay->command(COMMAND_CHARACTER_SET, &n);
					fill_sequence_box();
					fill_state_box();
					update_comboboxes();
					render = 1;
					
					SetFocus(::hWnd);
				}
			}
		} else if (src == hWndSequence) {
			if (codeNotify == 1) {
				int n = SendMessage(hWndSequence, CB_GETCURSEL, 0, 0);
				n = lookup_table_box_to_seq[n];
				
				if (n != framedisplay->get_sequence()) {
					framedisplay->command(COMMAND_SEQUENCE_SET, &n);
					fill_state_box();
					update_sequence_box();
					update_state_box();
					render = 1;
					
					SetFocus(::hWnd);
				}
			}
		} else if (src == hWndState) {
			if (codeNotify == 1) {
				int n = SendMessage(hWndState, CB_GETCURSEL, 0, 0);
				if (n != framedisplay->get_frame()) {
					framedisplay->command(COMMAND_FRAME_SET, &n);
					update_state_box();
					render = 1;
					
					SetFocus(::hWnd);
				}
			}
		}
		
		return;
	}
	
	// handle menu selections
	switch(id) {
	case IDM_FILE_OPEN_TOUHOU:
		do_open_folder("Open Touhou folder...", callback_touhou);
		break;
	case IDM_FILE_OPEN_ABK:
		do_open_folder("Open Akatsuki Blitzkampf folder...", callback_abk);
		break;
	case IDM_FILE_OPEN_MBAA:
		do_open_file("MBAA ISO or mepRofs.cvm%*.iso;mepRofs.cvm", callback_mbaa);
		break;
	case IDM_FILE_OPEN_MBAACC:
		do_open_file("MBAACC 0002.p%0002.p", callback_mbaacc);
		break;
	case IDM_FILE_OPEN_OUGON:
		do_open_folder("Open Ougon Musou Kyoku folder...", callback_ougon);
		break;
	case IDM_FILE_CLOSE:
		free_framedisplay();
		break;
	case IDM_FILE_EXIT:
		PostQuitMessage(0);
		break;
	case IDM_FRAME_PREV_CHARACTER:
		if (framedisplay) {
			framedisplay->command(COMMAND_CHARACTER_PREV, 0);
			fill_sequence_box();
			fill_state_box();
			update_comboboxes();
			render = 1;
		}
		break;
	case IDM_FRAME_NEXT_CHARACTER:
		if (framedisplay) {
			framedisplay->command(COMMAND_CHARACTER_NEXT, 0);
			fill_sequence_box();
			fill_state_box();
			update_comboboxes();
			render = 1;
		}
		break;
	case IDM_FRAME_PREV_SEQUENCE:
		if (framedisplay) {
			framedisplay->command(COMMAND_SEQUENCE_PREV, 0);
			fill_state_box();
			update_comboboxes();
			render = 1;
		}
		break;
	case IDM_FRAME_NEXT_SEQUENCE:
		if (framedisplay) {
			framedisplay->command(COMMAND_SEQUENCE_NEXT, 0);
			fill_state_box();
			update_comboboxes();
			render = 1;
		}
		break;
	case IDM_FRAME_PREV_STATE:
		if (framedisplay) {
			framedisplay->command(COMMAND_FRAME_PREV, 0);
			update_comboboxes();
			render = 1;
		}
		break;
	case IDM_FRAME_NEXT_STATE:
		if (framedisplay) {
			framedisplay->command(COMMAND_FRAME_NEXT, 0);
			update_comboboxes();
			render = 1;
		}
		break;
	case IDM_FRAME_DETAILED_PROPERTIES:
		toggle_frame_detailed_properties.toggle();
		render = 1;
		break;
	case IDM_FRAME_VIEW_SEQUENCE_TEXT:
		if (framedisplay) {
			textview("View Sequence Text", "This is a test.\n");
		}
		break;
	case IDM_FRAME_VIEW_FRAME_TEXT:
		if (framedisplay) {
			textview("View Frame Text", "This is a test.\n");
		}
		break;
	case IDM_VIEW_ANIMATE:
		toggle_view_animate.toggle();
		break;
	case IDM_VIEW_FLIP_HORIZONTALLY:
		toggle_view_flip.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_GRADIENT:
		toggle_view_draw_gradient.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_AXIS:
		toggle_view_draw_axis.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_FRAME_INFO:
		toggle_view_display_frame_info.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_FRAME_PROPS:
		toggle_view_display_frame_props.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_SPRITE:
		toggle_view_display_sprite.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_SOLID_BOXES:
		toggle_view_display_solid_boxes.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_COLLISION_BOX:
		toggle_view_display_collision_box.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_HIT_BOX:
		toggle_view_display_hit_box.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_ATTACK_BOX:
		toggle_view_display_attack_box.toggle();
		render = 1;
		break;
	case IDM_VIEW_DISPLAY_CLASH_BOX:
		toggle_view_display_clash_box.toggle();
		render = 1;
		break;
	case IDM_VIEW_NEXT_PALETTE:
		if (framedisplay) {
			framedisplay->command(COMMAND_PALETTE_NEXT, 0);
			render = 1;
		}
		break;
	case IDM_VIEW_PREV_PALETTE:
		if (framedisplay) {
			framedisplay->command(COMMAND_PALETTE_PREV, 0);
			render = 1;
		}
		break;
	case IDM_VIEW_DEFAULT_PALETTE:
		if (framedisplay) {
			int value = 0;
			framedisplay->command(COMMAND_PALETTE_PREV, &value);
			render = 1;
		}
		break;
	case IDM_CLONE_CREATE_CLONE:
		if (framedisplay) {
			Clone *clone = framedisplay->make_clone();
			if (clone) {
				if (clone_list.begin() == clone_list.end()) {
					EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_ALL_CLONES, MF_BYCOMMAND|MF_ENABLED);
					EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_CLONE, MF_BYCOMMAND|MF_ENABLED);
					EnableMenuItem(hMenuClone, IDM_CLONE_FLIP_CLONE, MF_BYCOMMAND|MF_ENABLED);
				}
				CloneData clone_data;
				clone_data.x = 1.0 - position_x;
				clone_data.y = position_y;
				clone_data.flip = 1 - toggle_view_flip.get();
				clone_data.clone = clone;
				
				clone_list.push_front(clone_data);
				
				render = 1;
			}
		}
		break;
	case IDM_CLONE_DESTROY_CLONE: {
			std::list<CloneData>::iterator j = clone_list.end();
			--j;
			if (j != clone_list.end()) {
				delete j->clone;
				clone_list.erase(j);
				
				if (clone_list.begin() == clone_list.end()) {
					EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_ALL_CLONES, MF_BYCOMMAND|MF_GRAYED);
					EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_CLONE, MF_BYCOMMAND|MF_GRAYED);
					EnableMenuItem(hMenuClone, IDM_CLONE_FLIP_CLONE, MF_BYCOMMAND|MF_GRAYED);
				}
				
				render = 1;
			}
		}
		break;
	case IDM_CLONE_DESTROY_ALL_CLONES: {
			while (clone_list.begin() != clone_list.end()) {
				std::list<CloneData>::iterator j = clone_list.begin();
				
				delete j->clone;
				
				clone_list.erase(j);
			}
			
			EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_ALL_CLONES, MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_CLONE, MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(hMenuClone, IDM_CLONE_FLIP_CLONE, MF_BYCOMMAND|MF_GRAYED);
		}
		break;
	case IDM_CLONE_FLIP_CLONE: {
			std::list<CloneData>::iterator j = clone_list.end();
			--j;
			if (j != clone_list.end()) {
				j->flip = !j->flip;
			}
			
			render = 1;
		}
		break;
	case IDM_MISC_SAVE_SPRITE:
		if (framedisplay) {
			// get filename from fdisp
			const char *filename = framedisplay->get_current_sprite_filename();
			int len;
			
			if (filename && (len = strlen(filename)) < 250) {
				char my_fname[256];
				strcpy(my_fname, filename);
				
				if (len > 4 && my_fname[len-4] == '.') {
					strcpy(my_fname + len - 4, ".png");
				} else {
					strcpy(my_fname + len, ".png");
				}
				
				// ask where to save to..
				
				do_save_file(my_fname, "PNG file%*.png", callback_save_sprite);
			}
		}
		break;
	case IDM_MISC_SAVE_ALL_SPRITES:
		if (framedisplay) {
			do_open_folder("Save all sprites to...", callback_save_all_sprites);
		}
		break;
	}
}

static void init_menu() {
	hMenubar = CreateMenu();
	
	// FILE MENU
	HMENU hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN_TOUHOU, "Open PC Touhou 7.5, 10.5, or 12.3...");
	AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN_ABK, "Open PC Akatsuki Blitzkampf...");
	/*
	AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN_MBAC, "Open PC Melty Blood Act Cadenza...");
	 */
	AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN_OUGON, "Open PC Ougon Musou Kyoku");
	AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN_MBAA, "Open PS2 Melty Blood Actress Again...");
	AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN_MBAACC, "Open PC Melty Blood AACC...");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_FILE_CLOSE, "Close");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_FILE_EXIT, "&Quit");
	
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "&File");

	hMenuFile = hMenu;
	
	// VIEW MENU
	hMenu = CreateMenu();
	
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_ANIMATE, "Animate Toggle\tSpace");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_FLIP_HORIZONTALLY, "Flip Horizontally");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_GRADIENT, "Display Gradient\tF1");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_AXIS, "Display Axis");

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_FRAME_INFO, "Display Current Frame Info");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_FRAME_PROPS, "Display Frame Properties");

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_SPRITE, "Display Sprite");
	
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_SOLID_BOXES, "Display Solid Boxes");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_COLLISION_BOX, "Display Collision Boxes");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_HIT_BOX, "Display Hit Boxes");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_ATTACK_BOX, "Display Attack Boxes");
	AppendMenu(hMenu, MF_STRING | MF_CHECKED, IDM_VIEW_DISPLAY_CLASH_BOX, "Display Melty Clash Boxes");

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING, IDM_VIEW_NEXT_PALETTE, "Next Palette\tTab");
	AppendMenu(hMenu, MF_STRING, IDM_VIEW_PREV_PALETTE, "Previous Palette");
	AppendMenu(hMenu, MF_STRING, IDM_VIEW_DEFAULT_PALETTE, "Default Palette");

	hMenuView = hMenu;
	
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "&View");

	// FRAME MENU
	hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_PREV_CHARACTER, "Previous Character\tPgUp");
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_NEXT_CHARACTER, "Next Character\tPgDn");
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_PREV_SEQUENCE, "Previous Sequence\tUp Arrow");
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_NEXT_SEQUENCE, "Next Sequence\tDown Arrow");
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_PREV_STATE, "Previous State\tLeft Arrow");
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_NEXT_STATE, "Next State\tRight Arrow");
	
	// disabled for now
	/*
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING | MF_UNCHECKED, IDM_FRAME_DETAILED_PROPERTIES, "Detailed Properties");

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING, IDM_FRAME_VIEW_SEQUENCE_TEXT, "View sequence text");
	AppendMenu(hMenu, MF_STRING, IDM_FRAME_VIEW_FRAME_TEXT, "View frame text");
	 */

	hMenuFrame = hMenu;
	
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "F&rame");

	// CLONE MENU
	hMenu = CreateMenu();
	
	AppendMenu(hMenu, MF_STRING, IDM_CLONE_CREATE_CLONE, "Create Clone");
	AppendMenu(hMenu, MF_STRING, IDM_CLONE_DESTROY_CLONE, "Destroy Current Clone");
	AppendMenu(hMenu, MF_STRING, IDM_CLONE_DESTROY_ALL_CLONES, "Destroy All Clones");

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, MF_STRING, IDM_CLONE_FLIP_CLONE, "Flip Current Clone");

	hMenuClone = hMenu;
	
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "&Clone");
	
	// MISC MENU
	hMenu = CreateMenu();
	
	AppendMenu(hMenu, MF_STRING, IDM_MISC_SAVE_SPRITE, "Save this sprite...");
	AppendMenu(hMenu, MF_STRING, IDM_MISC_SAVE_ALL_SPRITES, "Save all this character's sprites...");
	
	hMenuMisc = hMenu;
	
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "&Misc");
	
	// finalize
	SetMenu(hWnd, hMenubar);

	// set defaults
	EnableMenuItem(hMenuFile, IDM_FILE_CLOSE, MF_BYCOMMAND|MF_GRAYED);
	
	EnableMenuItem(hMenuView, IDM_VIEW_NEXT_PALETTE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuView, IDM_VIEW_PREV_PALETTE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuView, IDM_VIEW_DEFAULT_PALETTE, MF_BYCOMMAND|MF_GRAYED);
	
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_CHARACTER, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_CHARACTER, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_SEQUENCE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_SEQUENCE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_PREV_STATE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_NEXT_STATE, MF_BYCOMMAND|MF_GRAYED);
	
	EnableMenuItem(hMenuFrame, IDM_FRAME_VIEW_SEQUENCE_TEXT, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuFrame, IDM_FRAME_VIEW_FRAME_TEXT, MF_BYCOMMAND|MF_GRAYED);

	EnableMenuItem(hMenuClone, IDM_CLONE_CREATE_CLONE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_CLONE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuClone, IDM_CLONE_DESTROY_ALL_CLONES, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuClone, IDM_CLONE_FLIP_CLONE, MF_BYCOMMAND|MF_GRAYED);
	
	EnableMenuItem(hMenuMisc, IDM_MISC_SAVE_SPRITE, MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hMenuMisc, IDM_MISC_SAVE_ALL_SPRITES, MF_BYCOMMAND|MF_GRAYED);
	
	toggle_view_animate.update();
	toggle_view_flip.update();
	toggle_view_draw_gradient.update();
	toggle_view_draw_axis.update();
	toggle_view_display_frame_info.update();
	toggle_view_display_sprite.update();
	toggle_view_display_solid_boxes.update();
	toggle_view_display_collision_box.update();
	toggle_view_display_hit_box.update();
	toggle_view_display_attack_box.update();
	toggle_view_display_clash_box.update();
	toggle_frame_detailed_properties.update();
}

// ****************************************************** WINDOW LAYOUT CODE

static HWND make_combobox(HWND root, int x, int w) {
	HWND hWndCombo;
	
	DWORD dwBaseUnits = GetDialogBaseUnits();
	
	hWndCombo = CreateWindow(WC_COMBOBOXEX, "",
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
		(x * LOWORD(dwBaseUnits)) / 4,
		(1 * HIWORD(dwBaseUnits)) / 8,
		(w * LOWORD(dwBaseUnits)) / 4,
		(50 * HIWORD(dwBaseUnits)) / 8,
		root, NULL, hAppInst, NULL);
	
	return hWndCombo;
}

static bool init_layout() {
	hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, 0,
				WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE,
				0, 0, 0, 0,
				hWnd, 0, hAppInst, 0);
	if (!hWndToolbar) {
		return 0;
	}
	
	toolbar_height = 28;
	
	MoveWindow(hWndToolbar, 0, 0, 800, toolbar_height, 0);
	
	if (!register_opengl_window()) {
		return 0;
	}
	
	hWndOGL = CreateWindowEx(0, opengl_class_name, 0,
				WS_CHILD | WS_VISIBLE,
				0, 0, 0, 0,
				hWnd, 0, hAppInst, 0);
	
	if (!hWndOGL) {
		return 0;
	}
	
	if (!init_opengl(hWndOGL)) {
		return 0;
	}
	
	MoveWindow(hWndOGL, 0, toolbar_height, 800, 600 - toolbar_height, 0);
	
	// initialize comboboxes
	
	hWndCharacter = make_combobox(hWndToolbar, 2, 120);
	hWndSequence = make_combobox(hWndToolbar, 124, 120);
	hWndState = make_combobox(hWndToolbar, 246, 60);
	
	// this doesn't work. is there a better way of supporting SJIS?
	// Probably going to end up just adding some code to translate
	// from SJIS to UNICODE.
	SendMessage(hWndSequence, CB_SETLOCALE,
		(WPARAM)MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), SORT_JAPANESE_XJIS),
		(LPARAM)0);
	
	return 1;
}

// ****************************************************** BASE WINDOW CODE

static void callback_close(HWND) {
	PostQuitMessage(0);
}

static LRESULT CALLBACK window_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CREATE:
			//init_opengl(hWnd);
			break;
		HANDLE_MSG(hWnd, WM_CLOSE,	callback_close);
		HANDLE_MSG(hWnd, WM_COMMAND,	callback_command);
		case WM_PAINT:
			draw_opengl();
			break;
		case WM_SIZE: {
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				
				MoveWindow(hWndToolbar, 0, 0, width, toolbar_height, 1);
				MoveWindow(hWndOGL, 0, toolbar_height, width, height-toolbar_height, 1);
				break;
			}
		case WM_KEYDOWN:
			switch(wParam) {
			case VK_SPACE:
				toggle_view_animate.toggle();
				break;
			case VK_F1:
				toggle_view_draw_gradient.toggle();
				render = 1;
				break;
			}
			
			if (framedisplay) {
				switch(wParam) {
				case VK_UP:
				case VK_NUMPAD8:
					framedisplay->command(COMMAND_SEQUENCE_PREV, 0);
					fill_state_box();
					update_comboboxes();
					render = 1;
					break;
				case VK_DOWN:
				case VK_NUMPAD2:
					framedisplay->command(COMMAND_SEQUENCE_NEXT, 0);
					fill_state_box();
					update_comboboxes();
					render = 1;
					break;
				case VK_LEFT:
				case VK_NUMPAD4:
					framedisplay->command(COMMAND_FRAME_PREV, 0);
					update_comboboxes();
					render = 1;
					break;
				case VK_RIGHT:
				case VK_NUMPAD6:
					framedisplay->command(COMMAND_FRAME_NEXT, 0);
					update_comboboxes();
					render = 1;
					break;
				case VK_PRIOR:
				case VK_NUMPAD9:
					framedisplay->command(COMMAND_CHARACTER_PREV, 0);
					fill_sequence_box();
					fill_state_box();
					update_comboboxes();
					render = 1;
					break;
				case VK_NEXT:
				case VK_NUMPAD3:
					framedisplay->command(COMMAND_CHARACTER_NEXT, 0);
					fill_sequence_box();
					fill_state_box();
					update_comboboxes();
					render = 1;
					break;
				case VK_TAB:
					framedisplay->command(COMMAND_PALETTE_NEXT, 0);
					render = 1;
					break;
				default:
					break;
				}
			}
			break;
	}
	
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

static bool register_window() {
	WNDCLASSEX WndClassEx;
	
	memset(&WndClassEx, 0, sizeof(WndClassEx));
	WndClassEx.cbSize = sizeof(WndClassEx);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WndClassEx.lpfnWndProc = window_proc;
	WndClassEx.hInstance = hAppInst;
	WndClassEx.hCursor = LoadCursor(0, IDC_ARROW);
	WndClassEx.hbrBackground = 0; // CreateSolidBrush(0);
	WndClassEx.lpszClassName = class_name;
	
	if (!RegisterClassEx(&WndClassEx)) {
		return 0;
	}
	
	return 1;
}

static void close_window() {
	close_opengl();
	
	if (hMenubar) {
		DestroyMenu(hMenubar);
		hMenubar = 0;
	}
	
	if (hWnd) {
		DestroyWindow(hWnd);
		hWnd = 0;
	}
	
	UnregisterClass(class_name, hAppInst);
}

static bool init_window() {
	close_window();
	
	if (!register_window()) {
		return 0;
	}
	
	hWnd = CreateWindow(
			class_name, program_name, 
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
			NULL, NULL, hAppInst, NULL);
	
	if (!hWnd) {
		return 0;
	}
	
	/*
	if (!init_opengl()) {
		return 0;
	}
	 */
	
	init_menu();
	
	if (!init_layout()) {
		return 0;
	}
	
	return 1;
}

// ****************************************************** TIMER
static struct {
	int last_ticks;
	
	float tick_diff;
} TimerData;

void timer_init() {
	TimerData.last_ticks = GetTickCount();
	TimerData.tick_diff = 0.0;
}

void timer_update() {
	unsigned int ticks = GetTickCount();
	unsigned int diff = ticks - TimerData.last_ticks;
	
	TimerData.last_ticks = ticks;
	
	TimerData.tick_diff += diff;
}

bool timer_get_tick() {
	timer_update();
	
	if (TimerData.tick_diff > (1000.0/60.0)) {
		if (TimerData.tick_diff > (1000.0/30.0)) {
			TimerData.tick_diff = 0;
		} else {
			TimerData.tick_diff -= 1000.0/60.0;
		}
		
		return 1;
	}
	
	return 0;
}

// ****************************************************** MAIN

void run_window(int nShowCmd) {
	MSG msg;
	bool done = 0;
	
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);
	
	//nShowCmd = SW_NORMAL;
	
	SetForegroundWindow(hWnd);
	
	wglMakeCurrent(hDC, hGLRC);
	
	timer_init();
	
	render = 1;
	
	while (!done) {
		// handle events
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				done = 1;
				
				break;
			}
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		// run timed events
		while (timer_get_tick()) {
			if (framedisplay) {
				if (toggle_view_animate.get()) {
					int fr_id = framedisplay->get_frame();
				
					framedisplay->command(COMMAND_SUBFRAME_NEXT, 0);
				
					int nfr_id = framedisplay->get_frame();
				
					if (fr_id != nfr_id) {
						update_state_box();
						render = 1;
					}
				}
			}
		}
		
		// redraw if needed
		if (render) {
			draw_opengl();
			
			render = 0;
		}
		
		// sleep.
		Sleep(1);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLIne, int nShowCmd) {
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_USEREX_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);
	
	hAppInst = hInstance;
	
	if (!init_window()) {
		close_window();
		
		return 0;
	}
	
	run_window(nShowCmd);
	
	close_window();
	
	return 0;
}
