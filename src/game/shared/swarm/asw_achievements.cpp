#include "cbase.h"
#include "asw_achievements.h"
#include "asw_shareddefs.h"
#include "asw_gamerules.h"
#ifdef CLIENT_DLL
#include "c_asw_game_resource.h"
#include "c_asw_marine.h"
#include "c_asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
CASW_Achievement_Manager g_ASW_AchievementMgr;	// global achievement manager for Alien Swarm
CASW_Achievement_Manager* ASWAchievementManager() { return &g_ASW_AchievementMgr; }


bool LocalPlayerWasSpectating( void )
{
	C_ASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
		return true;

	C_ASW_Player *pLocalPlayer = static_cast< C_ASW_Player* >( C_BasePlayer::GetLocalPlayer() );
	if ( !pLocalPlayer )
		return true;

	if ( pGameResource->GetNumMarines( pLocalPlayer ) <= 0 )
		return true;

	return false;
}


CASW_Achievement_Manager::CASW_Achievement_Manager()
{

}


bool CASW_Achievement_Manager::Init()
{
	ListenForGameEvent( "alien_died" );

	return BaseClass::Init();
}

void CASW_Achievement_Manager::Shutdown()
{
	for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		m_vecAlienDeathEventListeners[i].RemoveAll();
	}

	BaseClass::Shutdown();
}

void CASW_Achievement_Manager::LevelInitPreEntity()
{
	for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		// clear list of achievements listening for events
		m_vecAlienDeathEventListeners[i].RemoveAll();

		// look through all achievements, see which ones we want to have listen for events
		int nCount = GetAchievementCount();
		for ( int k = 0; k < nCount; k++ )
		{
			CASW_Achievement *pAchievement = static_cast<CASW_Achievement*>( GetAchievementByIndex( k, i ) );
			if ( !pAchievement )
				continue;

			if ( pAchievement->GetFlags() & ACH_LISTEN_ALIEN_DEATH_EVENTS )
			{
				m_vecAlienDeathEventListeners[i].AddToTail( pAchievement );
			}
		}
	}
	BaseClass::LevelInitPreEntity();
}

void CASW_Achievement_Manager::FireGameEvent( IGameEvent *event )
{
	const char *name = event->GetName();
	if ( !Q_strcmp( name, "alien_died" ) )
	{
		int nMarineIndex = event->GetInt( "marine" );

		C_ASW_Marine *pMarine = nMarineIndex > 0 ? dynamic_cast<CASW_Marine*>( C_BaseEntity::Instance( nMarineIndex ) ) : NULL;
		for ( int j = 0; j < MAX_SPLITSCREEN_PLAYERS; ++j )
		{
			// look through all the kill event listeners and notify any achievements whose filters we pass
			FOR_EACH_VEC( m_vecAlienDeathEventListeners[j], iAchievement )
			{
				CASW_Achievement *pAchievement = m_vecAlienDeathEventListeners[j][iAchievement];

				if ( !pAchievement->IsActive() )
					continue;

				pAchievement->OnAlienDied( event->GetInt( "alien" ), pMarine, event->GetInt( "weapon" ) );
			}
		}
	}
	else
	{
		BaseClass::FireGameEvent( event );
	}
}

// =====================

CASW_Achievement::CASW_Achievement()
{
	m_nAlienClassFilter = 0;
	m_nWeaponClassFilter = 0;
}

bool CASW_Achievement::OnAlienDied( int nAlienClass, C_ASW_Marine *pMarine, int nWeaponClass )
{
	if ( !pMarine || !pMarine->GetCommander() || pMarine->GetCommander() != C_ASW_Player::GetLocalASWPlayer() || !pMarine->IsInhabited() )
		return false;

	if ( m_nAlienClassFilter != 0 && nAlienClass != m_nAlienClassFilter )
		return false;

	if ( m_nWeaponClassFilter != 0 && nWeaponClass != m_nWeaponClassFilter )
		return false;

	IncrementCount();
	return true;
}

const char *CASW_Achievement::GetIconPath()
{
	if ( !IsAchieved() )
	{
		return "swarm/EquipIcons/Locked";
	}
	static char szImage[ MAX_PATH ];	
	Q_snprintf( szImage, sizeof( szImage ), "achievements/%s", GetName() );

	return szImage;
}

// =====================

class CAchievement_No_Friendly_Fire : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_No_Friendly_Fire, ACHIEVEMENT_ASW_NO_FRIENDLY_FIRE, "ASW_NO_FRIENDLY_FIRE", 5, 10 );

