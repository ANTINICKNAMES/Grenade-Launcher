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
#include "CWeather.h"
#ifdef DEBUG
#include "CMessages.h"
#endif // DEBUG


uint32_t projectileInUse;
bool AddExplosion(CEntity* victim, CEntity* creator, eExplosionType explosionType, CVector& posn, unsigned int time, unsigned char makeSound, float camShake, unsigned char visibility) {
	return plugin::CallAndReturn<bool, 0x736A50, CEntity*, CEntity*, eExplosionType, CVector, unsigned int, unsigned char, float, unsigned char>(victim, creator, explosionType, posn, time, makeSound, camShake, visibility);
}
void RegStreak(uint32_t id, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha, CVector leftPoint, CVector rightPoint) {
	plugin::Call<0x721DC0, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t, CVector, CVector>(id, red, green, blue, alpha, leftPoint, rightPoint);
}
//CCustomProjectileInfo gaProjectileInfo[NUM_PROJECTILES];
//CObject* CCustomProjectileInfo::ms_apCustomProjectile[NUM_PROJECTILES];
std::vector<CCustomProjectileInfo> gaProjectileInfo;
std::vector<CObject*> CCustomProjectileInfo::ms_apCustomProjectile;
void
CCustomProjectileInfo::Initialise()
{
	//gaProjectileInfo.clear();
	gaProjectileInfo.resize(NUM_PROJECTILES);

	//ms_apCustomProjectile.clear();
	ms_apCustomProjectile.resize(NUM_PROJECTILES, nullptr);

	for (size_t i = 0; i < gaProjectileInfo.size(); i++) {
		gaProjectileInfo[i].m_eWeaponType = WEAPONTYPE_GRENADE;
		gaProjectileInfo[i].m_pSource = nullptr;
		gaProjectileInfo[i].m_nExplosionTime = 0;
		gaProjectileInfo[i].m_bInUse = false;
		gaProjectileInfo[i].m_bHasHitWallTooClose = false;
		gaProjectileInfo[i].m_nLastTimeSinceCreating = 0;
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
	auto& data = g_LaunchersData[weapon];
	//switch (weapon)
	//{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
	if (/*weapon == GLauncher::WEAPON_GLAUNCHER1 || weapon == GLauncher::WEAPON_GLAUNCHER2*/ weapon == data.WeaponID)
	{
		float vy = speed;
		time = CTimer::m_snTimeInMilliseconds + 10000;
		SpecialCollisionResponseCase = 5; // UNKNOWN5
		if (ped->IsPlayer()) {
			const auto& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
			int16_t mode = cam.m_nMode;

			if (mode == MODE_AIMWEAPON || mode == MODE_AIMWEAPON_ATTACHED) {
				matrix.GetForward() = cam.m_vecFront;
				matrix.GetUp() = cam.m_vecUp;
				matrix.GetRight().Cross(cam.m_vecUp * -1.f, cam.m_vecFront);
				matrix.GetPosition() = pos + cam.m_vecFront * 0.3f;

				velocity.FromMultiply3x3(matrix, CVector(0.05f, vy, 0.2f));
			}
			else {
				matrix = *ped->GetMatrix();
				// Fix the projectile not getting stuck in the wall if not aiming
				if (data.ProjectilesStuckInTheSurfaces) {
					CVector forward = matrix.GetForward();
					matrix.GetPosition() = pos + forward * 0.3f;
				}

				velocity.FromMultiply3x3(matrix, CVector(0.f, vy, 0.f));
			}
		}
		else {
			matrix = *ped->GetMatrix();

			if (data.ProjectilesStuckInTheSurfaces) {
				CVector forward = matrix.GetForward();
				matrix.GetPosition() = pos + forward * 0.3f;
			}

			velocity.FromMultiply3x3(matrix, CVector(0.f, vy, 0.f));
		}
		//break;
	}
	//default:
	//	break;
	//}

	size_t i = 0;

	while (i < gaProjectileInfo.size() && gaProjectileInfo[i].m_bInUse) i++;

	if (i == gaProjectileInfo.size())
		return false;
	//switch (weapon)
	if (weapon == data.WeaponID)
	{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
		ms_apCustomProjectile[i] = new CProjectile(data.ProjectileModelID);
		gaProjectileInfo[i].m_nLastTimeSinceCreating = CTimer::m_snTimeInMilliseconds;
		//break;
	//default: break;
	}

	//if (weapon == GLauncher::WEAPON_GLAUNCHER2)
		//ms_apCustomProjectile[i] = new CProjectile(GLauncher::PROJECTILE_MODEL_ID2);

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
	if (data.ProjectilesStuckInTheSurfaces) {
		CVector start = ms_apCustomProjectile[i]->GetPosition();
		CVector dir = Normalized(ms_apCustomProjectile[i]->m_vecMoveSpeed);
		CVector end = start + dir * 5.0f;

		CColPoint colPoint;
		CEntity* hitEntity = nullptr;

		CWorld::pIgnoreEntity = ms_apCustomProjectile[i];
		bool hit = CWorld::ProcessLineOfSight(start, end, colPoint, hitEntity, true, true, true, true, false, false, false, false);
		CWorld::pIgnoreEntity = nullptr;

		if (hit && (hitEntity && hitEntity->m_nType != ENTITY_TYPE_PED && hitEntity->m_nType != ENTITY_TYPE_VEHICLE)) {
			//float distToWall = (ped->GetPosition() - start).Magnitude();
			//const float maxDistanceForCloseShot = 1.0f;

			if (MagnitudeSqr(gaProjectileInfo[i].m_vecPos - pos) <= 1.0f) {
				ms_apCustomProjectile[i]->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
				ms_apCustomProjectile[i]->m_nPhysicalFlags.bApplyGravity = false;
				ms_apCustomProjectile[i]->m_fElasticity = 0.0f;
				ms_apCustomProjectile[i]->SetPosn(colPoint.m_vecPoint + colPoint.m_vecNormal * 0.01f);
				gaProjectileInfo[i].m_bHasHitWallTooClose = true;
			}
			else {
				gaProjectileInfo[i].m_bHasHitWallTooClose = false;
			}
		}
	}
	//else {
	//	gaProjectileInfo[i].m_bHasHitWallTooClose = false;
	//}
	return true;
}

void
CCustomProjectileInfo::RemoveProjectile(CCustomProjectileInfo* info, CObject* projectile)
{
	auto& data = g_LaunchersData[info->m_eWeaponType];
	if (/*info->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1 || info->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2*/ info->m_eWeaponType == data.WeaponID)
	//switch (info->m_eWeaponType) 
	{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
	
		AddExplosion(nullptr, info->m_pSource, EXPLOSION_GRENADE, projectile->GetPosition(), 0, 1, 1.f, 0);
	//	break;
	}
	projectileInUse--;

	info->m_bInUse = false;
	info->m_bHasHitWallTooClose = false;
	CWorld::Remove(projectile);
	delete projectile;
}

void
CCustomProjectileInfo::RemoveNotAdd(CEntity* entity, eWeaponType weaponType, CVector pos)
{
	auto& data = g_LaunchersData[weaponType];
	if (weaponType == data.WeaponID/*weaponType == GLauncher::WEAPON_GLAUNCHER1 || weaponType == GLauncher::WEAPON_GLAUNCHER2*/)
	//switch (weaponType) 
	{
	//case GLauncher::WEAPON_GLAUNCHER: //WEAPON_GLAUNCHER
		AddExplosion(nullptr, entity, EXPLOSION_GRENADE, pos, 0, 1, 1.f, 0);
	//	break;
	}
}

void
CCustomProjectileInfo::RemoveAllProjectiles()
{
	if (projectileInUse == 0)
		return;

	for (size_t i = 0; i < ms_apCustomProjectile.size(); i++) {
		if (gaProjectileInfo[i].m_bInUse) {
			projectileInUse--;

			gaProjectileInfo[i].m_bInUse = false;
			gaProjectileInfo[i].m_bHasHitWallTooClose = false;
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

	for (size_t i = 0; i < gaProjectileInfo.size(); i++) {
		if (gaProjectileInfo[i].m_pSource) {
			bool FarAwayEnough = !((gaProjectileInfo[i].m_vecPos - TheCamera.GetPosition()).MagnitudeSqr2D() < sq(300.0f));
			static uint Time = 0;
			bool SomeTimePassed = CTimer::m_snTimeInMilliseconds > gaProjectileInfo[i].m_nLastTimeSinceCreating + 5000;
			if (gaProjectileInfo[i].m_bHasHitWallTooClose && (FarAwayEnough || SomeTimePassed))
			{
				Time = CTimer::m_snTimeInMilliseconds;
				projectileInUse--;

				gaProjectileInfo[i].m_bInUse = false;
				gaProjectileInfo[i].m_bHasHitWallTooClose = false;
				CWorld::Remove(ms_apCustomProjectile[i]);
				delete ms_apCustomProjectile[i];
			}
		}
		if (!gaProjectileInfo[i].m_bInUse || gaProjectileInfo[i].m_bHasHitWallTooClose) continue;

		CPed* ped = (CPed*)gaProjectileInfo[i].m_pSource;
		if (ped != nullptr && ped->m_nType == ENTITY_TYPE_PED && !ped->IsPointerValid())
			gaProjectileInfo[i].m_pSource = nullptr;

		if (ms_apCustomProjectile[i] == nullptr) {
			projectileInUse--;

			gaProjectileInfo[i].m_bHasHitWallTooClose = false;

			gaProjectileInfo[i].m_bInUse = false;
			continue;
		}
		if (ms_apCustomProjectile[i]->m_fElasticity > 0.1f) {
			if (Abs(ms_apCustomProjectile[i]->m_vecMoveSpeed.x) < 0.05f && Abs(ms_apCustomProjectile[i]->m_vecMoveSpeed.y) < 0.05f && Abs(ms_apCustomProjectile[i]->m_vecMoveSpeed.z) < 0.05f) {
				ms_apCustomProjectile[i]->m_fElasticity = 0.03f;
			}
		}

		// Have the right model heading, instead of always having initial one
		CVector moveSpeed = ms_apCustomProjectile[i]->m_vecMoveSpeed;
		if (MagnitudeSqr(moveSpeed) > 0.0001f) {
			CVector forward = moveSpeed;
			forward.Normalise();

			CVector up(0.0f, 0.0f, 1.0f);
			CVector right;
			right.Cross(up, forward);
			right.Normalise();

			up.Cross(forward, right);
			up.Normalise();

			CMatrix* matrix = ms_apCustomProjectile[i]->GetMatrix();
			matrix->GetRight() = right;
			matrix->GetForward() = forward;
			matrix->GetUp() = up;
			matrix->GetPosition() = ms_apCustomProjectile[i]->GetPosition();

			ms_apCustomProjectile[i]->UpdateRwFrame();
		}

		const CVector& projectilePos = ms_apCustomProjectile[i]->GetPosition();
		CVector nextPos = CTimer::ms_fTimeStep * ms_apCustomProjectile[i]->m_vecMoveSpeed + projectilePos;

		//FxSystem_c::AddParticle();
		auto& data = g_LaunchersData[gaProjectileInfo[i].m_eWeaponType];
		auto const& vecPos = ms_apCustomProjectile[i]->GetPosition();
		auto vecScaledCam = TheCamera.m_mCameraMatrix.GetRight() * 0.07F;
		auto vecStreakStart = vecPos - vecScaledCam;
		auto vecStreakEnd = vecPos + vecScaledCam;
		if (data.HaveStreak && CVector2D(ms_apCustomProjectile[i]->m_vecMoveSpeed).Magnitude() > 0.03F) {
			RegStreak(reinterpret_cast<uint32_t>(ms_apCustomProjectile[i]), 100, 100, 100, 255, vecStreakStart, vecStreakEnd);
		}

		//if (!gaProjectileInfo[i].m_bHasHitWallTooClose) {
			if (CTimer::m_snTimeInMilliseconds <= gaProjectileInfo[i].m_nExplosionTime || gaProjectileInfo[i].m_nExplosionTime == 0) {
				if (gaProjectileInfo[i].m_eWeaponType == WEAPONTYPE_ROCKET) {
					CVector pos = ms_apCustomProjectile[i]->GetPosition();
					CWorld::pIgnoreEntity = ms_apCustomProjectile[i];
					if (ms_apCustomProjectile[i]->GetHasCollidedWithAnyObject()
						|| !CWorld::GetIsLineOfSightClear(gaProjectileInfo[i].m_vecPos, pos, true, true, true, true, false, false, false)) {
						RemoveProjectile(&gaProjectileInfo[i], ms_apCustomProjectile[i]);
					}
					CWorld::pIgnoreEntity = nullptr;
					ms_apCustomProjectile[i]->m_vecMoveSpeed *= 1.07f;
				}
				else if (gaProjectileInfo[i].m_eWeaponType == data.WeaponID /*gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1
					|| gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2*/) { // WEAPON_GLAUNCHER
					CVector pos = ms_apCustomProjectile[i]->GetPosition();
					CWorld::pIgnoreEntity = ms_apCustomProjectile[i];

					if (gaProjectileInfo[i].m_pSource == nullptr ||
						(MagnitudeSqr(gaProjectileInfo[i].m_vecPos - gaProjectileInfo[i].m_pSource->GetPosition()) >= 2.0f)) {
						if (CTimer::m_snTimeInMilliseconds >= gaProjectileInfo[i].m_nExplosionTime || ms_apCustomProjectile[i]->m_bHasContacted ||
							!CWorld::GetIsLineOfSightClear(gaProjectileInfo[i].m_vecPos, pos, true, true, true, true, false, false, false)) {
							RemoveProjectile(&gaProjectileInfo[i], ms_apCustomProjectile[i]);
						}
					}
					CWorld::pIgnoreEntity = nullptr;
				}
			}
		//}

		gaProjectileInfo[i].m_vecPos = ms_apCustomProjectile[i]->GetPosition();
	}
}

bool
CCustomProjectileInfo::IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool remove)
{
	bool result = false;
	for (size_t i = 0; i < ms_apCustomProjectile.size(); i++) {
		if (gaProjectileInfo[i].m_bInUse) {
			auto& data = g_LaunchersData[gaProjectileInfo[i].m_eWeaponType];
			if (gaProjectileInfo[i].m_eWeaponType == WEAPONTYPE_ROCKET 
				|| gaProjectileInfo[i].m_eWeaponType == WEAPONTYPE_MOLOTOV 
				|| gaProjectileInfo[i].m_eWeaponType == WEAPONTYPE_GRENADE 
				|| gaProjectileInfo[i].m_eWeaponType == data.WeaponID
				/*|| gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1
				|| gaProjectileInfo[i].m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2*/) { // WEAPON_GLAUNCHER
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