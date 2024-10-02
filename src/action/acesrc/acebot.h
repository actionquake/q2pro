///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LTK2/src/acesrc/acebot.h 9     29/02/00 11:20 Riever $
 *
 * $Log: /LTK2/src/acesrc/acebot.h $
 * 
 * 9     29/02/00 11:20 Riever
 * ChooseWeapon changed to qboolean
 * 
 * 8     27/02/00 13:07 Riever
 * Changed enums to defines for better compatibility.
 * 
 * 7     24/02/00 3:07 Riever
 * BOTUT_Cmd_Say_f proto added
 * 
 * 6     23/02/00 17:24 Riever
 * Added support for 'sv shownodes on/off'
 * Enabled creation of nodes for ALL doors. (Stage 1 of new method)
 * 
 * 5     21/02/00 23:45 Riever
 * Added GT_ goal selection support and ROAM code protos. Altered movement
 * trace lengths to be shorter.
 * 
 * 4     21/02/00 15:16 Riever
 * Bot now has the ability to roam on dry land. Basic collision functions
 * written. Active state skeletal implementation.
 * 
 * 3     20/02/00 20:27 Riever
 * Added new members and definitions ready for 2nd generation of bots.
 * 
 * 2     17/02/00 17:53 Riever
 * Fixed item list to be in the right order!
 * 
 */

///////////////////////////////////////////////////////////////////////	
//
//  acebot.h - Main header file for ACEBOT
// 
// 
///////////////////////////////////////////////////////////////////////

#ifndef _ACEBOT_H
#define _ACEBOT_H

#include "../botlib/botlib.h"
// Bots think at the server framerate to make sure they move smoothly.
#define BOT_FPS (game.framerate)

// Only 100 allowed for now (probably never be enough edicts for 'em)
#define MAX_BOTS 100

// Platform states
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

// Maximum nodes
//#define MAX_NODES 1200

// Node types
#define INVALID -1										// Invalid / Terminating node/links
#define NAV_INFINITE 99999	//rekkie -- for navigation: Dijkstra's algorithm
//
typedef enum
{
	NODE_NONE,
	NODE_MOVE,			// Move (forward, left, right, back)
	NODE_JUMPPAD,		// For large jumps across any distance
	NODE_LADDER,		// Ladder
	NODE_WATER,			// Water
	NODE_CROUCH,		// Crouching with a reduced hitbox/collision height
	NODE_BOXJUMP,		// Move jump + half size hitbox (for jumping into windows, on boxes, etc)
	NODE_POI,			// Point of Interest (POI) specific locations that you wish bots to visit from time to time
	NODE_POI_LOOKAT,	// When using a Point of Interest (POI), this is the node to look at

	NODE_STEP,			// For steps / stairs
	NODE_JUMP,			// Small jumps (+moveup)
	NODE_STAND_DROP,	// For dropping down while standing
	NODE_CROUCH_DROP,	// For dropping down while crouching
	NODE_UNSAFE_DROP,	// For dropping down while crouching, but causing minimal leg damage
	NODE_LADDER_UP,		// Ladder going up
	NODE_LADDER_DOWN,	// Ladder going down
	NODE_DOOR,			// Door node
	NODE_PLATFORM,		// Moving platforms
	NODE_TELEPORTER,	// Teleporters
	NODE_ITEM,			// Items
	NODE_GRAPPLE,		// CTF grapple hook
	NODE_SPAWNPOINT,	// Nodes tied to spawnpoint locations - these are auto generated for each map ready to be connected to user placed nodes
	NODE_LEARN,			// Special node - Human learning
	NODE_CTF_FLAG,		// CTF Flag
	NODE_ESP_TARGET,	// ESP Target
	NODE_ALL = 99		// For selecting all nodes
} nodetype_t;

// Density setting for nodes
#define NODE_DENSITY 96

// Maximum links per node
//#define MAXLINKS 12

//rekkie -- BSP -- s
//#undef MAX_NODES
//#define MAX_NODES 12288 //7168 //6144
//#undef MAXLINKS
#define MAXLINKS 32
//rekkie -- BSP -- e

#define MAX_BOTSKILL 10
extern cvar_t *bot_skill; // Skill setting for bots, range 0-10. 0 = easy, 10 = aimbot!
extern cvar_t* bot_skill_threshold; // Dynamic skill adjustment kicks in if a threshold has been hit
extern cvar_t *bot_remember; // How long (in seconds) the bot remembers an enemy after visibility has been lost
extern cvar_t* bot_reaction; // How long (in seconds) until the bot reacts to an enemy in sight
extern cvar_t *bot_showpath; // Show bot paths

//AQ2 ADD
extern cvar_t	*ltk_skill;		// Skill setting for bots, range 0-10	
extern cvar_t	*ltk_showpath;	// Toggles display of bot paths in debug mode
extern cvar_t	*ltk_chat;		// Chat setting for bots, off or on (0,1)
extern cvar_t	*ltk_routing;	// Set to 1 to drop nodes, otherwise you won't do it!
extern cvar_t	*ltk_botfile;	// Set this to adjust which botdata file to load, default is botdata
extern cvar_t	*ltk_loadbots;	// Set to 0 to not load bots from ltk_botfile value, 1 for normal operation

