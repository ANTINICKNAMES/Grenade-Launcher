#include "plugin.h"
#include "GLauncher.h"
#include "CObject.h"
#ifndef GRENADE_CRUTCH
#include "CCustomProjectileInfo.h"
#else
#include "CProjectileInfo.h"
#endif // !GRENADE_CRUTCH

#include "CCamera.h"
#include "CStats.h"
#include "CWorld.h"

#ifdef DEBUG
#include "CMessages.h"
#endif // DEBUG

using namespace plugin;
//#include "ini.h"
#include <unordered_map>
#include "IniReader.h"
int32_t NUM_PROJECTILES = 32;
//uint GLauncher::WEAPON_GLAUNCHER1;
//int GLauncher::PROJECTILE_MODEL_ID1;
//float GLauncher::GLAUNCHER_FORCE1;

//uint GLauncher::WEAPON_GLAUNCHER2;
//int GLauncher::PROJECTILE_MODEL_ID2;
//float GLauncher::GLAUNCHER_FORCE2;
#include "CEventGroup.h"
// 0x4ABA50
CEventGlobalGroup* GetEventGlobalGroup() 
{
	return CallAndReturn<CEventGlobalGroup*, 0x4ABA50>();
}
CEventGunShot::CEventGunShot(CEntity* entity, CVector startPoint, CVector endPoint, bool bHasNoSound) 
{
	CallMethod<0x4AC610, CEventGunShot*, CEntity*, CVector, CVector, bool>(this, entity, startPoint, endPoint, bHasNoSound);
}
CInterestingEvents& g_InterestingEvents = *(CInterestingEvents*)0xC0B058;
void Add(int type, CEntity* entity) 
{
	plugin::CallMethod<0x602590, CInterestingEvents*, int, CEntity*>(&g_InterestingEvents, type, entity);
}
std::unordered_map<int, LaunchersData> g_LaunchersData;
void GLauncher::ReadConfig() 
{
	CIniReader ini(PLUGIN_PATH((char*)"Grenade_Launcher.ini"));
	NUM_PROJECTILES = ini.ReadInteger("MAIN", "NUM_PROJECTILES", 32);
	for (int i = 1; i < 512; ++i) {
		std::string section = std::format("GRENADE_LAUNCHER{}", i);
		int weaponId1 = ini.ReadInteger(section, "WEAPON_ID", -1);
		if (weaponId1 != -1) {
			int modelId1 = ini.ReadInteger(section, "PROJECTILE_MODEL_ID", 0);
			float force1 = ini.ReadFloat(section, "GLAUNCHER_FORCE", 0.0f);
			bool streak = ini.ReadBoolean(section, "GLAUNCHER_PROJECTILE_STREAK", true);
			bool stucking = ini.ReadBoolean(section, "GLAUNCHER_PROJECTILE_STUCKING", false);
			g_LaunchersData[weaponId1] = LaunchersData{ weaponId1, modelId1, force1, streak, stucking };
		}
	}
}

bool __fastcall GLauncher::CheckForGLauncher(CWeapon* _this, int, CEntity* firingEntity, CVector* posn, CVector* effectPosn, CEntity* targetEntity, CVector* target, CVector* posnForDriveBy, bool a8, bool additionalEffects) {

#ifndef GRENADE_CRUTCH
	float power;
#else
	float power = (TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront.z + 1.f) * 0.75f;
#endif // !GRENADE_CRUTCH
	auto& data = g_LaunchersData[_this->m_eWeaponType];
	CPed* shooter = (CPed*)firingEntity;
	if (_this->m_eWeaponType /*!= GLauncher::WEAPON_GLAUNCHER1 && _this->m_eWeaponType != GLauncher::WEAPON_GLAUNCHER2*/ != data.WeaponID) //WEAPON_GLAUNCHER
		return _this->FireInstantHit(firingEntity, posn, effectPosn, targetEntity, target, posnForDriveBy, a8, additionalEffects);
	else {
		power = data.Force;
		/*if (_this->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1)
			power = GLAUNCHER_FORCE1;
		else if (_this->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2)
			power = GLAUNCHER_FORCE2;
		else power = 1.f;*/

		if (shooter == FindPlayerPed()) {
			// lil bit hacky, cuz when stat is equal 0 it will be 1 anyway... Should I do smt about that?
			//CStats::DecrementStat(0x7E, 1.f);
			CStats::IncrementStat(0x7F, 1.f);
			return GLauncher::FireProjectile(_this, firingEntity, posn, targetEntity, target, power);
		}
		return false;
	}
}

bool GLauncher::FireProjectile(CWeapon* _this, CEntity* shooter, CVector* posn, CEntity* targetEntity, CVector* target, float force) {

	CVector source, targetVector;

#ifndef GRENADE_CRUTCH
	eWeaponType projectileType = _this->m_eWeaponType;
#else
	eWeaponType projectileType = WEAPONTYPE_GRENADE;
#endif // !GRENADE_CRUTCH


	source = *posn;

	if (shooter->m_nType == ENTITY_TYPE_PED && ((CPed*)shooter)->IsPlayer()) {
		*posn += TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront;
	}
	else
		*posn += shooter->GetForward();

	targetVector = *posn;

	if (!CWorld::GetIsLineOfSightClear(source, targetVector, true, true, false, true, false, false, false))
	{
		if (_this->m_eWeaponType != WEAPONTYPE_GRENADE)

#ifndef GRENADE_CRUTCH
			CCustomProjectileInfo::RemoveNotAdd(shooter, projectileType, *posn);
#else
			CProjectileInfo::RemoveNotAdd(shooter, projectileType, *posn);
#endif // !GRENADE_CRUTCH

		else
		{
			if (shooter->m_nType == ENTITY_TYPE_PED)
			{
				source = shooter->GetPosition() - shooter->GetForward();
				source.z -= 0.4f;

				if (!CWorld::TestSphereAgainstWorld(source, 0.5f, nullptr, false, false, true, false, false, false))

#ifndef GRENADE_CRUTCH
					CCustomProjectileInfo::AddProjectile(shooter, WEAPONTYPE_GRENADE, source, 0.0f, nullptr, targetEntity);
#else
					CProjectileInfo::AddProjectile(shooter, WEAPONTYPE_GRENADE, source, 0.0f, nullptr, targetEntity);

#endif // !GRENADE_CRUTCH
				else

#ifndef GRENADE_CRUTCH
					CCustomProjectileInfo::RemoveNotAdd(shooter, WEAPONTYPE_GRENADE, *posn);
#else
					CProjectileInfo::RemoveNotAdd(shooter, WEAPONTYPE_GRENADE, *posn);
#endif // !GRENADE_CRUTCH
			}
		}
	}
	else

#ifndef GRENADE_CRUTCH
		CCustomProjectileInfo::AddProjectile(shooter, projectileType, *posn, force, nullptr, targetEntity);
#else
		CProjectileInfo::AddProjectile(shooter, projectileType, *posn, force, nullptr, targetEntity);
#endif // !GRENADE_CRUTCH


	CWorld::pIgnoreEntity = nullptr;

	// CCrime::ReportCrime(CRIME_EXPLOSION, shooter, shooter);
	Call<0x532010, int, CPed*, CPed*>(17, (CPed*)shooter, (CPed*)shooter);

	// Interesting Events (Really fucking interesting)
	Add(22, shooter);

	// Flee
	CEventGlobalGroup* gr = GetEventGlobalGroup();
	CEventGunShot gs = CEventGunShot{ shooter, *posn, *posn, false };
	AddEvent(gr, &gs, false);
	return true;
}