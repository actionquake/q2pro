/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// cl_ents.c -- entity parsing and management

#include "client.h"
#include "../refresh/gl.h" //rekkie -- Attach model to player

extern qhandle_t cl_mod_powerscreen;
extern qhandle_t cl_mod_laser;
extern qhandle_t cl_mod_dmspot;
extern qhandle_t cl_sfx_footsteps[12];
extern qhandle_t cl_sfx_landing[8];
int cl_laststep;    // used to not let same step sound be used twice in a row
extern qhandle_t cl_img_flare;

//rekkie -- Attach model to player -- s
typedef struct mod_vert_s {
    short   pos[3];
} mod_vert_t;

maliasvert_t attachment_verts[MAX_CLIENTS][512]; // Cached copy of original position of attachment's verts
mod_vert_t attachment_offsets[MAX_CLIENTS][512]; // Cached copy of original position of attachment's verts
vec3_t attachment_offset[MAX_CLIENTS][512]; // Offset of attachment's verts from frame to frame
static vec3_t org_vert[MAX_CLIENTS];
int prev_flerp = -1, prev_blerp = -1;

int last_client_frame[MAX_CLIENTS];
int last_client_model3[MAX_CLIENTS];

// ---------------------------------------------------------------------------------------------------------------
// Functions used to find a particular vertex in a model, by finding the difference between vertex changes.
// This is used to find the attachment point in the player model.
//
// 1) First we enable cl_thirdperson "1", then run gl_vert_diff "1" to cache the original positions of the verts.
// 2) The model's vertex data is then modified externally by a modeling program (such as Misfit Model 3D)
//      NOTE: When editing vertex Misfit Model 3D needs in animation viewer mode and be in frame 001 of 'stand'
// 3) Save the changes and run the command r_reload in q2pro, then run gl_vert_diff "1" again.
// 4) The output shows which vertex number(s) changed
// ---------------------------------------------------------------------------------------------------------------
extern cvar_t* gl_vert_diff;
qboolean model_diff_changed = false; // If a difference was found
static mod_vert_t* model_difference = NULL; // Cache the model's vertex data
static void MOD_GetModelDiff(short numverts, maliasvert_t* verts)
{
	Cvar_Set("gl_vert_diff", "0");

    if (model_difference == NULL)
    {
        model_difference = (mod_vert_t*)malloc(numverts * sizeof(mod_vert_t));
        if (model_difference)
            memset(model_difference, 0, numverts * sizeof(mod_vert_t));
        else
            return;
    }
	
    mod_vert_t* model_diff = model_difference;
	
    if (model_diff && verts)
    {
        short i;

        // Cache if empty
        if (model_diff->pos[0] == 0 && model_diff->pos[1] == 0 && model_diff->pos[2] == 0)
        {
			for (i = (numverts - 1); i > 0; i--, model_diff++, verts++)
			{
                model_diff->pos[0] = verts->pos[0];
                model_diff->pos[1] = verts->pos[1];
                model_diff->pos[2] = verts->pos[2];
                //Com_Printf("%s Recording vertex %d from [%d %d %d] to [%d %d %d] \n", __func__, i, verts->pos[0], verts->pos[1], verts->pos[2], model_diff->pos[0], model_diff->pos[1], model_diff->pos[2]);
			}
        }
		// Otherwise compare
        else
        {
            for (i = (numverts - 1); i > 0; i--, model_diff++, verts++)
            {
                //Com_Printf("%s Reading vertex %d from [%d %d %d] to [%d %d %d] \n", __func__, i, verts->pos[0], verts->pos[1], verts->pos[2], model_diff->pos[0], model_diff->pos[1], model_diff->pos[2]);
				
                if (model_diff->pos[0] != verts->pos[0] ||
                    model_diff->pos[1] != verts->pos[1] ||
                    model_diff->pos[2] != verts->pos[2])
                {
                    model_diff_changed = true;
					Com_Printf("%s Vertex %d changed from [%d %d %d] to [%d %d %d] \n", __func__, i,  model_diff->pos[0], model_diff->pos[1], model_diff->pos[2], verts->pos[0], verts->pos[1], verts->pos[2]);
                }
            }
			
            if (model_diff_changed)
            {
                free(model_difference);
                model_difference = NULL;
                model_diff_changed = false;
            }
        }
    }
}

static void MatrixMultiply(double* A, double* B, double* C)
{
    // A matrix multiplication (dot product) of two 4x4 matrices.
    // Actually, we are only using matrices with 3 rows and 4 columns.

    C[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8];
    C[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9];
    C[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10];
    C[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3];

    C[4] = A[4] * B[0] + A[5] * B[4] + A[6] * B[8];
    C[5] = A[4] * B[1] + A[5] * B[5] + A[6] * B[9];
    C[6] = A[4] * B[2] + A[5] * B[6] + A[6] * B[10];
    C[7] = A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7];

    C[8] = A[8] * B[0] + A[9] * B[4] + A[10] * B[8];
    C[9] = A[8] * B[1] + A[9] * B[5] + A[10] * B[9];
    C[10] = A[8] * B[2] + A[9] * B[6] + A[10] * B[10];
    C[11] = A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11];
}

#define FRAME_stand01           0
#define FRAME_stand02           1
#define FRAME_stand03           2
#define FRAME_stand04           3
#define FRAME_stand05           4
#define FRAME_stand06           5
#define FRAME_stand07           6
#define FRAME_stand08           7
#define FRAME_stand09           8
#define FRAME_stand10           9
#define FRAME_stand11           10
#define FRAME_stand12           11
#define FRAME_stand13           12
#define FRAME_stand14           13
#define FRAME_stand15           14
#define FRAME_stand16           15
#define FRAME_stand17           16
#define FRAME_stand18           17
#define FRAME_stand19           18
#define FRAME_stand20           19
#define FRAME_stand21           20
#define FRAME_stand22           21
#define FRAME_stand23           22
#define FRAME_stand24           23
#define FRAME_stand25           24
#define FRAME_stand26           25
#define FRAME_stand27           26
#define FRAME_stand28           27
#define FRAME_stand29           28
#define FRAME_stand30           29
#define FRAME_stand31           30
#define FRAME_stand32           31
#define FRAME_stand33           32
#define FRAME_stand34           33
#define FRAME_stand35           34
#define FRAME_stand36           35
#define FRAME_stand37           36
#define FRAME_stand38           37
#define FRAME_stand39           38
#define FRAME_stand40           39
#define FRAME_run1              40
#define FRAME_run2              41
#define FRAME_run3              42
#define FRAME_run4              43
#define FRAME_run5              44
#define FRAME_run6              45
#define FRAME_attack1           46
#define FRAME_attack2           47
#define FRAME_attack3           48
#define FRAME_attack4           49
#define FRAME_attack5           50
#define FRAME_attack6           51
#define FRAME_attack7           52
#define FRAME_attack8           53
#define FRAME_pain101           54
#define FRAME_pain102           55
#define FRAME_pain103           56
#define FRAME_pain104           57
#define FRAME_pain201           58
#define FRAME_pain202           59
#define FRAME_pain203           60
#define FRAME_pain204           61
#define FRAME_pain301           62
#define FRAME_pain302           63
#define FRAME_pain303           64
#define FRAME_pain304           65
#define FRAME_jump1             66
#define FRAME_jump2             67
#define FRAME_jump3             68
#define FRAME_jump4             69
#define FRAME_jump5             70
#define FRAME_jump6             71
#define FRAME_flip01            72
#define FRAME_flip02            73
#define FRAME_flip03            74
#define FRAME_flip04            75
#define FRAME_flip05            76
#define FRAME_flip06            77
#define FRAME_flip07            78
#define FRAME_flip08            79
#define FRAME_flip09            80
#define FRAME_flip10            81
#define FRAME_flip11            82
#define FRAME_flip12            83
#define FRAME_salute01          84
#define FRAME_salute02          85
#define FRAME_salute03          86
#define FRAME_salute04          87
#define FRAME_salute05          88
#define FRAME_salute06          89
#define FRAME_salute07          90
#define FRAME_salute08          91
#define FRAME_salute09          92
#define FRAME_salute10          93
#define FRAME_salute11          94
#define FRAME_taunt01           95
#define FRAME_taunt02           96
#define FRAME_taunt03           97
#define FRAME_taunt04           98
#define FRAME_taunt05           99
#define FRAME_taunt06           100
#define FRAME_taunt07           101
#define FRAME_taunt08           102
#define FRAME_taunt09           103
#define FRAME_taunt10           104
#define FRAME_taunt11           105
#define FRAME_taunt12           106
#define FRAME_taunt13           107
#define FRAME_taunt14           108
#define FRAME_taunt15           109
#define FRAME_taunt16           110
#define FRAME_taunt17           111
#define FRAME_wave01            112
#define FRAME_wave02            113
#define FRAME_wave03            114
#define FRAME_wave04            115
#define FRAME_wave05            116
#define FRAME_wave06            117
#define FRAME_wave07            118
#define FRAME_wave08            119
#define FRAME_wave09            120
#define FRAME_wave10            121
#define FRAME_wave11            122
#define FRAME_point01           123
#define FRAME_point02           124
#define FRAME_point03           125
#define FRAME_point04           126
#define FRAME_point05           127
#define FRAME_point06           128
#define FRAME_point07           129
#define FRAME_point08           130
#define FRAME_point09           131
#define FRAME_point10           132
#define FRAME_point11           133
#define FRAME_point12           134
#define FRAME_crstnd01          135
#define FRAME_crstnd02          136
#define FRAME_crstnd03          137
#define FRAME_crstnd04          138
#define FRAME_crstnd05          139
#define FRAME_crstnd06          140
#define FRAME_crstnd07          141
#define FRAME_crstnd08          142
#define FRAME_crstnd09          143
#define FRAME_crstnd10          144
#define FRAME_crstnd11          145
#define FRAME_crstnd12          146
#define FRAME_crstnd13          147
#define FRAME_crstnd14          148
#define FRAME_crstnd15          149
#define FRAME_crstnd16          150
#define FRAME_crstnd17          151
#define FRAME_crstnd18          152
#define FRAME_crstnd19          153
#define FRAME_crwalk1           154
#define FRAME_crwalk2           155
#define FRAME_crwalk3           156
#define FRAME_crwalk4           157
#define FRAME_crwalk5           158
#define FRAME_crwalk6           159
#define FRAME_crattak1          160
#define FRAME_crattak2          161
#define FRAME_crattak3          162
#define FRAME_crattak4          163
#define FRAME_crattak5          164
#define FRAME_crattak6          165
#define FRAME_crattak7          166
#define FRAME_crattak8          167
#define FRAME_crattak9          168
#define FRAME_crpain1           169
#define FRAME_crpain2           170
#define FRAME_crpain3           171
#define FRAME_crpain4           172
#define FRAME_crdeath1          173
#define FRAME_crdeath2          174
#define FRAME_crdeath3          175
#define FRAME_crdeath4          176
#define FRAME_crdeath5          177
#define FRAME_death101          178
#define FRAME_death102          179
#define FRAME_death103          180
#define FRAME_death104          181
#define FRAME_death105          182
#define FRAME_death106          183
#define FRAME_death201          184
#define FRAME_death202          185
#define FRAME_death203          186
#define FRAME_death204          187
#define FRAME_death205          188
#define FRAME_death206          189
#define FRAME_death301          190
#define FRAME_death302          191
#define FRAME_death303          192
#define FRAME_death304          193
#define FRAME_death305          194
#define FRAME_death306          195
#define FRAME_death307          196
#define FRAME_death308          197

//rekkie -- Attach model to player -- e
/*
=========================================================================

FRAME PARSING

=========================================================================
*/

// returns true if origin/angles update has been optimized out
static inline bool entity_is_optimized(const centity_state_t *state)
{
    return (cls.serverProtocol == PROTOCOL_VERSION_Q2PRO || cls.serverProtocol == PROTOCOL_VERSION_AQTION)
        && state->number == cl.frame.clientNum + 1
        && cl.frame.ps.pmove.pm_type < PM_DEAD;
}

static inline void
entity_update_new(centity_t *ent, const centity_state_t *state, const vec_t *origin)
{
    ent->trailcount = 1024;     // for diminishing rocket / grenade trails
    ent->flashlightfrac = 1.0f;

    // duplicate the current state so lerping doesn't hurt anything
    ent->prev = *state;
#if USE_FPS
    ent->prev_frame = state->frame;
    ent->event_frame = cl.frame.number;
#endif

    if (state->event == EV_PLAYER_TELEPORT ||
        state->event == EV_OTHER_TELEPORT ||
        (state->renderfx & RF_BEAM)) {
        // no lerping if teleported
        VectorCopy(origin, ent->lerp_origin);
        return;
    }

    // old_origin is valid for new entities,
    // so use it as starting point for interpolating between
    VectorCopy(state->old_origin, ent->prev.origin);
    VectorCopy(state->old_origin, ent->lerp_origin);
}

static inline void
entity_update_old(centity_t *ent, const centity_state_t *state, const vec_t *origin)
{
    int event = state->event;

#if USE_FPS
    // check for new event
    if (state->event != ent->current.event)
        ent->event_frame = cl.frame.number; // new
    else if (cl.frame.number - ent->event_frame >= cl.frametime.div)
        ent->event_frame = cl.frame.number; // refreshed
    else
        event = 0; // duplicated
#endif

    if (state->modelindex != ent->current.modelindex
        || state->modelindex2 != ent->current.modelindex2
        || state->modelindex3 != ent->current.modelindex3
        || state->modelindex4 != ent->current.modelindex4
        || event == EV_PLAYER_TELEPORT
        || event == EV_OTHER_TELEPORT
        || fabsf(origin[0] - ent->current.origin[0]) > 512
        || fabsf(origin[1] - ent->current.origin[1]) > 512
        || fabsf(origin[2] - ent->current.origin[2]) > 512
        || cl_nolerp->integer == 1) {
        // some data changes will force no lerping
        ent->trailcount = 1024;     // for diminishing rocket / grenade trails
        ent->flashlightfrac = 1.0f;

        // duplicate the current state so lerping doesn't hurt anything
        ent->prev = *state;
#if USE_FPS
        ent->prev_frame = state->frame;
#endif
        // no lerping if teleported or morphed
        VectorCopy(origin, ent->lerp_origin);
        return;
    }

#if USE_FPS
    // start alias model animation
    if (state->frame != ent->current.frame) {
        ent->prev_frame = ent->current.frame;
        ent->anim_start = cl.servertime - cl.frametime.time;
    }
#endif

    // shuffle the last state to previous
    ent->prev = ent->current;
}

static inline bool entity_is_new(const centity_t *ent)
{
    if (!cl.oldframe.valid)
        return true;    // last received frame was invalid

    if (ent->serverframe != cl.oldframe.number)
        return true;    // wasn't in last received frame

    if (cl_nolerp->integer == 2)
        return true;    // developer option, always new

    if (cl_nolerp->integer == 3)
        return false;   // developer option, lerp from last received frame

    if (cl.oldframe.number != cl.frame.number - 1)
        return true;    // previous server frame was dropped

    return false;
}

static void parse_entity_update(const centity_state_t *state)
{
    centity_t *ent = &cl_entities[state->number];
    const vec_t *origin;
    vec3_t origin_v;

    // if entity is solid, decode mins/maxs and add to the list
    if (state->solid && state->number != cl.frame.clientNum + 1
        && cl.numSolidEntities < MAX_PACKET_ENTITIES)
        cl.solidEntities[cl.numSolidEntities++] = ent;

    if (state->solid && state->solid != PACKED_BSP) {
        // encoded bbox
        if (cl.csr.extended)
            MSG_UnpackSolid32_Ver2(state->solid, ent->mins, ent->maxs);
        else if (cl.esFlags & MSG_ES_LONGSOLID)
            MSG_UnpackSolid32_Ver1(state->solid, ent->mins, ent->maxs);
        else
            MSG_UnpackSolid16(state->solid, ent->mins, ent->maxs);
        ent->radius = Distance(ent->maxs, ent->mins) * 0.5f;
    } else {
        VectorClear(ent->mins);
        VectorClear(ent->maxs);
        ent->radius = 0;
    }

    // work around Q2PRO server bandwidth optimization
    if (entity_is_optimized(state)) {
        VectorScale(cl.frame.ps.pmove.origin, 0.125f, origin_v);
        origin = origin_v;
    } else {
        origin = state->origin;
    }

    if (entity_is_new(ent)) {
        // wasn't in last update, so initialize some things
        entity_update_new(ent, state, origin);
    } else {
        entity_update_old(ent, state, origin);
    }

    ent->serverframe = cl.frame.number;
    ent->current = *state;

    // work around Q2PRO server bandwidth optimization
    if (entity_is_optimized(state)) {
        Com_PlayerToEntityState(&cl.frame.ps, &ent->current.s);
    }
}

