#include <raylib.h>
#include <raymath.h>
#include <eecs.h>
#include <algorithm>
#include <string>
#include <cstring>
#include "dcengine/ui.h"
#include "dcengine/math.h"

void draw_centered_texture(Texture2D tex, Rectangle rect, float scale)
{
  Vector2 pos = Vector2{rect.x + (rect.width - tex.width * scale) * 0.5f, rect.y + (rect.height - tex.height * scale) * 0.5f};
  DrawTextureEx(tex, pos, 0.f, scale, WHITE);
}
struct ColorPairs
{
    const char* cmd;
    Color col;
};
const ColorPairs colorPairs[] =
{
    {"~red~", RED},
    {"~green~", GREEN},
    {"~blue~", BLUE},
    {"~clear~", WHITE}
};

std::string clear_text_from_color_commands(const char* text)
{
    std::string buffer;
    const char* start = text;
    const char* cursor = text;
    for (const char *cursor = start; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '~')
        {
            if (cursor != start)
            {
                std::string prevStr = std::string(start, cursor - start);
                buffer += prevStr;
            }
            // process the command
            for (ColorPairs cp : colorPairs)
            {
                const int len = strlen(cp.cmd);
                if (strncmp(cursor, cp.cmd, len) == 0)
                {
                    cursor += len;
                    start = cursor;
                }
            }
        }
    }
    buffer += start;
    return buffer;
}

void draw_colored_font(Font font, const char* text, float x, float y, float size, int spacing, Color tint)
{
    const char* start = text;
    Color curColor = tint;
    const char* cursor = text;
    for (const char *cursor = start; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '~')
        {
            if (cursor != start)
            {
                std::string prevStr = std::string(start, cursor - start);
                DrawTextEx(font, prevStr.c_str(), tovec(x, y), size, spacing, curColor);
                x += MeasureTextEx(font, prevStr.c_str(), size, spacing).x + spacing;
            }
            // process the command
            for (ColorPairs cp : colorPairs)
            {
                const int len = strlen(cp.cmd);
                if (strncmp(cursor, cp.cmd, len) == 0)
                {
                    curColor = ColorTint(tint, cp.col);
                    cursor += len;
                    start = cursor;
                }
            }
        }
    }
    DrawTextEx(font, start, tovec(x, y), size, spacing, curColor);
}

void draw_font_with_shadow(Font font, const char* text, float x, float y, float size, int spacing, Color col, int thickness)
{
  Color shadowC = BLACK;
  shadowC.a = col.a;

  for (int dx = -thickness; dx <= thickness; dx++)
  {
      for (int dy = -thickness; dy <= thickness; dy++)
      {
          if (dx == 0 && dy == 0)
              continue;

          // circle-ish mask (looks better than square)
          if (dx*dx + dy*dy > thickness*thickness)
              continue;

          draw_colored_font(font, text, x + dx, y + dy, size, spacing, shadowC);
          //DrawTextEx(font, text, tovec(x + dx, y + dy), size, spacing, shadowC);
      }
  }

  draw_colored_font(font, text, x, y, size, spacing, col);
  //DrawTextEx(font, text, tovec(x, y), size, spacing, col);
}

float draw_bounded_font_with_shadow(Font font, const char* text, float x, float y, float width, float size, int spacing, float pad, Color col)
{
  std::string str = text;
  size_t start = 0;
  size_t next = str.find_first_of(" ", start);
  float xpos = x;
  float ypos = y;
  do
  {
    if (next == std::string::npos)
    {
      std::string sub = str.substr(start) + " ";
      Vector2 sz = MeasureTextEx(font, clear_text_from_color_commands(sub.c_str()).c_str(), size, spacing);
      if (xpos + sz.x > width)
      {
        xpos = x;
        ypos += sz.y + pad;
      }
      draw_font_with_shadow(font, sub.c_str(), xpos, ypos, size, spacing, col);
      xpos += sz.x;
      break;
    }
    std::string sub = str.substr(start, next - start) + " ";
    Vector2 sz = MeasureTextEx(font, clear_text_from_color_commands(sub.c_str()).c_str(), size, spacing);
    if (xpos + sz.x > width)
    {
      xpos = x;
      ypos += sz.y + pad;
    }
    draw_font_with_shadow(font, sub.c_str(), xpos, ypos, size, spacing, col);
    xpos += sz.x;
    start = next + 1;
    next = str.find_first_of(" ", start);
  } while (true);
  return ypos + size + pad;
}