class CAchievement_Shieldbug : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 1 );
		SetAlienClassFilter( CLASS_ASW_SHIELDBUG );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Shieldbug, ACHIEVEMENT_ASW_SHIELDBUG, "ASW_SHIELDBUG", 5, 20 );

class CAchievement_Grenade_Multi_Kill : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Grenade_Multi_Kill, ACHIEVEMENT_ASW_GRENADE_MULTI_KILL, "ASW_GRENADE_MULTI_KILL", 5, 30 );

class CAchievement_Accuracy : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Accuracy, ACHIEVEMENT_ASW_ACCURACY, "ASW_ACCURACY", 5, 40 );

class CAchievement_No_Damage_Taken : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_No_Damage_Taken, ACHIEVEMENT_ASW_NO_DAMAGE_TAKEN, "ASW_NO_DAMAGE_TAKEN", 5, 50 );

class CAchievement_Sentry_Gun_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 500 );
		SetStoreProgressInSteam( true );
	}

	bool OnAlienDied( int nAlienClass, C_ASW_Marine *pMarine, int nWeaponClass )
	{
		if ( !pMarine || !pMarine->GetCommander() || pMarine->GetCommander() != C_ASW_Player::GetLocalASWPlayer() || !pMarine->IsInhabited() )
			return false;

		if ( m_nAlienClassFilter != 0 && nAlienClass != m_nAlienClassFilter )
			return false;

		if ( !( nWeaponClass == CLASS_ASW_SENTRY_GUN || nWeaponClass == CLASS_ASW_SENTRY_FLAMER || nWeaponClass == CLASS_ASW_SENTRY_FREEZE || nWeaponClass == CLASS_ASW_SENTRY_CANNON ) )
			return false;

		IncrementCount();
		return true;
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Sentry_Gun_Kills, ACHIEVEMENT_ASW_SENTRY_GUN_KILLS, "ASW_SENTRY_GUN_KILLS", 5, 60 );

class CAchievement_Eggs_Before_Hatch : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Eggs_Before_Hatch, ACHIEVEMENT_ASW_EGGS_BEFORE_HATCH, "ASW_EGGS_BEFORE_HATCH", 5, 70 );

class CAchievement_Grub_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 100 );
		SetStoreProgressInSteam( true );
		SetAlienClassFilter( CLASS_ASW_GRUB );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Grub_Kills, ACHIEVEMENT_ASW_GRUB_KILLS, "ASW_GRUB_KILLS", 5, 80 );

class CAchievement_Melee_Parasite : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Melee_Parasite, ACHIEVEMENT_ASW_MELEE_PARASITE, "ASW_MELEE_PARASITE", 5, 90 );

class CAchievement_Melee_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Melee_Kills, ACHIEVEMENT_ASW_MELEE_KILLS, "ASW_MELEE_KILLS", 5, 100 );

class CAchievement_Barrel_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Barrel_Kills, ACHIEVEMENT_ASW_BARREL_KILLS, "ASW_BARREL_KILLS", 5, 110 );

class CAchievement_Infestation_Curing : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Infestation_Curing, ACHIEVEMENT_ASW_INFESTATION_CURING, "ASW_INFESTATION_CURING", 5, 120 );

class CAchievement_Fast_Wire_Hacks : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetStoreProgressInSteam( true );
		SetGoal( 10 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Fast_Wire_Hacks, ACHIEVEMENT_ASW_FAST_WIRE_HACKS, "ASW_FAST_WIRE_HACKS", 5, 130 );

class CAchievement_Fast_Computer_Hacks : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetStoreProgressInSteam( true );
		SetGoal( 10 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Fast_Computer_Hacks, ACHIEVEMENT_ASW_FAST_COMPUTER_HACKS, "ASW_FAST_COMPUTER_HACKS", 5, 140 );

