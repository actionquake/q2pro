#ifndef _BOTLIB_H
#define _BOTLIB_H

#define BOT_AAS_VERSION 1

#define BOT_NAV_VERSION 2
#define BOT_NAV_VERSION_1 1
#define BOT_NAV_VERSION_2 2
#define BOT_NAV_VERSION_MAX BOT_NAV_VERSION_2

// Bot move states
typedef enum
{
	BOT_MOVE_STATE_NONE = 0,    // Bot is booting up :-)
	BOT_MOVE_STATE_NAV = 1,         // Getting a navigational path
	//BOT_MOVE_STATE_NAV_NEXT,  // Getting next navigational path (if any)
	BOT_MOVE_STATE_MOVE = 3,        // Standard movement
	BOT_MOVE_STATE_WANDER,      // No navigation and no movement, try wandering around
	BOT_MOVE_STATE_STAND,       // Stand still and hold a position
	BOT_MOVE_STATE_FLEE,        // Running away from enemy force
	BOT_MOVE_STATE_COVER        // Under fire, take cover
} bot_move_state_t;


typedef struct bot_connections_s
{
	qboolean tried_adding_prev_bots; // Flag if we tried to add any bots from a previous map. Only called once per map.
	qboolean auto_balance_bots; // If the spawn manager should try to auto balance bots on teams
	qboolean scale_up; // If the manager should scale up the bots
	qboolean scale_dn; // If the manager should scale down the bots

	int total_bots;
	int total_humans; // Total humans in server
	int total_humans_playing; // Total humans in server and playing

	int total_team1; // Bots & Humans
	int total_team2;
	int total_team3;

	int spec_bots; // TP and DM
	int team1_bots; // TP and DM
	int team2_bots; // TP and 3T
	int team3_bots; // TP and 3T

	
	int spec_humans; // TP and DM
	int team1_humans; // TP and DM
	int team2_humans; // TP and 3T
	int team3_humans; // TP and 3T

	int desire_bots;	// How many bots we desire
	int desire_team1;
	int desire_team2;
	int desire_team3;
} bot_connections_t;
extern bot_connections_t bot_connections;

//the bot input, will be converted to a usercmd_t
typedef struct bot_input_s
{
	float thinktime;		//time since last output (in seconds)
	vec3_t dir;				//movement direction
	float speed;			//speed in the range [0, 400]
	vec3_t viewangles;		//the view angles
	int actionflags;		//one of the ACTION_? flags
	int radioflags;			//one of the aq2 radio flags (i.e. RADIO_REPORTIN)
	int weapon;				//weapon to use
	vec3_t look_at;			//location to look at
	int look_at_time;	//time spent looking at location
} bot_input_t;





// ===========================================================================
// botlib_ai.c
// ===========================================================================
void BOTLIB_Init(edict_t* self); // Initializing... HAL9000 is online.
void BOTLIB_Think(edict_t* self); // Thinking... I'm sorry Rekkie, I can't do that.
void BOTLIB_BotInputToUserCommand(edict_t* ent, bot_input_t* bi, usercmd_t* ucmd, vec3_t delta_angles, int time); // Translates bot input to actual Q2 movement calls
qboolean BOTLIB_Infront(edict_t* self, edict_t* other, float amount); // I see everything... in front of me
qboolean BOTLIB_OriginInfront(edict_t* self, vec3_t origin, float amount);
qboolean BOTLIB_MovingToward(edict_t* self, vec3_t origin, float amount);
qboolean BOTLIB_CanMove(edict_t* self, int direction); // Can bot move in direction
void BOTLIB_TouchingLadder(edict_t* self);
float BOTLIB_ThrowingKnifePitch(edict_t* self); // Get pitch required to reach knife throwing target
void BOTLIB_PlayerNoise(edict_t* who, vec3_t where, int type);
int BOTLIB_GetEquipment(edict_t* self); // Does bot need weapons, items, ammo? If so
//void BOTLIB_CheckCurrentWeapon(edict_t* self); //rekkie -- Check to make sure we're using a primary weapon where possible
void BOTLIB_GetWeaponsAndAmmo(edict_t* self); //rekkie -- Locate and pickup a primary weapon if we need one
short BOTLIB_FindVisibleAllies(edict_t* self);
qboolean BOTLIB_FindEnemy(edict_t* self);
void BOTLIB_PickLongRangeGoal(edict_t* self);