extern int lights_camera_action;

//AQ2 END

// Bot state types
#define STATE_STAND 0
#define STATE_MOVE 1
#define STATE_ATTACK 2
#define STATE_WANDER 3
#define STATE_FLEE 4
#define STATE_POSITION 5
#define STATE_COVER 6
#define STATE_ITEM 7	// Bot is grabbing an item

// New state definitions
#define	BS_WAIT		0
#define	BS_DEAD		1
#define	BS_ROAM		2
#define	BS_PASSIVE	3
#define	BS_ACTIVE	4
#define	BS_SECURE	5
#define	BS_RETREAT	6
#define	BS_HOLD		7
#define	BS_SUPPORT	8

// Secondary states
#define	BSS_NONE		0
#define	BSS_POSITION	1
#define	BSS_COLLECT		2
#define	BSS_SEEKENEMY	4
#define	BSS_ATTACK		8


// Goal Types (Extensible)
#define	GT_NONE			0
#define	GT_POSITION		1
#define	GT_ENEMY		2
#define	GT_ITEM			3


#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MOVE_FORWARD 2
#define MOVE_BACK 3

// Used in the BOTCOL functions
#define		TRACE_DOWN		128
#define		TRACE_DOWN_STRAFE	32 // don't go off ledges when strafing!
#define		TRACE_DIST		48 //dropped from 256
#define		TRACE_DIST_STRAFE	24 // watch that edge!
#define		TRACE_DIST_SHORT	32 // for forwards motion
#define		TRACE_DIST_LADDER	24
#define		TRACE_DIST_JUMP		128 // to jump over gaps
#define		TJUMP_DIST		40  //just enough to stand on
#define		TWATER_DIST		8	// For getting to edge in water
#define		TWEDGE_DIST		16 // ledge width required to leave water
#define		TCRAWL_DIST		32 // not worth crawling otherwise	
#define		TMOVE_DIST		16 // wall detection
#define		MASK_DEADLY		(CONTENTS_LAVA|CONTENTS_SLIME) // ouch!

// Movement speeds
#define	SPEED_CAREFUL	10
#define	SPEED_ROAM		100
#define SPEED_WALK      200
#define SPEED_RUN       400

#define EYES_FREQ       0.2     // Every n seconds the bot's eyes will be checked
#define ROOT2           1.41421 // Square root of 2 (i.e. ROOT2^2 = 2)
#define COS90           -0.34202
#define STRIDESIZE		24

#define VEC_ORIGIN tv(0,0,0)


typedef struct nodelink_s
{
	short int	targetNode;
	//rekkie -- DEV_1 -- s
	byte		targetNodeType; // From our node, what type of node do we need the target node to be to reach it
	//rekkie -- DEV_1 -- e
	float		cost;			// updated for pathsearch algorithm

}nodelink_t; // RiEvEr


// Node structure
typedef struct node_s
{
	int	area;			// Area/chunk/group this node belongs to -- optimise and diversify bot pathing
	unsigned int area_color; // Sets the area color
	vec3_t origin;		// The origin of the node
	byte type;			// Type of node (MOVE, JUMP, LADDER, etc)
	short int nodenum;	// Node number
	qboolean inuse;		// If the node is used, deleted nodes are set to 'unused'
	vec3_t normal;		// The surface normal the node sits on
	vec3_t mins;
	vec3_t maxs;
	vec3_t absmin;
	vec3_t absmax;
	float weight;		// Used to help diversify bot pathing
	//int num_vis_nodes;		// Total nodes visible to this node
	//int vis_nodes[1024];	// Store all vis nodes

	byte num_links;		// Total links this node has
	nodelink_t links[MAXLINKS];	// store all links. - RiEvEr
} node_t;
extern node_t *nodes;

typedef struct item_table_s
{
	int item;
	float weight;
	edict_t *ent;
	int node;

} item_table_t;

extern int num_players;
extern edict_t *players[MAX_CLIENTS];		// pointers to all players in the game

// extern decs
//rekkie -- DEV_1 -- s
extern node_t *unsorted_nodes; // Used to generate all links, so they can be sorted, then copied to nodes
#define MAX_PNODES 8096 //8096
//32768 Absolute max nodes
//extern node_t *nodes;
//extern node_t *nodes;
//node_t *nodes[MAX_PNODES];
//node_t nodes[MAX_PNODES];
//extern node_t nodes[MAX_PNODES];
//rekkie -- DEV_1 -- e

extern short int** path_table;
////short int path_table[MAX_PNODES][MAX_PNODES];
 
