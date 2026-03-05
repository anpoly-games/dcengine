#include <eecs.h>
#include <raylib.h>
#include <string>
#include <filesystem>

#include "editor_ui.h"
#include "editor.h"
#include "dcengine/math.h"
#include "dcengine/game.h"
#include "dcengine/ui.h"
#include "dcengine/level.h"

enum EditorState
{
  E_WORLD = 0,
  E_NEW_FILE,
  E_SAVE_PREV_NEW,
  E_SAVE_PREV_LOAD,
  E_LOAD_SELECT,
  E_SAVE_SELECT
};

static EntTypeList entType = E_FLOORS;
static EditorState editorState = E_WORLD;
static std::string editorLevelName = "";
static std::string editorLevelToLoad = "";
static bool filterSelected = false;


static void draw_right_editor_panel(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    if (editorState != E_WORLD)
        return;
    const float left = width * 0.7f;
    const float top = 0;
    DrawRectangle(left, top, width * 0.3f, height, Color{100, 100, 100, 200});
    const float lpad = 4 * scaleFactor;
    const float vpad = lpad;
    const char* types[E_ETL_NUM] = {"floors", "walls", "doors", "columns", "ceilings", "entities", "logic", "billboards"};

    const Vector2 mp = GetMousePosition();

    float xtabpos = left + lpad;
    float ytabpos = top + vpad;
    for (int i = 0; i < E_ETL_NUM; ++i)
    {
        const float hsize = 100 * scaleFactor;
        const float hspacing = 4 * scaleFactor;
        Color col = entType == i ? selectedColor : notSelectedColor;
        Rectangle rect = torect(xtabpos, ytabpos, hsize, 16 * scaleFactor);
        if (is_vec_in_rect(mp, rect))
        {
            Vector3 hsv = ColorToHSV(col);
            col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
            if (IsMouseButtonReleased(0))
            {
                if (entType != i)
                {
                    eecs::query_entities(reg, [&](eecs::EntityId, EntTypeList& selectedType, std::string& selectedPrefab)
                            {
                                selectedPrefab = ""; selectedType = (EntTypeList)i;
                            }, COMPID(EntTypeList, selectedType), COMPID(std::string, selectedPrefab));
                }
                entType = (EntTypeList)i;
            }
        }
        DrawRectangleRec(rect, col);
        DrawText(types[i], rect.x + lpad, rect.y, 16 * scaleFactor, WHITE);
        xtabpos += hsize + hspacing;
        if (xtabpos > width - (hsize + hspacing))
        {
            xtabpos = left + lpad;
            ytabpos += 16 * scaleFactor + hspacing;
        }
    }

    const float vspacing = 2 * scaleFactor;
    const float hspacing = 4 * scaleFactor;

    float ypos = ytabpos + 16 * scaleFactor + vpad * 2;
    const float fontSz = 16 * scaleFactor;

    static std::string filter;

    eecs::EntityWrap type = eecs::find_entity_wrap(reg, types[entType]);
    eecs::query_component(reg, type.eid, [&](const std::vector<eecs::EntityId>& children)
    {
        const float filterWd = 50.f * scaleFactor;
        Rectangle promptRect = torect(left + filterWd, ypos, width * 0.3 - filterWd, fontSz);
        DrawText("Filter:", left, ypos, fontSz, WHITE);
        DrawRectangleRec(promptRect, Color{50, 50, 50, 200});
        DrawText(filter.c_str(), left + filterWd + lpad, ypos, fontSz, WHITE);
        if (filterSelected)
        {
            int key = GetCharPressed();

            // Check if more characters have been pressed on the same frame
            while (key > 0)
            {
                // NOTE: Only allow keys in range [32..125]
                if ((key >= 32) && (key <= 125))
                    filter += (char)key;

                key = GetCharPressed();  // Check next character in the queue
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !filter.empty())
                filter.pop_back();
        }
        DrawRectangleLinesEx(promptRect, 1.f, filterSelected ? WHITE : BLACK);
        ypos += fontSz + vspacing;
        for (eecs::EntityId eid : children)
            eecs::entity_name(reg, eid, [&](const std::string& name)
            {
                if (name[0] == '_')
                    return;
                if (!filter.empty() && name.find(filter) == std::string::npos)
                    return;
                Rectangle rect = torect(left, ypos, width * 0.3f, fontSz);
                auto procSelection = [&](std::string& val)
                {
                    Color col = notSelectedColor;
                    if (val == name.c_str())
                        col = selectedColor;
                    if (is_vec_in_rect(mp, rect))
                    {
                        Vector3 hsv = ColorToHSV(col);
                        col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
                        if (IsMouseButtonReleased(0))
                        {
                            if (val != name.c_str())
                                val = name.c_str();
                            else
                                val = "";
                        }
                    }
                    DrawRectangleRec(rect, col);
                };
                eecs::query_entities(reg, [&](eecs::EntityId, std::string& selectedPrefab) { procSelection(selectedPrefab); }, COMPID(std::string, selectedPrefab));
                int sw = MeasureText(TextFormat("%s", name.c_str()), fontSz);
                DrawText(TextFormat("%s", name.c_str()), left + lpad, ypos, fontSz, WHITE);
                eecs::EntityWrap e = eecs::wrap_entity(reg, eid);
                e.query_comp([&](const Texture2D& texture_diff)
                {
                    Rectangle texRect = torect(left + lpad + sw + hspacing, ypos, fontSz, fontSz);
                    DrawTexturePro(texture_diff, torect(0, 0, texture_diff.width, texture_diff.height), texRect, tovec(0, 0), 0, WHITE);
                    if (is_vec_in_rect(mp, texRect))
                        DrawTextureEx(texture_diff, Vector2{left - lpad - texture_diff.width * scaleFactor * 10, ypos}, 0.f, scaleFactor * 10, WHITE);
                }, COMPID(const Texture2D, texture_diff));
                ypos += fontSz + vspacing;
            });
        if (IsMouseButtonReleased(0))
        {
            filterSelected = is_vec_in_rect(mp, promptRect);
        }
    }, COMPID(const std::vector<eecs::EntityId>, children));
}