typedef struct botlib_noises_s
{
	edict_t* owner[MAX_CLIENTS];	// The ent who made the noise

	// Noise timers
	int self_time[MAX_CLIENTS];
	int weapon_time[MAX_CLIENTS];
	int impact_time[MAX_CLIENTS];

	// Location of noise
	vec3_t self_origin[MAX_CLIENTS];
	vec3_t weapon_origin[MAX_CLIENTS];
	vec3_t impact_origin[MAX_CLIENTS];

} botlib_noises_t;
extern botlib_noises_t botlib_noises;

// Actionable flags
#define ACTION_NONE				BIT(0)	// No action taken
#define ACTION_ATTACK			BIT(1)	// Tap the attack button
#define ACTION_USE				BIT(2)
#define ACTION_RESPAWN			BIT(3)
#define ACTION_JUMP				BIT(4)	// Small jumps - tapping the jump button
#define ACTION_MOVEUP			BIT(5)
#define ACTION_CROUCH			BIT(7)
#define ACTION_MOVEDOWN			BIT(8)
#define ACTION_MOVEFORWARD		BIT(9)
#define ACTION_MOVEBACK			BIT(11)
#define ACTION_MOVELEFT			BIT(12)
#define ACTION_MOVERIGHT		BIT(13)
#define ACTION_BOXJUMP			BIT(15)	// Higher jumps
#define ACTION_TALK				BIT(16)
#define ACTION_GESTURE			BIT(17)
#define ACTION_WALK				BIT(19)
#define ACTION_AFFIRMATIVE		BIT(20)
#define ACTION_NEGATIVE			BIT(21)
#define ACTION_GETFLAG			BIT(23)
#define ACTION_GUARDBASE		BIT(24)
#define ACTION_PATROL			BIT(25)
#define ACTION_FOLLOWME			BIT(27)
#define ACTION_HOLDJUMP			BIT(28)	// Hold the jump button and release when touching ground
#define ACTION_HOLDPOS			BIT(29)	// Hold position
#define ACTION_JUMPPAD			BIT(30)


// ===========================================================================
// botlib_ctf.c
// ===========================================================================
qboolean BOTLIB_Carrying_Flag(edict_t* self);
int BOTLIB_InterceptFlagCarrier(edict_t* self, int team, float distance);
qboolean BOTLIB_IsFlagHome(edict_t* self, int team, float distance);
qboolean BOTLIB_IsFlagDropped(edict_t* self, int team, float distance);
void BOTLIB_Update_Flags_Status(void);
int BOTLIB_CTF_Get_Flag_Node(edict_t* ent);
int BOTLIB_NearestFlag(edict_t* self);
float BOTLIB_DistanceToFlag(edict_t* self, int flagType);
void BOTLIB_CTF_Goals(edict_t* self);

// ===========================================================================
// botlib_esp.c
// ===========================================================================
int BOTLIB_ESPGetTargetNode(edict_t *ent, edict_t* leader);
float BOTLIB_DistanceToLeader(edict_t* self, edict_t* leader);
int BOTLIB_FindMyLeaderNode(edict_t* self);
int BOTLIB_FindEnemyLeaderNode(edict_t* self, int teamNum);
void BOTLIB_ESP_Goals(edict_t* self);

