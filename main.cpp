#include <cstddef>
#include <Windows.h>

IniSettingCollection *IniSettingCollection::GetIniPrefs()
{
	return *(IniSettingCollection**)0x11F35A0;
}

template<typename T> requires (sizeof(T) == sizeof(int) && alignof(T) == alignof(int))
struct IniPrefSetting {
	T value;
	const char *name;

	constexpr IniPrefSetting(const char *name, T value) :
		name(name), value(value)
	{
		IniSettingCollection::GetIniPrefs()->AddSetting((Setting*)this);
	}

	virtual ~IniPrefSetting()
	{
		IniSettingCollection::GetIniPrefs()->RemoveSetting((Setting*)this);
	}

	virtual bool vfunc01()
	{
		return false;
	}
};

static auto &Ini()
{
	static struct {
		IniPrefSetting<float> fOffsetX = {"fOffsetX:Viewmodel",  2.f};
		IniPrefSetting<float> fOffsetY = {"fOffsetY:Viewmodel",  3.f};
		IniPrefSetting<float> fOffsetZ = {"fOffsetZ:Viewmodel", -5.f};
	} ini;
	return ini;
}

static void __fastcall hook_NiAVObject_SetLocalTranslation(
	NiAVObject *object, int, const NiVector3 &translation)
{
	object->m_transformLocal.translate = translation;

	if (auto *Bip = *(NiNode**)0x11E07D8; object != Bip)
		return;

	if (InterfaceManager::GetSingleton()->pipBoyMode != 0)
		return;

	object->m_transformLocal.translate.x += Ini().fOffsetX.value;
	object->m_transformLocal.translate.y += Ini().fOffsetY.value;
	object->m_transformLocal.translate.z += Ini().fOffsetZ.value;
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
	return !nvse->isEditor;
}

extern "C" __declspec(dllexport) bool NVSEPlugin_Load(NVSEInterface *nvse)
{
	// Add variables
	Ini();
	// Add offset after animating
	patch_call_rel32(0x4F047D, hook_NiAVObject_SetLocalTranslation);
	patch_call_rel32(0x4F0551, hook_NiAVObject_SetLocalTranslation);
	return true;
}