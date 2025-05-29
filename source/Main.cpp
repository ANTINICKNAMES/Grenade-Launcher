#include "plugin.h"
#include "GLauncher.h"
#include "CCustomProjectileInfo.h"

#ifdef DEBUG
#include "CMessages.h"
#endif // DEBUG

using namespace plugin;

static CdeclEvent <AddressList<0x564372, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> ClearForRestartEvent;
static CdeclEvent <AddressList<0x56A46E, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> ClearExcitingStuffFromAreaEvent;
static CdeclEvent <AddressList<0x56E975, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> MakePlayerSafeEvent;

class Grenade_Launcher {
public:
    Grenade_Launcher() {
		patch::RedirectCall(0x742495, GLauncher::CheckForGLauncher);

		plugin::Events::gameProcessEvent += []() {
			CCustomProjectileInfo::Update();
			};

		plugin::Events::initGameEvent += []() {
			GLauncher::ReadConfig();
			CCustomProjectileInfo::Initialise();
			};

		plugin::Events::reInitGameEvent += []() {
			GLauncher::ReadConfig();
			CCustomProjectileInfo::Initialise();
			};

		ClearForRestartEvent += []() {
			CCustomProjectileInfo::RemoveAllProjectiles();
			};

		ClearExcitingStuffFromAreaEvent += []() {
			CCustomProjectileInfo::RemoveAllProjectiles();
			};

		MakePlayerSafeEvent += []() {
			CCustomProjectileInfo::RemoveAllProjectiles();
			};
	};
} Grenade_LauncherPlugin;