typedef struct ctf_status_s
{
	edict_t* flag1; // Red flag
	edict_t* flag2; // Blue flag
	int flag1_home_node; // The node where the red flag initially spawns
	int flag2_home_node; // The node where the blue flag initially spawns
	int flag1_curr_node; // The node where the red flag currently resides
	int flag2_curr_node; // The node where the blue flag currently resides
	qboolean flag1_is_home; // If the red flag is home
	qboolean flag2_is_home; // If the blue flag is home
	qboolean flag1_is_dropped; // If the red flag was dropped
	qboolean flag2_is_dropped; // If the blue flag was dropped
	qboolean flag1_is_carried; // If the red flag is being carried
	qboolean flag2_is_carried; // If the blue flag is being carried
	edict_t* player_has_flag1; // If any ent has the red flag
	edict_t* player_has_flag2; // If any ent has the blue flag
	float team1_carrier_dist_to_home; // How close the red team carrier is to the home red flag node
	float team2_carrier_dist_to_home; // How close the blue team carrier is to the home blue flag node
} ctf_status_t;
extern ctf_status_t bot_ctf_status;

// ===========================================================================
// botlib_esp.c
// ===========================================================================
int BOTLIB_ESP_Get_Target_Node(edict_t* ent);
int BOTLIB_InterceptLeader(edict_t* self, int team, float distance);

typedef struct esp_status_s
{
	edict_t* esp_target; // ETV: target edict
	int esp_target_node; // ETV: the closest node where the target resides
	qboolean esp_target_captured; // ETV: if we've already captured the target this round
	float esp_leader_dist_to_target; // ETV: how close team1 leader is from the target
	int esp_team1_leader_area; // ATL: Approximate area where team1 leader is (helps bots find the bad guy)
	int esp_team2_leader_area; // ATL: Approximate area where team1 leader is (helps bots find the bad guy)
	int esp_team3_leader_area; // ATL: Approximate area where team1 leader is (helps bots find the bad guy)
} esp_status_t;
extern esp_status_t bot_esp_status;

// Get flag, retrieve flag, intercept flag carrier, etc.
typedef enum
{
	BOT_ESP_STATE_NONE, // No / Normal state (both teams actively attacking)
	BOT_ESP_COVER_TEAM_LEADER, // Cover the team leader
	BOT_ESP_ATTACK_TARGET, // Attack the target (enemy leader or target area)
	BOT_ESP_DEFEND_TARGET, // Defend the target (team leader or target area)
	BOT_ESP_RETREAT, // Retreat from the target (enemy leader or target area)
	BOT_ESP_WANDER, // Wander around, find enemies, equipment, etc
} bot_esp_state_t;

// Get flag, retrieve flag, intercept flag carrier, etc.
typedef enum
{
	BOT_CTF_STATE_NONE, // No state
	BOT_CTF_STATE_GET_ENEMY_FLAG, // Get enemy flag sitting in its home location
	BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG, // Get dropped enemy flag
	BOT_CTF_STATE_CAPTURE_ENEMY_FLAG, // Got flag, now capture it
	BOT_CTF_STATE_ATTACK_ENEMY_CARRIER, // Enemy has our flag, go after them
	BOT_CTF_STATE_RETURN_TEAM_FLAG, // Our team flag is on the ground, go return it
	BOT_CTF_STATE_COVER_FLAG_CARRIER, // We have the enemy flag, cover the bot who has it
	BOT_CTF_STATE_FORCE_MOVE_TO_FLAG, // Special case. Bot is forced to move in direction of flag.
	BOT_CTF_STATE_GET_DROPPED_ITEMS // Gets dropped weapons, items, etc from ground

} bot_ctf_state_t;

// ===========================================================================
// botlib_cmd.c
// ===========================================================================
qboolean BOTLIB_SV_Cmds(void);	// Server commands
qboolean BOTLIB_Commands(edict_t* ent); // Client commands


// ===========================================================================
// botlib_communication.c
// ===========================================================================
typedef enum
{
	CHAT_WELCOME,
	CHAT_KILLED,
	CHAT_INSULTS,
	CHAT_GOODBYE,
	CHAT_VICTORY,
	CHAT_RAGE,
} bot_chat_types_t;

