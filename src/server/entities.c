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

#include "server.h"

/*
=============================================================================

Encode a client frame onto the network channel

=============================================================================
*/

// some protocol optimizations are disabled when recording a demo
#define Q2PRO_OPTIMIZE(c) \
    (((c)->protocol == PROTOCOL_VERSION_Q2PRO || (c)->protocol == PROTOCOL_VERSION_AQTION) && !(c)->settings[CLS_RECORDING])

/*
=============
SV_TruncPacketEntities

Truncates remainder of entity_packed_t list, patching current frame to make
delta compression happy.
=============
*/
static bool SV_TruncPacketEntities(client_t *client, const client_frame_t *from,
                                   client_frame_t *to, int oldindex, int newindex)
{
    entity_packed_t *newent;
    const entity_packed_t *oldent;
    int i, oldnum, newnum, entities_mask, from_num_entities, to_num_entities;
    bool ret = true;

    if (!sv_trunc_packet_entities->integer || client->netchan.type)
        return false;

    SV_DPrintf(0, "Truncating frame %d at %u bytes for %s\n",
               client->framenum, msg_write.cursize, client->name);

    if (!from)
        from_num_entities = 0;
    else
        from_num_entities = from->num_entities;
    to_num_entities = to->num_entities;

    entities_mask = client->num_entities - 1;
    oldent = newent = NULL;
    while (newindex < to->num_entities || oldindex < from_num_entities) {
        if (newindex >= to->num_entities) {
            newnum = MAX_EDICTS;
        } else {
            i = (to->first_entity + newindex) & entities_mask;
            newent = &client->entities[i];
            newnum = newent->number;
        }

        if (oldindex >= from_num_entities) {
            oldnum = MAX_EDICTS;
        } else {
            i = (from->first_entity + oldindex) & entities_mask;
            oldent = &client->entities[i];
            oldnum = oldent->number;
        }

        if (newnum == oldnum) {
            // skip delta update
            *newent = *oldent;
            oldindex++;
            newindex++;
            continue;
        }

        if (newnum < oldnum) {
            // remove new entity from frame
            to->num_entities--;
            for (i = newindex; i < to->num_entities; i++) {
                client->entities[(to->first_entity + i    ) & entities_mask] =
                client->entities[(to->first_entity + i + 1) & entities_mask];
            }
            continue;
        }

        if (newnum > oldnum) {
            // drop the frame if entity list got too big.
            // should not normally happen.
            if (to->num_entities >= MAX_PACKET_ENTITIES) {
                ret = false;
                break;
            }

            // insert old entity into frame
            for (i = to->num_entities - 1; i >= newindex; i--) {
                client->entities[(to->first_entity + i + 1) & entities_mask] =
                client->entities[(to->first_entity + i    ) & entities_mask];
            }

            client->entities[(to->first_entity + newindex) & entities_mask] = *oldent;
            to->num_entities++;

            // should never go backwards
            to_num_entities = max(to_num_entities, to->num_entities);

            oldindex++;
            newindex++;
            continue;
        }
    }

    client->next_entity = to->first_entity + to_num_entities;
    return ret;
}