// an entity has just been parsed that has an event value
static void parse_entity_event(int number)
{
    centity_t *cent = &cl_entities[number];

    if (CL_FRAMESYNC) {
        // EF_TELEPORTER acts like an event, but is not cleared each frame
        if (cent->current.effects & EF_TELEPORTER)
            CL_TeleporterParticles(cent->current.origin);

        if (cent->current.morefx & EFX_TELEPORTER2)
            CL_TeleporterParticles2(cent->current.origin);

        if (cent->current.morefx & EFX_BARREL_EXPLODING)
            CL_BarrelExplodingParticles(cent->current.origin);
    }

#if USE_FPS
    if (cent->event_frame != cl.frame.number)
        return;
#endif

    switch (cent->current.event) {
    case EV_ITEM_RESPAWN:
        S_StartSound(NULL, number, CHAN_WEAPON, S_RegisterSound("items/respawn1.wav"), 1, ATTN_IDLE, 0);
        CL_ItemRespawnParticles(cent->current.origin);
        break;
    case EV_PLAYER_TELEPORT:
        S_StartSound(NULL, number, CHAN_WEAPON, S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
        CL_TeleportParticles(cent->current.origin);
        break;
    case EV_FOOTSTEP:
        if (cl_footsteps->integer) {
            CL_PlayFootstepSfx(-1, number, 1.0f, ATTN_NORM);
        }
        break;
    case EV_OTHER_FOOTSTEP:
        if (cl.csr.extended && cl_footsteps->integer)
            CL_PlayFootstepSfx(-1, number, 0.5f, ATTN_IDLE);
        break;
    case EV_LADDER_STEP:
        if (cl.csr.extended && cl_footsteps->integer)
            CL_PlayFootstepSfx(FOOTSTEP_ID_LADDER, number, 0.5f, ATTN_IDLE);
        break;
    case EV_FALLSHORT:
        if (strcmp(cl_enhanced_footsteps->string, "0") == 0) {
            S_StartSound(NULL, number, CHAN_AUTO, cl_sfx_landing[0], 1, ATTN_NORM, 0);
        } else {
            S_StartSound(NULL, number, CHAN_BODY, cl_sfx_landing[Q_rand() % 8], 1, ATTN_NORM, 0);
        }
        break;
    case EV_FALL:
        S_StartSound(NULL, number, CHAN_AUTO, S_RegisterSound("*fall2.wav"), 1, ATTN_NORM, 0);
        break;
    case EV_FALLFAR:
        S_StartSound(NULL, number, CHAN_AUTO, S_RegisterSound("*fall1.wav"), 1, ATTN_NORM, 0);
        break;
    }
}

static void set_active_state(void)
{
    cls.state = ca_active;

    cl.serverdelta = Q_align(cl.frame.number, CL_FRAMEDIV);
    cl.time = cl.servertime = 0; // set time, needed for demos
#if USE_FPS
    cl.keytime = cl.keyservertime = 0;
    cl.keyframe = cl.frame; // initialize keyframe to make sure it's valid
#endif

    // initialize oldframe so lerping doesn't hurt anything
    cl.oldframe.valid = false;
    cl.oldframe.ps = cl.frame.ps;
#if USE_FPS
    cl.oldkeyframe.valid = false;
    cl.oldkeyframe.ps = cl.keyframe.ps;
#endif

    cl.frameflags = 0;
    cl.initialSeq = cls.netchan.outgoing_sequence;

    if (cls.demo.playback) {
        // init some demo things
        CL_FirstDemoFrame();
    } else {
        // set initial cl.predicted_origin and cl.predicted_angles
        VectorScale(cl.frame.ps.pmove.origin, 0.125f, cl.predicted_origin);
        VectorScale(cl.frame.ps.pmove.velocity, 0.125f, cl.predicted_velocity);
        if (cl.frame.ps.pmove.pm_type < PM_DEAD &&
            cls.serverProtocol > PROTOCOL_VERSION_DEFAULT) {
            // enhanced servers don't send viewangles
            CL_PredictAngles();
        } else {
            // just use what server provided
            VectorCopy(cl.frame.ps.viewangles, cl.predicted_angles);
        }
    }

    SCR_EndLoadingPlaque();     // get rid of loading plaque
    SCR_LagClear();
    Con_Close(false);           // get rid of connection screen

    CL_CheckForPause();

    CL_UpdateFrameTimes();

    IN_Activate();

    if (!cls.demo.playback) {
        EXEC_TRIGGER(cl_beginmapcmd);
        Cmd_ExecTrigger("#cl_enterlevel");
    }
}

static void
check_player_lerp(server_frame_t *oldframe, server_frame_t *frame, int framediv)
{
    player_state_t *ps, *ops;
    centity_t *ent;
    int oldnum;

    // find states to interpolate between
    ps = &frame->ps;
    ops = &oldframe->ps;

    // no lerping if previous frame was dropped or invalid
    if (!oldframe->valid)
        goto dup;

    oldnum = frame->number - framediv;
    if (oldframe->number != oldnum)
        goto dup;

    // no lerping if player entity was teleported (origin check)
    if (abs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8 ||
        abs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8 ||
        abs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8) {
        goto dup;
    }

    // no lerping if player entity was teleported (event check)
    ent = &cl_entities[frame->clientNum + 1];
    if (ent->serverframe > oldnum &&
        ent->serverframe <= frame->number &&
#if USE_FPS
        ent->event_frame > oldnum &&
        ent->event_frame <= frame->number &&
#endif
        (ent->current.event == EV_PLAYER_TELEPORT
         || ent->current.event == EV_OTHER_TELEPORT)) {
        goto dup;
    }

    // no lerping if teleport bit was flipped
    if (!cl.csr.extended && (ops->pmove.pm_flags ^ ps->pmove.pm_flags) & PMF_TELEPORT_BIT)
        goto dup;

    if (cl.csr.extended && (ops->rdflags ^ ps->rdflags) & RDF_TELEPORT_BIT)
        goto dup;

    // no lerping if POV number changed
    if (oldframe->clientNum != frame->clientNum)
        goto dup;

    // developer option
    if (cl_nolerp->integer == 1)
        goto dup;

    return;

dup:
    // duplicate the current state so lerping doesn't hurt anything
    *ops = *ps;
}

/*
==================
CL_DeltaFrame

A valid frame has been parsed.
==================
*/
void CL_DeltaFrame(void)
{
    centity_t           *ent;
    int                 i, j;
    int                 framenum;
    int                 prevstate = cls.state;

    // getting a valid frame message ends the connection process
    if (cls.state == ca_precached)
        set_active_state();

    // set server time
    framenum = cl.frame.number - cl.serverdelta;
    cl.servertime = framenum * CL_FRAMETIME;
#if USE_FPS
    cl.keyservertime = (framenum / cl.frametime.div) * BASE_FRAMETIME;
#endif

    //rekkie -- Attach model to player -- s
    /*
	for (i = 0; i < cl.frame.numEntities; i++)
    {
        centity_t* ent = &cl_entities[i];
        model_t* model = MOD_ForHandle(cl.model_draw[ent->current.modelindex]);
        //model_t* model = MOD_ForHandle(cl.model_draw[state->modelindex]);
        if (model && strlen(model->name) > 0)
            Com_Printf("%s [%d][%s]\n", __func__, i, model->name);
    }
	*/
	/*
    for (i = 0; i < 256; i++)
    {
        model_t* model = MOD_ForHandle(cl.model_draw[i]);
        if (model && strlen(model->name) > 0)
            Com_Printf("%s [%d][%s]\n", __func__, i, model->name);
    }
	*/
    //rekkie -- Attach model to player -- e

    // rebuild the list of solid entities for this frame
    cl.numSolidEntities = 0;

    // initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing it's origin (such as MZ_LOGIN, etc)
    ent = &cl_entities[cl.frame.clientNum + 1];
    Com_PlayerToEntityState(&cl.frame.ps, &ent->current.s);

    // set current and prev, unpack solid, etc
    for (i = 0; i < cl.frame.numEntities; i++) {
        j = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        parse_entity_update(&cl.entityStates[j]);
    }

    // fire events. due to footstep tracing this must be after updating entities.
    for (i = 0; i < cl.frame.numEntities; i++) {
        j = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        parse_entity_event(cl.entityStates[j].number);
    }

    if (cls.demo.recording && !cls.demo.paused && !cls.demo.seeking && CL_FRAMESYNC) {
        CL_EmitDemoFrame();
    }

    if (prevstate == ca_precached)
        CL_GTV_Resume();
    else
        CL_GTV_EmitFrame();

    if (cls.demo.playback) {
        // this delta has nothing to do with local viewangles,
        // clear it to avoid interfering with demo freelook hack
        VectorClear(cl.frame.ps.pmove.delta_angles);
    }

    if (cl.oldframe.ps.pmove.pm_type != cl.frame.ps.pmove.pm_type) {
        IN_Activate();
    }

    check_player_lerp(&cl.oldframe, &cl.frame, 1);

#if USE_FPS
    if (CL_FRAMESYNC)
        check_player_lerp(&cl.oldkeyframe, &cl.keyframe, cl.frametime.div);
#endif

    CL_CheckPredictionError();

    SCR_SetCrosshairColor();
}

#if USE_DEBUG
// for debugging problems when out-of-date entity origin is referenced
void CL_CheckEntityPresent(int entnum, const char *what)
{
    centity_t *e;

    if (entnum == cl.frame.clientNum + 1) {
        return; // player entity = current
    }

    e = &cl_entities[entnum];
    if (e->serverframe == cl.frame.number) {
        return; // current
    }

    if (e->serverframe) {
        Com_LPrintf(PRINT_DEVELOPER,
                    "SERVER BUG: %s on entity %d last seen %d frames ago\n",
                    what, entnum, cl.frame.number - e->serverframe);
    } else {
        Com_LPrintf(PRINT_DEVELOPER,
                    "SERVER BUG: %s on entity %d never seen before\n",
                    what, entnum);
    }
}
#endif

//rekkie -- Attach model to player -- s
static float    frontlerp;
static float    backlerp;
static inline vec_t* get_static_normal(vec_t* normal, const maliasvert_t* vert)
{
    unsigned int lat = vert->norm[0];
    unsigned int lng = vert->norm[1];

    normal[0] = TAB_SIN(lat) * TAB_COS(lng);
    normal[1] = TAB_SIN(lat) * TAB_SIN(lng);
    normal[2] = TAB_COS(lat);

    return normal;
}
static inline vec_t* get_lerped_normal(vec_t* normal, const maliasvert_t* oldvert, const maliasvert_t* newvert)
{
    vec3_t oldnorm, newnorm, tmp;
    vec_t len;

    get_static_normal(oldnorm, oldvert);
    get_static_normal(newnorm, newvert);

    LerpVector2(oldnorm, newnorm, backlerp, frontlerp, tmp);

    // normalize result
    len = 1 / VectorLength(tmp);
    VectorScale(tmp, len, normal);

    return normal;
}
//rekkie -- Attach model to player -- e

/*
==========================================================================

INTERPOLATE BETWEEN FRAMES TO GET RENDERING PARMS

==========================================================================
*/

/*
===============
CL_AddPacketEntities

===============
*/
static void CL_AddPacketEntities(void)
{
    entity_t            ent;
    centity_state_t     *s1;
    float               autorotate, autobob;
    int                 i;
    int                 pnum;
    centity_t           *cent;
    int                 autoanim;
    clientinfo_t        *ci;
    unsigned int        effects, renderfx;
    bool                has_alpha;

    // bonus items rotate at a fixed rate
    autorotate = anglemod(cl.time * 0.1f);

    // brush models can auto animate their frames
    autoanim = 2 * cl.time / 1000;

    autobob = 5 * sin(cl.time / 400.0f);

    memset(&ent, 0, sizeof(ent));

    for (pnum = 0; pnum < cl.frame.numEntities; pnum++) {
        i = (cl.frame.firstEntity + pnum) & PARSE_ENTITIES_MASK;
        s1 = &cl.entityStates[i];

        cent = &cl_entities[s1->number];

        effects = s1->effects;
        renderfx = s1->renderfx;

        // set frame
        if (effects & EF_ANIM01)
            ent.frame = autoanim & 1;
        else if (effects & EF_ANIM23)
            ent.frame = 2 + (autoanim & 1);
        else if (effects & EF_ANIM_ALL)
            ent.frame = autoanim;
        else if (effects & EF_ANIM_ALLFAST)
            ent.frame = cl.time / 100;
        else
            ent.frame = s1->frame;

        // quad and pent can do different things on client
        if (effects & EF_PENT) {
            effects &= ~EF_PENT;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_RED;
        }

        if (effects & EF_QUAD) {
            effects &= ~EF_QUAD;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_BLUE;
        }

        if (effects & EF_DOUBLE) {
            effects &= ~EF_DOUBLE;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_DOUBLE;
        }

        if (effects & EF_HALF_DAMAGE) {
            effects &= ~EF_HALF_DAMAGE;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_HALF_DAM;
        }

        if (s1->morefx & EFX_DUALFIRE) {
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_LITE_GREEN;
        }

        // optionally remove the glowing effect
        if (cl_noglow->integer && !(renderfx & RF_BEAM))
            renderfx &= ~RF_GLOW;

        ent.oldframe = cent->prev.frame;
        ent.backlerp = 1.0f - cl.lerpfrac;

        if (renderfx & RF_BEAM) {
            // interpolate start and end points for beams
            LerpVector(cent->prev.origin, cent->current.origin,
                       cl.lerpfrac, ent.origin);
            LerpVector(cent->prev.old_origin, cent->current.old_origin,
                       cl.lerpfrac, ent.oldorigin);
        } else {
            if (s1->number == cl.frame.clientNum + 1) {
                // use predicted origin
                VectorCopy(cl.playerEntityOrigin, ent.origin);
                VectorCopy(cl.playerEntityOrigin, ent.oldorigin);
            } else {
                // interpolate origin
                LerpVector(cent->prev.origin, cent->current.origin,
                           cl.lerpfrac, ent.origin);
                VectorCopy(ent.origin, ent.oldorigin);
            }
#if USE_FPS
            // run alias model animation
            if (cent->prev_frame != s1->frame) {
                int delta = cl.time - cent->anim_start;
                float frac;

                if (delta > BASE_FRAMETIME) {
                    cent->prev_frame = s1->frame;
                    frac = 1;
                } else if (delta > 0) {
                    frac = delta * BASE_1_FRAMETIME;
                } else {
                    frac = 0;
                }

                ent.oldframe = cent->prev_frame;
                ent.backlerp = 1.0f - frac;
            }
#endif
        }

        if (effects & EF_BOB && !cl_nobob->integer) {
            ent.origin[2] += autobob;
            ent.oldorigin[2] += autobob;
        }

#if USE_AQTION
        if (IS_INDICATOR(renderfx) && !cl_indicators->integer && cls.demo.playback) {
            goto skip;
        }
#endif
        if ((effects & EF_GIB) && !cl_gibs->integer)
            goto skip;

        // create a new entity

        if (cl.csr.extended) {
            if (renderfx & RF_FLARE) {
                if (!cl_flares->integer)
                    goto skip;
                float fade_start = s1->modelindex2;
                float fade_end = s1->modelindex3;
                float d = Distance(cl.refdef.vieworg, ent.origin);
                if (d < fade_start)
                    goto skip;
                if (d > fade_end)
                    ent.alpha = 1;
                else
                    ent.alpha = (d - fade_start) / (fade_end - fade_start);
                ent.skin = 0;
                if (renderfx & RF_CUSTOMSKIN && (unsigned)s1->frame < cl.csr.max_images)
                    ent.skin = cl.image_precache[s1->frame];
                if (!ent.skin)
                    ent.skin = cl_img_flare;
                ent.scale = s1->scale ? s1->scale : 1;
                ent.flags = renderfx | RF_TRANSLUCENT;
                if (!s1->skinnum)
                    ent.rgba.u32 = U32_WHITE;
                else
                    ent.rgba.u32 = BigLong(s1->skinnum);
                ent.skinnum = s1->number;
                V_AddEntity(&ent);
                goto skip;
            }

            if (renderfx & RF_CUSTOM_LIGHT) {
                color_t color;
                if (!s1->skinnum)
                    color.u32 = U32_WHITE;
                else
                    color.u32 = BigLong(s1->skinnum);
                V_AddLight(ent.origin, DLIGHT_CUTOFF + s1->frame,
                           color.u8[0] / 255.0f,
                           color.u8[1] / 255.0f,
                           color.u8[2] / 255.0f);
                goto skip;
            }

            if (renderfx & RF_BEAM && s1->modelindex > 1) {
                CL_DrawBeam(ent.oldorigin, ent.origin, cl.model_draw[s1->modelindex]);
                goto skip;
            }
        }

        // tweak the color of beams
        if (renderfx & RF_BEAM) {
            // the four beam colors are encoded in 32 bits of skinnum (hack)
            ent.alpha = 0.30f;
            ent.skinnum = (s1->skinnum >> ((Q_rand() % 4) * 8)) & 0xff;
            ent.model = 0;
        } else {
            // set skin
            if (s1->modelindex == MODELINDEX_PLAYER) {
                // use custom player skin
                ent.skinnum = 0;
                ci = &cl.clientinfo[s1->skinnum & 0xff];
                ent.skin = ci->skin;
                ent.model = ci->model;
                if (!ent.skin || !ent.model) {
                    ent.skin = cl.baseclientinfo.skin;
                    ent.model = cl.baseclientinfo.model;
                    ci = &cl.baseclientinfo;
                }
                if (renderfx & RF_USE_DISGUISE) {
                    char buffer[MAX_QPATH];

                    Q_concat(buffer, sizeof(buffer), "players/", ci->model_name, "/disguise.pcx");
                    ent.skin = R_RegisterSkin(buffer);
                }
            } else {
                ent.skinnum = s1->skinnum;
                ent.skin = 0;
                ent.model = cl.model_draw[s1->modelindex];
                if (ent.model == cl_mod_laser || ent.model == cl_mod_dmspot)
                    renderfx |= RF_NOSHADOW;
            }
        }

        // allow skin override for remaster
        if (cl.csr.extended && renderfx & RF_CUSTOMSKIN && (unsigned)s1->skinnum < cl.csr.max_images) {
            ent.skin = cl.image_precache[s1->skinnum];
            ent.skinnum = 0;
        }

        // only used for black hole model right now, FIXME: do better
        if ((renderfx & RF_TRANSLUCENT) && !(renderfx & RF_BEAM))
            ent.alpha = 0.70f;

        // render effects (fullbright, translucent, etc)
        if (effects & EF_COLOR_SHELL)
            ent.flags = 0;  // renderfx go on color shell entity
        else
            ent.flags = renderfx;

        // calculate angles
        if (effects & EF_ROTATE) {  // some bonus items auto-rotate
            ent.angles[0] = 0;
            ent.angles[1] = autorotate;
            ent.angles[2] = 0;
        } else if (effects & EF_SPINNINGLIGHTS) {
            vec3_t forward;
            vec3_t start;

            ent.angles[0] = 0;
            ent.angles[1] = anglemod(cl.time / 2) + s1->angles[1];
            ent.angles[2] = 180;

            AngleVectors(ent.angles, forward, NULL, NULL);
            VectorMA(ent.origin, 64, forward, start);
            V_AddLight(start, 100, 1, 0, 0);
        } else if (s1->number == cl.frame.clientNum + 1) {
            VectorCopy(cl.playerEntityAngles, ent.angles);      // use predicted angles
        } else { // interpolate angles
            LerpAngles(cent->prev.angles, cent->current.angles,
                       cl.lerpfrac, ent.angles);
            // mimic original ref_gl "leaning" bug (uuugly!)
            if (s1->modelindex == MODELINDEX_PLAYER && cl_rollhack->integer)
                ent.angles[ROLL] = -ent.angles[ROLL];
        }

        if (s1->morefx & EFX_FLASHLIGHT) {
            vec3_t forward, start, end;
            trace_t trace;
            const int mask = CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_PLAYER;

            if (s1->number == cl.frame.clientNum + 1) {
                VectorMA(cl.refdef.vieworg, 256, cl.v_forward, end);
                VectorCopy(cl.refdef.vieworg, start);
            } else {
                AngleVectors(ent.angles, forward, NULL, NULL);
                VectorMA(ent.origin, 256, forward, end);
                VectorCopy(ent.origin, start);
            }

            CL_Trace(&trace, start, vec3_origin, vec3_origin, end, mask);
            LerpVector(start, end, cent->flashlightfrac, end);
            V_AddLight(end, 256, 1, 1, 1);

            // smooth out distance "jumps"
            CL_AdvanceValue(&cent->flashlightfrac, trace.fraction, 1);
        }

        if (s1->morefx & EFX_GRENADE_LIGHT)
            V_AddLight(ent.origin, 100, 1, 1, 0);

        if (s1->number == cl.frame.clientNum + 1) {
            if (effects & EF_FLAG1)
                V_AddLight(ent.origin, 225, 1.0f, 0.1f, 0.1f);
            else if (effects & EF_FLAG2)
                V_AddLight(ent.origin, 225, 0.1f, 0.1f, 1.0f);
            else if (effects & EF_TAGTRAIL)
                V_AddLight(ent.origin, 225, 1.0f, 1.0f, 0.0f);
            else if (effects & EF_TRACKERTRAIL)
                V_AddLight(ent.origin, 225, -1.0f, -1.0f, -1.0f);
            if (!cl.thirdPersonView) {
                //ent.flags |= RF_VIEWERMODEL;    // only draw from mirrors
                goto skip;
            }
        }

        // if set to invisible, skip
        if (!s1->modelindex)
            goto skip;

        if (effects & EF_BFG) {
            ent.flags |= RF_TRANSLUCENT;
            ent.alpha = 0.30f;
        }

        if (effects & EF_PLASMA) {
            ent.flags |= RF_TRANSLUCENT;
            ent.alpha = 0.6f;
        }

        if (effects & EF_SPHERETRANS) {
            ent.flags |= RF_TRANSLUCENT;
            if (effects & EF_TRACKERTRAIL)
                ent.alpha = 0.6f;
            else
                ent.alpha = 0.3f;
        }

        has_alpha = false;
        if (!(ent.flags & RF_TRANSLUCENT) && s1->alpha > 0 && s1->alpha < 1) {
            ent.flags |= RF_TRANSLUCENT;
            ent.alpha = s1->alpha;
            has_alpha = true;
        }

        ent.scale = s1->scale;

        // add to refresh list
        V_AddEntity(&ent);

        // color shells generate a seperate entity for the main model
        if (effects & EF_COLOR_SHELL) {
            // PMM - at this point, all of the shells have been handled
            // if we're in the rogue pack, set up the custom mixing, otherwise just
            // keep going
            if (!strcmp(fs_game->string, "rogue")) {
                // all of the solo colors are fine.  we need to catch any of the combinations that look bad
                // (double & half) and turn them into the appropriate color, and make double/quad something special
                if (renderfx & RF_SHELL_HALF_DAM) {
                    // ditch the half damage shell if any of red, blue, or double are on
                    if (renderfx & (RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE))
                        renderfx &= ~RF_SHELL_HALF_DAM;
                }

                if (renderfx & RF_SHELL_DOUBLE) {
                    // lose the yellow shell if we have a red, blue, or green shell
                    if (renderfx & (RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_GREEN))
                        renderfx &= ~RF_SHELL_DOUBLE;
                    // if we have a red shell, turn it to purple by adding blue
                    if (renderfx & RF_SHELL_RED)
                        renderfx |= RF_SHELL_BLUE;
                    // if we have a blue shell (and not a red shell), turn it to cyan by adding green
                    else if (renderfx & RF_SHELL_BLUE) {
                        // go to green if it's on already, otherwise do cyan (flash green)
                        if (renderfx & RF_SHELL_GREEN)
                            renderfx &= ~RF_SHELL_BLUE;
                        else
                            renderfx |= RF_SHELL_GREEN;
                    }
                }
            }
            ent.flags = renderfx | RF_TRANSLUCENT;
            ent.alpha = 0.30f;
            V_AddEntity(&ent);
        }

        ent.skin = 0;       // never use a custom skin on others
        ent.skinnum = 0;
        ent.flags = 0;
        ent.alpha = 0;

        // duplicate alpha
        if (has_alpha) {
            ent.flags = RF_TRANSLUCENT;
            ent.alpha = s1->alpha;
        }

        // duplicate for linked models
        if (s1->modelindex2) {
            if (s1->modelindex2 == MODELINDEX_PLAYER) {
                // custom weapon
                ci = &cl.clientinfo[s1->skinnum & 0xff];
                i = (s1->skinnum >> 8); // 0 is default weapon model
                if (i < 0 || i > cl.numWeaponModels - 1)
                    i = 0;
                ent.model = ci->weaponmodel[i];
                if (!ent.model) {
                    if (i != 0)
                        ent.model = ci->weaponmodel[0];
                    if (!ent.model)
                        ent.model = cl.baseclientinfo.weaponmodel[0];
                }
            } else
                ent.model = cl.model_draw[s1->modelindex2];

            // PMM - check for the defender sphere shell .. make it translucent
            if (!Q_strcasecmp(cl.configstrings[cl.csr.models + s1->modelindex2], "models/items/shell/tris.md2")) {
                ent.alpha = 0.32f;
                ent.flags = RF_TRANSLUCENT;
            }

            V_AddEntity(&ent);

            //PGM - make sure these get reset.
            ent.flags = 0;
            ent.alpha = 0;
        }

        //rekkie -- Attach model to player -- s

		// Original code
        if (s1->modelindex3) {
            ent.model = cl.model_draw[s1->modelindex3];
            V_AddEntity(&ent);
        }

        if (s1->modelindex4) {
            ent.model = cl.model_draw[s1->modelindex4];
            V_AddEntity(&ent);
        }
        //rekkie -- Attach model to player -- e


		
        //rekkie -- Attach model to player -- s
#if 0
        /*
        model_t* model = MOD_ForHandle(cl.model_draw[s1->modelindex]);
        if (model && strlen(model->name) > 0)
            Com_Printf("%s pnum[%d] s1[%d] name[%s]\n", __func__, pnum, s1->number, model->name);
        else
        {
            model = MOD_ForHandle(cl.clientinfo[s1->number - 1].model);
            if (model && strlen(model->name) > 0)
                Com_Printf("%s pnum[%d] s1[%d] name[%s]\n", __func__, pnum, s1->number, model->name);
        }
        */

        //Com_Printf("%s pnum[%d] s1[%d]\n", __func__, pnum, s1->number);

        //for (i = 0; i < MAX_CLIENTS; i++)
        //{
            //cl.clientinfo[i].model_name[0];
        //}


		
		//Cvar_Set("cl_thirdperson", "1");
        //cl_thirdperson->value = 1;

        //if (s1->modelindex && s1->modelindex3) // && !Q_strcasecmp(cl.configstrings[CS_MODELS + (s1->modelindex)], "players/male/tris.md2"))
        //if ((s1->renderfx & RF_ATTACHMENT) && s1->modelindex == 255 && s1->modelindex3) // && (s1->number - 1) == 0)      // && !Q_strcasecmp(cl.configstrings[CS_MODELS + (s1->modelindex)], "players/male/tris.md2"))
        //if (s1->modelindex3)
        //if (s1->modelindex3 && (s1->renderfx & RF_ATTACHMENT) && last_client_frame[s1->number - 1] != s1->frame)

        ////if (s1->number == cl.frame.clientNum)
        ////    Com_Printf("%s cl[%d] clfnum[%d] s1->frame[%d] frame[%d %d] flerp[%f] blerp[%f]\n", __func__, s1->number, cl.frame.clientNum, s1->frame, cent->current.frame, cent->prev.frame, (1.0f - ent.backlerp), ent.backlerp);

        if (last_client_frame[s1->number - 1] != s1->frame)
        {
            last_client_frame[s1->number - 1] = s1->frame;
			last_client_model3[s1->number - 1] = s1->modelindex3;
        }
        //if (s1->modelindex3 && (s1->renderfx & RF_ATTACHMENT))
        if (last_client_model3[s1->number - 1] && (s1->renderfx & RF_ATTACHMENT))
        {
            //if (s1->number == cl.frame.clientNum + 1 || cl.frame.clientNum)
            //    Com_Printf("%s cl[%d] clfnum[%d] s1->frame[%d] frame[%d %d] flerp[%f] blerp[%f]\n", __func__, s1->number, cl.frame.clientNum, s1->frame, cent->current.frame, cent->prev.frame, (1.0f - ent.backlerp), ent.backlerp);

            //Com_Printf("%s cl[%d] clfnum[%d] s1->frame[%d] frame[%d %d] flerp[%f] blerp[%f]\n", __func__, s1->number, cl.frame.clientNum, s1->frame, cent->current.frame, cent->prev.frame, (1.0f - ent.backlerp), ent.backlerp);
            //Com_Printf("%s flerp[%f] blerp[%f]\n", __func__, (1.0f - ent.backlerp), ent.backlerp);
			if (0)
            if (s1->number == cl.frame.clientNum + 1 || cl.frame.clientNum)
            {
                // Init part way through
                if (round(ent.backlerp * 100) != 0 && prev_flerp == -1 || prev_blerp == -1)
                {
					prev_flerp = ( round( (1.0f - ent.backlerp) * 100) ) - 1;
					prev_blerp = ( round( ent.backlerp * 100 ) ) + 1;
                }
				// Reset if we've gone back to the start
                if (round(ent.backlerp * 100) == 0)
                {
                    prev_flerp = 0;
                    prev_blerp = 100;
                }

                int flerp = round((1.0f - ent.backlerp) * 100);
                int blerp = round((ent.backlerp * 100));

                Com_Printf("%s lerpFB[%d %d] prevlerpFB[%d %d]\n", __func__, flerp, blerp, prev_flerp, prev_blerp);

                if ( round(ent.backlerp * 100) != 0 )
                {
                    //Com_Printf("%s lerpFB[%f %f] prevlerpFB[%f %f]\n", __func__, (1.0f - ent.backlerp), ent.backlerp, prev_flerp, prev_blerp);

					
                    //if (flerp != (prev_flerp + 1) && blerp != (prev_blerp - 1))
                    //     Com_Printf("%s cl[%d] clfnum[%d] s1->frame[%d] frame[%d %d] lerpFB[%f %f] prevlerpFB[%d %d]\n", __func__, s1->number, cl.frame.clientNum, s1->frame, cent->current.frame, cent->prev.frame, (1.0f - ent.backlerp), ent.backlerp, (float)(prev_flerp/100), (float)(prev_blerp/100));

                    // Accumulate
                    prev_flerp += 1;
                    prev_blerp -= 1;
                }
            }

			
            //last_client_frame[s1->number - 1] = s1->frame;
				
            //model_t* player_model = cl.model_draw[s1->modelindex];

            model_t * player_model = NULL, * model_index3 = NULL, * attach_model = NULL;
            /*
            ci = &cl.clientinfo[s1->skinnum & 0xff];
            player_model = ci->model;
            if (!player_model) {
                ci = &cl.baseclientinfo;
            }
            */
            //if (player_model != NULL && player_model->name != 0)
             //   Com_Printf("%s %s\n", __func__, player_model->name);

            player_model = MOD_ForHandle(cl.clientinfo[s1->number - 1].model);
            //if (player_model && strlen(player_model->name) > 0)
            //    Com_Printf("%s pnum[%d] s1[%d] name[%s]\n", __func__, pnum, s1->number, player_model->name);

            //player_model = MOD_ForHandle(ci->model);
            //player_model = MOD_ForHandle(ent.model);
            //player_model = MOD_ForHandle(cl.model_draw[s1->modelindex]);
            model_index3 = MOD_ForHandle(cl.model_draw[s1->modelindex3]);



            //s1->number            // Their client num
            //cl.frame.clientNum    // Our client num
            //player[0]     s1->number = 2, 3           // 1 (if thirdperson)
            //player[0]     cl.frame.clientNum = 0
            //player[1]     s1->number = 1, 3           // 2 (if thirdperson)
            //player[1]     cl.frame.clientNum = 1
            //player[2]     s1->number = 1, 2           // 3 (if thirdperson)
            //player[2]     cl.frame.clientNum = 2
   
			//s1->number - 1

            int attachmentNum = -1;
            qboolean haveAttachment = false;
            for (int a = 0; a < MAX_CLIENTWEAPONMODELS; a++)
            {
				// See if we've already generated the attachment
                if (!Q_strcasecmp(model_index3->name, cl.clientinfo[cl.frame.clientNum].attach_name[a]))
                {
                    attachmentNum = a;
                    haveAttachment = true;
                    break;
                }
            }

            // If client uses r_reload, or a new map loads
            // We need this because the game wipes the attachment upon reload, 
            // so we're checking if the attachment's qhandle exists (which it won't after a reload, but we have it cached)
            if (haveAttachment && MOD_AttachmentForHandle(cl.clientinfo[cl.frame.clientNum].attach_model[attachmentNum]) == NULL)
            {
                haveAttachment = false;
                // We no longer have the attachment, so clear it out of our collection
                cl.clientinfo[cl.frame.clientNum].attach_model[attachmentNum] = 0;
                Com_Printf("%s reloading MOD_AttachmentForHandle\n", __func__);
            }
			
            if (haveAttachment == false)
            {
                for (int a = 0; a < MAX_CLIENTWEAPONMODELS; a++)
                {
                    if (cl.clientinfo[cl.frame.clientNum].attach_model[a] == 0)
                    {
                        attachmentNum = a;
                        cl.clientinfo[cl.frame.clientNum].attach_model[a] = R_AttachmentRegisterModel(model_index3->name);
                        Q_strlcpy(cl.clientinfo[cl.frame.clientNum].attach_name[a], model_index3->name, sizeof(cl.clientinfo[cl.frame.clientNum].attach_name[a]));

                        /*
                        attach_model = MOD_ForHandle(cl.clientinfo[s1->number - 1].attach_model[a]);
                        if (attach_model)
                        {	
                            memset(attachment_offsets[s1->number - 1], 0, sizeof(mod_vert_t) * 512);
							
                            // Cache the attachment verts
                            int amv_count = attach_model->meshes[0].numverts;
                            while (amv_count--)
                            {
                                attachment_verts[s1->number - 1][amv_count].pos[0] = attach_model->meshes[0].verts[amv_count].pos[0];
                                attachment_verts[s1->number - 1][amv_count].pos[1] = attach_model->meshes[0].verts[amv_count].pos[1];
                                attachment_verts[s1->number - 1][amv_count].pos[2] = attach_model->meshes[0].verts[amv_count].pos[2];
                            }
                        }
						*/

                        break;
                    }
                }
            }

            if (attachmentNum >= 0)
                attach_model = MOD_ForHandle(cl.clientinfo[cl.frame.clientNum].attach_model[attachmentNum]);



            if (player_model && attach_model)
            {
                //ent.flags |= RF_ATTACHMENT;
                //ent.flags |= RF_FRAMELERP;
                
                // Chosen player vert to attach to
				int attach_to_vert = 268;
                // 268 = head top left front
				// 250 = head top left back
                // 100 = right shoulder blade
                // 253 = head
                // 266 = head

                // Allocate frame data if number of frames is 1
                if (ent.backlerp == 0 && attach_model->numframes == 1 && player_model->meshes[0].numverts && player_model->numframes)
                {
                    attach_model->meshes[0].num_vec3_verts = attach_model->meshes[0].numverts;
					
                    //vec_t  attachment_radius = { attach_model->frames[0].radius };
					//vec3_t attachment_bounds_min = { attach_model->frames[0].bounds[0][0], attach_model->frames[0].bounds[0][1], attach_model->frames[0].bounds[0][2] };
					//vec3_t attachment_bounds_max = { attach_model->frames[0].bounds[1][0], attach_model->frames[0].bounds[1][1], attach_model->frames[0].bounds[1][2] };
                    vec3_t attachment_scale = { attach_model->frames[0].scale[0], attach_model->frames[0].scale[1], attach_model->frames[0].scale[2] };
                    vec3_t attachment_translate = { attach_model->frames[0].translate[0], attach_model->frames[0].translate[1], attach_model->frames[0].translate[2] };
						
                    //attach_model->meshes[0].numverts = player_model->meshes[0].numverts;
                    attach_model->numframes = player_model->numframes;
                    attach_model->frames = Hunk_TryAlloc(&attach_model->hunk, player_model->numframes * sizeof(maliasframe_t));
                    attach_model->meshes[0].vec3_verts = Hunk_TryAlloc(&attach_model->hunk, (attach_model->meshes[0].numverts * player_model->numframes * sizeof(masliasvec3vert_t)));
                    attach_model->meshes[0].copy_verts = Hunk_TryAlloc(&attach_model->hunk, (attach_model->meshes[0].numverts * player_model->numframes * sizeof(maliasvert_t)));

                    vec3_t mins_fabs;
                    //float plr_fpos_x, plr_fpos_y, plr_fpos_z;
					//float attachment_fpos_x, attachment_fpos_y, attachment_fpos_z;
                    vec3_t plr_fpos, attachment_fpos;
                    int frame_count, plr_vert_count, attach_vert_count;

                    frame_count = 0;
                    while (frame_count < player_model->numframes) // Cycle through frames
                    {
                        maliasvert_t* plr_vert = &player_model->meshes[0].verts[frame_count * player_model->meshes[0].numverts];
                        vec3_t plr_scale = { player_model->frames[frame_count].scale[0], player_model->frames[frame_count].scale[1], player_model->frames[frame_count].scale[2] };
                        vec3_t plr_translate = { player_model->frames[frame_count].translate[0], player_model->frames[frame_count].translate[1], player_model->frames[frame_count].translate[2] };

                        plr_vert_count = player_model->meshes[0].numverts;
                        while (plr_vert_count--) // Cycle through player verts
                        {
                            plr_fpos[0] = (plr_vert->pos[0] * plr_scale[0]) + plr_translate[0];    // X
                            plr_fpos[1] = (plr_vert->pos[1] * plr_scale[1]) + plr_translate[1];    // Y
                            plr_fpos[2] = (plr_vert->pos[2] * plr_scale[2]) + plr_translate[2];    // Z

                            if (0)
                            if (frame_count == 0)
                            {
                                Com_Printf("%s plr f[%d] vert[%d] pos[%d %d %d] scale[%f %f %f] translate[%f %f %f] mins[%f %f %f] max[%f %f %f] fpos[%f %f %f]\n", __func__, frame_count, plr_vert_count,
                                    plr_vert->pos[0], plr_vert->pos[1], plr_vert->pos[2], plr_scale[0], plr_scale[1], plr_scale[2], plr_translate[0], plr_translate[1], plr_translate[2],
                                    player_model->frames[frame_count].bounds[0][0], player_model->frames[frame_count].bounds[0][1], player_model->frames[frame_count].bounds[0][2],
									player_model->frames[frame_count].bounds[1][0], player_model->frames[frame_count].bounds[1][1], player_model->frames[frame_count].bounds[1][2], 
                                    plr_fpos[0], plr_fpos[1], plr_fpos[2]);

								/*
                                vec3_t scale, trans;
                                short pos[3];
                                // scale = ( fabs(mins) + fabs(maxs) ) / 255
                                scale[0] = (fabs(player_model->frames[frame_count].bounds[0][0]) + player_model->frames[frame_count].bounds[1][0]) / 255;
                                scale[1] = (fabs(player_model->frames[frame_count].bounds[0][1]) + player_model->frames[frame_count].bounds[1][1]) / 255;
                                scale[2] = (fabs(player_model->frames[frame_count].bounds[0][2]) + player_model->frames[frame_count].bounds[1][2]) / 255;

                                mins_fabs[0] = fabs(player_model->frames[frame_count].bounds[0][0]);
                                mins_fabs[1] = fabs(player_model->frames[frame_count].bounds[0][1]);
                                mins_fabs[2] = fabs(player_model->frames[frame_count].bounds[0][2]);
                                float yplus = mins_fabs[1] + plr_fpos[1];
								float ydiv = yplus / player_model->frames[frame_count].scale[1];
                                short yshort = (short)ydiv;
								
								
                                // pos = (short)( fpos + fabs(mins) ) / scale)
                                pos[0] = (short)( (plr_fpos_x + mins_fabs[0]) / player_model->frames[frame_count].scale[0] );
                                pos[1] = (short)( (plr_fpos_y + mins_fabs[1]) / player_model->frames[frame_count].scale[1] );
                                pos[2] = (short)( (plr_fpos_z + mins_fabs[2]) / player_model->frames[frame_count].scale[2] );

                                // translate = mins
                                trans[0] = player_model->frames[frame_count].bounds[0][0];
                                trans[1] = player_model->frames[frame_count].bounds[0][1];
                                trans[2] = player_model->frames[frame_count].bounds[0][2];

                                Com_Printf("%s f[%d] scale[%f %f %f][%f %f %f] pos[%d %d %d][%d %d %d] trans[%f %f %f][%f %f %f] fpos[%f %f %f]\n", __func__, frame_count,
                                    plr_scale[0], plr_scale[1], plr_scale[2],
                                    scale[0], scale[1], scale[2],
                                    plr_vert->pos[0], plr_vert->pos[1], plr_vert->pos[2],
                                    pos[0], pos[1], pos[2],
                                    plr_translate[0], plr_translate[1], plr_translate[2],
                                    trans[0], trans[1], trans[2],
                                    plr_fpos[0], plr_fpos[1], plr_fpos[2]);
								*/
                            }
							
							// Chosen player vert
							if (plr_vert_count == attach_to_vert)
                            {
								//attach_model->frames[frame_count].radius = player_model->frames[frame_count].radius;

								// Init the min/max using the player model
                                attach_model->frames[frame_count].bounds[0][0] = player_model->frames[frame_count].bounds[0][0];
                                attach_model->frames[frame_count].bounds[0][1] = player_model->frames[frame_count].bounds[0][1];
                                attach_model->frames[frame_count].bounds[0][2] = player_model->frames[frame_count].bounds[0][2];
                                attach_model->frames[frame_count].bounds[1][0] = player_model->frames[frame_count].bounds[1][0];
                                attach_model->frames[frame_count].bounds[1][1] = player_model->frames[frame_count].bounds[1][1];
                                attach_model->frames[frame_count].bounds[1][2] = player_model->frames[frame_count].bounds[1][2];

								// Adjust the min/max based on the new vert positions
								// We need to pre-calculate the mins and maxs here before using them
                                maliasvert_t* attach_short_vert = &attach_model->meshes[0].verts[0];
                                attach_vert_count = attach_model->meshes[0].numverts;
                                while (attach_vert_count--) // Cycle through attachment verts
                                {
                                    // Get new position based on attachment and player vertex positions
                                    vec3_t pos;
									pos[0] = ((attach_short_vert->pos[0] * attachment_scale[0]) + attachment_translate[0]) + (plr_fpos[0]);
									pos[1] = ((attach_short_vert->pos[1] * attachment_scale[1]) + attachment_translate[1]) + (plr_fpos[1]);
									pos[2] = ((attach_short_vert->pos[2] * attachment_scale[2]) + attachment_translate[2]) + (plr_fpos[2]);
									 
									// Update mins/max if size is smaller/larger
                                    // Mins
                                    if (pos[0] < attach_model->frames[frame_count].bounds[0][0])
                                        attach_model->frames[frame_count].bounds[0][0] = pos[0];
                                    if (pos[1] < attach_model->frames[frame_count].bounds[0][1])
                                        attach_model->frames[frame_count].bounds[0][1] = pos[1];
                                    if (pos[2] < attach_model->frames[frame_count].bounds[0][2])
                                        attach_model->frames[frame_count].bounds[0][2] = pos[2];
                                    // Maxs
                                    if (pos[0] > attach_model->frames[frame_count].bounds[1][0])
                                        attach_model->frames[frame_count].bounds[1][0] = pos[0];
                                    if (pos[1] > attach_model->frames[frame_count].bounds[1][1])
                                        attach_model->frames[frame_count].bounds[1][1] = pos[1];
                                    if (pos[2] > attach_model->frames[frame_count].bounds[1][2])
                                        attach_model->frames[frame_count].bounds[1][2] = pos[2];

                                    attach_short_vert++;
                                }

                                //maliasvert_t* attach_short_vert = &attach_model->meshes[0].verts[frame_count * attach_model->meshes[0].numverts];
                                attach_short_vert = &attach_model->meshes[0].verts[0];
                                maliasvert_t* attach_short_copy_vert = &attach_model->meshes[0].copy_verts[frame_count * attach_model->meshes[0].numverts];
                                masliasvec3vert_t* attach_vec3_vert = &attach_model->meshes[0].vec3_verts[frame_count * attach_model->meshes[0].numverts];
                                attach_vert_count = attach_model->meshes[0].numverts;
                                while (attach_vert_count--)
                                {
									// Uncompress the vertex
                                    attachment_fpos[0] = (attach_short_vert->pos[0] * attachment_scale[0]) + attachment_translate[0];    // X
                                    attachment_fpos[1] = (attach_short_vert->pos[1] * attachment_scale[1]) + attachment_translate[1];    // Y
                                    attachment_fpos[2] = (attach_short_vert->pos[2] * attachment_scale[2]) + attachment_translate[2];    // Z


                                    const vec3_t anchor_vert = { 5.096580, -2.862133, 25.506166 };
                                    // Get the direction between anchor_vert and attachment_fpos
                                    vec3_t dir;
                                    VectorSubtract(anchor_vert, attachment_fpos, dir);
                                    // Get the yaw angle of the direction
                                    float yaw = atan2(dir[1], dir[0]) * 180 / M_PI;
                                    // Get the pitch angle of the direction
                                    float pitch = atan2(dir[2], sqrt(dir[0] * dir[0] + dir[1] * dir[1])) * 180 / M_PI;
                                    // Rotate the attachment_fpos based on the yaw and pitch                                    
                                    //attachment_fpos[0] = attachment_fpos[0] * cos(yaw * M_PI / 180) - attachment_fpos[1] * sin(yaw * M_PI / 180);
                                    //attachment_fpos[1] = attachment_fpos[0] * sin(yaw * M_PI / 180) + attachment_fpos[1] * cos(yaw * M_PI / 180);
                                    //attachment_fpos[2] = attachment_fpos[2];

                                    //rotation transformation matrix
                                    float rot[3][3];
                                    float xp, yp, a;
                                    xp = 0; //plr_fpos[0];
                                    yp = 0; // plr_fpos[1];
                                    //converting angle in degrees to radians
                                    a = (pitch * M_PI) / 180;
                                    //initializing the rotation transformation matrix as per the required information
                                    rot[0][0] = cos(a);
                                    rot[1][0] = -1 * sin(a);
                                    rot[2][0] = -xp * cos(a) + yp * sin(a) + xp;
                                    rot[0][1] = sin(a);
                                    rot[1][1] = cos(a);
                                    rot[2][1] = -xp * sin(a) - yp * cos(a) + yp;
                                    rot[0][2] = 0;
                                    rot[1][2] = 0;
                                    rot[2][2] = 1;
									//
                                    int i, j, k;    // loop variables
                                    //array for storing final image matrix  
                                    float res[3];
                                    //multiplying the object matrix with rotation transformation matrix  
                                    //for (i = 0; i < attach_vert_count; i++)
                                    {
                                        i = attach_vert_count;
                                        for (j = 0; j < 3; j++)
                                        {
                                            res[j] = 0;
                                            for (k = 0; k < 3; k++)
                                            {
                                                res[j] = res[j] + attachment_fpos[k] * rot[k][j];
                                            }
                                        }
                                    }
                                    //if (frame_count == 0)
									//    Com_Printf("%s attachment_fpos[%f %f %f] res[%f %f %f]\n", __func__, attachment_fpos[0], attachment_fpos[1], attachment_fpos[2], res[0], res[1], res[2]);
                                    attachment_fpos[0] = res[0];
                                    attachment_fpos[1] = res[1];
                                    attachment_fpos[2] = res[2];

                                    //converting angle in degrees to radians
                                    a = (yaw * M_PI) / 180;
                                    //initializing the rotation transformation matrix as per the required information
                                    rot[0][0] = cos(a);
                                    rot[1][0] = -1 * sin(a);
                                    rot[2][0] = -xp * cos(a) + yp * sin(a) + xp;
                                    rot[0][1] = sin(a);
                                    rot[1][1] = cos(a);
                                    rot[2][1] = -xp * sin(a) - yp * cos(a) + yp;
                                    rot[0][2] = 0;
                                    rot[1][2] = 0;
                                    rot[2][2] = 1;
                                    //multiplying the object matrix with rotation transformation matrix  
                                    //for (i = 0; i < attach_vert_count; i++)
                                    {
                                        i = attach_vert_count;
                                        for (j = 0; j < 3; j++)
                                        {
                                            res[j] = 0;
                                            for (k = 0; k < 3; k++)
                                            {
                                                res[j] = res[j] + attachment_fpos[k] * rot[k][j];
                                            }
                                        }
                                    }
                                    //if (frame_count == 0)
                                    //    Com_Printf("%s attachment_fpos[%f %f %f] res[%f %f %f]\n", __func__, attachment_fpos[0], attachment_fpos[1], attachment_fpos[2], res[0], res[1], res[2]);
                                    attachment_fpos[0] = res[0];
                                    attachment_fpos[1] = res[1];
                                    attachment_fpos[2] = res[2];

									
									// Get new position based on attachment and player vertex positions
                                    attach_vec3_vert->pos[0] = (attachment_fpos[0]) + (plr_fpos[0]);
                                    attach_vec3_vert->pos[1] = (attachment_fpos[1]) + (plr_fpos[1]);
                                    attach_vec3_vert->pos[2] = (attachment_fpos[2]) + (plr_fpos[2]);

                                    // Radius
                                    attach_model->frames[frame_count].radius = sqrt(attach_model->frames[frame_count].bounds[0][0] * attach_model->frames[frame_count].bounds[0][0] + attach_model->frames[frame_count].bounds[0][1] * attach_model->frames[frame_count].bounds[0][1] + attach_model->frames[frame_count].bounds[0][2] * attach_model->frames[frame_count].bounds[0][2]);

                                    // Mins (positive value version)
                                    mins_fabs[0] = fabs(attach_model->frames[frame_count].bounds[0][0]);
                                    mins_fabs[1] = fabs(attach_model->frames[frame_count].bounds[0][1]);
                                    mins_fabs[2] = fabs(attach_model->frames[frame_count].bounds[0][2]);

									// scale = ( fabs(mins) + fabs(maxs) ) / 255
                                    attach_model->frames[frame_count].scale[0] = (mins_fabs[0] + attach_model->frames[frame_count].bounds[1][0]) / 255;
									attach_model->frames[frame_count].scale[1] = (mins_fabs[1] + attach_model->frames[frame_count].bounds[1][1]) / 255;
									attach_model->frames[frame_count].scale[2] = (mins_fabs[2] + attach_model->frames[frame_count].bounds[1][2]) / 255;
									
									// pos = (short)( fpos + fabs(mins) ) / scale)
                                    attach_short_copy_vert->pos[0] = (short)( (attach_vec3_vert->pos[0] + mins_fabs[0]) / attach_model->frames[frame_count].scale[0] );
                                    attach_short_copy_vert->pos[1] = (short)( (attach_vec3_vert->pos[1] + mins_fabs[1]) / attach_model->frames[frame_count].scale[1] );
                                    attach_short_copy_vert->pos[2] = (short)( (attach_vec3_vert->pos[2] + mins_fabs[2]) / attach_model->frames[frame_count].scale[2] );
                                    
                                    // translate = mins
                                    attach_model->frames[frame_count].translate[0] = attach_model->frames[frame_count].bounds[0][0];
                                    attach_model->frames[frame_count].translate[1] = attach_model->frames[frame_count].bounds[0][1];
                                    attach_model->frames[frame_count].translate[2] = attach_model->frames[frame_count].bounds[0][2];

                                    // Copy normals
                                    attach_short_copy_vert->norm[0] = plr_vert->norm[0];
									attach_short_copy_vert->norm[1] = plr_vert->norm[1];
									
                                    attach_short_copy_vert++;
                                    attach_short_vert++;
                                    attach_vec3_vert++;
                                }
                            }
                            plr_vert++;
                        }
                        frame_count++;
                    }

					
					
					// Copy the new vert data back to the original model
                    // 
                    // Hunk_TryAlloc() was run twice on the same verts (aka meshes[0].verts = Hunk_TryAlloc)
					// Will this introduce memory leakage? Or just increase the overall size of the allocated memory hunk?
                    attach_model->meshes[0].verts = Hunk_TryAlloc(&attach_model->hunk, (attach_model->meshes[0].numverts * player_model->numframes * sizeof(maliasvert_t)));
                    frame_count = 0;
                    while (frame_count < player_model->numframes) // Cycle through frames
                    {
                        plr_vert_count = player_model->meshes[0].numverts;
                        while (plr_vert_count--) // Cycle through player verts
                        {
                            // Chosen player vert
                            if (plr_vert_count == attach_to_vert)
                            {
                                maliasvert_t * attach_short_vert = &attach_model->meshes[0].verts[frame_count * attach_model->meshes[0].numverts];
                                maliasvert_t* attach_short_copy_vert = &attach_model->meshes[0].copy_verts[frame_count * attach_model->meshes[0].numverts];
                                attach_vert_count = attach_model->meshes[0].numverts;
                                while (attach_vert_count--)
                                {
									// Copy verts
                                    attach_short_vert->pos[0] = attach_short_copy_vert->pos[0];    // X
                                    attach_short_vert->pos[1] = attach_short_copy_vert->pos[1];    // Y
                                    attach_short_vert->pos[2] = attach_short_copy_vert->pos[2];    // Z

                                    // Copy normals
                                    attach_short_vert->norm[0] = attach_short_copy_vert->norm[0];
                                    attach_short_vert->norm[1] = attach_short_copy_vert->norm[1];

                                    attach_short_copy_vert++;
                                    attach_short_vert++;
                                }
                            }
                        }
                        frame_count++;
                    }
                }
                //else
                {
                    //int saved_flags = ent.flags;
                    //backlerp = ent.backlerp;
                    //frontlerp = 1.0f - backlerp;
                    //ent.flags |= RF_ATTACHMENT;
                    //ent.flags |= RF_FRAMELERP;
                    //attach_model->meshes[0].num_vec3_verts = attach_model->meshes[0].numverts;
                    //if (frontlerp == 1.0)
                    {
                        ent.model = cl.clientinfo[cl.frame.clientNum].attach_model[attachmentNum];
                        V_AddEntity(&ent);

                        //if (s1->number == cl.frame.clientNum + 1 || cl.frame.clientNum)
                        //    Com_Printf("%s attachmentNum[%d] ent.model[%d]\n", __func__, attachmentNum, ent.model);
                    }
                    //ent.flags = saved_flags;
                }

                //Com_Printf("%s cl[%d] clfnum[%d] s1->frame[%d] frame[%d %d]\n", __func__, s1->number, cl.frame.clientNum, s1->frame, cent->current.frame, cent->prev.frame);
            }
			


            //Com_Printf("%s player_model[0x%x]  model_index3[0x%x]  attach_model[0x%x]\n", __func__, &player_model, &model_index3, &attach_model);

            //attach_model
            //cl.model_draw[iiiii] = R_RegisterModel(s);
            // don't need this model
            //Hunk_Free(&model->hunk);
            //memset(model, 0, sizeof(*model));
            if (0)
            if (player_model && attach_model)
            {
				
                //Com_Printf("%s %s %s %s %s %s\n", __func__, ci->name, ci->model_name, ci->skin_name, player_model->name, attach_model->name);
                //Com_Printf("%s %s numverts[%d] numindices[%d] numtris[%d]\n", __func__, player_model->name, player_model->meshes[0].numverts, player_model->meshes[0].numindices, player_model->meshes[0].numtris);
                //180 186 172

                //ent.oldframe = cent->prev.frame;
                //Com_Printf("%s frame %d %d\n", __func__, s1->frame, ent.oldframe);
                //player_model->frames->

                //Com_Printf("%s cl[%d] s1frame[%d] frame[%d %d] pmdl[%d] amdl[%d]\n", __func__, s1->number - 1, s1->frame, cent->current.frame, cent->prev.frame, cl.clientinfo[s1->number - 1].model, cl.clientinfo[s1->number - 1].attach_model[attachmentNum]);

                //ent->
                //player_model->meshes[0].verts[180].pos[0]
                //src_oldvert->pos[0] * oldscale[0] + src_newvert->pos[0] * newscale[0] + translate[0];
                //Com_Printf("%s frame %d %d [%f %f %f] [%f %f %f]\n", __func__, s1->frame, ent.oldframe, player_model->frames[s1->frame].scale[0], player_model->frames[s1->frame].scale[1], player_model->frames[s1->frame].scale[2], player_model->frames[ent.oldframe].scale[0], player_model->frames[ent.oldframe].scale[1], player_model->frames[ent.oldframe].scale[2]);
                //Com_Printf("%s frame %d [%f %f %f] [%f %f %f]\n", __func__, s1->frame, player_model->frames[s1->frame].scale[0], player_model->frames[s1->frame].scale[1], player_model->frames[s1->frame].scale[2], player_model->frames[s1->frame].translate[0], player_model->frames[s1->frame].translate[1], player_model->frames[s1->frame].translate[2]);
                //Com_Printf("%s vert[180].[%d %d %d]\n", __func__, player_model->meshes[0].verts[180].pos[0], player_model->meshes[0].verts[180].pos[1], player_model->meshes[0].verts[180].pos[2]);

                //cent = &cl_entities[s1->number];

                //maliasvert_t* src_oldvert = &player_model->meshes[0].verts[ent.oldframe * player_model->meshes[0].numverts];
                //maliasvert_t* src_newvert = &player_model->meshes[0].verts[ent.frame * player_model->meshes[0].numverts];

                maliasvert_t* org_oldvert = &player_model->meshes[0].verts[39 * player_model->meshes[0].numverts];
                maliasvert_t* org_newvert = &player_model->meshes[0].verts[0 * player_model->meshes[0].numverts];
				
                maliasvert_t* src_oldvert = &player_model->meshes[0].verts[cent->prev.frame * player_model->meshes[0].numverts];
                maliasvert_t* src_newvert = &player_model->meshes[0].verts[cent->current.frame * player_model->meshes[0].numverts];
                vec_t* dst_vert = tess.vertices;
                vec3_t new_vert;
                vec3_t old_vert;
                int count = player_model->meshes[0].numverts;
                //vec3_t oldscale = { player_model->frames[ent.oldframe].scale[0], player_model->frames[ent.oldframe].scale[1], player_model->frames[ent.oldframe].scale[2] };
                //vec3_t newscale = { player_model->frames[ent.frame].scale[0], player_model->frames[ent.frame].scale[1], player_model->frames[ent.frame].scale[2] };
                //vec3_t translate = { player_model->frames[ent.frame].translate[0], player_model->frames[ent.frame].translate[1], player_model->frames[ent.frame].translate[2] };

                vec3_t oldscale = { player_model->frames[cent->prev.frame].scale[0], player_model->frames[cent->prev.frame].scale[1], player_model->frames[cent->prev.frame].scale[2] };
                vec3_t newscale = { player_model->frames[cent->current.frame].scale[0], player_model->frames[cent->current.frame].scale[1], player_model->frames[cent->current.frame].scale[2] };
                vec3_t oldtranslate = { player_model->frames[cent->prev.frame].translate[0], player_model->frames[cent->prev.frame].translate[1], player_model->frames[cent->prev.frame].translate[2] };
                vec3_t translate = { player_model->frames[cent->current.frame].translate[0], player_model->frames[cent->current.frame].translate[1], player_model->frames[cent->current.frame].translate[2] };
                
				
                vec3_t prev_bounds_mins = { player_model->frames[cent->prev.frame].bounds[0][0], player_model->frames[cent->prev.frame].bounds[0][1], player_model->frames[cent->prev.frame].bounds[0][2] };
                vec3_t prev_bounds_maxs = { player_model->frames[cent->prev.frame].bounds[1][0], player_model->frames[cent->prev.frame].bounds[1][1], player_model->frames[cent->prev.frame].bounds[1][2] };
				vec3_t bounds_mins = { player_model->frames[cent->current.frame].bounds[0][0], player_model->frames[cent->current.frame].bounds[0][1], player_model->frames[cent->current.frame].bounds[0][2] };
                vec3_t bounds_maxs = { player_model->frames[cent->current.frame].bounds[1][0], player_model->frames[cent->current.frame].bounds[1][1], player_model->frames[cent->current.frame].bounds[1][2] };
                //vec3_t bounds_mins = { player_model->frames[0].bounds[0][0], player_model->frames[0].bounds[0][1], player_model->frames[0].bounds[0][2] };
                //vec3_t bounds_maxs = { player_model->frames[0].bounds[1][0], player_model->frames[0].bounds[1][1], player_model->frames[0].bounds[1][2] };
                float prev_multiplier_x = 0;
                float prev_multiplier_y = 0;
                float prev_multiplier_z = 0;
                float prev_fpos_x = 0;
                float prev_fpos_y = 0;
                float prev_fpos_z = 0;
                float multiplier_x = 0;
                float multiplier_y = 0;
                float multiplier_z = 0;
                float fpos_x = 0;
				float fpos_y = 0;
				float fpos_z = 0;
				//[0] = 0x0000027c329b0158 {-12.3710165, -13.1677227, -24.1077061}
                //[1] = 0x0000027c329b0164 {14.1422987, 6.15525818, 25.7014942}
                //if (cent->current.frame == 0)
                //    int iiiii = 0;

                

                /*
                // Cache the attachment verts
                if (attachment_verts[s1->number - 1][0].pos[0] == 0 && attachment_verts[s1->number - 1][0].pos[1] == 0 && attachment_verts[s1->number - 1][0].pos[2] == 0)
                {
                    int amv_count = attach_model->meshes[0].numverts;
                    while (amv_count--)
                    {
                        attachment_verts[s1->number - 1][amv_count].pos[0] = attach_model->meshes[0].verts[amv_count].pos[0];
                        attachment_verts[s1->number - 1][amv_count].pos[1] = attach_model->meshes[0].verts[amv_count].pos[1];
                        attachment_verts[s1->number - 1][amv_count].pos[2] = attach_model->meshes[0].verts[amv_count].pos[2];
                    }
                }
                */

				
                vec3_t normal = {0};
                float scale = 0.1;
                backlerp = ent.backlerp;
                frontlerp = 1.0f - backlerp;

                if (gl_vert_diff->value)
                {
                    MOD_GetModelDiff(player_model->meshes[0].numverts, &player_model->meshes[0].verts[0]);
                }

                if (cent->current.frame != 0)
                {
                    //Com_Printf("%s\n", __func__);
                }

				
                
				
                // Run once per frame	
                //if (last_client_frame[s1->number - 1] != s1->frame)
                src_oldvert = &player_model->meshes[0].verts[cent->prev.frame * player_model->meshes[0].numverts];
                src_newvert = &player_model->meshes[0].verts[cent->current.frame * player_model->meshes[0].numverts];
                count = player_model->meshes[0].numverts;
                if (0)
                while (count--)
                {
                    //last_client_frame[s1->number - 1] = s1->frame;

                    //Com_Printf("%s cl[%d] s1frame[%d] frame[%d %d] pmdl[%d] amdl[%d]\n", __func__, s1->number - 1, s1->frame, cent->current.frame, cent->prev.frame, cl.clientinfo[s1->number - 1].model, cl.clientinfo[s1->number - 1].attach_model[attachmentNum]);

                    if (cent->current.frame == 0)
                    {
                        //Com_Printf("%s frame[%d] vert[%d/%d].[%d %d %d]\n", __func__, cent->current.frame, count, player_model->meshes[0].numverts - 1, src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2]);
						
						//Com_Printf("%s frame[%d] vert[%d/%d] src_oldvert[%d %d %d] oldscale[%f %f %f] src_newvert[%d %d %d] newscale[%f %f %f] translate[%f %f %f]\n", __func__, cent->current.frame, count, player_model->meshes[0].numverts - 1, src_oldvert->pos[0], src_oldvert->pos[1], src_oldvert->pos[2], oldscale[0], oldscale[1], oldscale[2], src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2], newscale[0], newscale[1], newscale[2], translate[0], translate[1], translate[2]);
                    }
                    else
                    {
                        //break;
                    }
					
                    /*
                    get_lerped_normal(normal, src_oldvert, src_newvert);
                    dst_vert[0] =
                        normal[0] * scale +
                        src_oldvert->pos[0] * oldscale[0] +
                        src_newvert->pos[0] * newscale[0] + translate[0];
                    dst_vert[1] =
                        normal[1] * scale +
                        src_oldvert->pos[1] * oldscale[1] +
                        src_newvert->pos[1] * newscale[1] + translate[1];
                    dst_vert[2] =
                        normal[2] * scale +
                        src_oldvert->pos[2] * oldscale[2] +
                        src_newvert->pos[2] * newscale[2] + translate[2];
                    */
					/*
                    dst_vert[0] =
                        src_oldvert->pos[0] * oldscale[0] +
                        src_newvert->pos[0] * newscale[0] + translate[0];
                    dst_vert[1] =
                        src_oldvert->pos[1] * oldscale[1] +
                        src_newvert->pos[1] * newscale[1] + translate[1];
                    dst_vert[2] =
                        src_oldvert->pos[2] * oldscale[2] +
                        src_newvert->pos[2] * newscale[2] + translate[2];
					*/

                    //dst_vert[0] = (src_newvert->pos[0] * newscale[0]) + translate[0];
                    //dst_vert[1] = (src_newvert->pos[1] * newscale[1]) + translate[1];
                    //dst_vert[2] = (src_newvert->pos[2] * newscale[2]) + translate[2];


                    if (org_vert[s1->number - 1][0] == 0 && org_vert[s1->number - 1][1] == 0 && org_vert[s1->number - 1][2] == 0)
                    {
                        org_vert[s1->number - 1][0] = dst_vert[0];
                        org_vert[s1->number - 1][1] = dst_vert[1];
                        org_vert[s1->number - 1][2] = dst_vert[2];
                    }

                    
                    new_vert[0] = (src_newvert->pos[0] * newscale[0]) + translate[0];
                    new_vert[1] = (src_newvert->pos[1] * newscale[1]) + translate[1];
                    new_vert[2] = (src_newvert->pos[2] * newscale[2]) + translate[2];
                    old_vert[0] = (src_oldvert->pos[0] * oldscale[0]) + oldtranslate[0];
                    old_vert[1] = (src_oldvert->pos[1] * oldscale[1]) + oldtranslate[1];
                    old_vert[2] = (src_oldvert->pos[2] * oldscale[2]) + oldtranslate[2];
					
                    // count == 118     Middle of upper shoulder blades
					// count == 100     Right shoulder blade



					

                    //org_vert[s1->number - 1][0] = 7.660989;
                    //org_vert[s1->number - 1][1] = 21.690310;
                    //org_vert[s1->number - 1][2] = 59.229729;


					


//vert[266/472] src_oldvert[155 131 255] oldscale[0.103610 0.076111 0.193184] src_newvert[153 128 255] newscale[0.103974 0.075776 0.195330] translate[-12.371017 -13.167723 -24.107706]
//vert[253/472] src_oldvert[155 131 255] oldscale[0.103610 0.076111 0.193184] src_newvert[153 128 255] newscale[0.103974 0.075776 0.195330] translate[-12.371017 -13.167723 -24.107706]
					// (155 * 0.103610) + (153 * 0.103974) + -12.371017
					// 16.05955 + 15.908022 + -12.371017 = 19.596556
					// (131 * 0.076111) + (128 * 0.075776) + -13.167723
					// 9.970541 + 9.699328 + -13.167723 = 6.502146
					// (255 * 0.193184) + (255 * 0.195330) + -24.107706
					// 49.26192 + 49.80915 + -24.107706 = 74.963364
                    /*
                    // pos[0] (Z in Misfit Model 3D)
                    MOD_GetModelDiff Vertex 266 changed from [153 128 255] to [163 128 255]
                    MOD_GetModelDiff Vertex 253 changed from [153 128 255] to [163 128 255]
					// pos[1]  (X in Misfit Model 3D)

					// pos[2]  (Y in Misfit Model 3D)
					MOD_GetModelDiff Vertex 266 changed from [153 128 255] to [153 128 251]
					MOD_GetModelDiff Vertex 253 changed from [153 128 255] to [153 128 251]
					
					X = 3.536972    ->  4.536972
                    X = 153         ->  163
					
                    Y = -3.468344   ->
					
                    Z = 25.701495   ->  24.701495
					

					
					//mins[0] = {-12.3710165, -13.1677227, -24.1077061}
                    //maxs[1] = {14.1422987, 6.15525818, 25.7014942}

					
					X = 14.1422987 / 128 = 0.11048670859375
					Y = -3.468344
					Z = 25.701495 / 128 = 0.2007929296875
					Z = 0.2007929296875 * (255 - 128)

f[39] newvert[155 131 255] fpos[3.698956 -3.250379 25.190613]
f[39] newvert[155 131 255] fpos[2.404928 -2.491425 7.630130]
f[0] newvert[153 128 255] fpos[3.536972 -3.468344 25.701494]
f[0] newvert[153 128 255] fpos[2.291564 -2.670241 7.699349]
f[1] newvert[154 129 255] fpos[3.639995 -3.403536 25.152378]
f[1] newvert[154 129 255] fpos[2.353810 -2.608329 7.616862]
					
					*/

                    if (count == 253 || count == 266)
                    {					
                        //maliasvert_t* src_attach_vert = &src_newvert;

                        {
                            // X
                            //multiplier_x = (bounds_maxs[0] / 128);
                            //fpos_x = (src_newvert->pos[0] - 128) * multiplier_x;
                            fpos_x = (src_newvert->pos[0] * newscale[0]) + translate[0];
                            //if (src_newvert->pos[0] == 255)
                            //    fpos_x = (src_newvert->pos[0] - 127) * multiplier_x;
							
                            //prev_multiplier_x = (prev_bounds_maxs[0] / 128);
                            prev_fpos_x = (src_oldvert->pos[0] * oldscale[0]) + oldtranslate[0];
                            //if (src_oldvert->pos[0] == 255)
                            //    prev_fpos_x = (src_oldvert->pos[0] - 127) * oldscale[0] + oldtranslate[0]; //prev_multiplier_x;



                            // Y
                            //multiplier_y = (bounds_maxs[1] / 128);
                            // fpos_y = (src_newvert->pos[1] - 128) * multiplier_y;
                            fpos_y = (src_newvert->pos[1] * newscale[1]) + translate[1];
                            //if (src_newvert->pos[1] == 255)
                            //    fpos_y = (src_newvert->pos[1] - 127) * newscale[1] + translate[1]; //multiplier_y;

                            //prev_multiplier_y = (prev_bounds_maxs[1] / 128);
                            prev_fpos_y = (src_oldvert->pos[1] * oldscale[1]) + oldtranslate[1];
                            //if (src_oldvert->pos[1] == 255)
                            //    prev_fpos_y = (src_oldvert->pos[1] - 127) * prev_multiplier_y;
							
							
							
                            // Z
                            //multiplier_z = (bounds_maxs[2] / 128);
                            //fpos_z = (src_newvert->pos[2] - 128) * multiplier_z;
                            fpos_z = (src_newvert->pos[2] * newscale[2]) + translate[2];
                            //if (src_newvert->pos[2] == 255)
                            //    fpos_z = (src_newvert->pos[2] - 128) * multiplier_z;
							
                            prev_fpos_z = (src_oldvert->pos[2] * oldscale[2]) + oldtranslate[2];
							//prev_multiplier_z = (prev_bounds_maxs[2] / 128);
                            //prev_fpos_z = (src_oldvert->pos[2] - 128) * prev_multiplier_z;
                            //if (src_oldvert->pos[2] == 255)
                            //    prev_fpos_z = (src_oldvert->pos[2] - 127) * prev_multiplier_z;
                        }

                        //if (cent->current.frame == 0)
                        //    int iiiii = 0;
						

						
						
						
                        vec3_t v_diff = { 0 };
                        v_diff[0] = fpos_z - prev_fpos_z;
                        v_diff[1] = fpos_z - prev_fpos_z;
                        v_diff[2] = fpos_z - prev_fpos_z;

                        if (last_client_frame[s1->number - 1] != s1->frame)
                        {
                            last_client_frame[s1->number - 1] = s1->frame;
                            //Com_Printf("%s f[%d] blerp[%f] flerp[%f] newvert[%d] fpos[%f] oldvert[%d] prev_fpos[%f] diff[%f]\n", __func__, s1->frame, backlerp, frontlerp, src_newvert->pos[2], fpos_z, src_oldvert->pos[2], prev_fpos_z, v_diff[2]);
							//Com_Printf("%s f[%d] newvert[%d %d %d] fpos[%f %f %f] prev_fpos[%f %f %f]\n", __func__, s1->frame, src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2], fpos_x, fpos_y, fpos_z, prev_fpos_x, prev_fpos_y, prev_fpos_z);
							
                            //float bdiff = bounds_maxs[0] - bounds_mins[0];
                            //fpos_x *= (attach_model->frames[0].bounds[1][0] - attach_model->frames[0].bounds[0][0]) / (bounds_maxs[0] - bounds_mins[0]);
                            //fpos_y *= (attach_model->frames[0].bounds[1][1] - attach_model->frames[0].bounds[0][1]) / (bounds_maxs[1] - bounds_mins[1]);
                            //fpos_z *= (attach_model->frames[0].bounds[1][2] - attach_model->frames[0].bounds[0][2]) / (bounds_maxs[2] - bounds_mins[2]);

                            //Com_Printf("%s f[%d] newvert[%d %d %d] fpos[%f %f %f] prev_fpos[%f %f %f]\n", __func__, s1->frame, src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2], fpos_x, fpos_y, fpos_z, prev_fpos_x, prev_fpos_y, prev_fpos_z);

							//Com_Printf("%s angles[%f %f %f]\n", __func__, ent.angles[0], ent.angles[1], ent.angles[2]);
                        }
                        else
                        {
                            v_diff[0] = 0;
                            v_diff[1] = 0;
                            v_diff[2] = 0;


                            //fpos_x *= (attach_model->frames[0].bounds[1][0] - attach_model->frames[0].bounds[0][0]) / (bounds_maxs[0] - bounds_mins[0]);
                            //fpos_y *= (attach_model->frames[0].bounds[1][1] - attach_model->frames[0].bounds[0][1]) / (bounds_maxs[1] - bounds_mins[1]);
                            //fpos_z *= (attach_model->frames[0].bounds[1][2] - attach_model->frames[0].bounds[0][2]) / (bounds_maxs[2] - bounds_mins[2]);


                            //fpos_x *= (attach_model->frames[0].bounds[1][0] - attach_model->frames[0].bounds[0][0]) / (player_model->frames[0].bounds[1][0] - player_model->frames[0].bounds[0][0]);
                            //fpos_y *= (attach_model->frames[0].bounds[1][1] - attach_model->frames[0].bounds[0][1]) / (player_model->frames[0].bounds[1][1] - player_model->frames[0].bounds[0][1]);
                            //fpos_z *= (attach_model->frames[0].bounds[1][2] - attach_model->frames[0].bounds[0][2]) / (player_model->frames[0].bounds[1][2] - player_model->frames[0].bounds[0][2]);
                            
                        }

                        attach_model->frames[0].radius = player_model->frames[cent->prev.frame].radius;
                        attach_model->frames[0].bounds[0][0] = bounds_mins[0];
						attach_model->frames[0].bounds[0][1] = bounds_mins[1];
						attach_model->frames[0].bounds[0][2] = bounds_mins[2];
						attach_model->frames[0].bounds[1][0] = bounds_maxs[0];
						attach_model->frames[0].bounds[1][1] = bounds_maxs[1];
						attach_model->frames[0].bounds[1][2] = bounds_maxs[2];
						
                        //fpos_x *= (attach_model->frames[0].bounds[1][0] - attach_model->frames[0].bounds[0][0]) / (bounds_maxs[0] - bounds_mins[0]);
                        //fpos_y *= (attach_model->frames[0].bounds[1][1] - attach_model->frames[0].bounds[0][1]) / (bounds_maxs[1] - bounds_mins[1]);
                        //fpos_z *= (attach_model->frames[0].bounds[1][2] - attach_model->frames[0].bounds[0][2]) / (bounds_maxs[2] - bounds_mins[2]);

						//Com_Printf("%s f[%d] newvert[%d %d %d] fpos[%f %f %f]\n", __func__, s1->frame, src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2], fpos_x, fpos_y, fpos_z);

                        //attachment_offsets[s1->number - 1][count].pos[2] += v_diff[2];

                        //ent.origin[2] = ent.origin[2] + attachment_offsets[s1->number - 1][count].pos[2];
						
                        vec3_t old_diff = { 0 };
                        vec3_t new_diff = { 0 };
                        VectorCopy(ent.oldorigin, old_diff);
                        //VectorCopy(ent.origin, old_diff);
                        VectorCopy(ent.origin, new_diff);
                        //old_diff[0] = (ent.oldorigin[0] + prev_fpos_x) - player_model->frames[0].bounds[1][0];
                        //old_diff[1] = (ent.oldorigin[1] + prev_fpos_y) - player_model->frames[0].bounds[1][1];
                        //new_diff[0] = (ent.origin[0] + fpos_x) - player_model->frames[0].bounds[1][0];
                        //new_diff[1] = (ent.origin[1] + fpos_y) - player_model->frames[0].bounds[1][1];
						
                        old_diff[2] = (ent.oldorigin[0] + prev_fpos_x) + attachment_offsets[s1->number - 1][count].pos[0];
                        old_diff[2] = (ent.oldorigin[1] + prev_fpos_y) + attachment_offsets[s1->number - 1][count].pos[1];
                        old_diff[2] = (ent.oldorigin[2] + prev_fpos_z) + attachment_offsets[s1->number - 1][count].pos[2];
                        new_diff[2] = (ent.origin[0] + fpos_x) + attachment_offsets[s1->number - 1][count].pos[0];
                        new_diff[2] = (ent.origin[1] + fpos_y) + attachment_offsets[s1->number - 1][count].pos[1];
                        new_diff[2] = (ent.origin[2] + fpos_z) + attachment_offsets[s1->number - 1][count].pos[2];
                        //LerpVector2(old_diff, new_diff, backlerp, frontlerp, ent.origin);

                        
                        attachment_offsets[s1->number - 1][count].pos[0] = new_diff[0] - ent.origin[0];
                        attachment_offsets[s1->number - 1][count].pos[1] = new_diff[1] - ent.origin[1];
                        attachment_offsets[s1->number - 1][count].pos[2] = new_diff[2] - ent.origin[2];
						

                        //ent.oldorigin[0] = (ent.oldorigin[0] + prev_fpos_x);
                        //ent.oldorigin[1] = (ent.oldorigin[1] + prev_fpos_y);
                        //ent.oldorigin[2] = (ent.oldorigin[2] + prev_fpos_z);
                        //ent.origin[0] = (ent.origin[0] + (fpos_x)); //- 2.5;
                        //ent.origin[1] = (ent.origin[1] + (fpos_y)); //+ 2.5;
                        //ent.origin[2] = (ent.origin[2] + (fpos_z));

						//cent->current.origin[0] = (cent->current.origin[0] + (fpos_x));
						//cent->current.origin[1] = (cent->current.origin[1] + (fpos_y));
						//cent->current.origin[2] = (cent->current.origin[2] + (fpos_z));

                        //old_diff[2] = (ent.oldorigin[0] + prev_fpos_x);
                        //old_diff[2] = (ent.oldorigin[1] + prev_fpos_y);
                        //old_diff[2] = (ent.oldorigin[2] + prev_fpos_z);
                        //new_diff[2] = (ent.origin[0] + fpos_x);
                        //new_diff[2] = (ent.origin[1] + fpos_y);
                        //new_diff[2] = (ent.origin[2] + fpos_z);
                        //LerpVector(old_diff, new_diff, frontlerp, ent.origin);
						
                        //ent.origin[2] += 1;
                        //cent->current.origin[2] += 5;
                        //ent.angles[0] += 1;

                        //if (last_client_frame[s1->number - 1] != s1->frame)
                        {
                            //last_client_frame[s1->number - 1] = s1->frame;
                            //Com_Printf("%s newvert[%d %d %d] fpos[%f %f %f] diff[%f %f %f]\n", __func__, src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2], fpos_x, fpos_y, fpos_z, v_diff[0], v_diff[1], v_diff[2]);




                            vec3_t vert_diff = { 0 };
                            //vert_diff[0] = src_attach_vert->pos[0] - dst_vert[0];
                            //vert_diff[1] = src_attach_vert->pos[1] - dst_vert[1];
                            //vert_diff[2] = src_attach_vert->pos[2] - dst_vert[2];
                            vert_diff[0] = src_newvert->pos[0] - dst_vert[0];
                            vert_diff[1] = src_newvert->pos[1] - dst_vert[1];
                            vert_diff[2] = src_newvert->pos[2] - dst_vert[2];
                            //Com_Printf("%s frame[%d] src_attach_vert[%d %d %d]  dst_vert[%f %f %f]  vert_diff[%f %f %f]\n", __func__, cent->current.frame, src_attach_vert->pos[0], src_attach_vert->pos[1], src_attach_vert->pos[2], dst_vert[0], dst_vert[1], dst_vert[2], vert_diff[0], vert_diff[1], vert_diff[2]);

                            int amv_count = attach_model->meshes[0].numverts;
                            while (amv_count--)
                            {
                                vec3_t new_origin;

                                vert_diff[0] = new_vert[0] - org_vert[s1->number - 1][0];
                                vert_diff[1] = new_vert[1] - org_vert[s1->number - 1][1];
                                vert_diff[2] = new_vert[2] - org_vert[s1->number - 1][2];


                                if (cent->current.frame >= FRAME_stand01 && cent->current.frame <= FRAME_stand40)
                                    new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * multiplier_x);// - 112; // 56 * 2 = 112
                                else if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                                    new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * multiplier_x);// - 336; // 56 * 6 = 336
                                else
                                    new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * multiplier_x);// - 224; // 56 * 4 = 224

                                if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                                    new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * multiplier_y);// - 168; // 56 * 3 = 168
                                else
                                    new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * multiplier_y);// - 224; // 56 * 4 = 224

                                if (cent->current.frame >= FRAME_jump1 && cent->current.frame <= FRAME_jump2)
                                    new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * multiplier_z);// - 84; // 56
                                else if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                                    new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * multiplier_z);// - 168; // 56 * 3 = 168
                                else
                                    new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * multiplier_z);// - 126; // 56 + (56 / 2) = 84


                                //old_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0] + prev_fpos_x);
                                //old_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1] + prev_fpos_y);
                                //old_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2] + prev_fpos_z);
                                //new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0] + fpos_x);
                                //new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1] + fpos_y);
                                //new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2] + fpos_z);
                                //LerpVector2(old_origin, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);

                                //attach_model->frames[]

                                attach_model->meshes[0].verts[amv_count].pos[0] = (attachment_verts[s1->number - 1][amv_count].pos[0] + fpos_x);
								attach_model->meshes[0].verts[amv_count].pos[1] = (attachment_verts[s1->number - 1][amv_count].pos[1] + fpos_y);
								attach_model->meshes[0].verts[amv_count].pos[2] = (attachment_verts[s1->number - 1][amv_count].pos[2] + fpos_z);
								
                                

                                //new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + fpos_z;
								//new_origin[2] = attach_model->meshes[0].verts[amv_count].pos[2] + v_diff[2];
								
                                //LerpVector2(attach_model->meshes[0].verts[amv_count].pos, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);

                                //if (attach_model->meshes[0].verts[amv_count].pos[2] == 0)
                                //    attach_model->meshes[0].verts[amv_count].pos[2] = attachment_verts[s1->number - 1][amv_count].pos[2];
                                //else
                                //    attach_model->meshes[0].verts[amv_count].pos[2] += v_diff[2];

                            }
                        }
                    }










					if (0)
					if (count == 100)
                    {
                        //Com_Printf("%s frame[%d] vert[%d].[%f %f %f]\n", __func__, cent->current.frame, count, newscale[0], newscale[1], newscale[2]);
                        //Com_Printf("%s frame[%d] vert[%d].[%f %f %f]\n", __func__, cent->current.frame, count, translate[0], translate[1], translate[2]);
						
                        //Com_Printf("%s frame[%d] vert[%d].[%f %f %f]\n", __func__, cent->current.frame, count, dst_vert[0], dst_vert[1], dst_vert[2]);
                        //Com_Printf("%s frame[%d] vert[%d].[%d %d %d]\n", __func__, cent->current.frame, count, src_newvert->pos[0], src_newvert->pos[1], src_newvert->pos[2]);

						/*
                        get_lerped_normal(normal, org_oldvert, org_newvert);
                        org_vert[0] =
                            normal[0] * scale +
                            org_oldvert->pos[0] * player_model->frames[39].scale[0] +
                            org_newvert->pos[0] * player_model->frames[0].scale[0] + player_model->frames[0].translate[0];
                        org_vert[1] =
                            normal[1] * scale +
                            org_oldvert->pos[1] * player_model->frames[39].scale[1] +
                            org_newvert->pos[1] * player_model->frames[count].scale[1] + player_model->frames[0].translate[1];
                        org_vert[2] =
                            normal[2] * scale +
                            org_oldvert->pos[2] * player_model->frames[39].scale[2] +
                            org_newvert->pos[2] * player_model->frames[0].scale[2] + player_model->frames[0].translate[2];
						*/

						//Com_Printf("%s frame[%d] vert[%d] dst_vert[%f %f %f] org_vert[%f %f %f] dst_vert-org_vert[%f %f %f]\n", __func__, cent->current.frame, count, dst_vert[0], dst_vert[1], dst_vert[2], org_vert[s1->number - 1][0], org_vert[s1->number - 1][1], org_vert[s1->number - 1][2], dst_vert[0] - org_vert[s1->number - 1][0], dst_vert[1] - org_vert[s1->number - 1][1], dst_vert[2] - org_vert[s1->number - 1][2]);
                        //Com_Printf("%s frame[%d] vert[%d] dst_vert[%f %f %f] org_vert[%f %f %f]\n", __func__, cent->current.frame, count, dst_vert[0], dst_vert[1], dst_vert[2], org_vert[s1->number - 1][0], org_vert[s1->number - 1][1], org_vert[s1->number - 1][2]);

						//dst_vert      src_newvert
						//45.957470     169
						//15.465382     133
                        //---------     ---
                        //30.492088     36

						// Misfit Model - male.md2
                        //3.351532, 13.395693, -0.102110
						//1.824166, 2.695348, -5.218136

                        //V_AddLight(src_newvert->pos, 130, 1, 1, 1);
						
                        //Com_Printf("%s frame[%d] origin[%f %f %f]\n", __func__, cent->current.frame, ent.origin[0], ent.origin[1], ent.origin[2]);

                        //vec3_t frame_scale = { player_model->frames[0].scale[0], player_model->frames[0].scale[1], player_model->frames[0].scale[2] }; //
						//Com_Printf("%s frame[%d] org_scale[%f %f %f]\n", __func__, cent->current.frame, frame_scale[0], frame_scale[1], frame_scale[2]);

                        //maliasvert_t* src_init_vert = &player_model->meshes[0].verts[0];
                        //maliasvert_t* src_curr_vert = &player_model->meshes[0].verts[cent->current.frame * player_model->meshes[0].numverts];
						//Com_Printf("%s src_init_vert[%d %d %d]  src_curr_vert[%d %d %d]   src_init_vert-src_curr_vert[%d %d %d]\n", __func__, src_init_vert->pos[0], src_init_vert->pos[1], src_init_vert->pos[2], src_curr_vert->pos[0], src_curr_vert->pos[1], src_curr_vert->pos[2], src_init_vert->pos[0] - src_curr_vert->pos[0], src_init_vert->pos[1] - src_curr_vert->pos[1], src_init_vert->pos[2] - src_curr_vert->pos[2]);
						
                        


                        maliasvert_t* src_attach_vert = &src_newvert;
                        
                        //maliasvert_t* src_attach_vert = &player_model->meshes[0].verts[count * player_model->meshes[0].numverts];

                        vec3_t vert_diff = { 0 };
						vert_diff[0] = src_attach_vert->pos[0] - dst_vert[0];
						vert_diff[1] = src_attach_vert->pos[1] - dst_vert[1];
						vert_diff[2] = src_attach_vert->pos[2] - dst_vert[2];
						//Com_Printf("%s frame[%d] src_attach_vert[%d %d %d]  dst_vert[%f %f %f]  vert_diff[%f %f %f]\n", __func__, cent->current.frame, src_attach_vert->pos[0], src_attach_vert->pos[1], src_attach_vert->pos[2], dst_vert[0], dst_vert[1], dst_vert[2], vert_diff[0], vert_diff[1], vert_diff[2]);

                        /*
                        dst_vert[0] =
                            normal[0] * scale +
                            player_model->meshes[0].verts[0].pos[0] * player_model->frames[0].scale[0] +
                            src_newvert->pos[0] * newscale[0] + translate[0];
                        dst_vert[1] =
                            normal[1] * scale +
                            player_model->meshes[0].verts[0].pos[1] * player_model->frames[0].scale[1] +
                            src_newvert->pos[1] * newscale[1] + translate[1];
                        dst_vert[2] =
                            normal[2] * scale +
                            player_model->meshes[0].verts[0].pos[2] * player_model->frames[0].scale[2] +
                            src_newvert->pos[2] * newscale[2] + translate[2];
                        vert_diff[0] = player_model->meshes[0].verts[0].pos[0] - dst_vert[0];
                        vert_diff[1] = player_model->meshes[0].verts[0].pos[1] - dst_vert[1];
                        vert_diff[2] = player_model->meshes[0].verts[0].pos[2] - dst_vert[2];
						*/
						
						
                        int amv_count = attach_model->meshes[0].numverts;
                        while (amv_count--)
                        {
                            //attach_model->meshes[0].verts[amv_count].pos[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + src_newvert->pos[0];
                            //attach_model->meshes[0].verts[amv_count].pos[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + src_newvert->pos[1];
                            //attach_model->meshes[0].verts[amv_count].pos[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2];

                            //attach_model->meshes[0].verts[amv_count].pos[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + (dst_vert[0] * 3);
                            //attach_model->meshes[0].verts[amv_count].pos[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + (dst_vert[1] * 3);
                            //attach_model->meshes[0].verts[amv_count].pos[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + (dst_vert[2] * 3);
                            
                            //0.195540
							//0.138774


                            // Interpolate origin
                            vec3_t old_origin, new_origin;

						    /*
                            old_origin[0] = ((normal[0] * scale) + attachment_verts[s1->number - 1][amv_count].pos[0] + src_oldvert->pos[0]); //* (oldscale[0] / player_model->frames[0].scale[0]);
                            old_origin[1] = ((normal[1] * scale) + attachment_verts[s1->number - 1][amv_count].pos[1] + src_oldvert->pos[1]); //* (oldscale[1] / player_model->frames[0].scale[1]);
                            if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crstnd19)
                                old_origin[2] = ((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]); // * (oldscale[2] / player_model->frames[0].scale[2]) - 32;
                            else if (cent->prev.frame >= FRAME_jump1 && cent->prev.frame <= FRAME_jump6)
                                old_origin[2] = ((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]); // * (oldscale[2] / player_model->frames[0].scale[2]) + 56;
                            else
                                old_origin[2] = ((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]); //* (oldscale[2] / player_model->frames[0].scale[2]);
							
							
                            new_origin[0] = ((normal[0] * scale) + attachment_verts[s1->number - 1][amv_count].pos[0] + src_newvert->pos[0]); //* (newscale[0] / player_model->frames[0].scale[0]);
                            new_origin[1] = ((normal[1] * scale) + attachment_verts[s1->number - 1][amv_count].pos[1] + src_newvert->pos[1]); //* (newscale[1] / player_model->frames[0].scale[1]);
                            if (cent->current.frame >= FRAME_crstnd01 && cent->current.frame <= FRAME_crstnd19)
                                new_origin[2] = ((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]); //* (newscale[2] / player_model->frames[0].scale[2]) - 32;
                            else if (cent->current.frame >= FRAME_jump1 && cent->current.frame <= FRAME_jump6)
                                new_origin[2] = ((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]); //* (newscale[2] / player_model->frames[0].scale[2]) + 56;
                            else
                                new_origin[2] = ((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]); //* (newscale[2] / player_model->frames[0].scale[2]);
                            */
								
                            /*
                            old_origin[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + src_oldvert->pos[0];
                            old_origin[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + src_oldvert->pos[1];
                            old_origin[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2];

                            new_origin[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + src_newvert->pos[0];
                            new_origin[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + src_newvert->pos[1];
                            new_origin[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2];
							*/

							// Stand02 = 31.256542
                            // FRAME_jump1 = 15.261757
							// FRAME_jump2 = 21.771744
							// FRAME_jump3 = 29.474060
							// FRAME_jump4 = 42.276993
							// FRAME_jump5 = 52.597481
							// FRAME_jump6 = 39.954739
							// Stand01 = 30.700172
                            // Stand02 = 31.256542
                            /*
                            old_origin[0] = (((normal[0] * scale) + attachment_verts[s1->number - 1][amv_count].pos[0] + src_oldvert->pos[0]) - vert_diff[0] * 5);
                            old_origin[1] = (((normal[1] * scale) + attachment_verts[s1->number - 1][amv_count].pos[1] + src_oldvert->pos[1]) - vert_diff[1] * 5);
                            if (cent->prev.frame >= FRAME_jump1 && cent->prev.frame <= FRAME_jump2)
                                old_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]) + vert_diff[2]);
                            else if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                                old_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]) - vert_diff[2] * 3.25);
                            else
                                old_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]) - vert_diff[2] * 2);
							

                            new_origin[0] = (((normal[0] * scale) + attachment_verts[s1->number - 1][amv_count].pos[0] + src_newvert->pos[0]) - vert_diff[0] * 5);
                            new_origin[1] = (((normal[1] * scale) + attachment_verts[s1->number - 1][amv_count].pos[1] + src_newvert->pos[1]) - vert_diff[1] * 5);
							
                            if (cent->current.frame >= FRAME_jump1 && cent->current.frame <= FRAME_jump2)
                                new_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]) + vert_diff[2]);
                            else if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                                new_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]) - vert_diff[2] * 3.25);
                            else
                                new_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]) - vert_diff[2] * 2);
							*/


                            //src_oldvert->pos[0] * oldscale[0] + translate[0];
                            //src_newvert->pos[0] * newscale[0] + translate[0];
                            /*
                            old_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (src_oldvert->pos[0] * oldscale[0] + translate[0]);
                            old_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (src_oldvert->pos[1] * oldscale[1] + translate[1]);
                            old_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (src_oldvert->pos[2] * oldscale[2] + translate[2]);


                            new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (src_newvert->pos[0] * newscale[0] + translate[0]);
                            new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (src_newvert->pos[1] * newscale[1] + translate[1]);
                            new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (src_newvert->pos[2] * newscale[2] + translate[2]);
                            */

                            //maliasvert_t* src_init_vert = &player_model->meshes[0].verts[0];
                            //maliasvert_t* src_curr_vert = &player_model->meshes[0].verts[cent->current.frame * player_model->meshes[0].numverts];







                            //if (amv_count == 0)
                            //    Com_Printf("%s frame[%d] attachment_offset[%f %f %f]\n", __func__, cent->current.frame, dst_vert[0], dst_vert[1], dst_vert[2]);

                            //attach_model->meshes[0].verts[amv_count].pos[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + (dst_vert[0] - org_vert[s1->number - 1][0]) * 10;
							//attach_model->meshes[0].verts[amv_count].pos[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + (dst_vert[1] - org_vert[s1->number - 1][1]) * 10;
							//attach_model->meshes[0].verts[amv_count].pos[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + (dst_vert[2] - org_vert[s1->number - 1][2]) * 1;
							/*
                            vert_diff[0] = dst_vert[0] - org_vert[s1->number - 1][0];
							vert_diff[1] = dst_vert[1] - org_vert[s1->number - 1][1];
							vert_diff[2] = dst_vert[2] - org_vert[s1->number - 1][2];
                            //if (vert_diff[0] < 0)
                            //    vert_diff[0] = 0;
							//if (vert_diff[1] < 0)
							//	vert_diff[1] = 0;

                            new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10) - 224;
                            new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * 10) - 224;
                            new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2] + vert_diff[2]);// *3) - 168;
                            LerpVector2(attach_model->meshes[0].verts[amv_count].pos, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);
                            */
							
                            //*
                            vert_diff[0] = new_vert[0] - org_vert[s1->number - 1][0];
                            vert_diff[1] = new_vert[1] - org_vert[s1->number - 1][1];
                            vert_diff[2] = new_vert[2] - org_vert[s1->number - 1][2];
							/*
                            new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10) - 224;
                            new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * 10) - 224;
                            new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2]);// *3) - 168;
                            //LerpVector2(attach_model->meshes[0].verts[amv_count].pos, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);
							
                            if (cent->current.frame >= FRAME_stand01 && cent->current.frame <= FRAME_stand40)
                                attach_model->meshes[0].verts[amv_count].pos[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10) - 128;
                            else
                                attach_model->meshes[0].verts[amv_count].pos[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10) - 224;
                            attach_model->meshes[0].verts[amv_count].pos[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * 10) - 224;
                            attach_model->meshes[0].verts[amv_count].pos[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * 1);
							*/

                            if (cent->current.frame >= FRAME_stand01 && cent->current.frame <= FRAME_stand40)
                                new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10);// - 112; // 56 * 2 = 112
                            else if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
								new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10) - 336; // 56 * 6 = 336
                            else
								new_origin[0] = (attachment_verts[s1->number - 1][amv_count].pos[0]) + (vert_diff[0] * 10) - 224; // 56 * 4 = 224
							
                            if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                                new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * 10) - 168; // 56 * 3 = 168
                            else
                                new_origin[1] = (attachment_verts[s1->number - 1][amv_count].pos[1]) + (vert_diff[1] * 10);// - 224; // 56 * 4 = 224
							
                            if (cent->current.frame >= FRAME_jump1 && cent->current.frame <= FRAME_jump2)
								new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * 3) - 84; // 56
                            else if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
								new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * 3) - 168; // 56 * 3 = 168
                            else
                                new_origin[2] = (attachment_verts[s1->number - 1][amv_count].pos[2]) + (vert_diff[2] * 3) - 126; // 56 + (56 / 2) = 84

                            LerpVector2(attach_model->meshes[0].verts[amv_count].pos, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);


                            if (cent->prev.frame >= FRAME_crstnd01 && cent->prev.frame <= FRAME_crpain4)
                            {
                                //ent.angles[0] += 180;
								//ent.angles[1] += 10;

                                //old_origin[0] -= 256;
                                //old_origin[1] += 128;
								
                                //new_origin[0] -= 256;
                                //new_origin[1] += 128;
                            }
							

                            if (cent->prev.frame >= FRAME_jump1 && cent->prev.frame <= FRAME_jump6 && new_origin[2] < old_origin[2])
                            {
                                //new_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2]) - vert_diff[2] * 2);
                                //old_origin[2] = (((normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_oldvert->pos[2]) - vert_diff[2] * 2);
                            }

                            //LerpVector2(old_origin, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);
							

                            //attach_model->meshes[0].verts[amv_count].pos[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + dst_vert[0];
                            //attach_model->meshes[0].verts[amv_count].pos[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + dst_vert[1];
                            //attach_model->meshes[0].verts[amv_count].pos[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + dst_vert[2];
							
							//ent.origin[0] = cent->current.origin[0] + attach_model->meshes[0].verts[amv_count].pos[0];
							//ent.origin[1] = cent->current.origin[1] + attach_model->meshes[0].verts[amv_count].pos[1];
							//ent.origin[2] = cent->current.origin[2] + attach_model->meshes[0].verts[amv_count].pos[2] - 96;

                            //ent.origin[2] = cent->current.origin[2] + attach_model->meshes[0].verts[amv_count].pos[2] - vert_diff[2] - 256;
                            //ent.origin[2] = cent->current.origin[2] - vert_diff[2] + 32;


                            //old_origin[0] = cent->prev.origin[0];
                            //old_origin[1] = cent->prev.origin[1];
                            //old_origin[2] = cent->prev.origin[2] - vert_diff[2] + 32;
                            //new_origin[0] = cent->current.origin[0];
                            //new_origin[1] = cent->current.origin[1];
                            //new_origin[2] = cent->current.origin[2] - vert_diff[2] + 32;
                            //ent.oldorigin[2] = old_origin[2];
                            //ent.origin[2] = new_origin[2];

                            //if (cent->current.frame >= FRAME_crstnd01 && cent->current.frame <= FRAME_crstnd19 || cent->current.frame >= FRAME_jump1)
                            {
                                //attach_model->meshes[0].verts[amv_count].pos[0] = (normal[0] * scale) + attachment_verts[s1->number - 1][amv_count].pos[0] + src_newvert->pos[0];
                                //attach_model->meshes[0].verts[amv_count].pos[1] = (normal[1] * scale) + attachment_verts[s1->number - 1][amv_count].pos[1] + src_newvert->pos[1];
                                //attach_model->meshes[0].verts[amv_count].pos[2] = (normal[2] * scale) + attachment_verts[s1->number - 1][amv_count].pos[2] + src_newvert->pos[2];
                            }

                            

                            // Interpolate origin
                            //vec3_t old_origin, new_origin;
							//VectorCopy(attach_model->meshes[0].verts[amv_count].pos, old_origin);
                            //FRAME_run1 - FRAME_run5
                            //if (cent->current.frame >= 40 && cent->current.frame <= 45)
                            //{
                            //    new_origin[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + (dst_vert[0] * 4);
                            //    new_origin[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + (dst_vert[1] * 4);
                            //}
                            //else
                            //{
                            //    new_origin[0] = attachment_verts[s1->number - 1][amv_count].pos[0] + (dst_vert[0] * 1);
                            //    new_origin[1] = attachment_verts[s1->number - 1][amv_count].pos[1] + (dst_vert[1] * 1);
                            //}
							//new_origin[2] = attachment_verts[s1->number - 1][amv_count].pos[2] + (dst_vert[2] * 3);
                            //LerpVector2(old_origin, new_origin, backlerp, frontlerp, attach_model->meshes[0].verts[amv_count].pos);
                        }
						
                        //ent.origin[0] += dst_vert[0] - 16;
                        //ent.origin[1] += dst_vert[1] - 16;
                        //ent.origin[2] += dst_vert[2] - 32;
                        //ent.model = 0;
                        //V_AddEntity(&ent);

                        //break;
                    }

                    dst_vert += 4;

                    src_oldvert++;
                    src_newvert++;
                }
                //ent.skinnum;
				

                //// ent.model = cl.clientinfo[s1->number - 1].attach_model[attachmentNum];
                //// V_AddEntity(&ent);

                //int vf = s1->frame * player_model->meshes[0].numverts;
                //Com_Printf("%s vert[180].[%d %d %d]\n", __func__, player_model->meshes[0].verts[vf].pos[0], player_model->meshes[0].verts[vf].pos[1], player_model->meshes[0].verts[vf].pos[2]);



                //for (int av = 0; av < attach_model->meshes[0].numverts; av++)
                //{
                    //attach_model->meshes[0].verts[av].pos[0] = player_model->frames[s1->frame].
                //}


                //&player_model->meshes[i];
                //mesh->numindices, mesh->indices
                /*
                Com_Printf("%s\n Player verts\n", __func__);
                for (int pv = 0; pv < player_model->meshes[0].numverts; pv++)
                {
                    Com_Printf("%s [%d %d %d]\n", __func__,
                        player_model->meshes[0].verts[pv].pos[0],
                        player_model->meshes[0].verts[pv].pos[1],
                        player_model->meshes[0].verts[pv].pos[2]);
                }

                Com_Printf("\n%s\n Attachment verts\n", __func__);
                for (int av = 0; av < attach_model->meshes[0].numverts; av++)
                {
                    Com_Printf("%s [%d %d %d]\n", __func__,
                        attach_model->meshes[0].verts[av].pos[0],
                        attach_model->meshes[0].verts[av].pos[1],
                        attach_model->meshes[0].verts[av].pos[2]);
                }

                Com_Printf("%s\n\n", __func__);
                */
            }

            //if (ci && ci->name != 0)
            //{
                //Com_Printf("%s %s %s %s\n", __func__, ci->name, ci->model_name, ci->skin_name)
            //}
            //if (cl.baseclientinfo.name != 0)
            //{
                //Com_Printf("%s %s %s %s\n", __func__, cl.baseclientinfo.name, cl.baseclientinfo.model_name, cl.baseclientinfo.skin_name);
            //}

            /*
            model_t* player_model, * weapon_model, * attach_model;
            player_model = MOD_ForHandle(cl.model_draw[s1->modelindex]);
            weapon_model = MOD_ForHandle(cl.model_draw[s1->modelindex2]);
            attach_model = MOD_ForHandle(cl.model_draw[s1->modelindex3]);
            //if (player_model != NULL && player_model->name != 0)
            //    Com_Printf("%s %s\n", __func__, player_model->name);
            if (weapon_model != NULL && weapon_model->name != 0)
                Com_Printf("%s %s\n", __func__, weapon_model->name);
            //if (attach_model != NULL && attach_model->name != 0)
            //    Com_Printf("%s %s\n", __func__, attach_model->name);
            //if (player_model != NULL)
            {
                //for (int pm = 0; pm < player_model->nummeshes; pm++)
                {
                    //&player_model->meshes[i];
                    //mesh->numindices, mesh->indices
                    //for (int pv = 0; pv < player_model->meshes[i].numverts; pv++)
                    {
                        //player_model->meshes[i].verts[pm].pos[0];
                    }
                }
            }
            */
        }
        else if (s1->modelindex3)
        {
            ent.model = cl.model_draw[s1->modelindex3];
            V_AddEntity(&ent);
        }
		
        if (s1->modelindex4 && (s1->renderfx & RF_ATTACHMENT))
        {
            // TODO ---------------=-=-=-=--=--------------------------------------=-=-=--=-=--------------------------------------=-=-=-=-=---------------------------------
        }
        else if (s1->modelindex4) 
        {
            ent.model = cl.model_draw[s1->modelindex4];
            V_AddEntity(&ent);
        }
