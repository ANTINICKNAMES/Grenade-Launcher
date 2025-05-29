#include "plugin.h"

#include "CCustomProjectileInfo.h"
#include "GLauncher.h"

#include "CCamera.h"
#include "CGeneral.h"
#include "CHeli.h"
#include "eModelID.h"
#include "CPed.h"
#include "CPlane.h"
#include "CExplosion.h"
#include "CWeapon.h"
#include "CWorld.h"
#include "CTimer.h"
#include "CVector.h"

#include "CProjectile.h"

#ifdef DEBUG
#include "CMessages.h"
#endif // DEBUG


uint32_t projectileInUse;

CCustomProjectileInfo gaProjectileInfo[NUM_PROJECTILES];
CObject* CCustomProjectileInfo::ms_apCustomProjectile[NUM_PROJECTILES];

void
CCustomProjectileInfo::Initialise()
{
	for (int i = 0; i < ARRAY_SIZE(ms_apCustomProjectile); i++) {
		ms_apCustomProjectile[i] = nullptr;
		gaProjectileInfo[i].m_eWeaponType = WEAPON_GRENADE;
		gaProjectileInfo[i].m_pSource = nullptr;
		gaProjectileInfo[i].m_nExplosionTime = 0;
		gaProjectileInfo[i].m_bInUse = false;
	}

	projectileInUse = 0;
}

//CCustomProjectileInfo*
//CCustomProjectileInfo::GetProjectileInfo(int32_t id)
//{
//	return &gaProjectileInfo[id];
//}

bool
CCustomProjectileInfo::AddProjectile(CEntity* entity, eWeaponType weapon, CVector pos, float speed, CVector* direction, CEntity* victim)
{
	int8_t SpecialCollisionResponseCase = 0; //COLLRESPONSE_NONE;
	bool gravity = true;
	CMatrix matrix;
	float elasticity = 0.75f;
	CPed* ped = (CPed*)entity;
	int time = 2000;
	CVector velocity;

	//switch (weapon)
	//{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
	if (weapon == GLauncher::WEAPON_GLAUNCHER1 || weapon == GLauncher::WEAPON_GLAUNCHER2)
	{
		float vy = speed;
		time = CTimer::m_snTimeInMilliseconds + 10000;
		SpecialCollisionResponseCase = 5; // UNKNOWN5
		if (ped->IsPlayer()) {
			int16_t m_nMode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;

			if (m_nMode == MODE_AIMWEAPON || m_nMode == MODE_AIMWEAPON_ATTACHED) {
				matrix.GetForward() = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront;
				matrix.GetUp() = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecUp;
				matrix.GetRight().Cross(TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecUp * -1.f, TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront);
				matrix.GetPosition() = pos;
				if (weapon == GLauncher::WEAPON_GLAUNCHER2)
					velocity.FromMultiply3x3(matrix, CVector(0.05f, vy, 0.2f));
				else
					velocity.FromMultiply3x3(matrix, CVector(0.05f, vy, 0.2f));
			} else {
				matrix = *ped->GetMatrix();
				velocity.FromMultiply3x3(matrix, CVector(0.f, vy, 0.f));
			}
		}
		else {
			matrix = *ped->GetMatrix();
			velocity.FromMultiply3x3(matrix, CVector(0.f, vy, 0.f));
		}
		//break;
	}
	//default:
	//	break;
	//}

	int i = 0;

	while (i < ARRAY_SIZE(gaProjectileInfo) && gaProjectileInfo[i].m_bInUse) i++;

	if (i == ARRAY_SIZE(gaProjectileInfo))
		return false;

	//switch (weapon)
	if (weapon == GLauncher::WEAPON_GLAUNCHER1)
	{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
		ms_apCustomProjectile[i] = new CProjectile(GLauncher::PROJECTILE_MODEL_ID1);
		//break;
	//default: break;
	}

	if (weapon == GLauncher::WEAPON_GLAUNCHER2)
		ms_apCustomProjectile[i] = new CProjectile(GLauncher::PROJECTILE_MODEL_ID2);

	if (ms_apCustomProjectile[i] == nullptr)
		return false;

	gaProjectileInfo[i].m_eWeaponType = weapon;
	gaProjectileInfo[i].m_pSource = ped;
	ms_apCustomProjectile[i]->SetMatrix(matrix);
	ms_apCustomProjectile[i]->m_vecMoveSpeed = velocity;
	ms_apCustomProjectile[i]->m_nPhysicalFlags.bApplyGravity = gravity;

	gaProjectileInfo[i].m_nExplosionTime = time;
	ms_apCustomProjectile[i]->m_fElasticity = elasticity;
	ms_apCustomProjectile[i]->m_pObjectInfo->m_nSpecialColResponseCase = SpecialCollisionResponseCase;
	//ms_apCustomProjectile[i]->RegisterReference(reinterpret_cast<CEntity**>(&ms_apCustomProjectile[i]));
	ms_apCustomProjectile[i]->RegisterReference(&entity);

	projectileInUse++;

	gaProjectileInfo[i].m_bInUse = true;
	CWorld::Add(ms_apCustomProjectile[i]);

	gaProjectileInfo[i].m_vecPos = ms_apCustomProjectile[i]->GetPosition();

	if (entity && entity->m_nType == ENTITY_TYPE_PED && !ped->m_apCollidedEntities) {
		ped->m_apCollidedEntities[0] = ms_apCustomProjectile[i];
	}
	return true;
}

void
CCustomProjectileInfo::RemoveProjectile(CCustomProjectileInfo* info, CObject* projectile)
{
	if (info->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1 || info->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2)
	//switch (info->m_eWeaponType) 
	{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
	
		CExplosion::AddExplosion(nullptr, info->m_pSource, EXPLOSION_GRENADE, projectile->GetPosition(), 0, 1, 1.f, 0);
	//	break;
	}
	projectileInUse--;

	info->m_bInUse = false;
	CWorld::Remove(projectile);
	delete projectile;
}