#if AQTION_EXTENSION
static void SV_Ghud_SendUpdateToClient(client_t *client, const client_frame_t *oldframe, client_frame_t *frame)
{
	if (client->protocol == PROTOCOL_VERSION_AQTION && client->version >= PROTOCOL_VERSION_AQTION_GHUD)
	{
		int protocolflags, maxelements;

		maxelements = 0;
		protocolflags = 0;
		if (client->version >= PROTOCOL_VERSION_AQTION_GHUD2)
		{
			maxelements = MAX_GHUDS;
			protocolflags |= 1;
		}
		else
		{	
			maxelements = 64; // we have to obey old kinda broken ghud for older clients
		}

		if (oldframe != NULL) {
            memcpy(frame->ghud, oldframe->ghud, sizeof(frame->ghud)); // use oldframe as baseline in case we can't fit all the updates in one package
        } else {
            memset(frame->ghud, 0, sizeof(frame->ghud)); // initialize to zero if oldframe is NULL
        }
		MSG_WriteByte(svc_ghudupdate);
		qboolean written = false;
		int i;
		for (i = 0; i < maxelements; i++)
		{
			ghud_element_t *element = &client->ghud[i];
			size_t old_size;
			int uflags = 0;

			if (oldframe != NULL) {
                uflags = MSG_DeltaGhud(&oldframe->ghud[i], element, protocolflags);
                if (oldframe->ghud[i].flags & GHF_FORCE || element->flags & GHF_FORCE)
                    uflags |= 0x7F;
            } else {
                if (element->flags & GHF_FORCE)
                    uflags |= 0x7F;
            }

			if (!uflags)
				continue;

			old_size = msg_write.cursize;

			MSG_WriteByte(i);
			MSG_WriteGhud(element, uflags);

			if (msg_write.cursize >= client->netchan.maxpacketlen)
			{
				msg_write.cursize = old_size;
				break;
			}

			written = true;
			memcpy(&frame->ghud[i], element, sizeof(ghud_element_t)); // update the ghud since it made it into the frame
			element->flags &= ~GHF_FORCE;
		}

		if (written)
			MSG_WriteByte(255);
		else
			msg_write.cursize--;
	}
}
#endif

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entity_packed_t list to the message.
=============
*/
static bool SV_EmitPacketEntities(client_t *client, const client_frame_t *from,
                                  client_frame_t *to, int clientEntityNum, unsigned maxsize)
{
    entity_packed_t *newent;
    const entity_packed_t *oldent;
    int i, oldnum, newnum, oldindex, newindex, from_num_entities;
    msgEsFlags_t flags;
    bool ret = true;

    if (msg_write.cursize + 2 > maxsize)
        return false;

    if (!from)
        from_num_entities = 0;
    else
        from_num_entities = from->num_entities;

    newindex = 0;
    oldindex = 0;
    oldent = newent = NULL;
    while (newindex < to->num_entities || oldindex < from_num_entities) {
        if (msg_write.cursize + MAX_PACKETENTITY_BYTES > maxsize) {
            ret = SV_TruncPacketEntities(client, from, to, oldindex, newindex);
            break;
        }

        if (newindex >= to->num_entities) {
            newnum = MAX_EDICTS;
        } else {
            i = (to->first_entity + newindex) & (client->num_entities - 1);
            newent = &client->entities[i];
            newnum = newent->number;
        }

        if (oldindex >= from_num_entities) {
            oldnum = MAX_EDICTS;
        } else {
            i = (from->first_entity + oldindex) & (client->num_entities - 1);
            oldent = &client->entities[i];
            oldnum = oldent->number;
        }

        if (newnum == oldnum) {
            // Delta update from old position. Because the force parm is false,
            // this will not result in any bytes being emitted if the entity has
            // not changed at all. Note that players are always 'newentities',
            // this updates their old_origin always and prevents warping in case
            // of packet loss.
            flags = client->esFlags;
            if (newnum <= client->maxclients) {
                flags |= MSG_ES_NEWENTITY;
            }
            if (newnum == clientEntityNum) {
                flags |= MSG_ES_FIRSTPERSON;
                VectorCopy(oldent->origin, newent->origin);
                VectorCopy(oldent->angles, newent->angles);
            }
            MSG_WriteDeltaEntity(oldent, newent, flags);
            oldindex++;
            newindex++;
            continue;
        }

        if (newnum < oldnum) {
            // this is a new entity, send it from the baseline
            flags = client->esFlags | MSG_ES_FORCE | MSG_ES_NEWENTITY;
            oldent = client->baselines[newnum >> SV_BASELINES_SHIFT];
            if (oldent) {
                oldent += (newnum & SV_BASELINES_MASK);
            } else {
                oldent = &nullEntityState;
            }
            if (newnum == clientEntityNum) {
                flags |= MSG_ES_FIRSTPERSON;
                VectorCopy(oldent->origin, newent->origin);
                VectorCopy(oldent->angles, newent->angles);
            }
            MSG_WriteDeltaEntity(oldent, newent, flags);
            newindex++;
            continue;
        }

        if (newnum > oldnum) {
            // the old entity isn't present in the new message
            MSG_WriteDeltaEntity(oldent, NULL, MSG_ES_FORCE);
            oldindex++;
            continue;
        }
    }

    MSG_WriteShort(0);      // end of packetentities

// This was moved from SV_WriteFrameToClient_Aqtion() because its
// return type changed from void to bool and it's now returning SV_EmitPacketEntities()
// rather than just calling it
#if AQTION_EXTENSION
    if(from)
        SV_Ghud_SendUpdateToClient(client, from, to);
#endif
    return ret;
}