#define DIFFICULTY_CAMPAIGN_ACHIEVEMENT(DIFFICULTY_NAME, DifficultyName, iOffset, iDifficulty, PREFIX, SUFFIX, CampaignName) \
class CAchievement_ ## DifficultyName ## _Campaign_ ## CampaignName : public CASW_Achievement \
{ \
	void Init() \
	{ \
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS ); \
		SetStoreProgressInSteam( true ); \
		SetGoal( NELEMS( g_szAchievementMapNames ## CampaignName ) ); \
	} \
\
	virtual void ListenForEvents( void ) \
	{ \
		ListenForGameEvent( "mission_success" ); \
	} \
\
	void FireGameEvent_Internal( IGameEvent *event ) \
	{ \
		if ( !Q_stricmp( event->GetName(), "mission_success" ) && ASWGameRules() && ASWGameRules()->GetSkillLevel() >= iDifficulty && !ASWGameRules()->m_bChallengeActiveThisMission ) \
		{ \
			if ( LocalPlayerWasSpectating() ) \
				return; \
\
			const char *szMapName = event->GetString( "strMapName" ); \
			for ( int i = 0; i < NELEMS( g_szAchievementMapNames ## CampaignName ); i++ ) \
			{ \
				if ( !Q_stricmp( szMapName, g_szAchievementMapNames ## CampaignName[i] ) ) \
				{ \
					EnsureComponentBitSetAndEvaluate( i ); \
					break; \
				} \
			} \
		} \
	} \
}; \
DECLARE_ACHIEVEMENT_ORDER( CAchievement_ ## DifficultyName ## _Campaign_ ## CampaignName, (ACHIEVEMENT_ ## PREFIX ## DIFFICULTY_NAME ## SUFFIX), #PREFIX #DIFFICULTY_NAME #SUFFIX, 5, iOffset + iDifficulty - 1 )

#define DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(iOffset, PREFIX, SUFFIX, CampaignName) \
	DIFFICULTY_CAMPAIGN_ACHIEVEMENT(EASY, Easy, iOffset, 1, PREFIX, SUFFIX, CampaignName); \
	DIFFICULTY_CAMPAIGN_ACHIEVEMENT(NORMAL, Normal, iOffset, 2, PREFIX, SUFFIX, CampaignName); \
	DIFFICULTY_CAMPAIGN_ACHIEVEMENT(HARD, Hard, iOffset, 3, PREFIX, SUFFIX, CampaignName); \
	DIFFICULTY_CAMPAIGN_ACHIEVEMENT(INSANE, Insane, iOffset, 4, PREFIX, SUFFIX, CampaignName); \
	DIFFICULTY_CAMPAIGN_ACHIEVEMENT(IMBA, Imba, iOffset, 5, PREFIX, SUFFIX, CampaignName)

static const char *g_szAchievementMapNamesAllCoop[] =
{
	"asi-jac1-landingbay_01",
	"asi-jac1-landingbay_02",
	"asi-jac2-deima",
	"asi-jac3-rydberg",
	"asi-jac4-residential",
	"asi-jac6-sewerjunction",
	"asi-jac7-timorstation",
	"rd-ocs1storagefacility",
	"rd-ocs2landingbay7",
	"rd-ocs3uscmedusa",
	"rd-res1forestentrance",
	"rd-res2research7",
	"rd-res3miningcamp",
	"rd-res4mines",
	"rd-area9800LZ",
	"rd-area9800PP1",
	"rd-area9800PP2",
	"rd-area9800WL",
	"rd-tft1desertoutpost",
	"rd-tft2abandonedmaintenance",
	"rd-tft3spaceport",
	"rd-dc1_omega_city",
	"rd-dc2_breaking_an_entry",
	"rd-dc3_search_and_rescue",
	"rd-til1midnightport",
	"rd-til2roadtodawn",
	"rd-til3arcticinfiltration",
	"rd-til4area9800",
	"rd-til5coldcatwalks",
	"rd-til6yanaurusmine",
	"rd-til7factory",
	"rd-til8comcenter",
	"rd-til9syntekhospital",
	"rd-lan1_bridge",
	"rd-lan2_sewer",
	"rd-lan3_maintenance",
	"rd-lan4_vent",
	"rd-lan5_complex",
	"rd-reduction1",
	"rd-reduction2",
	"rd-reduction3",
	"rd-reduction4",
	"rd-reduction5",
	"rd-reduction6",
	"rd-par1unexpected_encounter",
	"rd-par2hostile_places",
	"rd-par3close_contact",
	"rd-par4high_tension",
	"rd-par5crucial_point",
	"rd-bonus_mission1",
	"rd-bonus_mission2",
	"rd-bonus_mission3",
	"rd-bonus_mission4",
	"rd-bonus_mission5",
	"rd-bonus_mission6",
	"rd-bonus_mission7",
};

static const char *g_szAchievementMapNamesJacob[] =
{
	"asi-jac1-landingbay_01",
	"asi-jac1-landingbay_02",
	"asi-jac2-deima",
	"asi-jac3-rydberg",
	"asi-jac4-residential",
	"asi-jac6-sewerjunction",
	"asi-jac7-timorstation",
};

static const char *g_szAchievementMapNamesOCS[] =
{
	"rd-ocs1storagefacility",
	"rd-ocs2landingbay7",
	"rd-ocs3uscmedusa",
};

static const char *g_szAchievementMapNamesRES[] =
{
	"rd-res1forestentrance",
	"rd-res2research7",
	"rd-res3miningcamp",
	"rd-res4mines",
};

static const char *g_szAchievementMapNamesArea9800[] =
{
	"rd-area9800LZ",
	"rd-area9800PP1",
	"rd-area9800PP2",
	"rd-area9800WL",
};

static const char *g_szAchievementMapNamesTFT[] =
{
	"rd-tft1desertoutpost",
	"rd-tft2abandonedmaintenance",
	"rd-tft3spaceport",
};

static const char *g_szAchievementMapNamesDC[] =
{
	"rd-dc1_omega_city",
	"rd-dc2_breaking_an_entry",
	"rd-dc3_search_and_rescue",
};

static const char *g_szAchievementMapNamesTIL[] =
{
	"rd-til1midnightport",
	"rd-til2roadtodawn",
	"rd-til3arcticinfiltration",
	"rd-til4area9800",
	"rd-til5coldcatwalks",
	"rd-til6yanaurusmine",
	"rd-til7factory",
	"rd-til8comcenter",
	"rd-til9syntekhospital",
};

static const char *g_szAchievementMapNamesLana[] =
{
	"rd-lan1_bridge",
	"rd-lan2_sewer",
	"rd-lan3_maintenance",
	"rd-lan4_vent",
	"rd-lan5_complex",
};

static const char *g_szAchievementMapNamesReduction[] =
{
	"rd-reduction1",
	"rd-reduction2",
	"rd-reduction3",
	"rd-reduction4",
	"rd-reduction5",
	"rd-reduction6",
};

static const char *g_szAchievementMapNamesPAR[] =
{
	"rd-par1unexpected_encounter",
	"rd-par2hostile_places",
	"rd-par3close_contact",
	"rd-par4high_tension",
	"rd-par5crucial_point",
};

DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(150, ASW_, _CAMPAIGN, Jacob);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(155, RD_, _CAMPAIGN_OCS, OCS);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(160, RD_, _CAMPAIGN_RES, RES);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(165, RD_, _CAMPAIGN_AREA9800, Area9800);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(170, RD_, _CAMPAIGN_TFT, TFT);
//DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(175, RD_, _CAMPAIGN_DC, DC);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(180, RD_, _CAMPAIGN_TIL, TIL);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(185, RD_, _CAMPAIGN_LAN, Lana);
//DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(190, RD_, _CAMPAIGN_REDUCTION, Reduction);
DIFFICULTY_CAMPAIGN_ACHIEVEMENTS(195, RD_, _CAMPAIGN_PAR, PAR);

class CAchievement_Kill_Grind_1 : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetStoreProgressInSteam( true );
		SetGoal( 1000 );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Kill_Grind_1, ACHIEVEMENT_ASW_KILL_GRIND_1, "ASW_KILL_GRIND_1", 5, 1225 );

class CAchievement_Kill_Grind_2 : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetStoreProgressInSteam( true );
		SetGoal( 5000 );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Kill_Grind_2, ACHIEVEMENT_ASW_KILL_GRIND_2, "ASW_KILL_GRIND_2", 5, 1226 );

class CAchievement_Kill_Grind_3 : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetStoreProgressInSteam( true );
		SetGoal( 25000 );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Kill_Grind_3, ACHIEVEMENT_ASW_KILL_GRIND_3, "ASW_KILL_GRIND_3", 5, 1227 );

class CAchievement_Kill_Grind_4 : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetStoreProgressInSteam( true );
		SetGoal( 100000 );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Kill_Grind_4, ACHIEVEMENT_ASW_KILL_GRIND_4, "ASW_KILL_GRIND_4", 5, 1228 );

class CAchievement_Speedrun_Landing_Bay : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Landing_Bay, ACHIEVEMENT_ASW_SPEEDRUN_LANDING_BAY, "ASW_SPEEDRUN_LANDING_BAY", 5, 1230 );

class CAchievement_Speedrun_Descent : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Descent, ACHIEVEMENT_ASW_SPEEDRUN_DESCENT, "ASW_SPEEDRUN_DESCENT", 5, 1231 );