#endif
        //rekkie -- Attach model to player -- e

        // duplicate alpha
        if (has_alpha) {
            ent.flags = RF_TRANSLUCENT;
            ent.alpha = s1->alpha;
        }

        if (s1->modelindex3) {
            ent.model = cl.model_draw[s1->modelindex3];
            V_AddEntity(&ent);
        }

        if (s1->modelindex4) {
            ent.model = cl.model_draw[s1->modelindex4];
            V_AddEntity(&ent);
        }

        if (effects & EF_POWERSCREEN) {
            ent.model = cl_mod_powerscreen;
            ent.oldframe = 0;
            ent.frame = 0;
            ent.flags = RF_TRANSLUCENT;
            ent.alpha = 0.30f;

            // remaster powerscreen is tiny and needs scaling
            if (cl.need_powerscreen_scale) {
                vec3_t forward, mid, tmp;
                VectorCopy(ent.origin, tmp);
                VectorAvg(cent->mins, cent->maxs, mid);
                VectorAdd(ent.origin, mid, ent.origin);
                AngleVectors(ent.angles, forward, NULL, NULL);
                VectorMA(ent.origin, cent->maxs[0], forward, ent.origin);
                ent.scale = cent->radius * 0.8f;
                ent.flags |= RF_FULLBRIGHT;
                V_AddEntity(&ent);
                VectorCopy(tmp, ent.origin);
            } else {
                ent.flags |= RF_SHELL_GREEN;
                V_AddEntity(&ent);
            }
        }

        if (s1->morefx & EFX_HOLOGRAM)
            CL_HologramParticles(ent.origin);

        // add automatic particle trails
        if (!(effects & EF_TRAIL_MASK))
            goto skip;

        if (effects & EF_ROCKET) {
            if (!(cl_disable_particles->integer & NOPART_ROCKET_TRAIL))
                CL_RocketTrail(cent->lerp_origin, ent.origin, cent);
            if (cl_dlight_hacks->integer & DLHACK_ROCKET_COLOR)
                V_AddLight(ent.origin, 200, 1, 0.23f, 0);
            else
                V_AddLight(ent.origin, 200, 1, 1, 0);
        } else if (effects & EF_BLASTER) {
            if (effects & EF_TRACKER) {
                CL_BlasterTrail2(cent->lerp_origin, ent.origin);
                V_AddLight(ent.origin, 200, 0, 1, 0);
            } else {
                CL_BlasterTrail(cent->lerp_origin, ent.origin);
                V_AddLight(ent.origin, 200, 1, 1, 0);
            }
        } else if (effects & EF_HYPERBLASTER) {
            if (effects & EF_TRACKER)
                V_AddLight(ent.origin, 200, 0, 1, 0);
            else
                V_AddLight(ent.origin, 200, 1, 1, 0);
        } else if (effects & EF_GIB) {
            CL_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
        } else if (effects & EF_GRENADE) {
            if (!(cl_disable_particles->integer & NOPART_GRENADE_TRAIL))
                CL_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
        } else if (effects & EF_FLIES) {
            CL_FlyEffect(cent, ent.origin);
        } else if (effects & EF_BFG) {
            if (effects & EF_ANIM_ALLFAST) {
                CL_BfgParticles(&ent);
                i = 200;
            } else {
                static const uint16_t bfg_lightramp[6] = {300, 400, 600, 300, 150, 75};
                i = bfg_lightramp[Q_clip(s1->frame, 0, 5)];
            }
            V_AddLight(ent.origin, i, 0, 1, 0);
        } else if (effects & EF_TRAP) {
            ent.origin[2] += 32;
            CL_TrapParticles(cent, ent.origin);
            i = (Q_rand() % 100) + 100;
            V_AddLight(ent.origin, i, 1, 0.8f, 0.1f);
        } else if (effects & EF_FLAG1) {
            CL_FlagTrail(cent->lerp_origin, ent.origin, 242);
            V_AddLight(ent.origin, 225, 1, 0.1f, 0.1f);
        } else if (effects & EF_FLAG2) {
            CL_FlagTrail(cent->lerp_origin, ent.origin, 115);
            V_AddLight(ent.origin, 225, 0.1f, 0.1f, 1);
        } else if (effects & EF_TAGTRAIL) {
            CL_TagTrail(cent->lerp_origin, ent.origin, 220);
            V_AddLight(ent.origin, 225, 1.0f, 1.0f, 0.0f);
        } else if (effects & EF_TRACKERTRAIL) {
            if (effects & EF_TRACKER) {
                float intensity = 50 + (500 * (sin(cl.time / 500.0f) + 1.0f));
                V_AddLight(ent.origin, intensity, -1.0f, -1.0f, -1.0f);
            } else {
                CL_Tracker_Shell(cent, ent.origin);
                V_AddLight(ent.origin, 155, -1.0f, -1.0f, -1.0f);
            }
        } else if (effects & EF_TRACKER) {
            CL_TrackerTrail(cent->lerp_origin, ent.origin, 0);
            V_AddLight(ent.origin, 200, -1, -1, -1);
        } else if (effects & EF_GREENGIB) {
            CL_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
        } else if (effects & EF_IONRIPPER) {
            CL_IonripperTrail(cent->lerp_origin, ent.origin);
            V_AddLight(ent.origin, 100, 1, 0.5f, 0.5f);
        } else if (effects & EF_BLUEHYPERBLASTER) {
            V_AddLight(ent.origin, 200, 0, 0, 1);
        } else if (effects & EF_PLASMA) {
            if (effects & EF_ANIM_ALLFAST)
                CL_BlasterTrail(cent->lerp_origin, ent.origin);
            V_AddLight(ent.origin, 130, 1, 0.5f, 0.5f);
        }

