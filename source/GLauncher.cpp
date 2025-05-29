#include "GLauncher.h"

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

#include "ini.h"

uint GLauncher::WEAPON_GLAUNCHER1;
int GLauncher::PROJECTILE_MODEL_ID1;
float GLauncher::GLAUNCHER_FORCE1;

uint GLauncher::WEAPON_GLAUNCHER2;
int GLauncher::PROJECTILE_MODEL_ID2;
float GLauncher::GLAUNCHER_FORCE2;

void GLauncher::ReadConfig() {

	mINI::INIFile file(PLUGIN_PATH((char*)"Grenade_Launcher.ini"));
	mINI::INIStructure ini;
	file.read(ini);

	std::string name = "Grenade_Launcher";
	//std::string formatted_str = std::format("{}{}", name, i);

	//const char* formatted_str2 = formatted_str.c_str();
	const char* formatted_str2 = name.c_str();

	std::string str1 = ini.get(formatted_str2).get("WEAPON_ID1");
	const char* strb1_1 = str1.c_str();

	std::string str2 = ini.get(formatted_str2).get("PROJECTILE_MODEL_ID1");
	const char* strb2_1 = str2.c_str();

	std::string str3 = ini.get(formatted_str2).get("GLAUNCHER_FORCE1");
	const char* strb3_1 = str3.c_str();

	std::string str4 = ini.get(formatted_str2).get("WEAPON_ID2");
	const char* strb4_1 = str4.c_str();

	std::string str5 = ini.get(formatted_str2).get("PROJECTILE_MODEL_ID2");
	const char* strb5_1 = str5.c_str();

	std::string str6 = ini.get(formatted_str2).get("GLAUNCHER_FORCE2");
	const char* strb6_1 = str6.c_str();
		
	//type[i] = std::atoi(strb1_1);
	GLauncher::WEAPON_GLAUNCHER1 = std::atoi(strb1_1);
	GLauncher::PROJECTILE_MODEL_ID1 = std::atoi(strb2_1);
	GLauncher::GLAUNCHER_FORCE1 = std::atof(strb3_1);

	GLauncher::WEAPON_GLAUNCHER2 = std::atoi(strb4_1);
	GLauncher::PROJECTILE_MODEL_ID2 = std::atoi(strb5_1);
	GLauncher::GLAUNCHER_FORCE2 = std::atof(strb6_1);
}

bool __fastcall GLauncher::CheckForGLauncher(CWeapon* _this, int, CEntity* firingEntity, CVector* posn, CVector* effectPosn, CEntity* targetEntity, CVector* target, CVector* posnForDriveBy, bool a8, bool additionalEffects) {

#ifndef GRENADE_CRUTCH
	float power;
#else
	float power = (TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront.z + 1.f) * 0.75f;
#endif // !GRENADE_CRUTCH

	if (_this->m_eWeaponType != GLauncher::WEAPON_GLAUNCHER1 && _this->m_eWeaponType != GLauncher::WEAPON_GLAUNCHER2) //WEAPON_GLAUNCHER
		return _this->FireInstantHit(firingEntity, posn, effectPosn, targetEntity, target, posnForDriveBy, a8, additionalEffects);
	else {
		CPed* shooter = (CPed*)firingEntity;

		if (_this->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER1)
			power = GLAUNCHER_FORCE1;
		else if (_this->m_eWeaponType == GLauncher::WEAPON_GLAUNCHER2)
			power = GLAUNCHER_FORCE2;
		else power = 1.f;

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
	eWeaponType projectileType = WEAPON_GRENADE;
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
		if (_this->m_eWeaponType != WEAPON_GRENADE)

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
					CCustomProjectileInfo::AddProjectile(shooter, WEAPON_GRENADE, source, 0.0f, nullptr, targetEntity);
#else
					CProjectileInfo::AddProjectile(shooter, WEAPON_GRENADE, source, 0.0f, nullptr, targetEntity);

#endif // !GRENADE_CRUTCH
				else

#ifndef GRENADE_CRUTCH
					CCustomProjectileInfo::RemoveNotAdd(shooter, WEAPON_GRENADE, *posn);
#else
					CProjectileInfo::RemoveNotAdd(shooter, WEAPON_GRENADE, *posn);
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
	plugin::Call<0x532010, int, CPed*, CPed*>(17, (CPed*)shooter, (CPed*)shooter);

	// Interesting Events
	//plugin::CallMethod<0x602590, CInterestingEvents*, int, CEntity*>(this, 22, shooter);

	return true;
}