void UpdateBotChat(void);
void BOTLIB_Wave(edict_t* ent, int type);
void BOTLIB_PrecacheRadioSounds(void);
void BOTLIB_AddRadioMsg(radio_t* radio, int sndIndex, int len, edict_t* from_player);
void BOTLIB_Radio(edict_t* self, usercmd_t* ucmd);
void BOTLIB_Chat(edict_t *bot, bot_chat_types_t chattype);
void BOTLIB_Say(edict_t* ent, char* pMsg, qboolean team_message);

// Wave (gesture) types
#define WAVE_FLIPOFF			1
#define WAVE_SALUTE				2
#define WAVE_TAUNT				3
#define WAVE_WAVE				4
#define WAVE_POINT				5

// Radio flags
#define RADIO_1					BIT(0)
#define RADIO_2					BIT(1)
#define RADIO_3					BIT(2)
#define RADIO_4					BIT(3)
#define RADIO_5					BIT(4)
#define RADIO_6					BIT(5)
#define RADIO_7					BIT(6)
#define RADIO_8					BIT(7)
#define RADIO_9					BIT(8)
#define RADIO_10				BIT(9)
#define RADIO_BACK				BIT(10)
#define RADIO_COVER				BIT(11)
#define RADIO_DOWN				BIT(12)
#define RADIO_ENEMY_DOWN		BIT(13)
#define RADIO_ENEMY_SIGHTED		BIT(14)
#define RADIO_FORWARD			BIT(15)
#define RADIO_GO				BIT(16)
#define RADIO_IM_HIT			BIT(17)
#define RADIO_LEFT				BIT(18)
#define RADIO_REPORTIN			BIT(19)
#define RADIO_RIGHT				BIT(20)
#define RADIO_TAKING_FIRE		BIT(21)
#define RADIO_TEAMMATE_DOWN		BIT(22)
#define RADIO_TEAM_REPORT_IN	BIT(23)
#define RADIO_UP				BIT(24)

// Each of the possible radio messages and their length
typedef struct bot_radio_msg_s
{
	char* msg;			// the msg name 
	int length;			// length in server frames (ie tenths of a second), rounded up
	int sndIndex;
} bot_radio_msg_t;

static bot_radio_msg_t bot_male_radio_msgs[] = {
  {"1", 6, 0},
  {"2", 6, 0},
  {"3", 8, 0},
  {"4", 7, 0},
  {"5", 8, 0},
  {"6", 9, 0},
  {"7", 8, 0},
  {"8", 7, 0},
  {"9", 7, 0},
  {"10", 6, 0},
  {"back", 6, 0},
  {"cover", 7, 0},
  {"down", 13, 0},
  {"enemyd", 10, 0},
  {"enemys", 9, 0},
  {"forward", 6, 0},
  {"go", 6, 0},
  {"im_hit", 7, 0},
  {"left", 7, 0},
  {"reportin", 9, 0},
  {"right", 6, 0},
  {"taking_f", 22, 0},
  {"teamdown", 13, 0},
  {"treport", 12, 0},
  {"up", 4, 0}
};

static bot_radio_msg_t bot_female_radio_msgs[] = {
  {"1", 5, 0},
  {"2", 5, 0},
  {"3", 5, 0},
  {"4", 5, 0},
  {"5", 5, 0},
  {"6", 8, 0},
  {"7", 7, 0},
  {"8", 5, 0},
  {"9", 5, 0},
  {"10", 5, 0},
  {"back", 6, 0},
  {"cover", 5, 0},
  {"down", 6, 0},
  {"enemyd", 9, 0},
  {"enemys", 9, 0},
  {"forward", 8, 0},
  {"go", 6, 0},
  {"im_hit", 7, 0},
  {"left", 8, 0},
  {"reportin", 9, 0},
  {"right", 5, 0},
  {"taking_f", 22, 0},
  {"teamdown", 10, 0},
  {"treport", 12, 0},
  {"up", 6, 0}
};
static const int bot_numMaleSnds = (sizeof(bot_male_radio_msgs) / sizeof(bot_male_radio_msgs[0]));
static const int bot_numFemaleSnds = (sizeof(bot_female_radio_msgs) / sizeof(bot_female_radio_msgs[0]));


