#include <nvse/prefix.h>
#include <nvse/GameUI.h>
#include <nvse/PluginAPI.h>
#include <nvse/NiObjects.h>
#include <cstddef>
#include <Windows.h>

class Setting {
	struct Vtable {
		virtual ~Vtable() = 0;
		virtual int vfunc01() = 0;
	};
	void **vtable;
public:
	float value;
	const char *name;

	~Setting()
	{
		((Vtable&)vtable).~Vtable();
	}
};

template<typename T>
class SettingT;

template<>
class SettingT<class INIPrefSettingCollection> : public Setting {
public:
	SettingT(const char *name, float value)
	{
		ThisStdCall(0x4DE370, this, name, value);
	}
};

namespace ini::Viewmodel {
auto fOffsetX = SettingT<INIPrefSettingCollection>("fOffsetX:Viewmodel",  2.f);
auto fOffsetY = SettingT<INIPrefSettingCollection>("fOffsetY:Viewmodel",  3.f);
auto fOffsetZ = SettingT<INIPrefSettingCollection>("fOffsetZ:Viewmodel", -5.f);
}

static void modify_transform()
{
	auto *viewmodel_root = *(NiNode**)(0x11E07D8);
	auto *bip01_rotate = viewmodel_root->m_parent;
	auto *bip01_translate = bip01_rotate->m_parent;
	auto *transform = &bip01_translate->dat0034;
	transform->translate.x = ini::Viewmodel::fOffsetX.value;
	transform->translate.y = ini::Viewmodel::fOffsetY.value;
	transform->translate.z = ini::Viewmodel::fOffsetZ.value - 118.F;
}

__declspec(naked) static void modify_transform_wrapper()
{
	__asm {
		pushad
		call modify_transform
		popad

		// Overwritten instructions
		mov esp, ebp
		pop ebp
		retn 8
	}
}

static void patch_jmp_rel32(const uintptr_t addr, const void *dest)
{
	DWORD old_protect;
	VirtualProtect((void*)addr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
	*(char*)addr = '\xE9'; // JMP opcode
	*(std::byte**)(addr + 1) = (std::byte*)dest - addr - 5;
	VirtualProtect((void*)addr, 5, old_protect, &old_protect);
}

extern "C" __declspec(dllexport) bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Viewmodel Adjustment";
	info->version = 1;
	return true;
}

extern "C" __declspec(dllexport) bool NVSEPlugin_Load(NVSEInterface *nvse)
{
	// Patch retns
	patch_jmp_rel32(0x4F059A, modify_transform_wrapper);
	return true;
}