skip:
        VectorCopy(ent.origin, cent->lerp_origin);
    }
}

static int shell_effect_hack(void)
{
    centity_t   *ent;
    int         flags = 0;

    ent = &cl_entities[cl.frame.clientNum + 1];
    if (ent->serverframe != cl.frame.number)
        return 0;

    if (!ent->current.modelindex)
        return 0;

    if (ent->current.effects & EF_PENT)
        flags |= RF_SHELL_RED;
    if (ent->current.effects & EF_QUAD)
        flags |= RF_SHELL_BLUE;
    if (ent->current.effects & EF_DOUBLE)
        flags |= RF_SHELL_DOUBLE;
    if (ent->current.effects & EF_HALF_DAMAGE)
        flags |= RF_SHELL_HALF_DAM;
    if (ent->current.morefx & EFX_DUALFIRE)
        flags |= RF_SHELL_LITE_GREEN;

    return flags;
}

/*
==============
CL_AddViewWeapon
==============
*/
static void CL_AddViewWeapon(void)
{
    player_state_t *ps, *ops;
    entity_t    gun;        // view model
    int         i, flags;

    // allow the gun to be completely removed
    if (cl_gun->integer < 1) {
        return;
    }

    if (info_hand->integer == 2 && cl_gun->integer == 1) {
        return;
    }

    // find states to interpolate between
    ps = CL_KEYPS;
    ops = CL_OLDKEYPS;

    memset(&gun, 0, sizeof(gun));

    if (gun_model) {
        gun.model = gun_model;  // development tool
    } else {
        gun.model = cl.model_draw[ps->gunindex & GUNINDEX_MASK];
        gun.skinnum = ps->gunindex >> GUNINDEX_BITS;
    }
    if (!gun.model) {
        return;
    }

    // set up gun position
    for (i = 0; i < 3; i++) {
        gun.origin[i] = cl.refdef.vieworg[i] + ops->gunoffset[i] +
                        CL_KEYLERPFRAC * (ps->gunoffset[i] - ops->gunoffset[i]);
        gun.angles[i] = cl.refdef.viewangles[i] + LerpAngle(ops->gunangles[i],
                        ps->gunangles[i], CL_KEYLERPFRAC);
    }

    VectorMA(gun.origin, cl_gun_y->value, cl.v_forward, gun.origin);
    VectorMA(gun.origin, cl_gun_x->value, cl.v_right, gun.origin);
    VectorMA(gun.origin, cl_gun_z->value, cl.v_up, gun.origin);

    VectorCopy(gun.origin, gun.oldorigin);      // don't lerp at all

    if (gun_frame) {
        gun.frame = gun_frame;  // development tool
        gun.oldframe = gun_frame;   // development tool
    } else {
        gun.frame = ps->gunframe;
        if (gun.frame == 0) {
            gun.oldframe = 0;   // just changed weapons, don't lerp from old
        } else {
            gun.oldframe = ops->gunframe;
            gun.backlerp = 1.0f - CL_KEYLERPFRAC;
        }
    }

    gun.flags = RF_MINLIGHT | RF_DEPTHHACK | RF_WEAPONMODEL;

    if (cl_gunalpha->value != 1) {
        gun.alpha = Cvar_ClampValue(cl_gunalpha, 0.1f, 1.0f);
        gun.flags |= RF_TRANSLUCENT;
    }

    V_AddEntity(&gun);

    // add shell effect from player entity
    flags = shell_effect_hack();
    if (flags) {
        gun.alpha = 0.30f * cl_gunalpha->value;
        gun.flags |= flags | RF_TRANSLUCENT;
        V_AddEntity(&gun);
    }

    // add muzzle flash
    if (!cl.weapon.muzzle.model)
        return;

    if (cl.time - cl.weapon.muzzle.time > 50) {
        cl.weapon.muzzle.model = 0;
        return;
    }

    gun.flags = RF_FULLBRIGHT | RF_DEPTHHACK | RF_WEAPONMODEL | RF_TRANSLUCENT;
    gun.alpha = 1.0f;
    gun.model = cl.weapon.muzzle.model;
    gun.skinnum = 0;
    gun.scale = cl.weapon.muzzle.scale;
    gun.backlerp = 0.0f;
    gun.frame = gun.oldframe = 0;

    vec3_t forward, right, up;
    AngleVectors(gun.angles, forward, right, up);

    VectorMA(gun.origin, cl.weapon.muzzle.offset[0], forward, gun.origin);
    VectorMA(gun.origin, cl.weapon.muzzle.offset[1], right, gun.origin);
    VectorMA(gun.origin, cl.weapon.muzzle.offset[2], up, gun.origin);

    VectorCopy(cl.refdef.viewangles, gun.angles);
    gun.angles[2] += cl.weapon.muzzle.roll;

    V_AddEntity(&gun);
}

