#include <SDL/SDL.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include "mbaa_framedisplay.h"
#include "mbaacc_framedisplay.h"
#include "touhou_framedisplay.h"
#include "abk_framedisplay.h"
#include "ougon_framedisplay.h"
#include "render.h"

static MBAACC_FrameDisplay fdisp;

static int window_width = 640;
static int window_height = 480;

static int draw_gradient = 1;
static float window_scale = 1.0;

static int position_x = 180;
static int position_y = 400;

static void setup_opengl() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, window_width, window_height, 0, -2048, 2048);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_DEPTH_TEST);

  render_font_init();
}

static void display_bg() {
  glBegin(GL_QUADS);
  glColor4f(0.0, 0.075, 0.075, 1.0);
  glVertex2f(0.0, 0.0);
  glVertex2f(window_width, 0.0);
  if (draw_gradient) {
    glColor4f(0.0, 0.15, 0.15, 1.0);
  }
  glVertex2f(window_width, window_height);
  glVertex2f(0.0, window_height);
  glEnd();
}
static void display_axis() {
  glColor4f(0.0, 0.8, 0.8, 1.0);
  glBegin(GL_LINES);
  glVertex2f(0.0, position_y);
  glVertex2f(window_width, position_y);
  glVertex2f(position_x, 0.0);
  glVertex2f(position_x, window_height);
  glEnd();
}

PFNGLBLENDEQUATIONEXTPROC pglBlendEquation = 0;

static void display_scene() {
  display_bg();
  display_axis();

  glPushMatrix();
  glTranslatef(position_x, position_y, 0.0);
  glScalef(window_scale, window_scale, 1.0);

  static const RenderProperties props = {1, 1, 1, 1, 1, 1};
  fdisp.render(&props);

  glPopMatrix();

  int ch_id = fdisp.get_character();
  int seq_id = fdisp.get_sequence();
  int seq_count = fdisp.get_sequence_count();
  int subfr_id = fdisp.get_subframe();
  int subfr_count = fdisp.get_subframe_count();
  int fr_id = fdisp.get_frame();
  int fr_count = fdisp.get_frame_count();
  int pal_id = fdisp.get_palette();
  const char* ch_name = fdisp.get_character_name(ch_id);

  char buf[128];

  sprintf(buf,
          "%3d %-20s seq %5d fr %5d sp %5d\n                           - "
          "%5d  - %5d  - %5d",
          pal_id, ch_name, seq_id + 1, subfr_id + 1, fr_id + 1, seq_count, subfr_count, fr_count);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  render_shaded_text(window_width - 550, window_height - 35, buf);

  fdisp.render_frame_properties(1, window_width, window_height);
}

int main(int argc, char** argv) {
  if (!fdisp.init()) {
    return 0;
  }

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Surface* surface = SDL_SetVideoMode(640, 480, 0, SDL_OPENGL | SDL_RESIZABLE);
  if (!surface) {
    return -1;
  }

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  pglBlendEquation = (PFNGLBLENDEQUATIONEXTPROC)SDL_GL_GetProcAddress("glBlendEquation");

  SDL_WM_SetCaption("MBAA FrameDisplay 0.1 - Simple GUI", 0);

  setup_opengl();

  SDL_GL_SwapBuffers();

  bool ms_down = 0;
  int ms_x = 0, ms_y = 0, start_x = 0, start_y = 0;
  int animate = 0;
  bool done = 0;
  bool render = 0;

  while (!done) {
    if (animate) {
      fdisp.command(COMMAND_SUBFRAME_NEXT, 0);
      render = 1;
    }

    if (render) {
      display_scene();
      SDL_GL_SwapBuffers();

      render = 0;
    }

    SDL_Delay(16);

    SDL_Event sdl_event;
    SDL_PumpEvents();

    while (SDL_PollEvent(&sdl_event)) {
      switch (sdl_event.type) {
        case SDL_KEYDOWN:
          switch (sdl_event.key.keysym.sym) {
            case SDLK_ESCAPE:
              done = 1;
              break;
            case SDLK_F1:
              draw_gradient = 1 - draw_gradient;
              render = 1;
              break;
            case SDLK_SPACE:
              animate = 1 - animate;
              break;
            case SDLK_UP:
            case SDLK_KP8:
              // prev seq
              fdisp.command(COMMAND_SEQUENCE_PREV, 0);
              render = 1;
              break;
            case SDLK_DOWN:
            case SDLK_KP2:
              // next seq
              fdisp.command(COMMAND_SEQUENCE_NEXT, 0);
              render = 1;
              break;
            case SDLK_LEFT:
            case SDLK_KP4:
              // prev frame
              fdisp.command(COMMAND_FRAME_PREV, 0);
              render = 1;
              break;
            case SDLK_RIGHT:
            case SDLK_KP6:
              // next frame
              fdisp.command(COMMAND_FRAME_NEXT, 0);
              render = 1;
              break;
            case SDLK_PAGEUP:
            case SDLK_KP9:
              // prev char
              fdisp.command(COMMAND_CHARACTER_PREV, 0);
              render = 1;
              break;
            case SDLK_PAGEDOWN:
            case SDLK_KP3:
              // next char
              fdisp.command(COMMAND_CHARACTER_NEXT, 0);
              render = 1;
              break;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
              // scale
              window_scale = window_scale / 1.1;
              render = 1;
              break;
            case SDLK_PLUS:
            case SDLK_EQUALS:
            case SDLK_KP_PLUS:
              window_scale = window_scale * 1.1;
              render = 1;
              break;
            case SDLK_BACKSPACE:
              window_scale = 1.0;
              render = 1;
              break;
            case SDLK_TAB:
              // flush textures
              fdisp.command(COMMAND_PALETTE_NEXT, 0);

              render = 1;
              break;
            default:
              break;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          ms_down = 1;
          ms_x = sdl_event.button.x;
          ms_y = sdl_event.button.y;
          start_x = position_x;
          start_y = position_y;
          break;
        case SDL_MOUSEBUTTONUP:
          ms_down = 0;
          break;
        case SDL_MOUSEMOTION:
          if (ms_down == 1) {
            position_x = start_x + sdl_event.motion.x - ms_x;
            position_y = start_y + sdl_event.motion.y - ms_y;
            render = 1;
          }
          break;
        case SDL_VIDEOEXPOSE:
          render = 1;
          break;
        case SDL_VIDEORESIZE:
          SDL_SetVideoMode(sdl_event.resize.w, sdl_event.resize.h, 0, SDL_OPENGL | SDL_RESIZABLE);
          glViewport(0, 0, sdl_event.resize.w, sdl_event.resize.h);

          window_width = sdl_event.resize.w;
          window_height = sdl_event.resize.h;

          setup_opengl();

          // flush textures
          break;
        case SDL_QUIT:
          done = 1;
          break;
      }
    }
  }

  fdisp.free();

  SDL_Quit();

  return 0;
}
