#include "funcbrush.hpp"

DEFINEVAR(CFuncBrush, m_iSolidity);

bool CFuncBrush::Init(CGameConfig* config, char* error, size_t maxlength)
{
	BEGIN_VAR("func_brush");
	OFFSETVAR_DATA(CFuncBrush, m_iSolidity);
	END_VAR;
	return true;
}