static client_frame_t *get_last_frame(client_t *client)
{
    client_frame_t *frame;

    if (client->lastframe <= 0) {
        // client is asking for a retransmit
        client->frames_nodelta++;
        return NULL;
    }

    client->frames_nodelta = 0;

    if (client->framenum - client->lastframe >= UPDATE_BACKUP) {
        // client hasn't gotten a good message through in a long time
        Com_DPrintf("%s: delta request from out-of-date packet.\n", client->name);
        return NULL;
    }

    // we have a valid message to delta from
    frame = &client->frames[client->lastframe & UPDATE_MASK];
    if (frame->number != client->lastframe) {
        // but it got never sent
        Com_DPrintf("%s: delta request from dropped frame.\n", client->name);
        return NULL;
    }

    if (client->next_entity - frame->first_entity > client->num_entities) {
        // but entities are too old
        Com_DPrintf("%s: delta request from out-of-date entities.\n", client->name);
        return NULL;
    }

    return frame;
}

/*
==================
SV_WriteFrameToClient_Default
==================
*/
bool SV_WriteFrameToClient_Default(client_t *client, unsigned maxsize)
{
    client_frame_t  *frame, *oldframe;
    player_packed_t *oldstate;
    int             lastframe;

    // this is the frame we are creating
    frame = &client->frames[client->framenum & UPDATE_MASK];

    // this is the frame we are delta'ing from
    oldframe = get_last_frame(client);
    if (oldframe) {
        oldstate = &oldframe->ps;
        lastframe = client->lastframe;
    } else {
        oldstate = NULL;
        lastframe = -1;
    }

    MSG_WriteByte(svc_frame);
    MSG_WriteLong(client->framenum);
    MSG_WriteLong(lastframe);   // what we are delta'ing from
    MSG_WriteByte(client->suppress_count);  // rate dropped packets
    client->suppress_count = 0;
    client->frameflags = 0;

    // send over the areabits
    MSG_WriteByte(frame->areabytes);
    MSG_WriteData(frame->areabits, frame->areabytes);

    // delta encode the playerstate
    MSG_WriteByte(svc_playerinfo);
    MSG_WriteDeltaPlayerstate_Default(oldstate, &frame->ps, 0);

    // delta encode the entities
    MSG_WriteByte(svc_packetentities);
    return SV_EmitPacketEntities(client, oldframe, frame, 0, maxsize);
}