// ===========================================================================
// botlib_items.c
// ===========================================================================


// ===========================================================================
// botlib_math.c
// ===========================================================================
qboolean BOTLIB_VectorCompare(vec3_t v1, vec3_t v2, const float epsilon);
bool	 BOTLIB_BoxIntersection(vec3_t amins, vec3_t amaxs, vec3_t bmins, vec3_t bmaxs);
qboolean BOTLIB_IsAimingAt(edict_t* self, edict_t* other);

// ===========================================================================
// botlib_movement.c
// ===========================================================================
qboolean BOTLIB_DoParabolaJump(edict_t* self, vec3_t targetpoint);
void BOTLIB_Look(edict_t* self, usercmd_t* ucmd);


// ===========================================================================
// botlib_nav.c
// ===========================================================================
// Find all possible paths -- s

#define MAX_NAV_AREAS 32
#define MAX_NAV_AREAS_EDGES 64
#define MAX_NAV_AREAS_PATHS 512 //512
#define MAX_NAV_AREAS_NODES 4096
extern int DFS_area_nodes[MAX_NAV_AREAS][MAX_NAV_AREAS_NODES]; // Area nodes - [(32 * 1024) * 4 bytes = 132k]
extern int DFS_area_edges[MAX_NAV_AREAS][MAX_NAV_AREAS_EDGES]; // Area edge nodes (area edges that connect to other areas) - [(32 * 64) * 4 bytes = 8k]



typedef struct {

	// Areas
	int total_areas; // Total number of areas
	//int area_nodes[MAX_NAV_AREAS][MAX_NAV_AREAS_NODES]; // A collection of all nodes in an area
	int** area_nodes;

	int adjacency_matrix[MAX_NAV_AREAS][MAX_NAV_AREAS]; // Store adjacency matrix of areas

	// Depth-first Search
	bool dfs_visited[MAX_NAV_AREAS]; // Visited areas
	int dfs_len; // 
	int dfs_path[MAX_NAV_AREAS]; // Temp path
	int dfs_paths[MAX_NAV_AREAS_PATHS][MAX_NAV_AREAS]; // Full path array [(512 * 32) * 4 bytes = 64k]
	int dfs_path_count; // Total unique paths found



} nav_area_t;
extern nav_area_t nav_area;

void BOTLIB_MallocAreaNodes(void);
void BOTLIB_FreeAreaNodes(void);

void BOTLIB_InitAreaNodes(void); // Init area nodes to INVALID
void BOTLIB_InitAreaConnections(void); // Init and make area connections
qboolean BOTLIB_CanGotoNode(edict_t* self, int goal_node, qboolean path_randomization); // Supports new and old pathing methods
qboolean BOTLIB_CanVisitAreaNode(edict_t* self, int goal_node); // Checks if possible to traverse from current node to goal_node
void BOTLIB_GetAreaPath(edict_t* self, int goal_node); // Sets a path from area-to-area
qboolean BOTLIB_GetNextAreaNode(edict_t* self);
void BOTLIB_UpdateAllAreaEdges(void); // Stores all the nodes within an area that connect to an external area (edge nodes)
int BOTLIB_GetRandomEdgeConnection(int area_1, int area_2); // Returns a random edge node that is inside the second area (so the bot traverses just inside the next area)
void BOTLIB_DepthFirstSearch(edict_t* self, int current, int destination); // Finds ALL possible paths from current to destination using the Depth-First Search (DFS) algorithm
void BOTLIB_RandomizeAreaColors(void); // Randomize area colors
void BOTLIB_AutoArea(edict_t* self);
qboolean BOTLIB_DijkstraAreaPath(edict_t* ent, int from, int to, qboolean path_randomization, int area, qboolean build_new_path);
qboolean BOTLIB_DijkstraPath(edict_t* ent, int from, int to, qboolean path_randomization);
// Find all possible paths -- e

