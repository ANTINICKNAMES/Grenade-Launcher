#pragma once

#include "plugin.h"

//#define GRENADE_CRUTCH
//#define NUM_PROJECTILES 32
using namespace plugin;
extern int32_t NUM_PROJECTILES;
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

inline float Abs(float x) { return fabsf(x); }
inline float MagnitudeSqr(CVector vec) { return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z; }
static CVector Normalized(CVector vec) 
{
	CVector out = vec;
	out.Normalise();
	return out;
}
//struct GLauncher {
//	unsigned int m_nWeapons;
//};
class GLauncher
{
public:
	static void ReadConfig();

	static bool __fastcall CheckForGLauncher(CWeapon* _this, int, CEntity* firingEntity, CVector* posn, CVector* effectPosn, CEntity* targetEntity, CVector* target, CVector* posnForDriveBy, bool a8, bool additionalEffects);
	static bool FireProjectile(CWeapon* _this, CEntity* shooter, CVector* posn, CEntity* targetEntity, CVector* target, float force);

//public:
	//static uint WEAPON_GLAUNCHER1;
	//static int PROJECTILE_MODEL_ID1;
	//static float GLAUNCHER_FORCE1;

	//static uint WEAPON_GLAUNCHER2;
	//static int PROJECTILE_MODEL_ID2;
	//static float GLAUNCHER_FORCE2;
};
class CEventGunShot : public CEventGroup {
public:
	CEventGunShot(CEntity* entity, CVector startPoint, CVector endPoint, bool bHasNoSound);
};
class CEvent;
class CInterestingEvents;
class CEventGlobalGroup;
static CEvent* AddEvent(CEventGlobalGroup* eventg, CEventGunShot* eventt, bool bValid = false)
{
	return CallMethodAndReturn<CEvent*, 0x4AB420, CEventGlobalGroup*, CEventGunShot*, bool>(eventg, eventt, bValid);
}

struct LaunchersData
{
	int32_t WeaponID;
	int32_t ProjectileModelID;
	float Force;
	bool HaveStreak;
};
extern std::unordered_map<int, LaunchersData> g_LaunchersData;