/*
==================
SV_WriteFrameToClient_Enhanced
==================
*/
bool SV_WriteFrameToClient_Enhanced(client_t *client, unsigned maxsize)
{
    client_frame_t  *frame, *oldframe;
    player_packed_t *oldstate;
    uint32_t        extraflags, delta;
    int             suppressed;
    byte            *b1, *b2;
    msgPsFlags_t    psFlags;
    int             clientEntityNum;

    // this is the frame we are creating
    frame = &client->frames[client->framenum & UPDATE_MASK];

    // this is the frame we are delta'ing from
    oldframe = get_last_frame(client);
    if (oldframe) {
        oldstate = &oldframe->ps;
        delta = client->framenum - client->lastframe;
    } else {
        oldstate = NULL;
        delta = 31;
    }

    // first byte to be patched
    b1 = SZ_GetSpace(&msg_write, 1);

    MSG_WriteLong((client->framenum & FRAMENUM_MASK) | (delta << FRAMENUM_BITS));

    // second byte to be patched
    b2 = SZ_GetSpace(&msg_write, 1);

    // send over the areabits
    MSG_WriteByte(frame->areabytes);
    MSG_WriteData(frame->areabits, frame->areabytes);

    // ignore some parts of playerstate if not recording demo
    psFlags = client->psFlags;
    if (!client->settings[CLS_RECORDING]) {
        if (client->settings[CLS_NOGUN]) {
            psFlags |= MSG_PS_IGNORE_GUNFRAMES;
            if (client->settings[CLS_NOGUN] != 2) {
                psFlags |= MSG_PS_IGNORE_GUNINDEX;
            }
        }
        if (client->settings[CLS_NOBLEND]) {
            psFlags |= MSG_PS_IGNORE_BLEND;
        }
        if (frame->ps.pmove.pm_type < PM_DEAD) {
            if (!(frame->ps.pmove.pm_flags & PMF_NO_PREDICTION)) {
                psFlags |= MSG_PS_IGNORE_VIEWANGLES;
            }
        } else {
            // lying dead on a rotating platform?
            psFlags |= MSG_PS_IGNORE_DELTAANGLES;
        }
    }

    clientEntityNum = 0;
    if (client->protocol == PROTOCOL_VERSION_Q2PRO) {
        if (frame->ps.pmove.pm_type < PM_DEAD && !client->settings[CLS_RECORDING]) {
            clientEntityNum = frame->clientNum + 1;
        }
        if (client->settings[CLS_NOPREDICT]) {
            psFlags |= MSG_PS_IGNORE_PREDICTION;
        }
        suppressed = client->frameflags;
    } else {
        suppressed = client->suppress_count;
    }

    // delta encode the playerstate
    extraflags = MSG_WriteDeltaPlayerstate_Enhanced(oldstate, &frame->ps, psFlags);

    if (client->protocol == PROTOCOL_VERSION_Q2PRO) {
        // delta encode the clientNum
        if ((oldframe ? oldframe->clientNum : 0) != frame->clientNum) {
            extraflags |= EPS_CLIENTNUM;
            if (client->version < PROTOCOL_VERSION_Q2PRO_CLIENTNUM_SHORT) {
                MSG_WriteByte(frame->clientNum);
            } else {
                MSG_WriteShort(frame->clientNum);
            }
        }
    }

    // save 3 high bits of extraflags
    *b1 = svc_frame | (((extraflags & 0x70) << 1));

    // save 4 low bits of extraflags
    *b2 = (suppressed & SUPPRESSCOUNT_MASK) |
          ((extraflags & 0x0F) << SUPPRESSCOUNT_BITS);

    client->suppress_count = 0;
    client->frameflags = 0;

    // delta encode the entities
    return SV_EmitPacketEntities(client, oldframe, frame, clientEntityNum, maxsize);
}