//extern node_t nodes[MAX_NODES];
extern item_table_t item_table[MAX_EDICTS];
extern qboolean debug_mode;
extern qboolean shownodes_mode;	// RiEvEr - for the new command "sv shownodes on/off"
extern int numnodes;
extern int num_items;

// id Function Protos I need
void     LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);
void     ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void     TossClientWeapon (edict_t *self);
void     ClientThink (edict_t *ent, usercmd_t *ucmd);
void     SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void     ClientUserinfoChanged (edict_t *ent, char *userinfo);
void     CopyToBodyQue (edict_t *ent);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void     Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);

// acebot_ai.c protos
qboolean ACEAI_CheckShot(edict_t *self);
void     ACEAI_Think (edict_t *self);
void     ACEAI_PickLongRangeGoal(edict_t *self);
void     ACEAI_PickShortRangeGoal(edict_t *self);
void	 ACEAI_PickSafeGoal(edict_t *self);
qboolean ACEAI_ChooseWeapon(edict_t *self);
void ACEAI_Cmd_Choose( edict_t *ent, char *s );
void ACEAI_Cmd_Choose_Weapon_Num( edict_t *ent, int num );
void ACEAI_Cmd_Choose_Item_Num( edict_t *ent, int num );
void ACEAI_Cmd_Choose_ItemKit_Num( edict_t *ent, int num );

// acebot_cmds.c protos
qboolean ACECM_Commands(edict_t *ent);
void     ACECM_Store(void);

// acebot_items.c protos
void     ACEIT_RebuildPlayerList( void );
qboolean ACEIT_IsVisible(edict_t *self, vec3_t goal);
qboolean ACEIT_IsReachable(edict_t *self,vec3_t goal);
qboolean ACEIT_ChangeWeapon (edict_t *ent, gitem_t *item);
//AQ2 ADD
qboolean ACEIT_ChangeMK23SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeHCSpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeSniperSpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeM4SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeM3SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeMP5SpecialWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_ChangeDualSpecialWeapon (edict_t *ent, gitem_t *item);
//AQ2 END
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other);
float    ACEIT_ItemNeed( edict_t *self, edict_t *item_ent );
int      ACEIT_ClassnameToIndex( char *classname );
void     ACEIT_BuildItemNodeTable (qboolean rebuild);

// acebot_movement.c protos
qboolean ACEMV_CanMove(edict_t *self, int direction);
qboolean ACEMV_SpecialMove(edict_t *self,usercmd_t *ucmd);
//void     ACEMV_Move(edict_t *self, usercmd_t *ucmd);
void     ACEMV_Attack (edict_t *self, usercmd_t *ucmd);
//void     ACEMV_Wander (edict_t *self, usercmd_t *ucmd);
//rekkie -- Quake3 -- s
// botlib_items.c
void BOTLIB_SmartWeaponSelection(edict_t* self);
// botlib_ai.c
int  BOTLIB_AutoAdjustSkill(edict_t* self);


void BOTLIB_Attack(edict_t* self, usercmd_t* ucmd);
void BOTLIB_Healing(edict_t* self, usercmd_t* ucmd);
void BOTLIB_PredictJumpPoint(edict_t* self, edict_t* target, vec3_t target_point, float target_viewheight, vec3_t target_velocity, vec3_t start, float bolt_speed, bool eye_height, float offset, vec3_t* aimdir, vec3_t* aimpoint);
qboolean BOTLIB_Jump_Takeoff(edict_t* self, edict_t* target, vec3_t target_point, float target_viewheight, vec3_t target_velocity);
// BOT RADIO

//rekkie -- Quake3 -- e

// acebot_nodes.c protos
void     ACEND_SetGoal(edict_t *self, int goal_node);
int      ACEND_FindCost(int from, int to);
int      ACEND_FindCloseReachableNode(edict_t *self, int dist, int type);
int      ACEND_FindClosestReachableNode(edict_t *self, int range, int type);
int		 ACEND_FindClosestReachableSafeNode(edict_t *self, int range, int type);
qboolean BOTLIB_FollowPath(edict_t *self);
int		 BOTLIB_MakeEntsSolid(solid_t* trigger_solid);
void	 BOTLIB_RestoreEntsSolidState(solid_t* trigger_solid);
qboolean BOTLIB_UTIL_CHECK_FOR_HURT(vec3_t origin);
void     ACEND_GrapFired(edict_t *self);
//qboolean ACEND_CheckForLadder(edict_t *self);
//void     ACEND_PathMap(edict_t *self);
void     BOTLIB_InitNodes(void);
void     ACEND_ShowNode(int node);
void     ACEND_DrawPath(edict_t *self);
void     ACEND_ShowPath(edict_t *self, int goal_node);
//void	BOTLIB_GenerateNodeVis(edict_t* self);
int		BOTLIB_TraceNodeBoxLine(vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs);
int		BOTLIB_TraceBoxNode(int from, int to);
int		BOTLIB_NodeTouchNodes(vec_t *origin, vec3_t normal, float size, vec3_t mins, vec3_t maxs, int* nodelist, const int maxnodes, int ignore_node);
int		BOTLIB_TestForNodeDist(vec_t* origin, float distance, vec3_t mins, vec3_t maxs);
//int      ACEND_AddNode(edict_t *self, int type);
//void     ACEND_UpdateNodeEdge(edict_t *self, int from, int to);
void     ACEND_RemoveNodeEdge(edict_t *self, int from, int to);
//void     ACEND_ResolveAllPaths(void);
//void     ACEND_SaveNodes(void);
//void     ACEND_LoadNodes(void);

