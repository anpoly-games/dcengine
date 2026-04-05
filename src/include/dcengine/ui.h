#pragma once
#include <raylib.h>
#include <array>
#include <functional>
#include "math.h"

constexpr Color selectedColor = Color{240, 230, 0, 220};
constexpr Color notSelectedColor = Color{120, 120, 120, 220};

enum FontCentering
{
  EFC_HCENTER = 1,
  EFC_VCENTER = 2
};

void draw_font_with_shadow(Font font, const char* text, float x, float y, float size, int spacing, Color col, int thickness = 1);
void draw_colored_font(Font font, const char* text, float x, float y, float size, int spacing, Color tint);
float draw_bounded_font_with_shadow(Font font, const char* text, float x, float y, float width, float size, int spacing, float pad, Color col);
void draw_centered_font_with_shadow(Font font, const char* text, Rectangle rect, float size, int spacing, Color col, FontCentering fc = FontCentering(EFC_HCENTER | EFC_VCENTER), int thickness = 1);
void draw_centered_block_with_shadow(Font font, int num, const char** text, Rectangle rect, float size, int spacing, Color col);
void draw_centered_texture(Texture2D tex, Rectangle rect, float scale);

void draw_button(Rectangle rect, const char* text, float scaleFactor, Color col);

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor);

void draw_game_ui(eecs::Registry& reg);

void register_ui(eecs::Registry& reg);
void create_ui_helper(eecs::Registry& reg, float width, float height, float scaleFactor);

template<typename Vec>
inline bool is_vec_in_rect(const Vec& v, Rectangle r) { return v.x >= r.x && v.y >= r.y && v.x <= r.x + r.width && v.y <= r.y + r.height; }

bool is_cursor_over_ui(eecs::Registry& reg);
bool is_ui_blocks_input(eecs::Registry& reg);

struct NineRect
{
  std::array<Texture2D, 9> rects;
  int sz;
};
NineRect create_9rect(Image img, int sz);
void draw_9rect(const NineRect& nr, Rectangle rect, float scale, Color col);

void draw_tiled_tex(Texture2D tex, Rectangle source, Rectangle dest, float scale, Color col);
void draw_button_9rect(const NineRect& nrect, Rectangle rect, Font font, const char* text, float fontSize, int spacing, float scaleFactor, Color col, const std::function<void()>& foo);

void draw_cube_matrix(const BoundingBox& bbox, const vec3f& pos, float rotation, Color col);

