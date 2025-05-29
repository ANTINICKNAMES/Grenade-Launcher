#pragma once

#include "plugin.h"

//#define GRENADE_CRUTCH
#define NUM_PROJECTILES 32
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

inline float Abs(float x) { return fabsf(x); }

//struct GLauncher {
//	unsigned int m_nWeapons;
//};

class GLauncher
{
public:
	static void ReadConfig();

	static bool __fastcall CheckForGLauncher(CWeapon* _this, int, CEntity* firingEntity, CVector* posn, CVector* effectPosn, CEntity* targetEntity, CVector* target, CVector* posnForDriveBy, bool a8, bool additionalEffects);
	static bool FireProjectile(CWeapon* _this, CEntity* shooter, CVector* posn, CEntity* targetEntity, CVector* target, float force);

public:
	static uint WEAPON_GLAUNCHER1;
	static int PROJECTILE_MODEL_ID1;
	static float GLAUNCHER_FORCE1;

	static uint WEAPON_GLAUNCHER2;
	static int PROJECTILE_MODEL_ID2;
	static float GLAUNCHER_FORCE2;
};