// acebot_spawn.c protos
//void	 ACESP_SaveBots(void);
//void	 ACESP_LoadBots(void);
void	 ACESP_LoadBotConfig(void);
edict_t *ACESP_SpawnBotFromConfig( char *inString );
void     ACESP_HoldSpawn(edict_t *self);
void     ACESP_PutClientInServer (edict_t *bot, qboolean respawn, int team);
void     ACESP_Respawn (edict_t *self);
edict_t *ACESP_FindFreeClient (void);
void     ACESP_SetName(edict_t *bot, char *name, char *skin, char *team);
edict_t *ACESP_SpawnBot (char *team, char *name, char *skin, char *userinfo);
//void     ACESP_ReAddBots(void);
void     ACESP_RemoveBot(char *name);
void	 safe_cprintf (edict_t *ent, int printlevel, const char *fmt, ...);
void     safe_centerprintf (edict_t *ent, const char *fmt, ...);
void     debug_printf (char *fmt, ...);
int 	 ACEND_FindClosestReachableNode(edict_t *self, int range, int type);

// bot_ai.c protos
qboolean	BOTAI_NeedToBandage(edict_t *bot);
void		BOTAI_PickLongRangeGoal(edict_t *self, int	iType);
void		BOTAI_PickShortRangeGoal(edict_t *bot);
void		BOTAI_SetGoal(edict_t *self, int goal_node);
void		BOTAI_Think(edict_t *bot);
qboolean	BOTAI_VisibleEnemy( edict_t *self );