void draw_top_editor_panel(float width, float height, float scaleFactor)
{
  const float left = 0;
  const float top = 0;
  const float pheight = height * 0.06f;
  const float hsize = 100 * scaleFactor;
  const float lpad = 4 * scaleFactor;
  const float vpad = lpad;
  DrawRectangle(left, top, lpad + (hsize + lpad) * 3, pheight, Color{100, 100, 100, 200});

  const Vector2 mp = GetMousePosition();
  Rectangle newFile = torect(left + lpad, top + vpad, hsize, pheight - vpad * 2);
  draw_button(newFile, "New", scaleFactor, notSelectedColor);
  if (is_vec_in_rect(mp, newFile) && IsMouseButtonReleased(0))
    editorState = E_SAVE_PREV_NEW;

  Rectangle loadFile = torect(left + lpad + 1 * (hsize + lpad), top + vpad, hsize, pheight - vpad * 2);
  draw_button(loadFile, "Load", scaleFactor, notSelectedColor);
  if (is_vec_in_rect(mp, loadFile) && IsMouseButtonReleased(0))
    editorState = E_LOAD_SELECT;

  Rectangle saveFile = torect(left + lpad + 2 * (hsize + lpad), top + vpad, hsize, pheight - vpad * 2);
  draw_button(saveFile, "Save", scaleFactor, notSelectedColor);
  if (is_vec_in_rect(mp, saveFile) && IsMouseButtonReleased(0))
    editorState = E_SAVE_SELECT;
    //save_level(reg, editorLevelName.c_str());
}