// Forward declaration
struct botlib_sll_t;

// // Node data for our linked list
// typedef struct {
// 	struct botlib_sll_t* next;	// Next node
// 	int	node;			// Node number
// 	float cost;			// Cost from previous node to this node
// 	int parent_node;	//rekkie -- the node we came from
// } botlib_sll_nodes_t;

// Copilot suggestion
typedef struct botlib_sll_nodes_t {
    struct botlib_sll_nodes_t* next;	// Next node
    int	node;			// Node number
    float cost;			// Cost from previous node to this node
    int parent_node;	//rekkie -- the node we came from
} botlib_sll_nodes_t;

// A double-ended singly linked list
// This allows data to be added to the front or back of the list.
typedef struct {
	botlib_sll_nodes_t* head; // Front
	botlib_sll_nodes_t* tail; // Back
} botlib_sll_t;
//}	ltklist_t;

// ===========================================================================
// botlib_nodes.c
// ===========================================================================
void BOTLIB_LinkNodesNearbyNode(edict_t* ent, int from);
qboolean BOTLIB_CanVisitNode(edict_t* self, int goal_node, qboolean path_randomization, int area, qboolean build_new_path);

void BOTLIB_GroupConnectedNodeArea(edict_t* self, int start_node);
void BOTLIB_SetAllNodeNormals(void);

// ===========================================================================
// botlib_spawn.c
// ===========================================================================
qboolean BOTLIB_SaveBotsFromPreviousMap(void);
qboolean BOTLIB_AddBotsFromPreviousMap(float percent);
void BOTLIB_RandomizeTeamNames(edict_t* bot); // Randomize team names in teamplay
void BOTLIB_RandomizeTeamSkins(edict_t* bot); // Randomize team skins in teamplay
int BOTLIB_RandomSkin(edict_t* bot, char* skin, int force_gender); // Pick a random skin and return the skin's gender
edict_t* BOTLIB_SpawnBot(int team, int force_gender, char* force_name, char* force_skin); // Spawn a bot
void BOTLIB_PutClientInServer(edict_t* bot, qboolean respawn, int team);
void BOTLIB_RemoveBot(char* name); // Remove bot by name or 'ALL' to remove all bots
void BOTLIB_RemoveTeamplayBot(int team); // Remove bot from team
void BOTLIB_ChangeBotTeam(int from_team, int to_team); // Change a bot [from team] ==> [to team]
void BOTLIB_CheckBotRules(void); // Adding/Removing/Auto-balance bots
void BOTLIB_SetUserinfo(edict_t* bot, const int team, int force_gender, char* force_name, char* force_skin);

// ===========================================================================
// botlib_spawnpoints.c
// ===========================================================================

// Custom spawnpoints
typedef struct {
	qboolean inuse;		// If the spawn point is in use, or has been deleted and is now free to replace
	vec3_t origin;		// Spawn point location
	vec3_t angles;		// Spawn point direction
} dc_sp_t;
extern dc_sp_t* dc_sp;
extern int dc_sp_count; // Total spawn points
extern qboolean dc_sp_edit; // If the spawn points have been made visible for editing
#define DC_SP_LIMIT 255 // Maximum spawn points
#define DC_SP_VERSION 1 // Version of spawn point file
void DC_Init_Spawnpoints(void); // Initialise spawn points
void DC_Free_Spawnpoints(void); // Free the spawn points
void DC_Add_Spawnpoint(edict_t* ent); // Add a spawn point at the player location
void DC_Remove_Spawnpoint(edict_t* self); // Remove a spawn point at the player location
void DC_Get_Map_Spawnpoints(void); // Find and add all the map spawn points to the dc_sp[] array
void BOTLIB_Show_Spawnpoints(void); // Find and display (as visible ents) all spawn points (map and custom)
void DC_Save_Spawnpoints(void);	// Save the spawn points
void DC_Load_Spawnpoints(void);	// Load the spawn points


