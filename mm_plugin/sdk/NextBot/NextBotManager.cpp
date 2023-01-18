#include "NextBotManager.h"
#include "interfaces.hpp"

FCall<NextBotManager&> TheNextBots;

//static int g_nRun;
//static int g_nSlid;
//static int g_nBlockedSlides;

ConVar* nb_update_framelimit = nullptr;
ConVar* nb_update_maxslide = nullptr;
ConVar* NextBotDebugHistory = nullptr;
ConVar* NextBotPathDrawIncrement = nullptr;
ConVar* NextBotPathSegmentInfluenceRadius = nullptr;
ConVar* NextBotPlayerStop = nullptr;
ConVar* NextBotStop = nullptr;
ConVar* NextBotSpeedLookAheadRange = nullptr;
ConVar* NextBotGoalLookAheadRange = nullptr;
ConVar* NextBotLadderAlignRange = nullptr;
ConVar* NextBotAllowClimbing = nullptr;
ConVar* NextBotAllowGapJumping = nullptr;
ConVar* NextBotAllowAvoiding = nullptr;
ConVar* NextBotDebugClimbing = nullptr;
ConVar* NextBotPathDrawSegmentCount = nullptr;
ConVar* nb_blind = nullptr;


bool NextBotManager::Init(CGameConfig* config, char* error, size_t maxlength)
{
	try
	{
		TheNextBots.Init(config, "TheNextBots");
	}
	catch (const std::exception& e)
	{
		snprintf(error, maxlength, "%s", e.what());
		return false;
	}

	NextBotSpeedLookAheadRange = icvar->FindVar("nb_speed_look_ahead_range");
	NextBotGoalLookAheadRange = icvar->FindVar("nb_goal_look_ahead_range");
	NextBotLadderAlignRange = icvar->FindVar("nb_ladder_align_range");
	NextBotAllowAvoiding = icvar->FindVar("nb_allow_avoiding");
	NextBotAllowClimbing = icvar->FindVar("nb_allow_climbing");
	NextBotAllowGapJumping = icvar->FindVar("nb_allow_gap_jumping");
	NextBotDebugClimbing = icvar->FindVar("nb_debug_climbing");
	NextBotDebugHistory = icvar->FindVar("nb_debug_history");
	NextBotPathDrawIncrement = icvar->FindVar("nb_path_draw_inc");
	NextBotPathSegmentInfluenceRadius = icvar->FindVar("nb_path_segment_influence_radius");
	NextBotPlayerStop = icvar->FindVar("nb_player_stop");
	NextBotStop = icvar->FindVar("nb_stop");
	NextBotPathDrawSegmentCount = icvar->FindVar("nb_path_draw_segment_count");
	nb_update_framelimit = icvar->FindVar("nb_update_framelimit");
	nb_update_maxslide = icvar->FindVar("nb_update_maxslide");
	nb_blind = icvar->FindVar("nb_blind");

	return (INextBot::Init(config, error, maxlength));
}

bool NextBotManager::ShouldUpdate(INextBot* bot)
{
	if (m_iUpdateTickrate < 1)
	{
		return true;
	}

	float frameLimit = nb_update_framelimit->GetFloat();
	float sumFrameTime = 0;
	if (bot->IsFlaggedForUpdate())
	{
		bot->FlagForUpdate(false);
		sumFrameTime = m_SumFrameTime * 1000.0;
		if (frameLimit > 0.0f)
		{
			if (sumFrameTime < frameLimit)
			{
				return true;
			}
			/*else if ( nb_update_debug.GetBool() )
			{
				Msg( "Frame %8d/tick %8d: frame out of budget (%.2fms > %.2fms)\n", gpGlobals->framecount, gpGlobals->tickcount, sumFrameTime, frameLimit );
			}*/
		}
	}

	int nTicksSlid = (gpGlobals->tickcount - bot->GetTickLastUpdate()) - m_iUpdateTickrate;

	if (nTicksSlid >= nb_update_maxslide->GetInt())
	{
		if (frameLimit == 0.0 || sumFrameTime < nb_update_framelimit->GetFloat() * 2.0)
		{
			//g_nBlockedSlides++;
			return true;
		}
	}

	/*if (nb_update_debug.GetBool())
	{
		if (nTicksSlid > 0)
		{
			g_nSlid++;
		}
	}*/

	return false;
}

void NextBotManager::NotifyBeginUpdate(INextBot* bot)
{
	/*if (nb_update_debug.GetBool())
	{
		g_nRun++;
	}*/

	m_botList.Unlink(bot->GetBotId());
	m_botList.LinkToTail(bot->GetBotId());
	bot->SetTickLastUpdate(gpGlobals->tickcount);

	m_CurUpdateStartTime = Plat_FloatTime();
}

void NextBotManager::NotifyEndUpdate(INextBot* bot)
{
	m_SumFrameTime += Plat_FloatTime() - m_CurUpdateStartTime;
}

int NextBotManager::Register(INextBot* bot)
{
	return m_botList.AddToHead(bot);
}

void NextBotManager::UnRegister(INextBot* bot)
{
	m_botList.Remove(bot->GetBotId());

	if (bot == m_selectedBot)
	{
		m_selectedBot = nullptr;
	}
}