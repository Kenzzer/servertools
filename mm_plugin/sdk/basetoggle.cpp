#include "basetoggle.hpp"

DEFINEVAR(CBaseToggle, m_toggle_state);

bool CBaseToggle::Init(CGameConfig* config, char* error, size_t maxlength)
{
	BEGIN_VAR("func_door");
	OFFSETVAR_DATA(CBaseToggle, m_toggle_state);
	END_VAR;
	return true;
}