static void CL_SetupFirstPersonView(void)
{
    player_state_t *ps, *ops;
    vec3_t kickangles;
    float lerp;

    // add kick angles
    if (cl_kickangles->integer) {
        ps = CL_KEYPS;
        ops = CL_OLDKEYPS;

        lerp = CL_KEYLERPFRAC;

        LerpAngles(ops->kick_angles, ps->kick_angles, lerp, kickangles);
        VectorAdd(cl.refdef.viewangles, kickangles, cl.refdef.viewangles);
    }

    // add the weapon
    CL_AddViewWeapon();

    cl.thirdPersonView = false;
}

/*
===============
CL_SetupThirdPersionView
===============
*/
static void CL_SetupThirdPersionView(void)
{
    vec3_t focus;
    float fscale, rscale;
    float dist, angle, range;
    trace_t trace;
    static const vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

    // if dead, set a nice view angle
    if (cl.frame.ps.stats[STAT_HEALTH] <= 0) {
        cl.refdef.viewangles[ROLL] = 0;
        cl.refdef.viewangles[PITCH] = 10;
    }

    VectorMA(cl.refdef.vieworg, 512, cl.v_forward, focus);

    cl.refdef.vieworg[2] += 8;

    cl.refdef.viewangles[PITCH] *= 0.5f;
    AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

    angle = DEG2RAD(cl_thirdperson_angle->value);
    range = cl_thirdperson_range->value;
    fscale = cos(angle);
    rscale = sin(angle);
    VectorMA(cl.refdef.vieworg, -range * fscale, cl.v_forward, cl.refdef.vieworg);
    VectorMA(cl.refdef.vieworg, -range * rscale, cl.v_right, cl.refdef.vieworg);

    CM_BoxTrace(&trace, cl.playerEntityOrigin, cl.refdef.vieworg,
                mins, maxs, cl.bsp->nodes, MASK_SOLID);
    if (trace.fraction != 1.0f) {
        VectorCopy(trace.endpos, cl.refdef.vieworg);
    }

    VectorSubtract(focus, cl.refdef.vieworg, focus);
    dist = sqrtf(focus[0] * focus[0] + focus[1] * focus[1]);

    cl.refdef.viewangles[PITCH] = -RAD2DEG(atan2(focus[2], dist));
    cl.refdef.viewangles[YAW] -= cl_thirdperson_angle->value;

    cl.thirdPersonView = true;
}