void draw_centered_font_with_shadow(Font font, const char* text, Rectangle rect, float size, int spacing, Color col, FontCentering fc, int thickness)
{
  const bool vcenter = fc & EFC_VCENTER;
  const bool hcenter = fc & EFC_HCENTER;
  std::string str = clear_text_from_color_commands(text);
  Vector2 sz = MeasureTextEx(font, str.c_str(), size, spacing);
  draw_font_with_shadow(font, text, rect.x + (hcenter ? (rect.width - sz.x) * 0.5f : 0), rect.y + (vcenter ? (rect.height - sz.y) * 0.5f : 0), size, spacing, col, thickness);
}

void draw_centered_block_with_shadow(Font font, int num, const char** text, Rectangle rect, float size, int spacing, Color col)
{
  const Vector2 sz = MeasureTextEx(font, text[0], size, spacing);
  const float lineHt = sz.y;
  float vstart = rect.y + (rect.height - lineHt * num) * 0.5f;
  for (int i = 0; i < num; ++i)
  {
      std::string str = clear_text_from_color_commands(text[i]);
      draw_centered_font_with_shadow(font, str.c_str(), torect(rect.x, vstart + lineHt * i, rect.width, lineHt), size, spacing, col);
  }
}

void draw_button(Rectangle rect, const char* text, float scaleFactor, Color col)
{
  Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, rect))
  {
    Vector3 hsv = ColorToHSV(col);
    col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
  }
  DrawRectangleRec(rect, col);
  draw_centered_font_with_shadow(GetFontDefault(), text, rect, 12.f * scaleFactor, 3, WHITE);
}

void draw_button_9rect(const NineRect& nr, Rectangle rect, Font font, const char* text, float fontSize, int spacing, float scaleFactor, Color col, const std::function<void()>& on_click)
{
  Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, rect))
  {
    Vector3 hsv = ColorToHSV(col);
    const char alpha = col.a;
    col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
    col.a = alpha;
    if (IsMouseButtonReleased(0))
        on_click();
  }
  draw_9rect(nr, rect, scaleFactor, col);
  draw_centered_font_with_shadow(font, text, rect, fontSize * scaleFactor, spacing, Color{255, 255, 255, col.a});
}

void register_ui(eecs::Registry& reg)
{
  // do nothing yet
}

void create_ui_helper(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    printf("setting scale to %.2f\n", scaleFactor);
    eecs::create_entity_wrap(reg)
        .set(COMPID(float, window_width), width)
        .set(COMPID(float, window_height), height)
        .set(COMPID(float, window_scaleFactor), scaleFactor);
}

NineRect create_9rect(Image img, int sz)
{
  NineRect res;
  res.sz = sz;
  res.rects[0] = LoadTextureFromImage(ImageFromImage(img, torect(0, 0, sz, sz)));
  res.rects[1] = LoadTextureFromImage(ImageFromImage(img, torect(sz, 0, img.width - 2 * sz, sz)));
  res.rects[2] = LoadTextureFromImage(ImageFromImage(img, torect(img.width - sz, 0, sz, sz)));

  res.rects[3] = LoadTextureFromImage(ImageFromImage(img, torect(0, sz, sz, img.height - 2 * sz)));
  res.rects[4] = LoadTextureFromImage(ImageFromImage(img, torect(sz, sz, img.width - 2 * sz, img.height - 2 * sz)));
  res.rects[5] = LoadTextureFromImage(ImageFromImage(img, torect(img.width - sz, sz, sz, img.height - 2 * sz)));

  res.rects[6] = LoadTextureFromImage(ImageFromImage(img, torect(0, img.height - sz, sz, sz)));
  res.rects[7] = LoadTextureFromImage(ImageFromImage(img, torect(sz, img.height - sz, img.width - 2 * sz, sz)));
  res.rects[8] = LoadTextureFromImage(ImageFromImage(img, torect(img.width - sz, img.height - sz, sz, sz)));

  for (Texture2D& tex : res.rects)
  {
    //SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
  }
  return res;
}

