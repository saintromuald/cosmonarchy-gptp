#pragma once

namespace bw { struct Dialog; }

// Status panel weapon, armor, shield and special icons (dialog indices 9-12)
void show_stat_icon_tooltip(bw::Dialog* dialog);

// Range circle for the weapon whose tooltip is showing - call once per game tick
void draw_hovered_weapon_range();