// ===========================================================================
// botlib_weapons.c
// ===========================================================================
int BOTLIB_CheckWeaponLoadedAmmo(edict_t* self, int item_num);
qboolean BOTLIB_ChangeSpecialWeapon(edict_t* ent, int item_num);
void BOTLIB_ReadyWeapon(edict_t* self);
qboolean BOTLIB_ChooseWeapon(edict_t* self);
void BOTLIB_Reload(edict_t* self);
qboolean BOTLIB_SniperZoom(edict_t* self);
//rekkie -- collecting weapons, items, ammo -- s
qboolean BOTLIB_CanTouchItem(edict_t* self, int typeNum);
qboolean BOTLIB_Need_Weapons(edict_t* self, int* primary_weapon, int* secondary_weapon);
qboolean BOTLIB_Need_Grenades(edict_t* self);
qboolean BOTLIB_Need_Knives(edict_t* self);
qboolean BOTLIB_Need_Dual_MK23(edict_t* self);
int BOTLIB_Need_Ammo(edict_t* self, int* primary_weapon, int* secondary_weapon);
int BOTLIB_Need_Special(edict_t* self, int* primary_weapon, int* secondary_weapon);
int BOTLIB_LocateFloorItem(edict_t* self, int* items_to_get, int items_counter);
int BOTLIB_GetEquipment(edict_t* self);
//rekkie -- collecting weapons, items, ammo -- e

// ===========================================================================
// botlib_utils.c and botlib_win.c
// ===========================================================================
void seed_random_number_generator(void);
void BOTLIB_SKILL_Init(edict_t* bot);
qboolean BOTLIB_SkillChance(float skilltype);
float BOTLIB_SkillMultiplier(float skill_level, bool increase_with_skill);
float BOTLIB_SKILL_Reaction(float reaction_skill);
void BOTLIB_Debug(const char *debugmsg, ...);

// ===========================================================================
// botlib_personality.c
// ===========================================================================
// Copy of bot_personality_t struct
// Most float values here are between -1 and 1
typedef struct temp_bot_personality_s
{
	float weapon_prefs[10];   				//-1 = Will never choose, 1 = Will always choose
	float item_prefs[6];       				//-1 = Will never choose, 1 = Will always choose
	float map_prefs;                        //-1 = Hate, 0 = Neutral, 1 = Love
	float combat_demeanor;                  //-1 = Timid | 1 = Aggressive
	float chat_demeanor;                    //-1 = Quiet | 1 = Chatty
	int leave_percent;                      // Percentage calculated that the bot will leave the map.  Recalculated/increases every time the bot dies.
	char* skin_pref;                        // Skin preference, if DM mode
	int pId;                                // Personality id (used as an index)
	qboolean isActive;                      // Determines if bot is active in game or not (avoid dupes)
} temp_bot_personality_t;

typedef struct {
    char* name;
    temp_bot_personality_t personality;
} temp_bot_mapping_t;
extern temp_bot_mapping_t bot_mappings[100];

void BOTLIB_PersonalityFile(void);
temp_bot_mapping_t* BOTLIB_LoadPersonalities(const char* filename);
void DeactivateBotPersonality(void);
qboolean BOTLIB_SetPersonality(edict_t* bot, int team, int force_gender);
void BOTLIB_LoadBotPersonality(edict_t* self);
void BOTLIB_FreeBotPersonality(edict_t* bot);
qboolean BotRageQuit(edict_t* self, qboolean frag_or_death);
qboolean BOTLIB_DoIChat(edict_t* bot);
void BOTLIB_BotPersonalityChooseWeapon(edict_t* bot);
void BOTLIB_BotPersonalityChooseItem(edict_t* bot);
void BOTLIB_BotPersonalityChooseItemKit(edict_t* bot);
qboolean BOTLIB_SpawnRush(edict_t* bot);

extern int loaded_bot_personalities;
extern int bot_personality_index;
#endif // _BOTLIB_H