bool SV_WriteFrameToClient_Aqtion(client_t *client, unsigned maxsize)
{
	client_frame_t  *frame, *oldframe;
	player_packed_t *oldstate;
	uint32_t        extraflags, delta;
	int             suppressed;
	byte            *b1, *b2;
	msgPsFlags_t    psFlags;
	int             clientEntityNum;

	// this is the frame we are creating
	frame = &client->frames[client->framenum & UPDATE_MASK];

	// this is the frame we are delta'ing from
	oldframe = get_last_frame(client);
	if (oldframe) {
		oldstate = &oldframe->ps;
		delta = client->framenum - client->lastframe;
	} else {
		oldstate = NULL;
		delta = 31;
	}

	// first byte to be patched
	b1 = SZ_GetSpace(&msg_write, 1);

	MSG_WriteLong((client->framenum & FRAMENUM_MASK) | (delta << FRAMENUM_BITS));

	// second byte to be patched
	b2 = SZ_GetSpace(&msg_write, 1);

	// send over the areabits
	MSG_WriteByte(frame->areabytes);
	MSG_WriteData(frame->areabits, frame->areabytes);

	// ignore some parts of playerstate if not recording demo
	psFlags = client->psFlags;
	if (!client->settings[CLS_RECORDING]) {
		if (client->settings[CLS_NOGUN]) {
			psFlags |= MSG_PS_IGNORE_GUNFRAMES;
			if (client->settings[CLS_NOGUN] != 2) {
				psFlags |= MSG_PS_IGNORE_GUNINDEX;
			}
		}
		if (client->settings[CLS_NOBLEND]) {
			psFlags |= MSG_PS_IGNORE_BLEND;
		}
		if (frame->ps.pmove.pm_type < PM_DEAD) {
			if (!(frame->ps.pmove.pm_flags & PMF_NO_PREDICTION)) {
				psFlags |= MSG_PS_IGNORE_VIEWANGLES;
			}
		} else {
			// lying dead on a rotating platform?
			psFlags |= MSG_PS_IGNORE_DELTAANGLES;
		}
	}

	clientEntityNum = 0;
	if (frame->ps.pmove.pm_type < PM_DEAD && !client->settings[CLS_RECORDING]) {
		clientEntityNum = frame->clientNum + 1;
	}
	if (client->settings[CLS_NOPREDICT]) {
		psFlags |= MSG_PS_IGNORE_PREDICTION;
	}
	suppressed = client->frameflags;
    
    if (client->csr->extended) {
        psFlags |= MSG_PS_EXTENSIONS;
    }

    // delta encode the playerstate
    MSG_WriteByte(svc_playerinfo);
	extraflags = MSG_WriteDeltaPlayerstate_Aqtion(oldstate, &frame->ps, psFlags);

	if (client->protocol == PROTOCOL_VERSION_AQTION) {
        // delta encode the clientNum
        if ((oldframe ? oldframe->clientNum : 0) != frame->clientNum) {
            extraflags |= EPS_CLIENTNUM;
            if (client->version < PROTOCOL_VERSION_AQTION_CLIENTNUM_SHORT) {
                MSG_WriteByte(frame->clientNum);
            } else {
                MSG_WriteShort(frame->clientNum);
            }
        }
    }

	// save 3 high bits of extraflags
	*b1 = svc_frame | (((extraflags & 0x70) << 1));

	// save 4 low bits of extraflags
	*b2 = (suppressed & SUPPRESSCOUNT_MASK) |
		((extraflags & 0x0F) << SUPPRESSCOUNT_BITS);

	client->suppress_count = 0;
	client->frameflags = 0;

	// delta encode the entities
    MSG_WriteByte(svc_packetentities);
	return SV_EmitPacketEntities(client, oldframe, frame, clientEntityNum, maxsize);
}


/*
=============================================================================

Build a client frame structure

=============================================================================
*/

#if USE_FPS
static void
fix_old_origin(const client_t *client, entity_packed_t *state, const edict_t *ent, int e)
{
    server_entity_t *sent = &sv.entities[e];
    int i, j, k;

    if (ent->s.renderfx & RF_BEAM)
        return;

    if (!ent->linkcount)
        return; // not linked in anywhere

    if (sent->create_framenum >= sv.framenum) {
        // created this frame. unfortunate for projectiles: they will move only
        // with 1/client->framediv fraction of their normal speed on the client
        return;
    }

    if (state->event == EV_PLAYER_TELEPORT && !Q2PRO_OPTIMIZE(client)) {
        // other clients will lerp from old_origin on EV_PLAYER_TELEPORT...
        VectorCopy(state->origin, state->old_origin);
        return;
    }

    if (sent->create_framenum > sv.framenum - client->framediv) {
        // created between client frames
        VectorScale(sent->create_origin, 8.0f, state->old_origin);
        return;
    }

    // find the oldest valid origin
    for (i = 0; i < client->framediv - 1; i++) {
        j = sv.framenum - (client->framediv - i);
        k = j & ENT_HISTORY_MASK;
        if (sent->history[k].framenum == j) {
            VectorScale(sent->history[k].origin, 8.0f, state->old_origin);
            return;
        }
    }

    // no valid old_origin, just use what game provided
}
#endif