// bot_collision.c protos
qboolean	BOTCOL_CanJumpForward(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanCrawl(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanStand(edict_t	*self);
qboolean	BOTCOL_CanJumpUp(edict_t	*self);
qboolean	BOTCOL_CanMoveForward(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanReachItem(edict_t *bot, vec3_t goal);
qboolean	BOTCOL_CheckShot(edict_t *bot);
qboolean	BOTCOL_WaterMoveForward(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanLeaveWater(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanMoveSafely(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanStrafeSafely(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CanJumpGap(edict_t	*self, vec3_t angles);
qboolean	BOTCOL_CheckBump (edict_t *bot, usercmd_t *ucmd);
qboolean	BOTCOL_Visible (edict_t *bot, edict_t *other);

// bot_combat.c protos
void		BOTCOM_Aim (edict_t *bot, edict_t *target, vec3_t angles);
void		BOTCOM_AimAt (edict_t *bot, vec3_t target, vec3_t angles);
void		BOTCOM_BasicAttack (edict_t *bot, usercmd_t *cmd, vec3_t vTarget);

// bot_movement.c protos
void		BOTMV_Roaming( edict_t *bot, vec3_t angles, usercmd_t *cmd);
float		BOTMV_FindBestDirection(edict_t	*bot, vec3_t vBestDest, vec3_t angles);
void		BOTMV_SetJumpVelocity(edict_t *bot, vec3_t angles, vec3_t vTempDest);
void		BOTMV_SetRandomDirection(edict_t *bot, vec3_t angles);

// bot_states.c protos
void		BOTST_Active( edict_t *bot, vec3_t angles, usercmd_t *cmd);
void		BOTST_Roaming( edict_t *bot, vec3_t angles, usercmd_t *cmd);

//bot_utility.c protos
void		BOTUT_Cmd_Say_f (edict_t *ent, char *pMsg);
void		BOTUT_MakeTargetVector(edict_t *bot, vec3_t angles, vec3_t vDest);
void		BOTUT_ShowNodes (edict_t *ent);
void		BOTUT_TempLaser( vec3_t start, vec3_t end);

// bot_weapon.c protos
int			BOTWP_ChangeMK23Mode(edict_t *bot);
int			BOTWP_ChangeSniperMode(edict_t *bot);
qboolean	BOTWP_ChooseWeapon(edict_t *bot);
int			BOTWP_GetMK23Mode(edict_t *bot);
int			BOTWP_GetSniperMode(edict_t *bot);
void		BOTWP_RemoveSniperZoomMode(edict_t *bot);

//rekkie -- BSP -- s

////////////////////////////////////////////////////////////////////////
// DAIC - Data's 'AI' Child ( taken from S3E16 of TNG )
// https://en.wikipedia.org/wiki/The_Offspring_(Star_Trek%3A_The_Next_Generation)
// "Data invites Deanna Troi, Wesley Crusher and Geordi La Forge to his lab and surprises them by introducing a featureless humanoid android, whom he created based on his own structural design and recent advances in Federation cybernetics technology, describing it as his child. He names the android Lal (after the Hindi word for "beloved") and encourages it to select a gender and appearance. With Troi's assistance and considering many of the on-board species as well as the databanks, Lal narrows down to four possibilities, including a Klingon male, which, as Troi points out, would make it "a friend for Worf", but in the end selects the appearance of a young female human becoming a Gynoid.Data first aids Lal with cognitiveand standard behavioral algorithms, as well encourages her to interact with other members of the crew to learn behavioraland social customs."
// Notice that Data is attempting to make her in his own image, as we are attempting to make a bot like a human player. His child can also pick its own genderand appearance, like LTK bots do in DM.Additionally, he trains his child with "with cognitive and standard behavioral algorithms".
////////////////////////////////////////////////////////////////////////
//
// TODO:
// - Make bot aim worst upon first few rounds by taking skill, minus 5 from it, then each shot adds +1 to skill until skill is back to normal.
//	 copy skill to default_skill, make a tmpskill and set it to skill and subtract -5, subtract tmpskill from skill in the code doing the aiming
//   (check if skill is less than 0, set it to 0), then add +1 to tmpskill. Repeat this until skill is back to normal. 
//   If the bot finds a new enemy or stops firing, reset skill and default_skill.
// 
// - When first adding reachabilities for the first time, set them to only use nodes on flat/sloped ground + ladders. 
//   For jumppads, record player's jumps from node to node and let the bot use these as jumppads so the bot can better pick jumps.
// 
// - Make bot aim and strafe
// 
// - Make bot be able to move while looking at any direction
// 
// - in acebot_movement, when the bot is moving and NOT on ground, check if bot touches another player/bot while in air, if so then set bot to wander or find a new path?
// 
// - Antsearch pathing could check if there is something blocking the path and if so, try to find a new path.
// 
// - Try to find a way to add a variety of paths: Add varience, randomness, and more path_table's to pick randomly from!
// 
// - Upon spawning, head to random spawn point.
// 
// - For the top of ladders with rungs (like the ladders in urban2 that have left/right poles and rungs going horizontally in the middle), the rungs don't always
//   come up high enough to meet the ground above (small ladder in ground level room that has a hole in roof going up).
// 
// - Add random 'action hero' bot names
// 
// - Make bot remember last enemy (or enemies) they encountered each round, attack then until they're dead or we gain a new enemy. Perhaps cycle through list of enemies
//   we encounter when we're not attacking.
// 
// - Punch grenades (allies or enemies), or catch grenade and add to inventory
// 
// - Bots aiming through water, some water opacity should limit the bot's ability to aim through water, via water
//
// - [Drg] Varekai � Today at 22:26
//	I've always wondered because we use punch to get to weird places and I imagined that how far someone went had more to do with their initial speed
//	as in, hitting them as they're just jumping would be more effective
//	darksaint � Today at 22:27
//	ReKTeK aka Mech � Today at 22 : 31
//	Having another quick skim of the code, it doesn't take velocity into account. However, if the player is airborne, they wouldn't be slowed down by any initial friction(if any is applied) plus the height would give extra distance
//	Drg] Varekai � Today at 22 : 32
//	so it is mostly random, pretty nuts
//	ReKTeK aka Mech � Today at 22 : 33
//	I suppose velocity should be taken into account
//	darksaint � Today at 22 : 35
//	Kicking someone in mid - air after being kicked should result in double damage :) using their body like a football
// 
// - Roundlimit: add a countdown at 30 seconds, 15 seconds...
// - Donations: name on bill board, name in credits (txt/menu), 
// - Bots attack and continue to move up/down ladders...
// - Make a beep/ding/windows sound when minimised and a round has ended.
// - Bot names, Diablo 2 pre, post fix names
// - Chance to shoot weapon out of hand
// - Make maps snow + fog for xmas
// - Make maps have a snow/fog/rain/etc. effect on the bot's view
// 
// Yogomaster, Shadowking, Dirty, Psy, MOG, and Rekkie are the best bots. Lord Slice [web]
// 

// Node Stuff
#define MAX_SP_NODES 256	// Maximum Spawn Spots (SP), and Spawn Spot Nodes (SPN)
#define MAX_POI_NODES 512	// Maximum POI nodes
#define MAX_DOOR_NODES 64	// Maximum Door nodes
#define MAX_VIS_NODES 320	// Maximum visibility nodes
//




//
//#define MOVE_FORWARD_LEFT 4		// +45 degrees left
//#define MOVE_FORWARD_RIGHT 5	// -45 degrees right
//#define MOVE_BACK_LEFT 6		// +135 degrees left
//#define MOVE_BACK_RIGHT 7		// -135 degrees right
// Acebot Node Stuff -- e

#define STEPSIZE 18

#define MINS_PLAYER_STANDING tv(-16, -16, -24)		// Player standing
#define MAXS_PLAYER_STANDING tv(16, 16, 32)

#define MINS_PLAYER_CROUCHING tv(-16, -16, -24)		// Player crouching
#define MAXS_PLAYER_SCROUCHING tv(16, 16, 4)

//rekkie -- DEV_1 -- Gib Players -- s
extern cvar_t* gib_bodies;
extern cvar_t* gib_heads;
//rekkie -- DEV_1 -- Gib Players -- e

extern cvar_t* bot_maxteam;
extern cvar_t* bot_playercount;
extern cvar_t* bot_rush;
extern cvar_t* bot_randvoice;
extern cvar_t* bot_randskill;
extern cvar_t* bot_randname;
extern cvar_t* bot_chat;
extern cvar_t* bot_personality;
extern cvar_t* bot_ragequit;
extern cvar_t* bot_countashuman;
extern cvar_t* bot_debug;
extern cvar_t* bot_count_min;
extern cvar_t* bot_count_max;
extern cvar_t* bot_rotate;
extern cvar_t* bot_reportasclient;
extern cvar_t* bot_navautogen;
//extern cvar_t* bot_randteamskin;

#define MAX_BOT_NAMES 64
typedef struct {
	char name[16];			// Name
}bot_names_t;
extern int	dc_total_male_names;		// Total male names
extern int	dc_total_female_names;		// Total female names
extern bot_names_t bot_male[MAX_BOT_NAMES];	// Cached copy
extern bot_names_t bot_female[MAX_BOT_NAMES];	// Cached copy









// Maximum nodes
//#define NODE_MAX 15000							// Max nodes -- 25,000
//#define NODE_MAXLINKS 16						// Maximum links per node
#define NODE_Z_HEIGHT 32						// Z height of node placement
#define NODE_Z_HALF_HEIGHT 24					// 1/2 of NODE_Z_HEIGHT
#define NODE_Z_HEIGHT_PLUS_STEPSIZE 50			// NODE_Z_HEIGHT + STEPSIZE
#define NODE_MAX_DISTANCE 999999				// Max distance for a node
#define NODE_SIZE 24							// Physical size of the node
#define NODE_GRID_SIZE 16 //(NODE_SIZE*2)		// Fit nodes on grid alignment
#define NODE_MAX_JUMP_HEIGHT 60					// Maximum height that a player can jump to reach a higher level surface, such as a box
#define NODE_MAX_FALL_HEIGHT 210				// Maximum height that a player can fall from without damage
#define NODE_MAX_SAFE_CROUCH_FALL_HEIGHT 214	// Maximum height that a player can crouch fall from without damage
#define NODE_MAX_CROUCH_FALL_HEIGHT 224			// Maximum height that a player can crouch fall from without damage
#define NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE 256	// Maximum height that a player can crouch fall + minor leg damage
#define NODE_CROUCH_HEIGHT 29					// Crouch height that a player can fit under
#define MAX_STEEPNESS 0.7						// Maximum normal value - steepness of the surface (0.7)

// NMesh constraints
#define MAX_NMESH_FACES 18432 //18432 //16384 //12192 //8096
#define MAX_REACHABILITY 64
#define FACETYPE_FLOOR 1
#define FACETYPE_SLOPE 2
#define FACETYPE_WALL 3
//#define FACETYPE_LADDER 4
// Triangles
typedef struct {
	vec3_t v[3]; // Verts of the triangle
	vec3_t center; // Centoid of the triangle
	int face; // The face this triangle belongs to
	int node; // Triangle node

	// Reachability of nearby triangles
	int reach_triangle[MAX_REACHABILITY]; // Nearby triangles reached
	vec3_t reach_origin[MAX_REACHABILITY][3]; // Nearby triangle coordinates
	int reach_face[MAX_REACHABILITY]; // The face that the nearby triangle resides in
	int reach_num; // The number of nearby triangles reached

	// Reachability of target triangles (jumping)
	//float reach_velocity[MAX_REACHABILITY]; // The velocity required to reach the target triangle
	//float reach_distance[MAX_REACHABILITY]; // The distance to the target triangle
	//float reach_time[MAX_REACHABILITY]; // The time it takes to reach the target triangle
	//vec3_t reach_direction[MAX_REACHABILITY]; // The direction to the target triangle

} ntris_t;
// Verts
typedef struct {
	vec3_t v;
	int node; // Vert node
} nvert_t;
// Edges
typedef struct {
	vec3_t v[2]; // Verts of current edge
	vec3_t center; // Center of the edge
	int node; // Edge node
} nedge_t;
// Faces
typedef struct nface_s {

	// Verts
	int num_verts; // Verts on current face
	vec3_t v[64]; // All verts of current floor surface

	// Edges
	nedge_t	edge[128]; // Max edges per face
	int num_edges; // Edges on current face

	// Triangles
	ntris_t tris[32]; // Max triangles per face
	int num_tris; // Triangles on current face

	int type;		// Surface type - FACETYPE_FLOOR, FACETYPE_SLOPE, FACETYPE_WALL
	int drawflags;	// Surface flags - SURF_SKY, SURF_TRANS33, SURF_TRANS66, ...
	int contents;	// Surface contents - CONTENTS_SOLID, CONTENTS_WINDOW, ...
	vec3_t normal;	// Surface normals

} nface_t;
typedef struct nmesh_s {

	// Totals
	int total_faces; // Faces
	int total_walls; // Walls
	int total_edges; // Edges
	int total_verts; // Verts
	int total_tris; // Triangles
	int total_reach; // Triangles reached

	// Floors & Walls
	nface_t face[MAX_NMESH_FACES]; // Max faces (flat, sloped, walls)

	unsigned bsp_checksum; // Map checksum

} nmesh_t;
//extern nmesh_t nmesh;

// Botlib A_TEAM
//void CheckBotRules(void);


// acebot_ai.c
qboolean ACEAI_IsEnemy(edict_t* self, edict_t* other);
short BOTLIB_EnemiesAlive(edict_t* self);
short BOTLIB_AlliesAlive(edict_t* self);

// DC Movement
void BOTLIB_MOV_Move(edict_t* self, usercmd_t* ucmd);
void BOTLIB_MOV_Wander(edict_t* self, usercmd_t* ucmd);
void BOTLIB_Wander(edict_t* self, usercmd_t* ucmd);
float VectorDistance(vec3_t start, vec3_t end);
float VectorDistanceXY(vec3_t start, vec3_t end);



// Navigation
#define ANT_FREQ	0.5	// Time gap between calls to the processor intensive search
// ----------- new Pathing Algorithm stuff -----
qboolean	AntPathMove(edict_t* ent);				// Called in item and enemy route functions
void		AntInitSearch(edict_t* ent);				// Resets all the path lists etc.
qboolean	AntStartSearch(edict_t* ent, int from, int to, qboolean path_randomization);	// main entry to path algorithms
qboolean	AntQuickPath(edict_t* ent, int from, int to);	// backup path system
qboolean	AntFindPath(edict_t* ent, int from, int to);		// Optimised path system
qboolean	AntLinkExists(int from, int to);	// Detects if we are off the path
// --------- AI Tactics Values -------------
enum {
	AIRoam,			// Basic item collection AI
	AIAttack,		// Basic Attack Enemy AI
	AIAttackCollect,// Attack Enemy while collecting Item
	AICamp,			// Camp at a suitable location and collect the item on respawn
	AISnipe,
	AIAmbush
};

qboolean BOTLIB_DrawPath(edict_t* self);						// Draw bot path
void	 SLLpush_front(botlib_sll_t* thelist, int nodedata);	// Add to the front of the list
void	 SLLpop_front(botlib_sll_t* thelist);					// Remove the iten from the front of the list
int		 BOTLIB_SLL_Query_Next_Node(botlib_sll_t* list);		//rekkie -- Query the SLL for the next node
int		 BOTLIB_SLL_Query_All_Nodes(edict_t* ent, botlib_sll_t* list, int* node_list, const int max_nodes); //rekkie -- Query and return all the nodes in the list
int		 SLLfront(botlib_sll_t* thelist);						// Get the integer value from the front of the list, without removing the item (Query the list)
void	 SLLpush_back(botlib_sll_t* thelist, int nodedata);	// Add to the back of the list
qboolean SLLempty(botlib_sll_t* thelist);						// See if the list is empty (false if not empty)
void	 SLLdelete(botlib_sll_t* thelist);						// Free all memory from a list





// BOTLIB Nodes
void	 BOTLIB_UTIL_ROTATE_CENTER(vec3_t pt1, vec3_t pt2, float rotate, float distance, vec3_t end); // Rotate around a center, move the vector out and return the new point
void	 BOTLIB_UTIL_NEAREST_BSP_FACE(vec3_t origin, int* floor, int* edges); // Gets the nearest BSP face and number of edges to the origin
void	 BOTLIB_ChangeNodeFunction(edict_t* ent);
void	 BOTLIB_MouseControlNodes(edict_t* ent, usercmd_t* ucmd);
int		 BOTLIB_Reachability(int from, int to);
qboolean BOTLIB_AdvanceToSelectNode(edict_t* self, int node);
qboolean NODES_AdvanceToNextNode(edict_t* self);
void	 BOTLIB_InitNavigation(edict_t* ent);
int		 BOTLIB_AddNode(vec3_t origin, vec3_t normal, byte type);
qboolean BOTLIB_AddNodeLink(int from, int to, byte type, qboolean do_cost);
void	 BOTLIB_SelfExpandingNodes(edict_t* ent, int node);
void	 BOTLIB_SelfExpandNodesFromSpawnpoints(edict_t* ent);
void	 BOTLIB_LinkAllNodesTogether(edict_t* ent);
void	 BOTLIB_RemoveAllNodeLinksFrom(int from);

#ifdef USE_ZLIB
void	 BOTLIB_SaveNavCompressed(void);
void	 BOTLIB_LoadNavCompressed(void);
#else
void	 BOTLIB_SaveNav(void);
void	 BOTLIB_LoadNav(void);
#endif
qboolean NodeTypeToString(edict_t* self, int type, char* string, const int max_string_size);

//void	 ACEND_FreeUnsortedNodes(void);
//qboolean ACEND_InitUnsortedNodes(void);
void	 ACEND_BSP(edict_t* ent);
void	 ACEND_BuildSpawnPointNodes(void);
void	 ACEND_CachePointOfInterestNodes(void);
void	 ACEND_BuildEntNodeTable(void);
void	 ACEND_BuildVisibilityNodes(void);
qboolean ACEND_IsNodeVisibleToNodes(short x, short y);
short	 ACEND_GetRandomVisibleNode(short x);
//void	 ACEND_ShowAllNodes(void);
void	 BOTLIB_FreeNodes(void); // Free all nodes from memory
qboolean DAIC_Add_Node(vec3_t origin, vec3_t normal, byte type); // Add node
//void	 ACEND_AutoAddNode(vec3_t origin, vec3_t normal, byte type);
void     ACEND_RemoveNode(edict_t* self, int nodenum);

qboolean ACEND_UpdateNodeReach(int from, int to);
//qboolean ACEND_FindLadder(vec3_t origin, vec3_t ladder_bottom, vec3_t ladder_top);
void	 ACEND_SaveAAS(qboolean update);
void	 ACEND_LoadAAS(qboolean force); // Load Area Awareness System. Force: generate a new AAS instead of loading from file
void	 GetBSPFaceTriangle(vec3_t origin, qboolean* found, int* floor, int* triangle);
qboolean LaunchPlayer(edict_t* ent, vec3_t target);
//int		 ACEND_JumpReachability(int from, int to, vec3_t origin, vec3_t target, vec3_t normal);


// DAIC Spawn
void	 ACESP_RemoveTeamplayBot(int team);
void	 DC_LoadRandomBotName(char* userinfo);
//void	 DC_LoadRandomBotName(byte gender, char* bot_name);

//rekkie -- BSP -- e


//rekkie -- Quake3 -- s
//===============================
// Quake 3 Botlib.h Code
//===============================
/*
* 	case 0:
		SetAnimation(ent, FRAME_flip01 - 1, FRAME_flip12, ANIM_WAVE); //flipoff
		break;
	case 1:
		SetAnimation(ent, FRAME_salute01 - 1, FRAME_salute11, ANIM_WAVE); //salute
		break;
	case 2:
		SetAnimation(ent, FRAME_taunt01 - 1, FRAME_taunt17, ANIM_WAVE); //taunt
		break;
	case 3:
		SetAnimation(ent, FRAME_wave01 - 1, FRAME_wave11, ANIM_WAVE); //wave
		break;
	case 4:
	default:
		SetAnimation(ent, FRAME_point01 - 1, FRAME_point12, ANIM_WAVE); //point
*/


//===============================
// Quake 3 Multithreading Code
//===============================
// qthreads.h -- https://github.com/DaemonEngine/daemonmap/blob/d91a0d828e27f75c74e5c3398b2778036e8544f6/tools/quake3/common/qthreads.h
void BOTLIB_THREAD_LOADAAS(qboolean force);		// Init and run this function threaded
void BOTLIB_THREADING_LOADAAS(void* param);		// Running in a thread
typedef struct loadaas_s // Struct used to send parameters to the threaded function
{
	qboolean force;
} loadaas_t;

void BOTLIB_THREADED_DijkstraPath(edict_t* ent, int from, int to);
void BOTLIB_THREADING_DijkstraPath(void* param);
typedef struct dijkstra_path_s // Struct used to send parameters to the threaded function
{
	edict_t *ent;
	int from;
	int to;
} dijkstra_path_t;
//
extern int numthreads;
void ThreadSetDefault(void);
int GetThreadWork(void);
//void RunThreadsOnIndividual(int workcnt, qboolean showpacifier, void (*func)(int));
//void RunThreadsOnIndividual(int workcnt, qboolean showpacifier, void (*func), void *param);
//void RunThreadsOn(int workcnt, qboolean showpacifier, void (*func)(int));
void RunThreadsOn(int workcnt, qboolean showpacifier, void (*func)(int));
//void RunThreadsOn(int workcnt, qboolean showpacifier, void(*func), void* param);
void ThreadLock(void);
void ThreadUnlock(void);
//rekkie -- Quake3 -- e


extern int num_poi_nodes;
extern int poi_nodes[MAX_POI_NODES];
extern edict_t* node_ents[MAX_EDICTS]; // If the node is attached to an entity (such as a NODE_DOOR being attached to a func_door_rotating or func_door entity)
#endif
