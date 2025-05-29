#include "plugin.h"
#include "GLauncher.h"
#include "CCustomProjectileInfo.h"
#include "debugmenu_public.h"
#ifdef DEBUG
#include "CMessages.h"
#endif // DEBUG

using namespace plugin;
DebugMenuAPI gDebugMenuAPI;
CdeclEvent <AddressList<0x564372, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> ClearForRestartEvent;
CdeclEvent <AddressList<0x56A46E, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> ClearExcitingStuffFromAreaEvent;
CdeclEvent <AddressList<0x56E975, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> MakePlayerSafeEvent;
class Grenade_Launcher {
public:
    Grenade_Launcher() {
		patch::RedirectCall(0x742495, GLauncher::CheckForGLauncher);
		if (DebugMenuLoad()) 
		{
			DebugMenuAddInt32("GLauncher", "Projectile array size", &NUM_PROJECTILES, nullptr, 1, 1, 1000, nullptr);
			DebugMenuAddCmd("GLauncher", "Reload the .ini", GLauncher::ReadConfig);
		}
		Events::processScriptsEvent += []() {
			CCustomProjectileInfo::Update();
			static int32_t OldSize = NUM_PROJECTILES;

			if (NUM_PROJECTILES != OldSize) {
			//	gaProjectileInfo.clear();
				gaProjectileInfo.resize(NUM_PROJECTILES);

				//CCustomProjectileInfo::ms_apCustomProjectile.clear();
				CCustomProjectileInfo::ms_apCustomProjectile.resize(NUM_PROJECTILES, nullptr);
				OldSize = NUM_PROJECTILES;
			}
		};

		Events::initGameEvent += []() 
		{
			GLauncher::ReadConfig();
			CCustomProjectileInfo::Initialise();
		};

		Events::reInitGameEvent += []() 
		{
			GLauncher::ReadConfig();
			CCustomProjectileInfo::Initialise();
		};

		ClearForRestartEvent += []() 
		{
			CCustomProjectileInfo::RemoveAllProjectiles();
		};

		ClearExcitingStuffFromAreaEvent += []() 
		{
			CCustomProjectileInfo::RemoveAllProjectiles();
		};

		MakePlayerSafeEvent += []() 
		{
			CCustomProjectileInfo::RemoveAllProjectiles();
		};
	};
} Grenade_LauncherPlugin;
