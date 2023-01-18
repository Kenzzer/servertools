// NextBotInterface.cpp
// Implentation of system methods for NextBot interface
// Author: Michael Booth, May 2006
//========= Copyright Valve Corporation, All rights reserved. ============//

#include "sdk/baseentity.hpp"

#include "NextBotInterface.h"
#include "NextBotBodyInterface.h"
#include "NextBotManager.h"

#include "tier0/vprof.h"

// development only, off by default for 360
extern ConVar* NextBotDebugHistory;

int INextBot::vtable_entries = -1;
VCall<IBody*> INextBot::vGetBodyInterface;
VCall<IIntention*> INextBot::vGetIntentionInterface;
VCall<ILocomotion*> INextBot::vGetLocomotionInterface;
VCall<void, const char*> INextBot::vDisplayDebugText;

bool INextBot::Init(CGameConfig* config, char* error, size_t maxlength)
{
	try
	{
		vGetBodyInterface.Init(&INextBot::GetBodyInterface);
		vGetIntentionInterface.Init(&INextBot::GetIntentionInterface);
		vGetLocomotionInterface.Init(&INextBot::GetLocomotionInterface);
		vDisplayDebugText.Init(&INextBot::DisplayDebugText);

		vtable_entries = vDisplayDebugText.GetOffset() + 1;
	}
	catch (const std::exception & e)
	{
		snprintf(error, maxlength, "%s", e.what());
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------------------------------------------
INextBot::INextBot( void ) : m_debugHistory( MAX_NEXTBOT_DEBUG_HISTORY, 0 )	// CUtlVector: grow to max length, alloc 0 initially
{
	m_tickLastUpdate = -999;
	m_id = -1;
	m_componentList = NULL;
	m_debugDisplayLine = 0;

	m_immobileTimer.Invalidate();
	m_immobileCheckTimer.Invalidate();
	m_immobileAnchor = vec3_origin;

	m_currentPath = NULL;

	// register with the manager
	m_id = TheNextBots().Register(this);
}


//----------------------------------------------------------------------------------------------------------------
INextBot::~INextBot()
{
	ResetDebugHistory();

	// tell the manager we're gone
	TheNextBots().UnRegister(this);

	// delete Intention first, since destruction of Actions may access other components
	if ( m_baseIntention )
		delete m_baseIntention;

	if ( m_baseLocomotion )
		delete m_baseLocomotion;

	if ( m_baseBody )
		delete m_baseBody;

	if ( m_baseVision )
		delete m_baseVision;
}


//----------------------------------------------------------------------------------------------------------------
void INextBot::Reset( void )
{
	m_tickLastUpdate = -999;
	m_debugType = 0;
	m_debugDisplayLine = 0;

	m_immobileTimer.Invalidate();
	m_immobileCheckTimer.Invalidate();
	m_immobileAnchor = vec3_origin;

	for( INextBotComponent *comp = m_componentList; comp; comp = comp->m_nextComponent )
	{
		comp->Reset();
	}
}


//----------------------------------------------------------------------------------------------------------------
void INextBot::ResetDebugHistory( void )
{
	for ( int i=0; i<m_debugHistory.Count(); ++i )
	{
		delete m_debugHistory[i];
	}

	m_debugHistory.RemoveAll();
}


//----------------------------------------------------------------------------------------------------------------
bool INextBot::BeginUpdate()
{
	if (TheNextBots().ShouldUpdate(this))
	{
		TheNextBots().NotifyBeginUpdate(this);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------
void INextBot::EndUpdate( void )
{
	TheNextBots().NotifyEndUpdate(this);
}

//----------------------------------------------------------------------------------------------------------------
void INextBot::Update( void )
{
	VPROF_BUDGET( "INextBot::Update", "NextBot" );

	m_debugDisplayLine = 0;

	UpdateImmobileStatus();

	// update all components
	for( INextBotComponent *comp = m_componentList; comp; comp = comp->m_nextComponent )
	{
		if ( comp->ComputeUpdateInterval() )
		{
			comp->Update();
		}
	}
}


//----------------------------------------------------------------------------------------------------------------
void INextBot::Upkeep( void )
{
	VPROF_BUDGET( "INextBot::Upkeep", "NextBot" );

	// do upkeep for all components
	for( INextBotComponent *comp = m_componentList; comp; comp = comp->m_nextComponent )
	{
		comp->Upkeep();
	}
}


//----------------------------------------------------------------------------------------------------------------
bool INextBot::SetPosition( const Vector &pos )
{
	IBody *body = GetBodyInterface();
	if (body)
	{
		return body->SetPosition( pos );
	}
	
	// fall back to setting raw entity position
	GetEntity()->SetAbsOrigin( pos );
	return true;
}


//----------------------------------------------------------------------------------------------------------------
const Vector &INextBot::GetPosition( void ) const
{
	return const_cast< INextBot * >( this )->GetEntity()->GetAbsOrigin();
}


//----------------------------------------------------------------------------------------------------------------
/**
 * Return true if given actor is our enemy
 */
bool INextBot::IsEnemy( const CBaseEntity *them ) const
{
	if ( them == NULL )
		return false;
		
	// this is not strictly correct, as spectators are not enemies
	return const_cast< INextBot * >( this )->GetEntity()->GetTeamNumber() != them->GetTeamNumber();
}


//----------------------------------------------------------------------------------------------------------------
/**
 * Return true if given actor is our friend
 */
bool INextBot::IsFriend( const CBaseEntity  *them ) const
{
	if ( them == NULL )
		return false;
		
	return const_cast< INextBot * >( this )->GetEntity()->GetTeamNumber() == them->GetTeamNumber();
}


//----------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'them' is actually me
 */
bool INextBot::IsSelf( const CBaseEntity *them ) const
{
	if ( them == NULL )
		return false;

	return const_cast< INextBot * >( this )->GetEntity()->entindex() == them->entindex();
}


//----------------------------------------------------------------------------------------------------------------
/**
 * Components call this to register themselves with the bot that contains them
 */
void INextBot::RegisterComponent( INextBotComponent *comp )
{
	// add to head of singly linked list
	comp->m_nextComponent = m_componentList;
	m_componentList = comp;
}


//----------------------------------------------------------------------------------------------------------------
bool INextBot::IsRangeLessThan( CBaseEntity *subject, float range ) const
{
	Vector botPos;
	CBaseEntity *bot = const_cast< INextBot * >( this )->GetEntity();
	if ( !bot || !subject )
		return 0.0f;

	bot->CollisionProp()->CalcNearestPoint( subject->WorldSpaceCenter(), &botPos );
	float computedRange = subject->CollisionProp()->CalcDistanceFromPoint( botPos );
	return computedRange < range;
}


//----------------------------------------------------------------------------------------------------------------
bool INextBot::IsRangeLessThan( const Vector &pos, float range ) const
{
	Vector to = pos - GetPosition();
	return to.IsLengthLessThan( range );
}


//----------------------------------------------------------------------------------------------------------------
bool INextBot::IsRangeGreaterThan( CBaseEntity *subject, float range ) const
{
	Vector botPos;
	CBaseEntity *bot = const_cast< INextBot * >( this )->GetEntity();
	if ( !bot || !subject )
		return true;

	bot->CollisionProp()->CalcNearestPoint( subject->WorldSpaceCenter(), &botPos );
	float computedRange = subject->CollisionProp()->CalcDistanceFromPoint( botPos );
	return computedRange > range;
}


//----------------------------------------------------------------------------------------------------------------
bool INextBot::IsRangeGreaterThan( const Vector &pos, float range ) const
{
	Vector to = pos - GetPosition();
	return to.IsLengthGreaterThan( range );
}


//----------------------------------------------------------------------------------------------------------------
float INextBot::GetRangeTo( CBaseEntity *subject ) const
{
	Vector botPos;
	CBaseEntity *bot = const_cast< INextBot * >( this )->GetEntity();
	if ( !bot || !subject )
		return 0.0f;

	bot->CollisionProp()->CalcNearestPoint( subject->WorldSpaceCenter(), &botPos );
	float computedRange = subject->CollisionProp()->CalcDistanceFromPoint( botPos );
	return computedRange;
}


//----------------------------------------------------------------------------------------------------------------
float INextBot::GetRangeTo( const Vector &pos ) const
{
	Vector to = pos - GetPosition();
	return to.Length();
}


//----------------------------------------------------------------------------------------------------------------
float INextBot::GetRangeSquaredTo( CBaseEntity *subject ) const
{
	Vector botPos;
	CBaseEntity *bot = const_cast< INextBot * >( this )->GetEntity();
	if ( !bot || !subject )
		return 0.0f;

	bot->CollisionProp()->CalcNearestPoint( subject->WorldSpaceCenter(), &botPos );
	float computedRange = subject->CollisionProp()->CalcDistanceFromPoint( botPos );
	return computedRange * computedRange;
}


//----------------------------------------------------------------------------------------------------------------
float INextBot::GetRangeSquaredTo( const Vector &pos ) const
{
	Vector to = pos - GetPosition();
	return to.LengthSqr();	
}

//----------------------------------------------------------------------------------------------------------------
/**
 * Return the name of this bot for debugging purposes
 */
const char *INextBot::GetDebugIdentifier( void ) const
{
	const int nameSize = 256;
	static char name[ nameSize ];
	
	Q_snprintf( name, nameSize, "%s(#%d)", const_cast< INextBot * >( this )->GetEntity()->GetClassname(), const_cast< INextBot * >( this )->GetEntity()->entindex() );

	return name;
}


//----------------------------------------------------------------------------------------------------------
void INextBot::DisplayDebugText( const char *text ) const
{
	m_debugDisplayLine++;
	//const_cast< INextBot * >( this )->GetEntity()->EntityText( m_debugDisplayLine++, text, 0.1 );
}


//--------------------------------------------------------------------------------------------------------
void INextBot::DebugConColorMsg( NextBotDebugType debugType, const Color &color, const char *fmt, ... )
{
	bool isDataFormatted = false;

	va_list argptr;
	char data[ MAX_NEXTBOT_DEBUG_LINE_LENGTH ];

	if ( !NextBotDebugHistory->GetBool() )
	{
		if ( m_debugHistory.Count() )
		{
			ResetDebugHistory();
		}
		return;
	}

	// Don't bother with event data - it's spammy enough to overshadow everything else.
	if ( debugType == NEXTBOT_EVENTS )
		return;

	if ( !isDataFormatted )
	{
		va_start(argptr, fmt);
		Q_vsnprintf(data, sizeof( data ), fmt, argptr);
		va_end(argptr);
		isDataFormatted = true;
	}

	int lastLine = m_debugHistory.Count() - 1;
	if ( lastLine >= 0 )
	{
		NextBotDebugLineType *line = m_debugHistory[lastLine];
		if ( line->debugType == debugType && V_strstr( line->data, "\n" ) == NULL )
		{
			// append onto previous line
			V_strncat( line->data, data, MAX_NEXTBOT_DEBUG_LINE_LENGTH );
			return;
		}
	}

	// Prune out an old line if needed, keeping a pointer to re-use the memory
	NextBotDebugLineType *line = NULL;
	if ( m_debugHistory.Count() == MAX_NEXTBOT_DEBUG_HISTORY )
	{
		line = m_debugHistory[0];
		m_debugHistory.Remove( 0 );
	}

	// Add to debug history
	if ( !line )
	{
		line = new NextBotDebugLineType;
	}
	line->debugType = debugType;
	V_strncpy( line->data, data, MAX_NEXTBOT_DEBUG_LINE_LENGTH );
	m_debugHistory.AddToTail( line );
}


//--------------------------------------------------------------------------------------------------------
// build a vector of debug history of the given types
void INextBot::GetDebugHistory( unsigned int type, CUtlVector< const NextBotDebugLineType * > *lines ) const
{
	if ( !lines )
		return;

	lines->RemoveAll();

	for ( int i=0; i<m_debugHistory.Count(); ++i )
	{
		NextBotDebugLineType *line = m_debugHistory[i];
		if ( line->debugType & type )
		{
			lines->AddToTail( line );
		}
	}
}


//--------------------------------------------------------------------------------------------------------
void INextBot::UpdateImmobileStatus( void )
{
	if ( m_immobileCheckTimer.IsElapsed() )
	{
		m_immobileCheckTimer.Start( 1.0f );

		// if we haven't moved farther than this in 1 second, we're immobile
		if ( ( GetEntity()->GetAbsOrigin() - m_immobileAnchor ).IsLengthGreaterThan( GetImmobileSpeedThreshold() ) )
		{
			// moved far enough, not immobile
			m_immobileAnchor = GetEntity()->GetAbsOrigin();
			m_immobileTimer.Invalidate();
		}
		else
		{
			// haven't escaped our anchor - we are immobile
			if ( !m_immobileTimer.HasStarted() )
			{
				m_immobileTimer.Start();
			}
		}
	}
}