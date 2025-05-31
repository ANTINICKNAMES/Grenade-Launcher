#pragma once

#include "GLauncher.h"

#include "CVector.h"
#include "eWeaponType.h"

class CEntity;
class CObject;

class CCustomProjectileInfo
{
public:
	eWeaponType m_eWeaponType;
	CEntity* m_pSource;
	uint32_t m_nExplosionTime;
	bool m_bInUse;
	bool m_bHasHitWallTooClose;
	uint32_t m_nLastTimeSinceCreating;
	CVector m_vecPos;

public:
	//static CCustomProjectileInfo* GetProjectileInfo(int32_t id);
	//static CObject* ms_apCustomProjectile[NUM_PROJECTILES];
	static std::vector<CObject*> ms_apCustomProjectile;

	static void Initialise();
	static bool AddProjectile(CEntity* ped, eWeaponType weapon, CVector pos, float speed, CVector* direction, CEntity* victim);
	static void RemoveProjectile(CCustomProjectileInfo* info, CObject* projectile);
	static void RemoveNotAdd(CEntity* entity, eWeaponType weaponType, CVector pos);
	static void RemoveAllProjectiles();
	static void Update();
	static bool IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool remove);

};

extern std::vector<CCustomProjectileInfo> gaProjectileInfo;
//extern CCustomProjectileInfo gaCustomProjectileInfo[NUM_PROJECTILES];