void draw_tiled_tex(Texture2D tex, Rectangle source, Rectangle dest, float scale, Color col)
{
  float x = 0;
  while (x < source.width)
  {
    float ww = std::min((float)tex.width, (float)source.width - x);
    float y = 0;
    while (y < source.height)
    {
      float hh = std::min((float)tex.height, (float)source.height - y);
      DrawTexturePro(tex, torect(0, 0, ww, hh),
          torect(dest.x + x * scale, dest.y + y * scale, ww * scale, hh * scale), Vector2{0, 0}, 0, col);
      y += hh;
    }
    x += ww;
  }
}

void draw_9rect(const NineRect& nr, Rectangle rect, float scale, Color col)
{
  Vector2 csz = Vector2{rect.width / scale - nr.sz * 2, rect.height / scale - nr.sz * 2};
  std::array<float, 3> hzSz =   {(float)nr.sz, (float)csz.x, (float)nr.sz};
  std::array<float, 3> vertSz = {(float)nr.sz, (float)csz.y, (float)nr.sz};
  std::array<float, 3> hzGrid =
  {
    rect.x,
    rect.x + nr.sz * scale,
    rect.x + nr.sz * scale + csz.x * scale
  };
  std::array<float, 3> vertGrid =
  {
    rect.y,
    rect.y + nr.sz * scale,
    rect.y + nr.sz * scale + csz.y * scale
  };

  for (size_t i = 0; i < 9; ++i)
  {
    size_t x = i % 3;
    size_t y = i / 3;
    draw_tiled_tex(nr.rects[i], torect(0, 0, hzSz[x], vertSz[y]), torect(hzGrid[x], vertGrid[y], hzSz[x] * scale, vertSz[y] * scale), scale, col);
  }
}

void draw_cube_matrix(const BoundingBox& bbox, const vec3f& pos, float rotation, Color col)
{
  Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, rotation*DEG2RAD);
  Matrix matTranslation = MatrixTranslate(pos.x, pos.y, pos.z);

  Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
  Vector3 vertices[8];
  int idx = 0;
  for (float z : {bbox.min.z, bbox.max.z})
    for (float y : {bbox.min.y, bbox.max.y})
      for (float x : {bbox.min.x, bbox.max.x})
        vertices[idx++] = Vector3{x, y, z} * matTransform;
  /*
       3+----+7
       /|   /|
      / |  / |
    2+----+6 |
     | 1+-|--+5
     | /  | /
     |/   |/
    0+----+4

     y
     ^ /z
     |/
     o-->x
  */
  const int indices[] =
  {
    0, 4, 6,
    0, 6, 2,
    1, 0, 2,
    1, 2, 3,
    4, 5, 7,
    4, 7, 6,
    0, 1, 5,
    0, 5, 4,
    1, 3, 7,
    1, 7, 5,
    2, 6, 7,
    2, 7, 3
  };
  for (size_t i = 0; i < sizeof(indices) / sizeof(indices[0]); i += 3)
    DrawTriangle3D(vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]], col);

  const int lineIndices[] =
  {
    0, 4,
    4, 6,
    6, 2,
    2, 0,
    4, 5,
    5, 7,
    7, 6,
    7, 3,
    3, 1,
    1, 5,
    3, 2,
    1, 0
  };
  for (size_t i = 0; i < sizeof(lineIndices) / sizeof(lineIndices[0]); i += 2)
    DrawLine3D(vertices[lineIndices[i]], vertices[lineIndices[i+1]], Color{col.r, col.g, col.b, 255});
}
