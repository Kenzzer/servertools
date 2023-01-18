// NextBotCombatCharacter.h
// Next generation bot system
// Author: Michael Booth, April 2005
//========= Copyright Valve Corporation, All rights reserved. ============//
#pragma once

#include "NextBotInterface.h"
#include "NextBotManager.h"

#ifdef TERROR
#include "player_lagcompensation.h"
#endif

class NextBotCombatCharacter;
struct animevent_t;

extern ConVar* NextBotStop;

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
/**
 * A Next Bot derived from CBaseCombatCharacter
 */
class NextBotCombatCharacter : public CBaseCombatCharacter
{
};

//-----------------------------------------------------------------------------------------------------
class NextBotDestroyer
{
public:
	NextBotDestroyer( int team );
	bool operator() ( INextBot *bot );
	int m_team;			// the team to delete bots from, or TEAM_ANY for any team
};