static bool SV_EntityVisible(const client_t *client, const edict_t *ent, const byte *mask)
{
    if (ent->num_clusters == -1)
        // too many leafs for individual check, go by headnode
        return CM_HeadnodeVisible(CM_NodeNum(client->cm, ent->headnode), mask);

    // check individual leafs
    for (int i = 0; i < ent->num_clusters; i++)
        if (Q_IsBitSet(mask, ent->clusternums[i]))
            return true;

    return false;
}

static bool SV_EntityAttenuatedAway(const vec3_t org, const edict_t *ent)
{
    float dist = Distance(org, ent->s.origin);
    float dist_mult = SOUND_LOOPATTENUATE;

    if (ent->x.loop_attenuation && ent->x.loop_attenuation != ATTN_STATIC)
        dist_mult = ent->x.loop_attenuation * SOUND_LOOPATTENUATE_MULT;

    return (dist - SOUND_FULLVOLUME) * dist_mult > 1.0f;
}

#define IS_MONSTER(ent) \
    ((ent->svflags & (SVF_MONSTER | SVF_DEADMONSTER)) == SVF_MONSTER || (ent->s.renderfx & RF_FRAMELERP))

#define IS_HI_PRIO(ent) \
    (ent->s.number <= sv_client->maxclients || IS_MONSTER(ent) || ent->solid == SOLID_BSP)

#define IS_GIB(ent) \
    (sv_client->csr->extended ? (ent->s.renderfx & RF_LOW_PRIORITY) : (ent->s.effects & (EF_GIB | EF_GREENGIB)))

#define IS_LO_PRIO(ent) \
    (IS_GIB(ent) || (!ent->s.modelindex && !ent->s.effects))

static vec3_t clientorg;

static int entpriocmp(const void *p1, const void *p2)
{
    const edict_t *a = *(const edict_t **)p1;
    const edict_t *b = *(const edict_t **)p2;

    bool hi_a = IS_HI_PRIO(a);
    bool hi_b = IS_HI_PRIO(b);
    if (hi_a != hi_b)
        return hi_b - hi_a;

    bool lo_a = IS_LO_PRIO(a);
    bool lo_b = IS_LO_PRIO(b);
    if (lo_a != lo_b)
        return lo_a - lo_b;

    float dist_a = DistanceSquared(a->s.origin, clientorg);
    float dist_b = DistanceSquared(b->s.origin, clientorg);
    if (dist_a > dist_b)
        return 1;
    return -1;
}

static int entnumcmp(const void *p1, const void *p2)
{
    const edict_t *a = *(const edict_t **)p1;
    const edict_t *b = *(const edict_t **)p2;
    return a->s.number - b->s.number;
}

