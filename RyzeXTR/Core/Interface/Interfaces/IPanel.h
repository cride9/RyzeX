#pragma once

class IPanel : ROP::VirtualCallable_t<ROP::ClientGadget_t> {
public:
	void SetKeyBoardInputEnabled(unsigned int nVGUIPanel, bool bState)
	{
		CallVFunc<void, 31U>(this, nVGUIPanel, bState);
	}

	void SetMouseInputEnabled(unsigned int nVGUIPanel, bool bState)
	{
		CallVFunc<void, 32U>(this, nVGUIPanel, bState);
	}

	const char* GetName(unsigned int nVGUIPanel)
	{
		return CallVFunc<const char*, 36U>(this, nVGUIPanel);
	}

	const char* GetNameClass(unsigned int nVGUIPanel)
	{
		return CallVFunc<const char*, 37U>(this, nVGUIPanel);
	}
};