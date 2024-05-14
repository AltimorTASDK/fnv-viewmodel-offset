#include <cstddef>
#include <Windows.h>

struct IniPrefSetting {
	void **vtable;
	float value;
	const char *name;

	IniPrefSetting(const char *name, float value)
	{
		ThisCall(0x4DE370, this, name, value);
	}
};

namespace ini::Viewmodel {
auto fOffsetX = IniPrefSetting("fOffsetX:Viewmodel",  2.f);
auto fOffsetY = IniPrefSetting("fOffsetY:Viewmodel",  3.f);
auto fOffsetZ = IniPrefSetting("fOffsetZ:Viewmodel", -5.f);
}

static void __fastcall hook_NiAVObject_SetLocalTranslation(
	NiAVObject *object, int, const NiVector3 &translation)
{
	object->m_transformLocal.translate = translation;

	if (auto *Bip = *(NiNode**)0x11E07D8; object != Bip)
		return;

	if (InterfaceManager::GetSingleton()->pipBoyMode != 0)
		return;

	object->m_transformLocal.translate.x += ini::Viewmodel::fOffsetX.value;
	object->m_transformLocal.translate.y += ini::Viewmodel::fOffsetY.value;
	object->m_transformLocal.translate.z += ini::Viewmodel::fOffsetZ.value;
}

static void patch_call_rel32(const uintptr_t addr, const void *dest)
{
	DWORD old_protect;
	VirtualProtect((void*)addr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
	*(char*)addr = '\xE8'; // CALL opcode
	*(std::byte**)(addr + 1) = (std::byte*)dest - addr - 5;
	VirtualProtect((void*)addr, 5, old_protect, &old_protect);
}

extern "C" __declspec(dllexport) bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Viewmodel Adjustment";
	info->version = 2;
	return true;
}

extern "C" __declspec(dllexport) bool NVSEPlugin_Load(NVSEInterface *nvse)
{
	// Add offset after animating
	patch_call_rel32(0x4F047D, hook_NiAVObject_SetLocalTranslation);
	patch_call_rel32(0x4F0551, hook_NiAVObject_SetLocalTranslation);
	return true;
}