static void draw_new_file(eecs::Registry& reg, Rectangle panelRect, float scaleFactor)
{
  const float vpad = 4 * scaleFactor;
  const float textSize = 12.f * scaleFactor;
  Rectangle editboxRect = torect(
      panelRect.x + panelRect.width * 0.2f,
      panelRect.y + panelRect.height * 0.5f - textSize * 0.5f - vpad,
      panelRect.width * 0.6f, textSize + 2.f * vpad);
  static std::string newFileName = "";
  Rectangle promptRect = torect(editboxRect.x, editboxRect.y - textSize - vpad, editboxRect.width, textSize);
  draw_centered_font_with_shadow(GetFontDefault(), "Select a filename:",  promptRect, 12.f * scaleFactor, 3, WHITE);
  DrawRectangleRec(editboxRect, Color{50, 50, 50, 200});
  DrawRectangleLinesEx(editboxRect, 1.f, BLACK);

  int key = GetCharPressed();

  // Check if more characters have been pressed on the same frame
  while (key > 0)
  {
    // NOTE: Only allow keys in range [32..125]
    if ((key >= 32) && (key <= 125))
      newFileName += (char)key;

    key = GetCharPressed();  // Check next character in the queue
  }

  if (IsKeyPressed(KEY_BACKSPACE) && !newFileName.empty())
    newFileName.resize(newFileName.size() - 1);

  draw_font_with_shadow(GetFontDefault(), newFileName.c_str(), editboxRect.x + vpad, editboxRect.y + vpad, textSize, 3, WHITE);

  Rectangle okButtonRect = torect(editboxRect.x,
      editboxRect.y + editboxRect.height + vpad,
      editboxRect.width * 0.5f - vpad * 0.5f,
      textSize + 2.f * vpad);
  Rectangle cancelButtonRect = torect(okButtonRect.x + okButtonRect.width + vpad,
      editboxRect.y + editboxRect.height + vpad,
      editboxRect.width * 0.5f - vpad * 0.5f,
      textSize + 2.f * vpad);
  draw_button(okButtonRect, "OK", scaleFactor, notSelectedColor);
  draw_button(cancelButtonRect, "Cancel", scaleFactor, notSelectedColor);
  const Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, okButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ENTER))
  {
    if (!editorLevelName.empty())
      editorState = E_SAVE_PREV_NEW;
    else
    {
      editorLevelName = newFileName;
      newFileName = "";
      editorState = E_WORLD;

      restart_world(reg);
    }
  }
  if (is_vec_in_rect(mp, cancelButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ESCAPE))
  {
    editorState = E_WORLD;
  }
}

template<typename Callable>
static float draw_list_of_files(Rectangle panelRect, float ypos, float scaleFactor, Callable c)
{
  const float vpad = 4 * scaleFactor;
  const float textSize = 12.f * scaleFactor;
  std::filesystem::path levelsPath{"res/levels"};
  Vector2 mp = GetMousePosition();
  for (const auto& dirEntry : std::filesystem::directory_iterator(levelsPath))
  {
    auto path = dirEntry.path();
    Rectangle underpan = torect(panelRect.x + vpad, ypos, panelRect.width - 2 * vpad, textSize + 2 * vpad);
    Color col = notSelectedColor;
    if (is_vec_in_rect(mp, underpan))
    {
      Vector3 hsv = ColorToHSV(col);
      col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
      if (IsMouseButtonReleased(0))
        c(path.filename().string().c_str());
    }
    DrawRectangleRec(underpan, col);
    draw_font_with_shadow(GetFontDefault(), path.filename().string().c_str(), underpan.x + vpad, underpan.y + vpad, textSize, 3, WHITE);
    ypos = underpan.y + underpan.height + vpad;
  }
  return ypos;
}

static void draw_load_select(eecs::Registry& reg, Rectangle panelRect, float scaleFactor)
{
  const float vpad = 4 * scaleFactor;
  const float textSize = 12.f * scaleFactor;
  Rectangle promptRect = torect(panelRect.x, panelRect.y + vpad, panelRect.width, textSize + vpad * 2);
  draw_centered_font_with_shadow(GetFontDefault(), "Select a file to load",  promptRect, textSize, 3, WHITE);

  // go through all files in the directory
  float ypos = promptRect.y + promptRect.height + vpad;
  ypos = draw_list_of_files(panelRect, ypos, scaleFactor, [&](const char* fname)
  {
    load_level(reg, fname);
    editorLevelName = fname;
    editorState = E_WORLD;
  });
  Rectangle okButtonRect = torect(panelRect.x + vpad,
      ypos,
      panelRect.width * 0.5f - vpad * 2,
      textSize + 2.f * vpad);
  //draw_button(okButtonRect, "OK", scaleFactor, notSelectedColor);
  Rectangle cancelButtonRect = torect(okButtonRect.x + okButtonRect.width + vpad * 2,
      ypos,
      panelRect.width * 0.5f - vpad * 2,
      textSize + 2.f * vpad);
  draw_button(cancelButtonRect, "Cancel", scaleFactor, notSelectedColor);
  Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, cancelButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ESCAPE))
    editorState = E_WORLD;
}