void
CCustomProjectileInfo::RemoveNotAdd(CEntity* entity, eWeaponType weaponType, CVector pos)
{
	if (weaponType == GLauncher::WEAPON_GLAUNCHER1 || weaponType == GLauncher::WEAPON_GLAUNCHER2)
	//switch (weaponType) 
	{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
		CExplosion::AddExplosion(nullptr, entity, EXPLOSION_GRENADE, pos, 0, 1, 1.f, 0);
	//	break;
	}
}

void
CCustomProjectileInfo::RemoveAllProjectiles()
{
	if (projectileInUse == 0)
		return;

	for (int i = 0; i < ARRAY_SIZE(ms_apCustomProjectile); i++) {
		if (gaProjectileInfo[i].m_bInUse) {
			projectileInUse--;

			gaProjectileInfo[i].m_bInUse = false;
			CWorld::Remove(ms_apCustomProjectile[i]);
			delete ms_apCustomProjectile[i];
		}
	}
}

void
CCustomProjectileInfo::Update()
{
	if (projectileInUse == 0)
		return;

	for (int i = 0; i < ARRAY_SIZE(gaProjectileInfo); i++) {
		if (!gaProjectileInfo[i].m_bInUse) continue;

		CPed* ped = (CPed*)gaProjectileInfo[i].m_pSource;
		if (ped != nullptr && ped->m_nType == ENTITY_TYPE_PED && !ped->IsPointerValid())
			gaProjectileInfo[i].m_pSource = nullptr;

		if (ms_apCustomProjectile[i] == nullptr) {
			projectileInUse--;

			gaProjectileInfo[i].m_bInUse = false;
			continue;
		}
		if (ms_apCustomProjectile[i]->m_fElasticity > 0.1f) {
			if (Abs(ms_apCustomProjectile[i]->m_vecMoveSpeed.x) < 0.05f && Abs(ms_apCustomProjectile[i]->m_vecMoveSpeed.y) < 0.05f && Abs(ms_apCustomProjectile[i]->m_vecMoveSpeed.z) < 0.05f) {
				ms_apCustomProjectile[i]->m_fElasticity = 0.03f;
			}
		}
		const CVector& projectilePos = ms_apCustomProjectile[i]->GetPosition();
		CVector nextPos = CTimer::ms_fTimeStep * ms_apCustomProjectile[i]->m_vecMoveSpeed + projectilePos;

		//FxSystem_c::AddParticle();

		if (CTimer::m_snTimeInMilliseconds <= gaProjectileInfo[i].m_nExplosionTime || gaProjectileInfo[i].m_nExplosionTime == 0) {
			if (gaProjectileInfo[i].m_eWeaponType == WEAPON_ROCKET) {
				CVector pos = ms_apCustomProjectile[i]->GetPosition();
				CWorld::pIgnoreEntity = ms_apCustomProjectile[i];
				if (ms_apCustomProjectile[i]->GetHasCollidedWithAnyObject()
					|| !CWorld::GetIsLineOfSightClear(gaProjectileInfo[i].m_vecPos, pos, true, true, true, true, false, false, false)) {
					RemoveProjectile(&gaProjectileInfo[i], ms_apCustomProjectile[i]);
				}
				CWorld::pIgnoreEntity = nullptr;
				ms_apCustomProjectile[i]->m_vecMoveSpeed *= 1.07f;

			}
			else if (gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1 
				|| gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2) { // WEAPON_GLAUNCHER
				CVector pos = ms_apCustomProjectile[i]->GetPosition();
				CWorld::pIgnoreEntity = ms_apCustomProjectile[i];

				if (gaProjectileInfo[i].m_pSource == nullptr ||
					((gaProjectileInfo[i].m_vecPos - gaProjectileInfo[i].m_pSource->GetPosition()).MagnitudeSqr() >= 2.0f)) {
					if (CTimer::m_snTimeInMilliseconds >= gaProjectileInfo[i].m_nExplosionTime || ms_apCustomProjectile[i]->m_bHasContacted ||
						!CWorld::GetIsLineOfSightClear(gaProjectileInfo[i].m_vecPos, pos, true, true, true, true, false, false, false)) {
						RemoveProjectile(&gaProjectileInfo[i], ms_apCustomProjectile[i]);
					}
				}
				CWorld::pIgnoreEntity = nullptr;
			}
		}

		gaProjectileInfo[i].m_vecPos = ms_apCustomProjectile[i]->GetPosition();
	}
}

bool
CCustomProjectileInfo::IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool remove)
{
	bool result = false;
	for (int i = 0; i < ARRAY_SIZE(ms_apCustomProjectile); i++) {
		if (gaProjectileInfo[i].m_bInUse) {
			if (gaProjectileInfo[i].m_eWeaponType == WEAPON_ROCKET 
				|| gaProjectileInfo[i].m_eWeaponType == WEAPON_MOLOTOV 
				|| gaProjectileInfo[i].m_eWeaponType == WEAPON_GRENADE
				|| gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1
				|| gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2) { // WEAPON_GLAUNCHER
				const CVector& pos = ms_apCustomProjectile[i]->GetPosition();
				if (pos.x >= x1 && pos.x <= x2 && pos.y >= y1 && pos.y <= y2 && pos.z >= z1 && pos.z <= z2) {
					result = true;
					if (remove) {
						projectileInUse--;
						gaProjectileInfo[i].m_bInUse = false;
						CWorld::Remove(ms_apCustomProjectile[i]);
						delete ms_apCustomProjectile[i];
					}
				}
			}
		}
	}
	return result;
}