class CAchievement_Speedrun_Deima : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Deima, ACHIEVEMENT_ASW_SPEEDRUN_DEIMA, "ASW_SPEEDRUN_DEIMA", 5, 1232 );

class CAchievement_Speedrun_Rydberg : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Rydberg, ACHIEVEMENT_ASW_SPEEDRUN_RYDBERG, "ASW_SPEEDRUN_RYDBERG", 5, 1233 );

class CAchievement_Speedrun_Residential : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Residential, ACHIEVEMENT_ASW_SPEEDRUN_RESIDENTIAL, "ASW_SPEEDRUN_RESIDENTIAL", 5, 1234 );

class CAchievement_Speedrun_Sewer : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Sewer, ACHIEVEMENT_ASW_SPEEDRUN_SEWER, "ASW_SPEEDRUN_SEWER", 5, 1235 );

class CAchievement_Speedrun_Timor : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_Timor, ACHIEVEMENT_ASW_SPEEDRUN_TIMOR, "ASW_SPEEDRUN_TIMOR", 5, 1236 );

#define DECLARE_RD_SPEEDRUN_ACHIEVEMENT( suffix, order ) \
	class CAchievement_Speedrun_ ## suffix : public CASW_Achievement \
	{ \
		void Init() \
		{ \
			SetFlags( ACH_SAVE_GLOBAL ); \
			SetGoal( 1 ); \
		} \
		/* server fires an event for this achievement, no other code within achievement necessary */ \
	}; \
	DECLARE_ACHIEVEMENT_ORDER( CAchievement_Speedrun_ ## suffix, ACHIEVEMENT_RD_SPEEDRUN_ ## suffix, "RD_SPEEDRUN_" #suffix, 5, order )

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( OCS_STORAGE_FACILITY, 1237 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( OCS_LANDING_BAY_7, 1238 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( OCS_USC_MEDUSA, 1239 );

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( RES_FOREST_ENTRANCE, 1240 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( RES_RESEARCH_7, 1241 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( RES_MINING_CAMP, 1242 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( RES_MINES, 1243 );

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( AREA9800_LZ, 1244 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( AREA9800_PP1, 1245 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( AREA9800_PP2, 1246 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( AREA9800_WL, 1247 );

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TFT_DESERT_OUTPOST, 1248 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TFT_ABANDONED_MAINTENANCE, 1249 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TFT_SPACEPORT, 1250 );

// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( DC_OMEGA_CITY, 1251 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( DC_BREAKING_AN_ENTRY, 1252 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( DC_SEARCH_AND_RESCUE, 1253 );

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_MIDNIGHT_PORT, 1254 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_ROAD_TO_DAWN, 1255 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_ARCTIC_INFILTRATION, 1256 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_AREA9800, 1257 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_COLD_CATWALKS, 1258 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_YANAURUS_MINE, 1259 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_FACTORY, 1260 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_COM_CENTER, 1261 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( TIL_SYNTEK_HOSPITAL, 1262 );

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( LAN_BRIDGE, 1263 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( LAN_SEWER, 1264 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( LAN_MAINTENANCE, 1265 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( LAN_VENT, 1266 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( LAN_COMPLEX, 1267 );

// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( REDUCTION_1, 1268 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( REDUCTION_2, 1269 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( REDUCTION_3, 1270 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( REDUCTION_4, 1271 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( REDUCTION_5, 1272 );
// DECLARE_RD_SPEEDRUN_ACHIEVEMENT( REDUCTION_6, 1273 );

DECLARE_RD_SPEEDRUN_ACHIEVEMENT( PAR_UNEXPECTED_ENCOUNTER, 1274 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( PAR_HOSTILE_PLACES, 1275 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( PAR_CLOSE_CONTACT, 1276 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( PAR_HIGH_TENSION, 1277 );
DECLARE_RD_SPEEDRUN_ACHIEVEMENT( PAR_CRUCIAL_POINT, 1278 );

class CAchievement_Group_Heal : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Group_Heal, ACHIEVEMENT_ASW_GROUP_HEAL, "ASW_GROUP_HEAL", 5, 2300 );

class CAchievement_Group_Damage_Amp : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Group_Damage_Amp, ACHIEVEMENT_ASW_GROUP_DAMAGE_AMP, "ASW_GROUP_DAMAGE_AMP", 5, 2310 );

class CAchievement_Fast_Reloads_In_A_Row : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Fast_Reloads_In_A_Row, ACHIEVEMENT_ASW_FAST_RELOADS_IN_A_ROW, "ASW_FAST_RELOADS_IN_A_ROW", 5, 2320 );

class CAchievement_Fast_Reload : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}

	virtual void ListenForEvents( void )
	{
		ListenForGameEvent( "fast_reload" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( !Q_stricmp( event->GetName(), "fast_reload" ) )
		{
			int nMarineIndex = event->GetInt( "marine" );
			C_ASW_Marine *pMarine = nMarineIndex > 0 ? dynamic_cast<CASW_Marine*>( C_BaseEntity::Instance( nMarineIndex ) ) : NULL;
			if ( pMarine && pMarine->IsInhabited() && pMarine->GetCommander() == C_ASW_Player::GetLocalASWPlayer() )
			{
				IncrementCount();
			}
		}
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Fast_Reload, ACHIEVEMENT_ASW_FAST_RELOAD, "ASW_FAST_RELOAD", 5, 2330 );

class CAchievement_All_Healing : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_All_Healing, ACHIEVEMENT_ASW_ALL_HEALING, "ASW_ALL_HEALING", 5, 2340 );

class CAchievement_Protect_Tech : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Protect_Tech, ACHIEVEMENT_ASW_PROTECT_TECH, "ASW_PROTECT_TECH", 5, 2350 );

class CAchievement_Stun_Grenade : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Stun_Grenade, ACHIEVEMENT_ASW_STUN_GRENADE, "ASW_STUN_GRENADE", 5, 2360 );

class CAchievement_Weld_Door : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	
	virtual void ListenForEvents( void )
	{
		ListenForGameEvent( "door_welded" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( !Q_stricmp( event->GetName(), "door_welded" ) )
		{
			if ( event->GetBool( "inhabited" ) )
			{
				C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
				if ( !pLocalPlayer )
					return;

				int iUserID = event->GetInt( "userid" );
				if ( iUserID != pLocalPlayer->GetUserID() )
					return;

				IncrementCount();
			}
		}
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Weld_Door, ACHIEVEMENT_ASW_WELD_DOOR, "ASW_WELD_DOOR", 5, 2370 );

class CAchievement_Dodge_Ranger_Shot : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Dodge_Ranger_Shot, ACHIEVEMENT_ASW_DODGE_RANGER_SHOT, "ASW_DODGE_RANGER_SHOT", 5, 2380 );

class CAchievement_Boomer_Kill_Early : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Boomer_Kill_Early, ACHIEVEMENT_ASW_BOOMER_KILL_EARLY, "ASW_BOOMER_KILL_EARLY", 5, 2390 );

class CAchievement_Unlock_All_Weapons : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	
	virtual void ListenForEvents( void )
	{
		ListenForGameEvent( "level_up" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( !Q_stricmp( event->GetName(), "level_up" ) )
		{
			if ( event->GetInt( "level" ) >= ASW_NUM_EXPERIENCE_LEVELS )
			{
				IncrementCount();
			}
		}
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Unlock_All_Weapons, ACHIEVEMENT_ASW_UNLOCK_ALL_WEAPONS, "ASW_UNLOCK_ALL_WEAPONS", 5, 2400 );

class CAchievement_Freeze_Grenade : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Freeze_Grenade, ACHIEVEMENT_ASW_FREEZE_GRENADE, "ASW_FREEZE_GRENADE", 5, 2410 );

class CAchievement_Kill_Without_Friendly_Fire : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Kill_Without_Friendly_Fire, ACHIEVEMENT_ASW_KILL_WITHOUT_FRIENDLY_FIRE, "ASW_KILL_WITHOUT_FRIENDLY_FIRE", 5, 2420 );

class CAchievement_Tech_Survives : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Tech_Survives, ACHIEVEMENT_ASW_TECH_SURVIVES, "ASW_TECH_SURVIVES", 5, 2430 );

class CAchievement_Ammo_Resupply : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 10 );
		SetStoreProgressInSteam( true );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Ammo_Resupply, ACHIEVEMENT_ASW_AMMO_RESUPPLY, "ASW_AMMO_RESUPPLY", 5, 2440 );

class CAchievement_Campaign_No_Deaths : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths, ACHIEVEMENT_ASW_CAMPAIGN_NO_DEATHS, "ASW_CAMPAIGN_NO_DEATHS", 5, 1000 );

class CAchievement_Campaign_No_Deaths_OCS : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_OCS, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_OCS, "RD_CAMPAIGN_NO_DEATHS_OCS", 5, 1001 );

class CAchievement_Campaign_No_Deaths_RES : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_RES, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_RES, "RD_CAMPAIGN_NO_DEATHS_RES", 5, 1002 );

class CAchievement_Campaign_No_Deaths_Area9800 : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_Area9800, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_AREA9800, "RD_CAMPAIGN_NO_DEATHS_AREA9800", 5, 1003 );

