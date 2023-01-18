// NextBotManager.h
// Author: Michael Booth, May 2006
//========= Copyright Valve Corporation, All rights reserved. ============//
#pragma once

#include "NextBotInterface.h"
#include "helpers.hpp"

class CTerrorPlayer;

class NextBotManager
{
public:
	static bool Init(CGameConfig* config, char* error, size_t maxlength);

	virtual ~NextBotManager() = 0;
	virtual void Update(void) = 0;
	virtual void OnMapLoaded(void) = 0;
	virtual void OnRoundRestart(void) = 0;
	virtual void OnBeginChangeLevel(void) = 0;
	virtual void OnKilled(CBaseCombatCharacter* victim, const CTakeDamageInfo& info) = 0;
	virtual void OnSound(CBaseEntity* source, const Vector& pos, KeyValues* keys) = 0;
	virtual void OnSpokeConcept(CBaseCombatCharacter* who, AIConcept_t concept, AI_Response* response) = 0;
	virtual void OnWeaponFired(CBaseCombatCharacter* whoFired, CBaseEntity* weapon) = 0;

	bool ShouldUpdate(INextBot* bot);
	void NotifyBeginUpdate(INextBot* bot);
	void NotifyEndUpdate(INextBot* bot);

	template <typename Functor>
	bool ForEachBot(Functor &func)
	{
		for (int i=m_botList.Head(); i != m_botList.InvalidIndex(); i = m_botList.Next(i))
		{
			if (!func(m_botList[i]))
			{
				return false;
			}
		}
		return true;
	}

	template < typename Functor >
	bool ForEachCombatCharacter( Functor &func )
	{
		for( int i=m_botList.Head(); i != m_botList.InvalidIndex(); i = m_botList.Next( i ) )
		{
			if ( !func( m_botList[i]->GetEntity() ) )
			{

				return false;
			}
		}

		return true;
	}
protected:
	friend class INextBot;
	int Register(INextBot* bot);
	void UnRegister(INextBot* bot);

	CUtlLinkedList<INextBot*> m_botList;
	int m_iUpdateTickrate;
	double m_CurUpdateStartTime;
	double m_SumFrameTime;
	unsigned int m_debugType;
	struct DebugFilter
	{
		int index;
		enum { MAX_DEBUG_NAME_SIZE = 128 };
		char name[ MAX_DEBUG_NAME_SIZE ];
	};
	CUtlVector<DebugFilter> m_debugFilterList;
	INextBot* m_selectedBot;
};

extern FCall<NextBotManager&> TheNextBots;