static void CL_FinishViewValues(void)
{
    centity_t *ent;

    if (!cl_thirdperson->integer)
        goto first;

    ent = &cl_entities[cl.frame.clientNum + 1];
    if (ent->serverframe != cl.frame.number)
        goto first;

    if (!ent->current.modelindex)
        goto first;

    CL_SetupThirdPersionView();
    return;

first:
    CL_SetupFirstPersonView();
}

static inline float lerp_client_fov(float ofov, float nfov, float lerp)
{
    if (cls.demo.playback) {
        int fov = info_fov->integer;

        if (fov < 1)
            fov = 90;
        else if (fov > 160)
            fov = 160;

        if (info_uf->integer & UF_LOCALFOV)
            return fov;

        if (!(info_uf->integer & UF_PLAYERFOV)) {
            if (ofov >= 90)
                ofov = fov;
            if (nfov >= 90)
                nfov = fov;
        }
    }

    return ofov + lerp * (nfov - ofov);
}

/*
===============
CL_CalcViewValues

Sets cl.refdef view values and sound spatialization params.
Usually called from CL_AddEntities, but may be directly called from the main
loop if rendering is disabled but sound is running.
===============
*/
void CL_CalcViewValues(void)
{
    player_state_t *ps, *ops;
#if USE_FPS
    player_state_t *keyps, *keyops;
    keyps = &cl.keyframe.ps;
    keyops = &cl.oldkeyframe.ps;
#endif
    vec3_t viewoffset;
    float lerp;

    if (!cl.frame.valid) {
        return;
    }

    // find states to interpolate between
    ps = &cl.frame.ps;
    ops = &cl.oldframe.ps;

    lerp = cl.lerpfrac;

    // calculate the origin
    if (!cls.demo.playback && cl_predict->integer && !(ps->pmove.pm_flags & PMF_NO_PREDICTION)) {
        // use predicted values
        unsigned delta = cls.realtime - cl.predicted_step_time;
        float backlerp = lerp - 1.0f;

        VectorMA(cl.predicted_origin, backlerp, cl.prediction_error, cl.refdef.vieworg);
        LerpVector(ops->viewoffset, ps->viewoffset, lerp, viewoffset);

        // smooth out stair climbing
        if (cl.predicted_step < 127 * 0.125f) {
            delta <<= 1; // small steps
        }
        if (delta < 100) {
            cl.refdef.vieworg[2] -= cl.predicted_step * (100 - delta) * 0.01f;
        }

        if (cl_predict_crouch->integer == 2 || (cl_predict_crouch->integer && cl.view_predict))
        {
#if USE_FPS
            viewoffset[2] = cl.predicted_viewheight[1];
            viewoffset[2] -= (cl.predicted_viewheight[1] - cl.predicted_viewheight[0]) * cl.keylerpfrac;
#else
            viewoffset[2] = cl.predicted_viewheight[1];
            viewoffset[2] -= (cl.predicted_viewheight[1] - cl.predicted_viewheight[0]) * lerp;
#endif
        }

    } else {
        int i;
        // just use interpolated values
        for (i = 0; i < 3; i++) {
            cl.refdef.vieworg[i] = SHORT2COORD(ops->pmove.origin[i] +
                lerp * (ps->pmove.origin[i] - ops->pmove.origin[i]));
        }
#if USE_FPS
		LerpVector(keyops->viewoffset, keyps->viewoffset, cl.keylerpfrac, viewoffset);
#else
    LerpVector(ops->viewoffset, ps->viewoffset, lerp, viewoffset);
#endif
    }

    // if not running a demo or on a locked frame, add the local angle movement
    if (cls.demo.playback) {
        if (cls.key_dest == KEY_GAME && Key_IsDown(K_SHIFT)) {
            VectorCopy(cl.viewangles, cl.refdef.viewangles);
        } else {
            LerpAngles(ops->viewangles, ps->viewangles, lerp,
                       cl.refdef.viewangles);
        }
    } else if (ps->pmove.pm_type < PM_DEAD) {
        // use predicted values
        VectorCopy(cl.predicted_angles, cl.refdef.viewangles);
    }
#if USE_FPS
    else if (keyops->pmove.pm_type < PM_DEAD && cls.serverProtocol > PROTOCOL_VERSION_DEFAULT) {
#else
    else if (ops->pmove.pm_type < PM_DEAD && cls.serverProtocol > PROTOCOL_VERSION_DEFAULT) {
#endif
        // lerp from predicted angles, since enhanced servers
        // do not send viewangles each frame
#if USE_FPS
        LerpAngles(cl.predicted_angles, keyps->viewangles, cl.keylerpfrac, cl.refdef.viewangles);
#else
        LerpAngles(cl.predicted_angles, ps->viewangles, lerp, cl.refdef.viewangles);
#endif
    } else {
        // just use interpolated values
        LerpAngles(ops->viewangles, ps->viewangles, lerp, cl.refdef.viewangles);
    }

    // don't interpolate blend color
    Vector4Copy(ps->blend, cl.refdef.blend);

#if USE_FPS
    ps = &cl.keyframe.ps;
    ops = &cl.oldkeyframe.ps;

    lerp = cl.keylerpfrac;
#endif

    // interpolate field of view
    cl.fov_x = lerp_client_fov(ops->fov, ps->fov, lerp);
    cl.fov_y = V_CalcFov(cl.fov_x, 4, 3);

    AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

    VectorCopy(cl.refdef.vieworg, cl.playerEntityOrigin);
    VectorCopy(cl.refdef.viewangles, cl.playerEntityAngles);

    if (cl.playerEntityAngles[PITCH] > 180) {
        cl.playerEntityAngles[PITCH] -= 360;
    }

    cl.playerEntityAngles[PITCH] = cl.playerEntityAngles[PITCH] / 3;

    VectorAdd(cl.refdef.vieworg, viewoffset, cl.refdef.vieworg);

    VectorCopy(cl.refdef.vieworg, listener_origin);
    VectorCopy(cl.v_forward, listener_forward);
    VectorCopy(cl.v_right, listener_right);
    VectorCopy(cl.v_up, listener_up);
}


/*
===============
CL_AddEntities

Emits all entities, particles, and lights to the refresh
===============
*/
void CL_AddEntities(void)
{
    CL_CalcViewValues();
    CL_FinishViewValues();
    CL_AddPacketEntities();
    CL_AddTEnts();
    CL_AddParticles();
    CL_AddDLights();
    CL_AddLightStyles();
    LOC_AddLocationsToScene();
}

/*
===============
CL_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
void CL_GetEntitySoundOrigin(unsigned entnum, vec3_t org)
{
    centity_t   *ent;
    mmodel_t    *cm;
    vec3_t      mid;

    if (entnum >= cl.csr.max_edicts)
        Com_Error(ERR_DROP, "%s: bad entity", __func__);

    if (!entnum || entnum == listener_entnum) {
        // should this ever happen?
        VectorCopy(listener_origin, org);
        return;
    }

    // interpolate origin
    // FIXME: what should be the sound origin point for RF_BEAM entities?
    ent = &cl_entities[entnum];
    LerpVector(ent->prev.origin, ent->current.origin, cl.lerpfrac, org);

    // offset the origin for BSP models
    if (ent->current.solid == PACKED_BSP) {
        cm = cl.model_clip[ent->current.modelindex];
        if (cm) {
            VectorAvg(cm->mins, cm->maxs, mid);
            VectorAdd(org, mid, org);
        }
    }
}
