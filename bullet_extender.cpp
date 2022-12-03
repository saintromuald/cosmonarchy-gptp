#include "bullet_extender.h"
#include "hook_tools.h"
#include "scbw/structures/Bullet.h"

Bullet new_bullet_table[BulletLimit];

void __declspec(naked) reorderBulletArray_patch_8A771() {
	static const u32 jump_48A784 = 0x0048A784;
	static const u32 jump_48A85A = 0x0048A85A;
	__asm {
		CMP     EAX, BulletLimit
		JNE     offset exit_48A85A //can't make TwoByte JNE
		JMP     jump_48A784
	exit_48A85A:
		JMP     jump_48A85A
	}
};

void __declspec(naked) reorderBulletArray_patch_8A78F() {
	static const u32 jump_48A799 = 0x0048A799;
	__asm {
		INC     EAX
		CMP     EAX, BulletLimit
		MOV     ds:[0x0064EED8], ECX
		JMP     jump_48A799
	}
};

void __declspec(naked) reorderBulletArray_patch_8A831() {
	static const u32 jump_48A836 = 0x0048A836;
	static const u32 jump_48A84A = 0x0048A84A;
	__asm {
		CMP     EAX, BulletLimit
		JNE     offset exit_48A84A // can't make TwoByte JNE
		JMP     jump_48A836
	exit_48A84A:
		JMP     jump_48A84A
	}
};

void __declspec(naked) reorderBulletArray_patch_8A855() {
	static const u32 jump_48A85A = 0x0048A85A;
	__asm {
		PUSH    ESI
		INC     EAX
		CMP     EAX, BulletLimit
		JMP     jump_48A85A
	}
};

namespace hooks {
	void inject_bullet_extender() {
		// 0048C260 create_bullet
		nops(0x0048C26E, 14); // Remove valkyrie bullet count check

		// 0048A690 initialize_bullet_array
		memory_patch(0x0048A691, &new_bullet_table);
		memory_patch(0x0048A69B, (u32)BulletLimit * sizeof(Bullet));
		memory_patch(0x0048A6BD, new_bullet_table + 1); // new_bullet_table[1]
		memory_patch(0x0048A6C2, (u32)BulletLimit - 1);
		memory_patch(0x0048A6C7, &new_bullet_table);
		memory_patch(0x0048A6E1, (char*)new_bullet_table + 0x4); // new_bullet_table[0].next
		memory_patch(0x0048A6EA, (char*)new_bullet_table + 0x4); // new_bullet_table[0].next
		memory_patch(0x0048A6F5, (char*)new_bullet_table + 0x4); // new_bullet_table[0].next

		// 0048AB00 read_bullet_data
		memory_patch(0x0048AB94, &new_bullet_table);
		memory_patch(0x0048AC64, &new_bullet_table);

		// 0048AEB0 write_bullets
		memory_patch(0x0048AEB9, &new_bullet_table);
		memory_patch(0x0048AEBE, (u32)BulletLimit);
		memory_patch(0x0048AED7, (char*)new_bullet_table + 0x12); // new_bullet_table[0].sprite
		memory_patch(0x0048AEDC, (u32)BulletLimit);
		memory_patch(0x0048AF48, &new_bullet_table);
		memory_patch(0x0048AFA4, &new_bullet_table);
		memory_patch(0x0048AFA9, (u32)BulletLimit);

		// 0048A9A0 pack_bullet_data
		memory_patch(0x0048AA39, &new_bullet_table);
		memory_patch(0x0048AA62, &new_bullet_table);

		// 0048AE40 read_bullets_array
		memory_patch(0x0048AE80, &new_bullet_table);
		memory_patch(0x0048AE85, (u32)BulletLimit);
		memory_patch(0x0048AE47, (u32)sizeof(new_bullet_table));
		memory_patch(0x0048AE4C, &new_bullet_table);

		// 0048A720 reorder_bullet_array
		jump_patch(reorderBulletArray_patch_8A771, 0x0048A771);
		jump_patch(reorderBulletArray_patch_8A78F, 0x0048A78F, 5);
		jump_patch(reorderBulletArray_patch_8A831, 0x0048A831);
		jump_patch(reorderBulletArray_patch_8A855, 0x0048A855);
		memory_patch(0x0048A78B, &new_bullet_table);
		memory_patch(0x0048A7A5, &new_bullet_table);
		memory_patch(0x0048A851, &new_bullet_table);
		memory_patch(0x0048A86F, (char*)new_bullet_table + 0x4); // new_bullet_table[0].next
		memory_patch(0x0048A87F, &new_bullet_table);
	}
}