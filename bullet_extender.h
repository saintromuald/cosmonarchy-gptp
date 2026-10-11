#pragma once
#include "BW/structures.h"

extern Bullet new_bullet_table[BulletLimit];

namespace hooks {
	void inject_bullet_extender();
}