/*
=============
SV_BuildClientFrame

Decides which entities are going to be visible to the client, and
copies off the playerstat and areabits.
=============
*/
void SV_BuildClientFrame(client_t *client)
{
    int         i, e;
    vec3_t      org;
    edict_t     *ent;
    edict_t     *clent;
    client_frame_t  *frame;
    entity_packed_t *state;
    const mleaf_t   *leaf;
    int         clientarea, clientcluster;
    byte        clientphs[VIS_MAX_BYTES];
    byte        clientpvs[VIS_MAX_BYTES];
    bool        need_clientnum_fix;
    int         max_packet_entities;
    edict_t     *edicts[MAX_EDICTS];
    int         num_edicts;
    qboolean (*visible)(edict_t *, edict_t *) = NULL;
    qboolean (*customize)(edict_t *, edict_t *, customize_entity_t *) = NULL;
    customize_entity_t temp;

    clent = client->edict;
    if (!clent->client)
        return;        // not in game yet

    Q_assert(client->entities);

    // this is the frame we are creating
    frame = &client->frames[client->framenum & UPDATE_MASK];
    frame->number = client->framenum;
    frame->sentTime = com_eventTime; // save it for ping calc later
    frame->latency = -1; // not yet acked

    client->frames_sent++;

    // find the client's PVS
    SV_GetClient_ViewOrg(client, org);

    leaf = CM_PointLeaf(client->cm, org);
    clientarea = leaf->area;
    clientcluster = leaf->cluster;

    // calculate the visible areas
    frame->areabytes = CM_WriteAreaBits(client->cm, frame->areabits, clientarea);
    if (!frame->areabytes && (client->protocol != PROTOCOL_VERSION_Q2PRO && client->protocol != PROTOCOL_VERSION_AQTION)) {
        frame->areabits[0] = 255;
        frame->areabytes = 1;
    }

    // grab the current player_state_t
    if (IS_NEW_GAME_API)
        MSG_PackPlayerNew(&frame->ps, clent->client);
    else
        MSG_PackPlayerOld(&frame->ps, clent->client);

    // grab the current clientNum
    if (g_features->integer & GMF_CLIENTNUM) {
        frame->clientNum = SV_GetClient_ClientNum(client);
        if (!VALIDATE_CLIENTNUM(client->csr, frame->clientNum)) {
            Com_WPrintf("%s: bad clientNum %d for client %d\n",
                        __func__, frame->clientNum, client->number);
            frame->clientNum = client->number;
        }
    } else {
        frame->clientNum = client->number;
    }

    // fix clientNum if out of range for older version of Q2PRO protocol
    need_clientnum_fix = client->protocol == PROTOCOL_VERSION_Q2PRO
        && client->version < PROTOCOL_VERSION_Q2PRO_CLIENTNUM_SHORT
        && frame->clientNum >= CLIENTNUM_NONE;

    // limit maximum number of entities in client frame
    max_packet_entities =
        sv_max_packet_entities->integer > 0 ? sv_max_packet_entities->integer :
        client->csr->extended ? MAX_PACKET_ENTITIES : MAX_PACKET_ENTITIES_OLD;

    if (gex && gex->apiversion >= GAME_API_VERSION_EX_ENTITY_VISIBLE) {
        visible = gex->EntityVisibleToClient;
        customize = gex->CustomizeEntityToClient;
    }

    CM_FatPVS(client->cm, clientpvs, org);
    BSP_ClusterVis(client->cm->cache, clientphs, clientcluster, DVIS_PHS);

    // build up the list of visible entities
    frame->num_entities = 0;
    frame->first_entity = client->next_entity;

    num_edicts = 0;
    for (e = 1; e < client->ge->num_edicts; e++) {
        ent = EDICT_NUM2(client->ge, e);

        // ignore entities not in use
        if (!ent->inuse && (g_features->integer & GMF_PROPERINUSE))
            continue;

        // ignore ents without visible models
        if (ent->svflags & SVF_NOCLIENT)
            continue;

        // ignore ents without visible models unless they have an effect
        if (!HAS_EFFECTS(ent))
            continue;

        // ignore gibs if client says so
        if (client->settings[CLS_NOGIBS]) {
            if (ent->s.effects & EF_GIB && !(client->csr->extended && ent->s.effects & EF_ROCKET))
                continue;
            if (ent->s.effects & EF_GREENGIB)
                continue;
        }

        // ignore flares if client says so
        if (client->csr->extended && ent->s.renderfx & RF_FLARE && client->settings[CLS_NOFLARES])
            continue;

        // ignore if not touching a PV leaf
        if (ent != clent && !sv_novis->integer && !(client->csr->extended && ent->svflags & SVF_NOCULL)) {
            // check area
            if (!CM_AreasConnected(client->cm, clientarea, ent->areanum)) {
                // doors can legally straddle two areas, so
                // we may need to check another one
                if (!CM_AreasConnected(client->cm, clientarea, ent->areanum2)) {
                    continue;        // blocked by a door
                }
            }

            // beams just check one point for PHS
            // remaster uses different sound culling rules
            bool beam_cull = ent->s.renderfx & RF_BEAM;
            bool sound_cull = client->csr->extended && ent->s.sound;

            if (!SV_EntityVisible(client, ent, (beam_cull || sound_cull) ? clientphs : clientpvs))
                continue;

            // don't send sounds if they will be attenuated away
            if (sound_cull) {
                if (SV_EntityAttenuatedAway(org, ent)) {
                    if (!ent->s.modelindex)
                        continue;
                    if (!beam_cull && !SV_EntityVisible(client, ent, clientpvs))
                        continue;
                }
            } else if (!ent->s.modelindex) {
                if (Distance(org, ent->s.origin) > 400)
                    continue;
            }
        }

        SV_CheckEntityNumber(ent, e);

        // optionally skip it
        if (visible && !visible(clent, ent))
            continue;

        edicts[num_edicts++] = ent;

        if (num_edicts == max_packet_entities && !sv_prioritize_entities->integer)
            break;
    }

    // prioritize entities on overflow
    if (num_edicts > max_packet_entities) {
        VectorCopy(org, clientorg);
        sv_client = client;
        sv_player = client->edict;
        qsort(edicts, num_edicts, sizeof(edicts[0]), entpriocmp);
        sv_client = NULL;
        sv_player = NULL;
        num_edicts = max_packet_entities;
        qsort(edicts, num_edicts, sizeof(edicts[0]), entnumcmp);
    }

    for (i = 0; i < num_edicts; i++) {
        ent = edicts[i];
        e = ent->s.number;

		entity_state_t ent_state;
		ent_state = ent->s;
#if AQTION_EXTENSION
		if (GE_customizeentityforclient)
			if (!GE_customizeentityforclient(client->edict, ent, &ent_state))
				continue;
#endif

        // add it to the circular client_entities array
        state = &client->entities[client->next_entity & (client->num_entities - 1)];

        // optionally customize it
        if (customize && customize(clent, ent, &temp)) {
            Q_assert(temp.s.number == e);
            MSG_PackEntity(state, &temp.s, ENT_EXTENSION(client->csr, &temp));
        } else {
            MSG_PackEntity(state, &ent->s, ENT_EXTENSION(client->csr, ent));
        }

#if USE_FPS
        // fix old entity origins for clients not running at
        // full server frame rate
        if (client->framediv != 1)
            fix_old_origin(client, state, ent, e);
#endif

        // clear footsteps
        if (client->settings[CLS_NOFOOTSTEPS] && (state->event == EV_FOOTSTEP
            || (client->csr->extended && (state->event == EV_OTHER_FOOTSTEP ||
                                          state->event == EV_LADDER_STEP)))) {
            state->event = 0;
        }

        // hide POV entity from renderer, unless this is player's own entity
        if (e == frame->clientNum + 1 && ent != clent &&
            (!Q2PRO_OPTIMIZE(client) || need_clientnum_fix)) {
            state->modelindex = 0;
        }

#if USE_MVD_CLIENT
        if (sv.state == ss_broadcast) {
            // spectators only need to know about inline BSP models
            if (!client->csr->extended && state->solid != PACKED_BSP)
                state->solid = 0;
        } else
#endif
        if (ent->owner == clent) {
            // don't mark players missiles as solid
            state->solid = 0;
        } else if (client->esFlags & MSG_ES_LONGSOLID && !client->csr->extended) {
            state->solid = sv.entities[e].solid32;
        }

        frame->num_entities++;
        client->next_entity++;
    }

    if (need_clientnum_fix)
        frame->clientNum = client->infonum;
}