class CAchievement_Campaign_No_Deaths_TFT : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_TFT, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_TFT, "RD_CAMPAIGN_NO_DEATHS_TFT", 5, 1004 );

// class CAchievement_Campaign_No_Deaths_DC : public CASW_Achievement
// {
// 	void Init()
// 	{
// 		SetFlags( ACH_SAVE_GLOBAL );
// 		SetGoal( 1 );
// 	}
// 	// server fires an event for this achievement, no other code within achievement necessary
// };
// DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_DC, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_DC, "RD_CAMPAIGN_NO_DEATHS_DC", 5, 1005 );

class CAchievement_Campaign_No_Deaths_TIL : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_TIL, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_TIL, "RD_CAMPAIGN_NO_DEATHS_TIL", 5, 1006 );

class CAchievement_Campaign_No_Deaths_LAN : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_LAN, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_LAN, "RD_CAMPAIGN_NO_DEATHS_LAN", 5, 1007 );

// class CAchievement_Campaign_No_Deaths_REDUCTION : public CASW_Achievement
// {
// 	void Init()
// 	{
// 		SetFlags( ACH_SAVE_GLOBAL );
// 		SetGoal( 1 );
// 	}
// 	// server fires an event for this achievement, no other code within achievement necessary
// };
// DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_REDUCTION, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_REDUCTION, "RD_CAMPAIGN_NO_DEATHS_REDUCTION", 5, 1008 );