static void draw_save_select(eecs::Registry& reg, Rectangle panelRect, float scaleFactor)
{
  const float vpad = 4 * scaleFactor;
  const float textSize = 12.f * scaleFactor;
  Rectangle promptRect = torect(panelRect.x, panelRect.y + vpad, panelRect.width, textSize + vpad * 2);
  draw_centered_font_with_shadow(GetFontDefault(), "Select a file to save to",  promptRect, textSize, 3, WHITE);

  // go through all files in the directory
  float ypos = promptRect.y + promptRect.height + vpad;
  ypos = draw_list_of_files(panelRect, ypos, scaleFactor, [&](const char* fname)
  {
    save_level(reg, fname);
    editorLevelName = fname;
    editorState = E_WORLD;
  });

  Rectangle editboxRect = torect(
      panelRect.x + panelRect.width * 0.2f,
      ypos + vpad + textSize,
      panelRect.width * 0.6f, textSize + 2.f * vpad);
  static std::string newFileName = "";
  Rectangle typePromptRect = torect(editboxRect.x, editboxRect.y - textSize - vpad, editboxRect.width, textSize);
  draw_centered_font_with_shadow(GetFontDefault(), "Type a filename:", typePromptRect, 12.f * scaleFactor, 3, WHITE);
  DrawRectangleRec(editboxRect, Color{50, 50, 50, 200});
  DrawRectangleLinesEx(editboxRect, 1.f, BLACK);

  const Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, editboxRect))
  {
    SetMouseCursor(MOUSE_CURSOR_IBEAM);
    int key = GetCharPressed();

    // Check if more characters have been pressed on the same frame
    while (key > 0)
    {
      // NOTE: Only allow keys in range [32..125]
      if ((key >= 32) && (key <= 125))
        newFileName += (char)key;

      key = GetCharPressed();  // Check next character in the queue
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !newFileName.empty())
      newFileName.resize(newFileName.size() - 1);
  }
  else
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

  draw_font_with_shadow(GetFontDefault(), newFileName.c_str(), editboxRect.x + vpad, editboxRect.y + vpad, textSize, 3, WHITE);

  Rectangle okButtonRect = torect(editboxRect.x,
      editboxRect.y + editboxRect.height + vpad,
      editboxRect.width * 0.5f - vpad * 0.5f,
      textSize + 2.f * vpad);
  Rectangle cancelButtonRect = torect(okButtonRect.x + okButtonRect.width + vpad,
      editboxRect.y + editboxRect.height + vpad,
      editboxRect.width * 0.5f - vpad * 0.5f,
      textSize + 2.f * vpad);
  draw_button(okButtonRect, "OK", scaleFactor, notSelectedColor);
  draw_button(cancelButtonRect, "Cancel", scaleFactor, notSelectedColor);
  if (is_vec_in_rect(mp, okButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ENTER))
  {
    save_level(reg, newFileName.c_str());
    editorLevelName = newFileName;
    newFileName = "";
    editorState = E_WORLD;
  }
  if (is_vec_in_rect(mp, cancelButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ESCAPE))
  {
    editorState = E_WORLD;
  }
}

