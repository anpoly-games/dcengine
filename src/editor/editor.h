#pragma once

void register_editor(eecs::Registry& reg);

void save_level(eecs::Registry& reg, const char* filename);

enum EntTypeList
{
  E_FLOORS = 0,
  E_WALLS,
  E_DOORS,
  E_COLUMNS,
  E_CEILINGS,
  E_ENTITIES,
  E_LOGIC,
  E_BILLBOARDS,
  E_ETL_NUM
};

struct SelectedEntity {};