class CAchievement_Campaign_No_Deaths_PAR : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Campaign_No_Deaths_PAR, ACHIEVEMENT_RD_CAMPAIGN_NO_DEATHS_PAR, "RD_CAMPAIGN_NO_DEATHS_PAR", 5, 1009 );

class CAchievement_Rifle_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_RIFLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Rifle_Kills, ACHIEVEMENT_ASW_RIFLE_KILLS, "ASW_RIFLE_KILLS", 5, 1460 );

class CAchievement_PRifle_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_PRIFLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_PRifle_Kills, ACHIEVEMENT_ASW_PRIFLE_KILLS, "ASW_PRIFLE_KILLS", 5, 1470 );

class CAchievement_Autogun_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_AUTOGUN );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Autogun_Kills, ACHIEVEMENT_ASW_AUTOGUN_KILLS, "ASW_AUTOGUN_KILLS", 5, 1480 );

class CAchievement_Shotgun_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_SHOTGUN );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Shotgun_Kills, ACHIEVEMENT_ASW_SHOTGUN_KILLS, "ASW_SHOTGUN_KILLS", 5, 1490 );

class CAchievement_Vindicator_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_ASSAULT_SHOTGUN );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Vindicator_Kills, ACHIEVEMENT_ASW_VINDICATOR_KILLS, "ASW_VINDICATOR_KILLS", 5, 1500 );