static void draw_save_prev(eecs::Registry& reg, Rectangle panelRect, float scaleFactor)
{
  const float vpad = 4 * scaleFactor;
  const float textSize = 12.f * scaleFactor;
  const Rectangle promptRect = torect(panelRect.x, panelRect.y + panelRect.height * 0.4f, panelRect.width, panelRect.height * 0.2f);
  draw_centered_font_with_shadow(GetFontDefault(), "Do you want to save your current work?", promptRect, 12.f * scaleFactor, 3, WHITE);

  Rectangle okButtonRect = torect(promptRect.x,
      promptRect.y + promptRect.height + vpad,
      promptRect.width * 0.5f - vpad * 0.5f,
      textSize + 2.f * vpad);
  Rectangle cancelButtonRect = torect(okButtonRect.x + okButtonRect.width + vpad,
      promptRect.y + promptRect.height + vpad,
      promptRect.width * 0.5f - vpad * 0.5f,
      textSize + 2.f * vpad);
  draw_button(okButtonRect, "YES", scaleFactor, notSelectedColor);
  draw_button(cancelButtonRect, "NO", scaleFactor, notSelectedColor);
  const Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, okButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ENTER))
  {
    save_level(reg, editorLevelName.c_str());
    if (editorState == E_SAVE_PREV_NEW)
      restart_world(reg);
    else if (editorState == E_SAVE_PREV_LOAD)
    {
      load_level(reg, editorLevelToLoad.c_str());
      editorLevelToLoad = "";
    }
    editorState = E_WORLD;
  }
  if (is_vec_in_rect(mp, cancelButtonRect) && IsMouseButtonPressed(0) || IsKeyPressed(KEY_ESCAPE))
  {
    editorState = E_WORLD;
  }
}

static void draw_dialogs(eecs::Registry& reg, float width, float height, float scaleFactor)
{
  if (editorState == E_WORLD)
    return;

  if (editorState == E_NEW_FILE)
  {
    Rectangle panelRect = torect(width * 0.2f, height * 0.3f, width * 0.6f, height * 0.4f);
    DrawRectangleRec(panelRect, Color{100, 100, 100, 200});
    draw_new_file(reg, panelRect, scaleFactor);
  }
  else if (editorState == E_SAVE_PREV_NEW || editorState == E_SAVE_PREV_LOAD)
  {
    Rectangle panelRect = torect(width * 0.2f, height * 0.3f, width * 0.6f, height * 0.4f);
    DrawRectangleRec(panelRect, Color{100, 100, 100, 200});
    draw_save_prev(reg, panelRect, scaleFactor);
  }
  else if (editorState == E_LOAD_SELECT)
  {
    Rectangle panelRect = torect(width * 0.2f, height * 0.1f, width * 0.6f, height * 0.8f);
    DrawRectangleRec(panelRect, Color{100, 100, 100, 200});
    draw_load_select(reg, panelRect, scaleFactor);
  }
  else if (editorState == E_SAVE_SELECT)
  {
    Rectangle panelRect = torect(width * 0.2f, height * 0.1f, width * 0.6f, height * 0.8f);
    DrawRectangleRec(panelRect, Color{100, 100, 100, 200});
    draw_save_select(reg, panelRect, scaleFactor);
  }
}

void draw_editor_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
  draw_right_editor_panel(reg, width, height, scaleFactor);
  draw_top_editor_panel(width, height, scaleFactor);
  draw_dialogs(reg, width, height, scaleFactor);
}


static bool is_cursor_over_editor_ui(float width, float height, float scaleFactor)
{
  // TODO: make panels an objects or something!
  if (editorState != E_WORLD)
    return true;
  const Vector2 mp = GetMousePosition();
  {
    const float left = width * 0.7f;
    const float top = 0;
    const Rectangle rightPanRect = torect(left, top, width * 0.3f, height);
    if (is_vec_in_rect(mp, rightPanRect))
      return true;
  }
  {
    const float left = 0;
    const float top = 0;
    const float pheight = height * 0.06f;
    const float hsize = 100 * scaleFactor;
    const float lpad = 4 * scaleFactor;
    const Rectangle leftTopPanRect = torect(left, top, lpad + (hsize + lpad) * 3, pheight);
    if (is_vec_in_rect(mp, leftTopPanRect))
      return true;
  }
  return false;
}


bool is_cursor_over_ui(eecs::Registry& reg)
{
    bool res = false;
    eecs::query_entities(reg, [&](eecs::EntityId, float window_width, float window_height, float window_scaleFactor)
    {
        res |= is_cursor_over_editor_ui(window_width, window_height, window_scaleFactor);
    }, COMPID(const float, window_width), COMPID(const float, window_height), COMPID(const float, window_scaleFactor));
    return res;
}

bool is_ui_blocks_input(eecs::Registry& reg)
{
  if (editorState != E_WORLD || filterSelected)
    return true;
  return false;
}

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
  draw_editor_ui(reg, width, height, scaleFactor);
  DrawFPS(20, height - 40);
}

