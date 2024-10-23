// This is set to 1 if either atl or etv are 1
extern cvar_t *esp;

// Discrete game modes
extern cvar_t *atl;
extern cvar_t *etv;

#define IS_LEADER(ent) (teams[(ent)->client->resp.team].leader == (ent))
#define	HAVE_LEADER(teamNum) (teams[(teamNum)].leader)
#define MAX_ESP_STRLEN 32

// Game default settings
#define ESP_DEFAULT_RESPAWN_TIME	10
#define ESP_RED_SKIN				"male/ctf_r"
#define ESP_BLUE_SKIN				"male/ctf_b"
#define ESP_GREEN_SKIN				"male/ctf_g"
#define ESP_RED_LEADER_SKIN			"male/babarracuda"
#define ESP_BLUE_LEADER_SKIN		"male/blues"
#define ESP_GREEN_LEADER_SKIN		"male/hulk2"
#define ESP_RED_TEAM				"The B-Team"
#define ESP_BLUE_TEAM				"Mall Cops"
#define ESP_GREEN_TEAM				"Mop-up Crew"
#define ESP_RED_LEADER_NAME			"B. A. Barracuda"
#define ESP_BLUE_LEADER_NAME		"Frank the Cop"
#define ESP_GREEN_LEADER_NAME		"The Incredible Chulk"

int EspCapturePointOwner( edict_t *flag );
void EspRemember( const edict_t *ent, const gitem_t *item );
qboolean EspLoadConfig( const char *mapname );
void EspSetupStatusbar( void );
void SetEspStats( edict_t *ent );

typedef enum
{
  ESP_STATE_START,
  ESP_STATE_PLAYING
} espstate_t;

typedef enum
{
  ESPMODE_ATL,
  ESPMODE_ETV,
  ESPMODE_MAX
} espmode_t;

#define ESPMODE_ATL_NAME			"Assassinate the Leader"
#define ESPMODE_ETV_NAME			"Escort the VIP"

#define ESPMODE_ATL_SNAME			"atl"
#define ESPMODE_ETV_SNAME			"etv"

typedef struct espsettings_s
{
	char author[MAX_ESP_STRLEN*3];
	char name[MAX_ESP_STRLEN];
	espmode_t esp_mode;
	edict_t *custom_spawns[TEAM_TOP][MAX_SPAWNS];
	qboolean custom_skins;
	int halftime;
	qboolean escortcap;
	char target_name[MAX_ESP_STRLEN];
	edict_t *capturepoint;
	edict_t *lastkilledleader;
	qboolean esp_live_round;
	edict_t *round_spawnpoint[TEAM_TOP];
} espsettings_t;

extern espsettings_t espsettings;

extern gitem_t *team_flag[TEAM_TOP];

typedef enum {
    ESP_HALFTIME_WARNING,
    ESP_LEADER_DIED,
    MESSAGE_THREE,
    // Add more messages as needed
    NUM_MESSAGES
} EspTimedMessageIndex;

#define DF_ESP_FORCEJOIN	131072

// Team score bonuses
#define TS_TEAM_BONUS                      1   // this is the bonus point teams get for fragging enemy leader

// Individual score bonuses
#define ESP_LEADER_FRAG_BONUS   	        5	// points player receives for fragging enemy leader
#define ESP_LEADER_CAPTURE_BONUS            5  // points player receives if they are leader and they successfully touch escort marker
#define ESP_LEADER_ESCORT_BONUS				2  // points player receives if they are not the leader and they are near the leader when point is captured
#define ESP_LEADER_DANGER_PROTECT_BONUS 	2	// bonus for fragging someone who has recently hurt your leader
#define ESP_LEADER_PROTECT_BONUS    		1	// bonus for fragging someone while either you or your target are near your leader
#define ESP_FLAG_DEFENSE_BONUS    		1	// bonus for fragging someone while either you or your target are near your flag
#define ESP_LEADER_HARASS_BONUS             2   // points for attacking defenders of the leader

// Score bonus critieria/limitations
#define ESP_TARGET_PROTECT_RADIUS   		400	// the radius around an object being defended where a target will be worth extra frags
#define ESP_ATTACKER_PROTECT_RADIUS 		400	// the radius around an object being defended where an attacker will get extra frags when making kills
#define ESP_ATTACKER_HARASS_RADIUS			400	// the radius around an object being defended where an attacker will get extra frags when assaulting the leader
#define ESP_BONUS_COOLDOWN					500	// the number of frames after a bonus is awarded before another bonus can be awarded	

edict_t* EspGetLeader(int teamNum);
void EspForceEspionage(espmode_t espmode);
void EspSetTeamSpawns(int, char *);
int EspGetRespawnTime(edict_t *ent);

void Cmd_Volunteer_f(edict_t * ent);
qboolean EspSetLeader(int teamNum, edict_t *ent);
qboolean AllTeamsHaveLeaders(void);
void EspLeaderLeftTeam( edict_t *ent );
void EspPunishment(int teamNum);
void EspRespawnPlayer(edict_t *ent);

edict_t *SelectEspSpawnPoint (edict_t * ent);
int EspReportLeaderDeath(edict_t *ent);
int EspSpawnpointCount(int teamNum);
void EspResetCapturePoint( void );
void GenerateMedKit(qboolean instant);

void EspTouchCapturePoint( edict_t *marker, edict_t *player, cplane_t *plane, csurface_t *surf );
void EspScoreBonuses(edict_t * targ, edict_t * attacker);

qboolean EspCheckRules (void);
qboolean HasFlag (edict_t * ent);

void EspSkinCheck(void);
void ResetPlayers (void);
void GetEspScores(int *t1score, int *t2score);
void EspCapReward(edict_t *);
void EspAnnounceDetails( qboolean timewarning );
qboolean EspChooseRandomLeader(int teamNum);
qboolean EspLeaderCheck(void);
void EspEndOfRoundCleanup(void);
void EspRespawnLCA(edict_t *ent);
void EspCleanUp(void);
void EspDebug(void);
extern qboolean esp_punishment_phase;
extern int esp_spawnpoint_index[TEAM_TOP];
extern edict_t* chosenSpawnpoint[TEAM_TOP];
extern edict_t* etvTarget;