class CAchievement_Pistol_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_PISTOL );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Pistol_Kills, ACHIEVEMENT_ASW_PISTOL_KILLS, "ASW_PISTOL_KILLS", 5, 1510 );

class CAchievement_PDW_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_PDW );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_PDW_Kills, ACHIEVEMENT_ASW_PDW_KILLS, "ASW_PDW_KILLS", 5, 1520 );

class CAchievement_Tesla_Gun_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_TESLA_GUN );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Tesla_Gun_Kills, ACHIEVEMENT_ASW_TESLA_GUN_KILLS, "ASW_TESLA_GUN_KILLS", 5, 1530 );

class CAchievement_Railgun_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_RAILGUN );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Railgun_Kills, ACHIEVEMENT_ASW_RAILGUN_KILLS, "ASW_RAILGUN_KILLS", 5, 1540 );

class CAchievement_Flamer_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_FLAMER );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Flamer_Kills, ACHIEVEMENT_ASW_FLAMER_KILLS, "ASW_FLAMER_KILLS", 5, 1550 );

class CAchievement_Chainsaw_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_CHAINSAW );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Chainsaw_Kills, ACHIEVEMENT_ASW_CHAINSAW_KILLS, "ASW_CHAINSAW_KILLS", 5, 1560 );

class CAchievement_Minigun_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_MINIGUN );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Minigun_Kills, ACHIEVEMENT_ASW_MINIGUN_KILLS, "ASW_MINIGUN_KILLS", 5, 1570 );

class CAchievement_Sniper_Rifle_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_SNIPER_RIFLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Sniper_Rifle_Kills, ACHIEVEMENT_ASW_SNIPER_RIFLE_KILLS, "ASW_SNIPER_RIFLE_KILLS", 5, 1580 );

class CAchievement_Grenade_Launcher_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_GRENADE_LAUNCHER );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Grenade_Launcher_Kills, ACHIEVEMENT_ASW_GRENADE_LAUNCHER_KILLS, "ASW_GRENADE_LAUNCHER_KILLS", 5, 1590 );

class CAchievement_DEagle_Kills : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_DEAGLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_DEagle_Kills, ACHIEVEMENT_ASW_DEAGLE_KILLS, "ASW_DEAGLE_KILLS", 5, 1591 );

class CAchievement_Devastator_Kills : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_DEVASTATOR );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Devastator_Kills, ACHIEVEMENT_ASW_DEVASTATOR_KILLS, "ASW_DEVASTATOR_KILLS", 5, 1592 );

class CAchievement_Combat_Rifle_Kills : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 250 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_COMBAT_RIFLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Combat_Rifle_Kills, ACHIEVEMENT_ASW_COMBAT_RIFLE_KILLS, "ASW_COMBAT_RIFLE_KILLS", 5, 1593 );

class CAchievement_Hornet_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 100 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_HORNET_BARRAGE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Hornet_Kills, ACHIEVEMENT_ASW_HORNET_KILLS, "ASW_HORNET_KILLS", 5, 1600 );

class CAchievement_Laser_Mine_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 100 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_LASER_MINES );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Laser_Mine_Kills, ACHIEVEMENT_ASW_LASER_MINE_KILLS, "ASW_LASER_MINE_KILLS", 5, 1610 );

class CAchievement_Mine_Kills : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 100 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_FIRE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Mine_Kills, ACHIEVEMENT_ASW_MINE_KILLS, "ASW_MINE_KILLS", 5, 1620 );

class CAchievement_GasGrenade_Kills : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 500 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_GAS_GRENADE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_GasGrenade_Kills, ACHIEVEMENT_RD_GAS_GRENADE_KILLS, "RD_GAS_GRENADE_KILLS", 5, 1630 );

class CAchievement_HeavyRifle_Kills : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 500 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_HEAVY_RIFLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_HeavyRifle_Kills, ACHIEVEMENT_RD_HEAVY_RIFLE_KILLS, "RD_HEAVY_RIFLE_KILLS", 5, 1640 );

class CAchievement_MedicalSMG_Kills : public CASW_Achievement
{
	void Init()
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_ALIEN_DEATH_EVENTS );
		SetGoal( 500 );
		SetStoreProgressInSteam( true );
		SetWeaponClassFilter( CLASS_ASW_MEDRIFLE );
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_MedicalSMG_Kills, ACHIEVEMENT_RD_MEDICAL_SMG_KILLS, "RD_MEDICAL_SMG_KILLS", 5, 1650 );

class CAchievement_Mission_No_Deaths : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	// server fires an event for this achievement, no other code within achievement necessary
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Mission_No_Deaths, ACHIEVEMENT_ASW_MISSION_NO_DEATHS, "ASW_MISSION_NO_DEATHS", 5, 999 );

class CAchievement_Hardcore : public CASW_Achievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL );
		SetGoal( 1 );
	}
	
	virtual void ListenForEvents( void )
	{
		ListenForGameEvent( "mission_success" );
	}

	void FireGameEvent_Internal( IGameEvent *event )
	{
		if ( !Q_stricmp( event->GetName(), "mission_success" ) && ASWGameRules() && ASWGameRules()->GetSkillLevel() >= 5
			&& CAlienSwarm::IsHardcoreFF() && CAlienSwarm::IsOnslaught() )
		{
			if ( LocalPlayerWasSpectating() )
				return;

			// reactivedrop: if player's marine didn't survive, don't award Hardcore achievement
			C_ASW_Player *pLocalPlayer = static_cast< C_ASW_Player* >( C_BasePlayer::GetLocalPlayer() );
			if ( !pLocalPlayer )
				return;

			if ( !pLocalPlayer->GetMarine() || pLocalPlayer->GetMarine()->GetHealth() <= 0 )
				return;

			const char *szMapName = event->GetString( "strMapName" );
			for ( int i = 0; i < NELEMS( g_szAchievementMapNamesAllCoop ); i++ )
			{
				if ( !Q_stricmp( szMapName, g_szAchievementMapNamesAllCoop[i] ) )
				{
					IncrementCount();
					break;
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT_ORDER( CAchievement_Hardcore, ACHIEVEMENT_ASW_HARDCORE, "ASW_HARDCORE", 5, 3185 );

CON_COMMAND_F( rd_achievement_order, "", FCVAR_HIDDEN )
{
	Msg( "package reactivedropstats\n\nvar achievementOrder = []string{\n" );
	int iCount = ASWAchievementManager()->GetAchievementCount();
	for ( int i = 0; i < iCount; i++ )
	{
		Msg( "\t\"%s\",\n", ASWAchievementManager()->GetAchievementByDisplayOrder( i, STEAM_PLAYER_SLOT )->GetName() );
	}
	Msg( "}\n" );
}
#endif