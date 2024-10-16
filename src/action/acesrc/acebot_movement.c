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
 * $Header: /LTK2/src/acesrc/acebot_movement.c 7     29/02/00 22:58 Riever $
 *
 * $Log: /LTK2/src/acesrc/acebot_movement.c $
 * 
 * 7     29/02/00 22:58 Riever
 * Corner avoidance code added.
 * 
 * 6     29/02/00 11:18 Riever
 * Attack function changes:
 *   Sniper accuracy increased
 *   JumpKick attack added
 *   Knife throw added
 *   Safety check before advancing for knife or kick attack
 * Jump Changed to use a velocity hack.
 * Jump node support fixed and working well.
 * Ledge handling code removed - no longer used.
 * 
 * 5     24/02/00 20:02 Riever
 * New door handling code in place.
 * 
 * User: Riever       Date: 23/02/00   Time: 23:18
 * New door handling code in place - must have route file.
 * User: Riever       Date: 21/02/00   Time: 15:16
 * Bot now has the ability to roam on dry land. Basic collision functions
 * written. Active state skeletal implementation.
 * User: Riever       Date: 20/02/00   Time: 20:27
 * Added new members and definitions ready for 2nd generation of bots.
 * 
 */
	
///////////////////////////////////////////////////////////////////////
//  acebot_movement.c - This file contains all of the 
//                      movement routines for the ACE bot
//           
///////////////////////////////////////////////////////////////////////

#include "../g_local.h"

#include "acebot.h"
#include "botchat.h"

qboolean ACEAI_CheckShot(edict_t *self);
void ACEMV_ChangeBotAngle (edict_t *ent);
//qboolean	ACEND_LadderForward( edict_t *self );
qboolean ACEMV_CanJump(edict_t *self);

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes = 0, c_no = 0;

qboolean M_CheckBottom( edict_t *ent )
{
	vec3_t mins = {0,0,0}, maxs = {0,0,0}, start = {0,0,0}, stop = {0,0,0};
	trace_t trace;
	int x = 0, y = 0;
	float mid = 0, bottom = 0;
	
	VectorAdd( ent->s.origin, ent->mins, mins );
	VectorAdd( ent->s.origin, ent->maxs, maxs );
	
	// if all of the points under the corners are solid world, don't bother
	// with the tougher checks
	// the corners must be within 16 of the midpoint
	start[2] = mins[2] - 1;
	for( x = 0; x <= 1; x ++ )
		for( y = 0; y <= 1; y ++ )
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if( gi.pointcontents(start) != CONTENTS_SOLID )
				goto realcheck;
		}
	
	c_yes ++;
	return true;  // we got out easy
	
realcheck:
	c_no ++;
	//
	// check it for real...
	//
	start[2] = mins[2];
	
	// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
	start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
	stop[2] = start[2] - 2 * STEPSIZE;
	trace = gi.trace( start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID );
	
	if( trace.fraction == 1.0 )
		return false;
	mid = bottom = trace.endpos[2];
	
	// the corners must be within 16 of the midpoint        
	for( x = 0; x <= 1; x ++ )
		for( y = 0; y <= 1; y ++ )
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];
			
			trace = gi.trace( start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID );
			
			if( trace.fraction != 1.0 && trace.endpos[2] > bottom )
				bottom = trace.endpos[2];
			if( trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE )
				return false;
		}
	
	c_yes ++;
	return true;
}

//=============================================================
// CanMoveSafely (dronebot)
//=============================================================
// Checks for lava and slime
#define	MASK_DEADLY				(CONTENTS_LAVA|CONTENTS_SLIME)
//#define TRACE_DIST_SHORT 48
//#define TRACE_DOWN 96
//#define VEC_ORIGIN tv(0,0,0)
//

qboolean	CanMoveSafely(edict_t	*self, vec3_t angles)
{
	vec3_t	dir, angle, dest1, dest2;
	trace_t	trace;
	//float	this_dist;

//	self->bot_ai.next_safety_time = level.time + EYES_FREQ;

	VectorClear(angle);
	angle[1] = angles[1];

	AngleVectors(angle, dir, NULL, NULL);
	
	// create a position in front of the bot
	VectorMA(self->s.origin, TRACE_DIST_SHORT, dir, dest1);

	// Modified to check  for crawl direction
	//trace = gi.trace(self->s.origin, mins, self->maxs, dest, self, MASK_SOLID);
	//BOTUT_TempLaser (self->s.origin, dest1);
	trace = gi.trace(self->s.origin, tv(-16,-16,0), tv(16,16,0), dest1, self, MASK_PLAYERSOLID);

	// Check if we are looking inside a wall!
	if (trace.startsolid)
		return (true);

	if (trace.fraction > 0)
	{	// check that destination is onground, or not above lava/slime
		dest1[0] = trace.endpos[0];
		dest1[1] = trace.endpos[1];
		dest1[2] = trace.endpos[2] - 28;
		//this_dist = trace.fraction * TRACE_DIST_SHORT;

		if (gi.pointcontents(dest1) & MASK_PLAYERSOLID)
			return (true);


		// create a position a distance below it
		VectorCopy( trace.endpos, dest2);
		dest2[2] -= TRACE_DOWN + max(lights_camera_action, self->client->uvTime) * 32;
		//BOTUT_TempLaser (trace.endpos, dest2);
		trace = gi.trace(trace.endpos, VEC_ORIGIN, VEC_ORIGIN, dest2, self, MASK_PLAYERSOLID | MASK_DEADLY);

		if ((trace.fraction == 1.0)										// long drop!
			|| (trace.contents & MASK_DEADLY)							// avoid SLIME or LAVA
			|| (trace.ent && (trace.ent->touch == hurt_touch))			// avoid MOD_TRIGGER_HURT
			|| (trace.surface && (trace.surface->flags & SURF_SKY)))	// avoid falling onto skybox
		{
			return (false);
		}
		else
		{
			return (true);
		}
	}
	//gi.bprintf(PRINT_HIGH,"Default failure from LAVACHECK\n");
	return (false);
}
///////////////////////////////////////////////////////////////////////
// Checks if bot can move (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime. 
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_CanMove(edict_t *self, int direction)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);
	
	if (direction == MOVE_LEFT)
		angles[1] += 90;
	else if (direction == MOVE_RIGHT)
		angles[1] -= 90;
	else if (direction == MOVE_BACK)
		angles[1] -= 180;

	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 36, 0, 24);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
		
	VectorSet(offset, 36, 0, -110); // RiEvEr reduced drop distance
	G_ProjectSource (self->s.origin, offset, forward, right, end);
	
	//AQ2 ADDED MASK_SOLID
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID|MASK_OPAQUE);
	
	if (((tr.fraction == 1.0) && !((lights_camera_action || self->client->uvTime) && CanMoveSafely(self, angles))) // avoid falling after LCA
		|| (tr.contents & MASK_DEADLY)							  // avoid SLIME or LAVA
		|| (tr.ent && (tr.ent->touch == hurt_touch)))			  // avoid MOD_TRIGGER_HURT
	{
		if (self->last_door_time < level.framenum)
		{
			if (debug_mode && (level.framenum % HZ == 0))
				debug_printf("%s: move blocked (ACEMV_CanMove)\n", self->client->pers.netname);

			Cmd_OpenDoor_f(self);	// Open the door
			//self->last_door_time = level.framenum + 3 * HZ; // wait!
		}
		return false;
	}
	
	return true; // yup, can move
}

///////////////////////////////////////////////////////////////////////
// Checks if bot can jump (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime. 
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_CanJumpInternal(edict_t *self, int direction)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);
	
	if(direction == MOVE_LEFT)
		angles[1] += 90;
	else if(direction == MOVE_RIGHT)
		angles[1] -= 90;
	else if(direction == MOVE_BACK)
		angles[1] -=180;

	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 132, 0, 24);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
		
	VectorSet(offset, 132, 0, -110); // RiEvEr reduced drop distance
	G_ProjectSource (self->s.origin, offset, forward, right, end);
	
	//AQ2 ADDED MASK_SOLID
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID|MASK_OPAQUE);
	
	if (((tr.fraction == 1.0) && !((lights_camera_action || self->client->uvTime) && CanMoveSafely(self, angles))) // avoid falling after LCA
		|| (tr.contents & MASK_DEADLY)                 // avoid SLIME or LAVA
		|| (tr.ent && (tr.ent->touch == hurt_touch))) // avoid MOD_TRIGGER_HURT
	{
		if (self->last_door_time < level.framenum)
		{
			if (debug_mode && (level.framenum % HZ == 0))
				debug_printf("%s: move blocked (ACEMV_CanJumpInternal)\n", self->client->pers.netname);

			Cmd_OpenDoor_f(self);	// Open the door
			//self->last_door_time = level.framenum + 3 * HZ; // wait!
		}
		return false;
	}
	
	return true; // yup, can move
}

qboolean ACEMV_CanJump(edict_t *self)
{
	return( (ACEMV_CanJumpInternal(self, MOVE_LEFT)) &&
			(ACEMV_CanJumpInternal(self, MOVE_RIGHT)) &&
			(ACEMV_CanJumpInternal(self, MOVE_BACK)) &&
			(ACEMV_CanJumpInternal(self, MOVE_FORWARD)) );

}

///////////////////////////////////////////////////////////////////////
// Handle special cases of crouch/jump
//
// If the move is resolved here, this function returns
// true.
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_SpecialMove(edict_t *self, usercmd_t *ucmd)
{
	vec3_t dir,forward,right,start,end,offset;
	vec3_t top;
	trace_t tr; 
	
	// Get current direction
	VectorCopy(self->client->ps.viewangles,dir);
	dir[YAW] = self->s.angles[YAW];
	AngleVectors (dir, forward, right, NULL);

	VectorSet(offset, 0, 0, 0); // changed from 18,0,0
	G_ProjectSource (self->s.origin, offset, forward, right, start);
	offset[0] += 18;
	G_ProjectSource (self->s.origin, offset, forward, right, end);
	
	// trace it
	start[2] += 18; // so they are not jumping all the time
	end[2] += 18;
	tr = gi.trace (start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);
		
//RiEvEr	if(tr.allsolid)
	if( tr.fraction < 1.0)
	{
		//rekkie -- DEV_1 -- s
		// Handles cases where bots get stuck on each other, for example in teamplay where they can block each other
		// So lets try to make them jump over each other
		if (tr.ent && tr.ent->is_bot && self->groundentity)
		{
			if (random() > 0.5) // try jumping
			{
				//if (debug_mode && level.framenum % HZ == 0)
					//debug_printf("%s %s: move blocked ----------------------- [[[[[ JUMPING ]]]]]] ---------\n", __func__, self->client->pers.netname);
				self->velocity[2] += 400;
				//ucmd->upmove = SPEED_RUN;
				ucmd->forwardmove = SPEED_RUN;
				return false;
			}
			else // try crouching
			{
				//if (debug_mode && level.framenum % HZ == 0)
					//debug_printf("%s %s: move blocked ----------------------- [[[[[ CROUCH ]]]]]] ---------\n", __func__, self->client->pers.netname);
				ucmd->upmove = -SPEED_RUN;
				ucmd->forwardmove = SPEED_RUN;
				return false;
			}
		}
		//rekkie -- DEV_1 -- e

		//RiEvEr
		if (tr.ent && Q_stricmp(tr.ent->classname, "func_door_rotating") == 0)
		{
			ucmd->forwardmove = -SPEED_RUN; // back up in case it's trying to open
			return true;
		}
		//R

		// Check for crouching
		start[2] -= 14;
		end[2] -= 14;

		// Set up for crouching check
		VectorCopy(self->maxs,top);
		top[2] = 0.0; // crouching height
		tr = gi.trace (start, self->mins, top, end, self, MASK_PLAYERSOLID);
		
		// Crouch
//RiEvEr		if(!tr.allsolid) 
		if( tr.fraction == 1.0 )
		{
			ucmd->forwardmove = SPEED_RUN;
			ucmd->upmove = -SPEED_RUN;
			return true;
		}
		
		// Check for jump
		start[2] += 32;
		end[2] += 32;
		tr = gi.trace (start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

//RiEvEr		if(!tr.allsolid)
		if( tr.fraction == 1.0)
		{	
			ucmd->forwardmove = SPEED_RUN;
			ucmd->upmove = SPEED_RUN;
			return true;
		}
		//RiEvEr
		// My corner management code
		if( BOTCOL_CheckBump(self, ucmd ))
		{
			if( BOTCOL_CanStrafeSafely(self, self->s.angles ) )
			{
				ucmd->sidemove = self->bot_strafe;
				return true;
			}
		}
		//R
	}
	
	return false; // We did not resolve a move here
}


///////////////////////////////////////////////////////////////////////
// Checks for obstructions in front of bot
//
// This is a function I created origianlly for ACE that
// tries to help steer the bot around obstructions.
//
// If the move is resolved here, this function returns true.
///////////////////////////////////////////////////////////////////////
qboolean ACEMV_CheckEyes(edict_t *self, usercmd_t *ucmd)
{
	vec3_t  forward, right;
	vec3_t  leftstart, rightstart,focalpoint;
	vec3_t  /* upstart,*/upend;
	vec3_t  dir,offset;

	trace_t traceRight,traceLeft,/*traceUp,*/ traceFront; // for eyesight

	// Get current angle and set up "eyes"
	VectorCopy(self->s.angles,dir);
	AngleVectors (dir, forward, right, NULL);
	
	// Let them move to targets by walls
	if(!self->movetarget)
//		VectorSet(offset,200,0,4); // focalpoint 
		VectorSet(offset,64,0,4); // focalpoint 
	else
		VectorSet(offset,36,0,4); // focalpoint 
	
	G_ProjectSource (self->s.origin, offset, forward, right, focalpoint);

	// Check from self to focalpoint
	// Ladder code
	VectorSet(offset,36,0,0); // set as high as possible
	G_ProjectSource (self->s.origin, offset, forward, right, upend);
	//AQ2 ADDED MASK_SOLID
	traceFront = gi.trace(self->s.origin, self->mins, self->maxs, upend, self, MASK_SOLID|MASK_OPAQUE);
		
	if(traceFront.contents & 0x8000000) // using detail brush here cuz sometimes it does not pick up ladders...??
	{
		ucmd->upmove = SPEED_RUN;
		ucmd->forwardmove = SPEED_RUN;
		return true;
	}
	
	// If this check fails we need to continue on with more detailed checks
	if(traceFront.fraction == 1)
	{	
		ucmd->forwardmove = SPEED_RUN;
		return true;
	}

	VectorSet(offset, 0, 18, 4);
	G_ProjectSource (self->s.origin, offset, forward, right, leftstart);
	
	offset[1] -= 36; // want to make sure this is correct
	//VectorSet(offset, 0, -18, 4);
	G_ProjectSource (self->s.origin, offset, forward, right, rightstart);

	traceRight = gi.trace(rightstart, NULL, NULL, focalpoint, self, MASK_OPAQUE);
	traceLeft = gi.trace(leftstart, NULL, NULL, focalpoint, self, MASK_OPAQUE);

	// Wall checking code, this will degenerate progressivly so the least cost 
	// check will be done first.
		
	// If open space move ok
	if(traceRight.fraction != 1 || traceLeft.fraction != 1 || strcmp(traceLeft.ent->classname,"func_door")!=0)
	{
//@@ weird code...
/*		// Special uppoint logic to check for slopes/stairs/jumping etc.
		VectorSet(offset, 0, 18, 24);
		G_ProjectSource (self->s.origin, offset, forward, right, upstart);

		VectorSet(offset,0,0,200); // scan for height above head
		G_ProjectSource (self->s.origin, offset, forward, right, upend);
		traceUp = gi.trace(upstart, NULL, NULL, upend, self, MASK_OPAQUE);
			
		VectorSet(offset,200,0,200*traceUp.fraction-5); // set as high as possible
		G_ProjectSource (self->s.origin, offset, forward, right, upend);
		traceUp = gi.trace(upstart, NULL, NULL, upend, self, MASK_OPAQUE);

		// If the upper trace is not open, we need to turn.
		if(traceUp.fraction != 1)*/
		{						
			if(traceRight.fraction > traceLeft.fraction)
				self->s.angles[YAW] += (1.0 - traceLeft.fraction) * 45.0;
			else
				self->s.angles[YAW] += -(1.0 - traceRight.fraction) * 45.0;
			
			ucmd->forwardmove = SPEED_RUN;
			ACEMV_ChangeBotAngle(self);
			return true;
		}
	}
				
	return false;
}

///////////////////////////////////////////////////////////////////////
// Make the change in angles a little more gradual, not so snappy
// Subtle, but noticeable.
// 
// Modified from the original id ChangeYaw code...
///////////////////////////////////////////////////////////////////////
void ACEMV_ChangeBotAngle (edict_t *ent)
{
	float   ideal_yaw;
	float   ideal_pitch;
	float   current_yaw;
	float   current_pitch;
	float   speed;
	vec3_t  ideal_angle;
	float   yaw_move = 0.f;
	float   pitch_move = 0.f;
	float   move_ratio = 1.f;

	// Normalize the move angle first
	VectorNormalize(ent->move_vector);

	current_yaw = anglemod(ent->s.angles[YAW]);
	current_pitch = anglemod(ent->s.angles[PITCH]);

	vectoangles (ent->move_vector, ideal_angle);

	ideal_yaw = anglemod(ideal_angle[YAW]);
	ideal_pitch = anglemod(ideal_angle[PITCH]);

	// Raptor007: Compensate for M4 climb.
	if( (ent->client->weaponstate == WEAPON_FIRING) && (ent->client->weapon == FindItem(M4_NAME)) )
		ideal_pitch -= ent->client->kick_angles[PITCH];

	// Yaw
	if (current_yaw != ideal_yaw)
	{	
		yaw_move = ideal_yaw - current_yaw;
		speed = ent->yaw_speed / (float) BASE_FRAMERATE;
		if (ideal_yaw > current_yaw)
		{
			if (yaw_move >= 180)
				yaw_move = yaw_move - 360;
		}
		else
		{
			if (yaw_move <= -180)
				yaw_move = yaw_move + 360;
		}
		if (yaw_move > 0)
		{
			if (yaw_move > speed)
				yaw_move = speed;
		}
		else
		{
			if (yaw_move < -speed)
				yaw_move = -speed;
		}
	}

	// Pitch
	if (current_pitch != ideal_pitch)
	{	
		pitch_move = ideal_pitch - current_pitch;
		speed = ent->yaw_speed / (float) BASE_FRAMERATE;
		if (ideal_pitch > current_pitch)
		{
			if (pitch_move >= 180)
				pitch_move = pitch_move - 360;
		}
		else
		{
			if (pitch_move <= -180)
				pitch_move = pitch_move + 360;
		}
		if (pitch_move > 0)
		{
			if (pitch_move > speed)
				pitch_move = speed;
		}
		else
		{
			if (pitch_move < -speed)
				pitch_move = -speed;
		}
	}

	// Raptor007: Interpolate towards desired changes at higher fps.
	if( ! FRAMESYNC )
	{
		int frames_until_sync = FRAMEDIV - (level.framenum - 1) % FRAMEDIV;
		move_ratio = 1.f / (float) frames_until_sync;
	}

	ent->s.angles[YAW] = anglemod( current_yaw + yaw_move * move_ratio );
	ent->s.angles[PITCH] = anglemod( current_pitch + pitch_move * move_ratio );
}

/*
///////////////////////////////////////////////////////////////////////
// Set bot to move to it's movetarget. (following node path)
///////////////////////////////////////////////////////////////////////
void ACEMV_MoveToGoal(edict_t *self, usercmd_t *ucmd)
{
	// If a rocket or grenade is around deal with it
	// Simple, but effective (could be rewritten to be more accurate)
	if(strcmp(self->movetarget->classname,"rocket")==0 ||
	   strcmp(self->movetarget->classname,"grenade")==0)
	{
		VectorSubtract (self->movetarget->s.origin, self->s.origin, self->move_vector);
		ACEMV_ChangeBotAngle(self);
		if(debug_mode)
			debug_printf("%s: Oh crap a rocket!\n",self->client->pers.netname);

		// strafe left/right
		if( (self->bot_strafe <= 0) && ACEMV_CanMove(self, MOVE_LEFT) )
			ucmd->sidemove = -SPEED_RUN;
		else if(ACEMV_CanMove(self, MOVE_RIGHT))
			ucmd->sidemove = SPEED_RUN;
		else
			ucmd->sidemove = 0;
		self->bot_strafe = ucmd->sidemove;
		return;

	}
	else
	{
		{
			// Are we there yet?
			vec3_t v;
			trace_t	tTrace;
			vec3_t	vStart, vDest;

			if( nodes[self->bot.current_node].type == NODE_DOOR )
			{
			
				else
				{
					VectorCopy( self->s.origin, vStart );
					VectorCopy( nodes[self->current_node].origin, vDest );
					VectorSubtract( self->s.origin, nodes[self->current_node].origin, v );
				}

				if(VectorLength(v) < 32)
				{
					// See if we have a clear shot at it
					PRETRACE();
					tTrace = gi.trace( vStart, tv(-16,-16,-8), tv(16,16,8), vDest, self, MASK_PLAYERSOLID);
					POSTTRACE();
					if( tTrace.fraction <1.0 )
					{
						if( (strcmp( tTrace.ent->classname, "func_door_rotating" ) == 0)
						||  (strcmp( tTrace.ent->classname, "func_door") == 0) )
						{
							// The door is in the way
							// See if it's moving
							if( (VectorLength(tTrace.ent->avelocity) > 0.0)
							||  ((tTrace.ent->moveinfo.state != STATE_BOTTOM) && (tTrace.ent->moveinfo.state != STATE_TOP)) )
							{
								if( self->last_door_time < level.framenum )
									self->last_door_time = level.framenum;
								if( ACEMV_CanMove( self, MOVE_BACK ) )
									ucmd->forwardmove = -SPEED_WALK;
								return;
							}
							else
							{
								// Open it
								if( (self->last_door_time < level.framenum - 2.0 * HZ) &&
									(tTrace.ent->moveinfo.state != STATE_TOP) &&
									(tTrace.ent->moveinfo.state != STATE_UP) )
								{
									Cmd_OpenDoor_f ( self );	// Open the door
									self->last_door_time = level.framenum + random() * 2.5 * HZ; // wait!
									ucmd->forwardmove = 0;
									VectorSubtract( self->movetarget->s.origin, self->s.origin, self->move_vector );
									ACEMV_ChangeBotAngle( self );
									return;
								}
							}
						}
						else if( tTrace.ent->client )
						{
							// Back up slowly - may have a teammate in the way
							if( self->last_door_time < level.framenum )
								self->last_door_time = level.framenum;
							if( ACEMV_CanMove( self, MOVE_BACK ) )
								ucmd->forwardmove = -SPEED_WALK;
							return;
						}
					}
					else
					{
						// If trace from bot to next node hits rotating door, it should just strafe toward the path.
						if( VectorLength(v) < 32 )
						{
							PRETRACE();
							tTrace = gi.trace( vStart, tv(-16,-16,-8), tv(16,16,8), vDest, self, MASK_PLAYERSOLID );
							POSTTRACE();
							if( (tTrace.fraction < 1.)
							&&  (strcmp( tTrace.ent->classname, "func_door_rotating" ) == 0)
							&&  (VectorLength(tTrace.ent->avelocity) > 0.) )
							{
								if( self->next_node != self->current_node )
								{
									// Decide if the bot should strafe to stay on course.
									vec3_t dist = {0,0,0};
									VectorSubtract( nodes[self->next_node].origin, nodes[self->current_node].origin, v );
									v[2] = 0;
									VectorSubtract( self->s.origin, nodes[self->next_node].origin, dist );
									dist[2] = 0;
									if( (DotProduct( v, dist ) > 0) && (VectorLength(v) > 16) )
									{
										float left = 0;
										VectorNormalize( v );
										VectorRotate2( v, 90 );
										left = DotProduct( v, dist );
										if( (left > 16) && (! self->groundentity || ACEMV_CanMove( self, MOVE_RIGHT )) )
											ucmd->sidemove = SPEED_RUN;
										else if( (left < -16) && (! self->groundentity || ACEMV_CanMove( self, MOVE_LEFT )) )
											ucmd->sidemove = -SPEED_RUN;
									}
								}
								else
								{
									ucmd->sidemove = 0;
									if( (self->bot_strafe >= 0) && ACEMV_CanMove( self, MOVE_RIGHT ) )
										ucmd->sidemove = SPEED_RUN;
									else if( ACEMV_CanMove( self, MOVE_LEFT ) )
										ucmd->sidemove = -SPEED_RUN;
									self->bot_strafe = ucmd->sidemove;
								}
								if( ACEMV_CanMove( self, MOVE_BACK ) )
									ucmd->forwardmove = -SPEED_RUN;
								VectorSubtract( self->movetarget->s.origin, self->s.origin, self->move_vector );
								ACEMV_ChangeBotAngle( self );
								return;
							}
						}
					}
				}
			}
		}

		// Set bot's movement direction
		VectorSubtract (self->movetarget->s.origin, self->s.origin, self->move_vector);
		ACEMV_ChangeBotAngle(self);
		ucmd->forwardmove = SPEED_RUN;
		return;
	}
}
*/

#if 0
///////////////////////////////////////////////////////////////////////
// Main movement code. (following node path)
///////////////////////////////////////////////////////////////////////
void ACEMV_Move(edict_t *self, usercmd_t *ucmd)
{
	trace_t tr;
	float distance = 0, distance_xy; // Distance XYZ and Distance XY
	vec3_t dist = { 0 }, dist_xy = { 0 }; // Distance XYZ and Distance XY
	vec3_t velocity;
	int i = 0, current_node_type = INVALID, next_node_type = INVALID;

	// Do not follow path when teammates are still inside us.
	if( OnTransparentList(self) )
	{
		//rekkie -- DEV_1 -- s

		// If the bot has just spawned, then we need to wait a bit before we start moving.
		if (self->just_spawned) // set by SpawnPlayers() in a_team.c
		{
			self->just_spawned = false;
			self->just_spawned_go = true; // Bot is ready, when wander_timeout is reached.
			self->state = STATE_MOVE;
			if (random() < 0.3)
				self->just_spawned_timeout = level.framenum + (random() * 7) * HZ;	// Long wait
			else if (random() < 0.7)
				self->just_spawned_timeout = level.framenum + (random() * 3) * HZ;	// Average wait
			else
				self->just_spawned_timeout = level.framenum + random() * HZ;		// Short wait
			return;
		}
		if (self->just_spawned_go && self->just_spawned_timeout > level.framenum) // Wait
		{
			return; // It's not time to move yet, wait!
		}
		else if (self->just_spawned_go)
		{
			// Now we can move!
			self->just_spawned_go = false;
			//ACEAI_PickLongRangeGoal(self); // Lets pick a long range goal
			return;
		}
		if (0) // Don't wander
		{
		//rekkie -- DEV_1 -- e

		self->state = STATE_WANDER;
		self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
		return;

		//rekkie -- DEV_1 -- s
		}
		//rekkie -- DEV_1 -- e
	}

	// Get current and next node back from nav code.
	if(!BOTLIB_FollowPath(self))
	{
		if(!teamplay->value || (teamplay->value && level.framenum >= self->teamPauseTime))
		{
			self->state = STATE_WANDER;
			self->wander_timeout = level.framenum + 1.0 * HZ;
		}
		else
		{
			// Teamplay mode - just fan out and chill
			if (self->state == STATE_FLEE)
			{
				self->state = STATE_POSITION;
				self->wander_timeout = level.framenum + 3.0 * HZ;
			}
			else
			{
				self->state = STATE_POSITION;
				self->wander_timeout = level.framenum + 1.0 * HZ;
			}
		}
		self->goal_node = INVALID;
		return;
	}

	//rekkie -- DEV_1 -- s
	if (self->current_node == INVALID || self->next_node == INVALID) // rekkie -- safey check because current / next node can be INVALID
	{
		self->state = STATE_WANDER;
		self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
		return;
	}
	//rekkie -- DEV_1 -- e

	current_node_type = nodes[self->current_node].type;

	//rekkie -- DEV_1 -- s
	// 
	// original code
	// next_node_type = nodes[self->next_node].type;
	// 
	// modified code
	// We now get the next node type from the link between current and next node
	// Using the link from the current node to the next node, find the next node type
	int targetNodeLink = INVALID;
	//if (current_node_type == NODE_LADDER_UP && nodes[self->next_node].type == NODE_LADDER_DOWN)
	if (nodes[self->next_node].type == NODE_LADDER_UP || nodes[self->next_node].type == NODE_LADDER_DOWN)
	{
		next_node_type = nodes[self->next_node].type;
		//PrintNodeType(current_node_type, next_node_type);
	}
	else
	for (i = 0; i < MAXLINKS; i++)
	{
		if (nodes[self->current_node].links[i].targetNode == self->next_node)
		{
			targetNodeLink = i;
			next_node_type = nodes[self->current_node].links[i].targetNodeType; // Next node type
			PrintNodeType(current_node_type, next_node_type);
			break;
			
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//float speed = VectorLength(nodes[self->current_node].links[i].targetVelocity);
			/*
			vec3_t dist, velocity;
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			//dist[2] = 0; // Lose the vertical component
			float distance = VectorLength(dist);
			float gravity = self->gravity * sv_gravity->value;
			ACEMV_ChangeBotAngle(self);
			//ucmd->forwardmove = SPEED_RUN;
			int curr_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
			if (self->groundentity && curr_node == self->current_node)
			{
				float jump_height = sqrt(2 * (gravity * distance)); // Calculate the jump height to get to the target
				float time = distance / (self->velocity[2] + jump_height / 2.0); // Calculate the time it will take to get to the target
				VectorScale(dist, 1.0 / time, velocity); // Calculate the velocity at the end of the jump
				velocity[2] = jump_height / 2.0;

				// If the target is above the player, increase the velocity to get to the target
				float z_height;
				if (nodes[self->next_node].origin[2] > self->s.origin[2])
				{
					z_height = (nodes[self->next_node].origin[2] - self->s.origin[2]) + NODE_Z_HEIGHT;
					velocity[2] += z_height;
				}
				else
				{
					z_height = (self->s.origin[2] - nodes[self->next_node].origin[2]) - NODE_Z_HEIGHT;
					velocity[2] -= z_height;
				}

				// Calculate speed from velocity
				//float speed = VectorLength(velocity);
				//gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);

				// Limit max speed
				//if (speed < 750 && z_height < 155 && jump_height <= 965) // current_node_type == NODE_LADDER
				{
					// Set the velocity of the player to the calculated velocity
					VectorCopy(velocity, self->velocity);
					VectorCopy(velocity, self->client->oldvelocity);
					VectorCopy(velocity, self->avelocity);
					return;
				}
			}
			if (self->groundentity && curr_node != self->next_node)
			{
				{
					Com_Printf("%s %s failed to jumppad up, trying alternative path to goal\n", __func__, self->client->pers.netname);
					BOTLIB_SetGoal(self, self->goal_node);
				}
				else
				{
					Com_Printf("%s %s failed to jumppad up, finding new goal\n", __func__, self->client->pers.netname);
					self->state = STATE_WANDER; // Aquire a new node
				}
				return;
				//ucmd->forwardmove = SPEED_RUN;
				//ucmd->upmove = SPEED_RUN;
			}
			
			// Guide the bot in when close to the target
			if (distance <= 48) // || (distance < 96 && (nodes[self->next_node].type == NODE_LADDER_UP || nodes[self->next_node].type == NODE_LADDER_DOWN)))
			{
				VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				distance = VectorLength(dist);	// Get the absolute length

				// Apply it
				if (distance > 48)
					VectorScale(dist, SPEED_RUN, self->velocity);
				else if (distance > 24)
					VectorScale(dist, 350, self->velocity);
				else
					VectorScale(dist, 300, self->velocity);
			}

			return;
			*/
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			
			//if (next_node_type == NODE_LEARN) // && self->groundentity)
			//{}
			//break;
		}
	}
	//rekkie -- DEV_1 -- e

	//rekkie -- DEV_1 -- s
	//if (current_node_type == NODE_JUMPPAD || next_node_type == NODE_JUMPPAD)
	//	gi.dprintf("%s %s [ct:%i] [nt:%i]\n", __func__, self->client->pers.netname, current_node_type, next_node_type);

	// If the next node is high enough, make it a jump
	/*
	if (next_node_type == NODE_MOVE && self->client->leg_damage == 0)
	{
		
		// See if there is a gap / hole between the current and next node, if so we need to jump across
		vec3_t pt_25, pt_50, pt_75;
		VectorAdd(nodes[self->current_node].origin, nodes[self->next_node].origin, pt_25);	// 25%
		VectorAdd(nodes[self->current_node].origin, nodes[self->next_node].origin, pt_50);  // 50%
		VectorAdd(nodes[self->current_node].origin, nodes[self->next_node].origin, pt_75);	// 75%
		VectorScale(pt_25, 0.25, pt_25); // Scale 25%
		VectorScale(pt_50, 0.50, pt_50); // Scale 50%
		VectorScale(pt_75, 0.75, pt_75); // Scale 75%
		// Using the points between the current and next node, trace a line down and see if one of them hits air
		trace_t tr_25, tr_50, tr_75;
		vec3_t end_25, end_50, end_75;
		VectorCopy(pt_25, end_25);
		VectorCopy(pt_50, end_50);
		VectorCopy(pt_75, end_75);
		end_25[2] -= NODE_MAX_JUMP_HEIGHT;
		end_50[2] -= NODE_MAX_JUMP_HEIGHT;
		end_75[2] -= NODE_MAX_JUMP_HEIGHT;
		tr_25 = gi.trace(pt_25, vec3_origin, vec3_origin, end_25, self, MASK_SOLID);
		tr_50 = gi.trace(pt_50, vec3_origin, vec3_origin, end_50, self, MASK_SOLID);
		tr_75 = gi.trace(pt_75, vec3_origin, vec3_origin, end_75, self, MASK_SOLID);
		// If one of our trace points hit nothing
		if ( (tr_25.fraction == 1.0 && tr_25.allsolid == false) ||
			(tr_50.fraction == 1.0 && tr_50.allsolid == false) ||
			(tr_75.fraction == 1.0 && tr_75.allsolid == false) )
		{
			next_node_type = NODE_JUMPPAD;
		}

		// Otherwise lets see if the next_node is higher than current_node, if so jump to it
		//else
		{
			// Distance between current and next node.
			//VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			//distance = VectorLength(dist);

			// Calculate if nodes[self->next_node].origin[2] is higher than self->s.origin[2]
			float higher = 0; float lower = 0;
			if (nodes[self->next_node].origin[2] > nodes[self->current_node].origin[2])
				higher = (nodes[self->next_node].origin[2] - nodes[self->current_node].origin[2]);
			else if (nodes[self->next_node].origin[2] < nodes[self->current_node].origin[2])
				lower = (nodes[self->current_node].origin[2] - nodes[self->next_node].origin[2]);
			//if (distance >= 128 || higher >= NODE_MAX_JUMP_HEIGHT || lower >= NODE_MAX_JUMP_HEIGHT)
			//if (higher >= NODE_MAX_JUMP_HEIGHT) // || lower >= NODE_MAX_CROUCH_FALL_HEIGHT)
			if (higher) // || lower)
			{
				next_node_type = NODE_JUMPPAD;
			}
		}
	}
	*/
	
	if (self->client->leg_damage > 0) // Do we have leg damage?
		next_node_type = NODE_MOVE; // Force NODE_MOVE so we can safely stumble to the next target node
	
	//if ((current_node_type == NODE_LADDER || next_node_type == NODE_LADDER) && OnLadder(self) == false)
	//	next_node_type = NODE_JUMPPAD;
	//rekkie -- DEV_1 -- e
		
	///////////////////////////
	// Move To Goal
	///////////////////////////
	if (self->movetarget)
		ACEMV_MoveToGoal(self,ucmd);

	else if( (self->next_node != self->current_node) && (current_node_type != NODE_LADDER) )
	{
		// Decide if the bot should strafe to stay on course.
		vec3_t v = {0,0,0};
		VectorSubtract( nodes[self->next_node].origin, nodes[self->current_node].origin, v );
		v[2] = 0;
		VectorSubtract( self->s.origin, nodes[self->next_node].origin, dist );
		dist[2] = 0;
		if( (DotProduct( v, dist ) > 0) && (VectorLength(v) > 16) )
		{
			float left = 0;
			VectorNormalize( v );
			VectorRotate2( v, 90 );
			left = DotProduct( v, dist );
			if( (left > 16) && (! self->groundentity || ACEMV_CanMove( self, MOVE_RIGHT )) )
				ucmd->sidemove = SPEED_RUN;
			else if( (left < -16) && (! self->groundentity || ACEMV_CanMove( self, MOVE_LEFT )) )
				ucmd->sidemove = -SPEED_RUN;
		}
	}

	if(current_node_type == NODE_GRAPPLE)
	{
		if( self->last_door_time < level.framenum )
		{
			ucmd->forwardmove = SPEED_WALK; //walk forwards a little
//			Cmd_OpenDoor_f ( self );	// Open the door
			self->last_door_time = level.framenum + random() * 2.0 * HZ; // wait!
		}

	}
		
/*	////////////////////////////////////////////////////////
	// Grapple
	///////////////////////////////////////////////////////
	if(next_node_type == NODE_GRAPPLE)
	{
		ACEMV_ChangeBotAngle(self);
		ACEIT_ChangeWeapon(self,FindItem("grapple"));	
		ucmd->buttons = BUTTON_ATTACK;
		return;
	}
	// Reset the grapple if hangin on a graple node
	if(current_node_type == NODE_GRAPPLE)
	{
		CTFPlayerResetGrapple(self);
		return;
	}*/

	//rekkie -- DEV_1 -- s
	// Try opening the door 'remotely' before we reach it
	if (next_node_type == NODE_DOOR)
	{
		edict_t* node_ent = node_ents[self->next_node];
		if (node_ent && (strcmp(node_ent->classname, "func_door_rotating") == 0 || strcmp(node_ent->classname, "func_door") == 0))
		{
			/*
			trace_t	tTrace;
			vec3_t	vStart, vDest;
			if (next_node_type == NODE_DOOR)
				VectorCopy(nodes[self->current_node].origin, vStart);
			else
				VectorCopy(self->s.origin, vStart);
			VectorCopy(nodes[self->next_node].origin, vDest);
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			dist[2] = 0;
			distance = VectorLength(dist);
			// See if we have a clear shot at it
			PRETRACE();
			tTrace = gi.trace(vStart, tv(-16, -16, -8), tv(16, 16, 8), vDest, self, MASK_PLAYERSOLID);
			POSTTRACE();
			if ((tTrace.fraction < 1.0) && (distance < 512))
			{
				if (tTrace.ent->client)
				{
					// Back up slowly - may have a teammate in the way
					if (self->last_door_time < level.framenum)
						self->last_door_time = level.framenum;
					if (ACEMV_CanMove(self, MOVE_BACK))
						ucmd->forwardmove = -SPEED_WALK;
					else if (self->tries && self->groundentity)
						ucmd->upmove = SPEED_RUN;
					return;
				}
			}

#define DOOR_TOGGLE 32
			if ((node_ent->spawnflags & DOOR_TOGGLE) && (node_ent->moveinfo.state == STATE_UP || node_ent->moveinfo.state == STATE_TOP))
			{
				//if (ACEMV_CanMove(self, MOVE_BACK))
				//	ucmd->forwardmove = -SPEED_WALK; //walk backwards a little
				//else if (self->tries && self->groundentity)
				//	ucmd->upmove = SPEED_RUN;
				//ucmd->forwardmove = SPEED_WALK;
				//return;
			}
			*/

			/*
			// The door is in the way
			// See if it's moving
			if ((VectorLength(node_ent->avelocity) > 0.0) || ((node_ent->moveinfo.state != STATE_BOTTOM) && (node_ent->moveinfo.state != STATE_DOWN)))
			{
				if (self->last_door_time < level.framenum)
					self->last_door_time = level.framenum;
				if (ACEMV_CanMove(self, MOVE_BACK))
					ucmd->forwardmove = -SPEED_WALK; //walk backwards a little
				else if (self->tries && self->groundentity)
					ucmd->upmove = SPEED_RUN;
				return;
			}
			else*/
			{
				// Open it
				if ((self->last_door_time < level.framenum - 2.0 * HZ) && (node_ent->moveinfo.state != STATE_TOP) && (node_ent->moveinfo.state != STATE_UP))
				{
					//Cmd_OpenDoor_f(self);	// Open the door
					door_use(node_ent, self, self); // Open the door

					self->last_door_time = level.framenum + random() * 2.5 * HZ; // wait!
					if (self->tries && self->groundentity)
						ucmd->upmove = SPEED_RUN;
					ucmd->forwardmove = 0;
					return;
				}
			}
		}
	}
	//rekkie -- DEV_1 -- e

	////////////////////////////////////////////////////////
	// Doors - RiEvEr
	// modified by Werewolf
	///////////////////////////////////////////////////////
	if(current_node_type == NODE_DOOR)
	{
		// We are trying to go through a door
		trace_t	tTrace;
		vec3_t	vStart, vDest;

		if( next_node_type == NODE_DOOR )
			VectorCopy( nodes[self->current_node].origin, vStart );
		else
			VectorCopy( self->s.origin, vStart );

		VectorCopy( nodes[self->next_node].origin, vDest );
		VectorSubtract( nodes[self->next_node].origin, self->s.origin, dist );
		dist[2] = 0;
		distance = VectorLength(dist);

		// See if we have a clear shot at it
		PRETRACE();
		tTrace = gi.trace( vStart, tv(-16,-16,-8), tv(16,16,8), vDest, self, MASK_PLAYERSOLID);
		POSTTRACE();

		if( (tTrace.fraction < 1.0) && (distance < 512) )
		{
			if( (strcmp( tTrace.ent->classname, "func_door_rotating" ) == 0)
			||  (strcmp( tTrace.ent->classname, "func_door") == 0) )
			{
				// The door is in the way
				// See if it's moving
				if( (VectorLength(tTrace.ent->avelocity) > 0.0)
				||  ((tTrace.ent->moveinfo.state != STATE_BOTTOM) && (tTrace.ent->moveinfo.state != STATE_TOP)) )
				{
					if( self->last_door_time < level.framenum )
						self->last_door_time = level.framenum;
					if( ACEMV_CanMove( self, MOVE_BACK ) )
						ucmd->forwardmove = -SPEED_WALK; //walk backwards a little
					else if( self->tries && self->groundentity )
						ucmd->upmove = SPEED_RUN;
					return;
				}
				else
				{
					// Open it
					if( (self->last_door_time < level.framenum - 2.0 * HZ) &&
						(tTrace.ent->moveinfo.state != STATE_TOP) &&
						(tTrace.ent->moveinfo.state != STATE_UP) )
					{
						Cmd_OpenDoor_f ( self );	// Open the door
						self->last_door_time = level.framenum + random() * 2.5 * HZ; // wait!
						if( self->tries && self->groundentity )
							ucmd->upmove = SPEED_RUN;
						ucmd->forwardmove = 0;
						return;
					}
				}
			}
			else if( tTrace.ent->client )
			{
				// Back up slowly - may have a teammate in the way
				if( self->last_door_time < level.framenum )
					self->last_door_time = level.framenum;
				if( ACEMV_CanMove( self, MOVE_BACK ) )
					ucmd->forwardmove = -SPEED_WALK;
				else if( self->tries && self->groundentity )
					ucmd->upmove = SPEED_RUN;
				return;
			}
		}
	}
	
	////////////////////////////////////////////////////////
	// Platforms
	///////////////////////////////////////////////////////
	if( (next_node_type == NODE_PLATFORM) && ((current_node_type != NODE_PLATFORM) || (self->current_node == self->next_node)) )
	{
		// Check if the platform is directly above us.
		vec3_t start, above;
		trace_t tr;
		VectorCopy( self->s.origin, start );
		VectorCopy( self->s.origin, above );
		start[2] += 32;
		above[2] += 2048;
		tr = gi.trace( start, tv(-24,-24,-8), tv(24,24,8), above, self, MASK_PLAYERSOLID );
		if( (tr.fraction < 1.0) && tr.ent && (tr.ent->use == Use_Plat) )
		{
			// If it is directly above us, get out of the way.
			ucmd->sidemove = 0;
			if( ACEMV_CanMove( self, MOVE_BACK ) )
				ucmd->forwardmove = -SPEED_WALK;
			else if( (self->bot_strafe <= 0) && ACEMV_CanMove( self, MOVE_LEFT ) )
				ucmd->sidemove = -SPEED_WALK;
			else if( ACEMV_CanMove( self, MOVE_RIGHT ) )
				ucmd->sidemove = SPEED_WALK;
			self->bot_strafe = ucmd->sidemove;
			ACEMV_ChangeBotAngle(self);
			return;
		}
	}
	if(current_node_type != NODE_PLATFORM && next_node_type == NODE_PLATFORM)
	{
		// check to see if lift is down?
		for(i=0;i<num_items;i++)
			if(item_table[i].node == self->next_node)
				if(item_table[i].ent->moveinfo.state != STATE_BOTTOM)
				    return; // Wait for elevator
	}
	if( (current_node_type == NODE_PLATFORM) && self->groundentity
	&&  ((self->groundentity->moveinfo.state == STATE_UP) || (self->groundentity->moveinfo.state == STATE_DOWN)) )
	{
		// Standing on moving elevator.
		ucmd->forwardmove = 0;
		ucmd->sidemove = 0;
		ucmd->upmove = 0;
		if( VectorLength(self->groundentity->velocity) < 8 )
			ucmd->upmove = SPEED_RUN;
		ACEMV_ChangeBotAngle(self);
		if( debug_mode && (level.framenum % HZ == 0) )
			debug_printf( "%s: platform move state %i\n", self->client->pers.netname, self->groundentity->moveinfo.state );
		return;
	}
	if( (current_node_type == NODE_PLATFORM) && (next_node_type == NODE_PLATFORM) && (self->current_node != self->next_node) )
	{
		// Move to the center
		self->move_vector[2] = 0; // kill z movement	
		if(VectorLength(self->move_vector) > 10)
			ucmd->forwardmove = SPEED_WALK; // walk to center
				
		ACEMV_ChangeBotAngle(self);
		
		return; // No move, riding elevator
	}

	//rekkie -- DEV_1 -- s
	////////////////////////////////////////////////////////
	// Jumpad Nodes
	///////////////////////////////////////////////////////
	if (next_node_type == NODE_JUMPPAD)
	{
		// ----- TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO ----- 
		// ----- TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO ----- 
		// ----- TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO ----- 
		//
		// 
		// 
		// ----- TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO ----- 
		// ----- TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO ----- 
		// ----- TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO -----  TODO ----- 

		// Move while jumping
		//ucmd->forwardmove = SPEED_RUN;
		//ucmd->upmove = SPEED_RUN;
		
		vec3_t dist;
		VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
		//dist[2] = 0; // Lose the vertical component
		float next_node_distance = VectorLength(dist);
		/*
		// If the bot's distance is moving away from the target (failed the jump)
		// Try to find another route, otherwise find a new node and go there instead
		if (self->prev_next_node == self->next_node)
		{
			if (next_node_distance < self->next_node_distance) // Update if getting closer
				self->next_node_distance = next_node_distance;
			else if (next_node_distance > 64 && next_node_distance >= self->next_node_distance + NODE_Z_HALF_HEIGHT) // If getting further away we failed the jump
			{
				self->current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
				{
					Com_Printf("%s %s failed to jumppad up, trying alternative path to goal\n", __func__, self->client->pers.netname);
					BOTLIB_SetGoal(self, self->goal_node);
				}
				else
				{
					Com_Printf("%s %s failed to jumppad up, finding new goal\n", __func__, self->client->pers.netname);
					self->state = STATE_WANDER; // Aquire a new node
				}
				return;
			}
		}	
		if (self->prev_next_node != self->next_node) // Update previous next node and distance
		{
			self->prev_next_node = self->next_node;
			self->next_node_distance = next_node_distance;
		}
		*/
		




		float gravity = self->gravity * sv_gravity->value;

		ACEMV_ChangeBotAngle(self);
		ucmd->forwardmove = SPEED_RUN;
		//if (self->groundentity || next_node_distance < NODE_Z_HEIGHT)
		//int curr_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
		// distance between curr_node and self->current_node
		vec3_t curr_node_dist;
		VectorSubtract(nodes[self->current_node].origin, self->s.origin, curr_node_dist);
		float curr_node_distance = VectorLength(curr_node_dist);
		//if (self->groundentity || curr_node_distance <= 8 || (current_node_type == NODE_LADDER_UP && curr_node_distance < 24)) // && curr_node == self->current_node))
		if (curr_node_distance <= 32 || (current_node_type == NODE_LADDER_UP && curr_node_distance < 24)) // && curr_node == self->current_node))
		{
			// Recalculate the dist with the vertical component included
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			float distance = VectorLength(dist);

			// Calculate the jump height to get to the target
			float jump_height = sqrt(2 * (gravity * distance));

			// Calculate the time it will take to get to the target
			float time = distance / (self->velocity[2] + jump_height / 2.0);

			// Calculate the velocity at the end of the jump
			VectorScale(dist, 1.0 / time, velocity);

			velocity[2] = jump_height / 2.0;

			// If the target is above the player, increase the velocity to get to the target
			float z_height;
			if (nodes[self->next_node].origin[2] > self->s.origin[2])
			{
				z_height = (nodes[self->next_node].origin[2] - self->s.origin[2]) + NODE_Z_HEIGHT * 2;
				velocity[2] += z_height;
			}
			else
			{
				z_height = (self->s.origin[2] - nodes[self->next_node].origin[2]) - NODE_Z_HEIGHT;
				velocity[2] -= z_height;
			}

			// Calculate speed from velocity
			//float speed = VectorLength(velocity);
			//gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);

			// Limit max speed
			//if (speed < 750 && z_height < 155 && jump_height <= 965) // current_node_type == NODE_LADDER
			{
				// Set the velocity of the player to the calculated velocity
				VectorCopy(velocity, self->velocity);
				VectorCopy(velocity, self->client->oldvelocity);
				VectorCopy(velocity, self->avelocity);
				
				//return;
			}
		}

		// Guide the bot in when close to the target
		else if (next_node_distance <= 128) // || (next_node_distance < 96 && (nodes[self->next_node].type == NODE_LADDER_UP || nodes[self->next_node].type == NODE_LADDER_DOWN)))
		{
			tr = gi.trace(self->s.origin, tv(-14, -14, -22), tv(14, 14, 2), nodes[self->next_node].origin, g_edicts, MASK_PLAYERSOLID);
			if (tr.fraction > 0.9)
			{
				VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				next_node_distance = VectorLength(dist);	// Get the absolute length

				// Apply it
				if (next_node_distance > 48)
					VectorScale(dist, SPEED_RUN, self->velocity);
				else if (next_node_distance > 24)
					VectorScale(dist, 350, self->velocity);
				else
					VectorScale(dist, 300, self->velocity);
			}
		}

		// Bot fell below current and next target
		if (self->s.origin[2] + NODE_Z_HEIGHT * 2 < nodes[self->current_node].origin[2])
		{
			self->current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
			if (self->current_node == INVALID)
			{
				self->state = STATE_WANDER;
				self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
				return;
			}
			{
				if (debug_mode) Com_Printf("%s %s failed to jumppad up, trying alternative path to goal\n", __func__, self->client->pers.netname);
				BOTLIB_SetGoal(self, self->goal_node);
			}
		}



		return;



		


		// Bot will jump to the jump pad
		//trace_t tr;
		//VectorCopy(nodes[self->next_node].origin, tr.endpos);
		//tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->next_node].origin, self, MASK_SOLID | MASK_OPAQUE);	//AQ2 added MASK_SOLID
		//if (self->groundentity || current_node_type == NODE_LADDER && tr.fraction == 1.0)

		if (self->groundentity || current_node_type == NODE_LADDER)
		{
			// Calculate the distance to the target
			float gravity = self->gravity * sv_gravity->value;
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			distance = VectorLength(dist);

			//if (self->s.origin[2] <= nodes[self->next_node].origin[2])
			//	distance *= 1.5; // Increases the height of the parabola (jump height) - Default is 1.0

			//distance += ent->viewheight;
			// Calculate the jump height to get to the target
			float jump_height = sqrt(2 * gravity * distance);
			// Calculate the time it will take to get to the target
			float time = distance / (self->velocity[2] + jump_height / 2.0);
			// Calculate the velocity at the end of the jump
			VectorScale(dist, 1.0 / time, velocity);
			velocity[2] = (jump_height / 2.0);

			/*
			// Get the z height distance between origin and target
			vec3_t zdiff, oz, tz;
			// Copy origin to oz
			VectorCopy(self->s.origin, oz);
			// Copy target to tz
			VectorCopy(nodes[self->next_node].origin, tz);
			// Set X and Y on oz and tz to zero
			oz[0] = 0; oz[1] = 0;
			tz[0] = 0; tz[1] = 0;
			// Get the z height distance between oz and tz
			VectorSubtract(oz, tz, zdiff);
			// Add the difference to jump velocity
			float z_height = 0;
			if (self->s.origin[2] <= nodes[self->next_node].origin[2])
			{
				z_height = VectorLength(zdiff) + 24;// + 56;
				velocity[2] += z_height;
			}
			else
			{
				velocity[2] -= VectorLength(zdiff);
			}
			*/

			// Calculate speed from velocity
			float speed = VectorLength(velocity);
			//gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);
			// Limit max speed
			//if (speed < 750 && z_height < 155 && jump_height <= 965) // current_node_type == NODE_LADDER
			{
				// Set the velocity of the player
				VectorCopy(velocity, self->velocity);
				// Don't take falling damage immediately from this
				if (self->client)
					VectorCopy(self->velocity, self->client->oldvelocity);
				return;
			}
		}

		ucmd->forwardmove = SPEED_RUN;
		
		// Cannot reach target, pick new goal
		//self->state = STATE_WANDER;
		return;




		// make sure it is visible
		//trace_t tr;
		//vec3_t maxs = { 1,1,1 };
		//vec3_t mins = { 1,1,1 };
		//tr = gi.trace(self->s.origin, mins, maxs, nodes[self->next_node].origin, self, MASK_SOLID | MASK_OPAQUE);	//AQ2 added MASK_SOLID
		//if (tr.fraction < 0.9)
		//	return;






		// Distance between self and current node
		//vec3_t dist_curr;
		//VectorSubtract(self->s.origin, nodes[self->current_node].origin, dist);
		//VectorCopy(dist, dist_curr);
		//float distance_to_curr_node = VectorLength(dist_curr);	// Get the absolute length
		//qboolean curr_node_is_above = false;
		//if (dist_curr[2] >= 0)
		//	curr_node_is_above = true;


		//vec3_t dist_next;
		// Distance between current node and next node
		//VectorSubtract(nodes[self->current_node].origin, nodes[self->next_node].origin, dist_next);
		//float distance_to_next_node = VectorLength(dist_next);	// Get the absolute length
		//qboolean next_node_is_above = false;
		//if (dist_next[2] >= 0)
		//	next_node_is_above = true;

		//if (distance_to_curr_node > 128)
		//	return;


		if (self->groundentity)
		{



			ucmd->forwardmove = SPEED_RUN;

			/*
			// Jump only when we are moving the correct direction.
			vec3_t dist_other;
			VectorCopy(self->velocity, velocity);
			VectorCopy(dist, dist_other);
			velocity[2] = 0;
			VectorNormalize(velocity);
			VectorNormalize(dist_other);
			if (DotProduct(velocity, dist_other) < 0.8)
				goto NO_JUMPPAD;
			*/


			ucmd->upmove = SPEED_RUN;



			self->last_door_time = level.framenum + 0.25 * HZ; // wait!

			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			//dist[2] = 0;	// Lose the vertical component
			distance = VectorLength(dist);	// Get the absolute length

			//float multi = 1;
			//if (distance > 64)
			//	multi = (distance / 128);
			//multi = (SPEED_RUN * multi) + 56;

			//vec3_t	dir, angle;
			//AngleVectors(angle, dir, NULL, NULL);
			//VectorScale(dir, multi, self->velocity);

			float multi = 1;
			//if (distance > 64)
			//multi = (distance / 80); // 160
			//multi = (SPEED_RUN * multi);
			multi = distance;
			vec3_t vDir;
			//VectorCopy(bot->s.origin, vStart);
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, vDir);
			VectorNormalize(vDir);
			VectorScale(vDir, multi, self->velocity);

			//debug_printf("NODE_JUMPPAD Velocity UP: %f\n", self->velocity[2]);

			//*
			// Clip velocity
			float minVel = -401;
			float maxVel = 401;
			if (self->velocity[0] < minVel)
				self->velocity[0] = minVel;
			else if (self->velocity[0] > maxVel)
				self->velocity[0] = maxVel;

			if (self->velocity[1] < minVel)
				self->velocity[1] = minVel;
			else if (self->velocity[1] > maxVel)
				self->velocity[1] = maxVel;

			if (self->velocity[2] < 0)
				self->velocity[2] = 0;
			else if (self->velocity[2] > maxVel)
				self->velocity[2] = maxVel;
			//*/

			/*
			if (0)
			{
				if (distance_to_curr_node < 24)// && (DotProduct(velocity, dist_other) > 0.8)) // close and facing right dir
				{
					//if (dist[2] < -32) // Going down
					{
						//self->velocity[2] = 0;
						//debug_printf("NODE_JUMPPAD Velocity DOWN: %f\n", self->velocity[2]);
					}

					if (dist[2] > STEPSIZE || next_node_type == NODE_JUMPPAD) // Going up
					{
						//VectorScale(self->s.angles, multi, self->velocity);
						//self->velocity[2] = min(400, self->velocity[2]);

						self->velocity[2] = multi;
						//self->velocity[2] = min(SPEED_RUN, self->velocity[2]);
						debug_printf("NODE_JUMPPAD Velocity UP: %f dist[2]: %f\n", self->velocity[2], dist[2]);
						//ucmd->upmove = SPEED_RUN;
					}
					else
					{
						self->velocity[2] = 0;
						debug_printf("NODE_JUMPPAD Velocity DOWN: %f\n", self->velocity[2]);
					}
				}
				else
					return;
			}
			*/


			//vec3_t      start, end;
			//vec3_t      forward, right;
			//vec3_t      offset;
			//AngleVectors(self->s.angles, forward, right, NULL);
			//VectorSet(offset, 0, 0, self->viewheight - 8);
			//VectorMA(self->s.origin, distance_to_next_node, forward, nodes[self->next_node].origin);
			//VectorScale(self->movedir, self->speed * 10, self->velocity);

			/*
			// Set up a jump move
			if (distance < 256)
				ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 256);
			else
				ucmd->forwardmove = SPEED_RUN;
			*/

			ACEMV_ChangeBotAngle(self);

			return;
		}

		//return;

		//if (self->speed < 1000)
		//	self->speed = 1000;
		//VectorScale(self->movedir, self->speed * 10, self->velocity);
		//return;

		//if (distance_to_next_node < 64 && curr_node_is_above && self->last_door_time < level.framenum)
		//if (distance_to_next_node < 64 && self->last_door_time < level.framenum)
		//if (distance_to_curr_node < 64 || self->velocity[2] < 0)
		//if (distance_to_curr_node < 128 || self->s.origin[2] > nodes[self->next_node].origin[2] || self->velocity[2] < 400)
		if ((self->velocity[2] <= -100 || self->velocity[2] >= 400))// && distance_to_curr_node > 32)
		{
			//self->last_door_time = level.framenum + 0.25 * HZ; // wait!

			// If not moving in midair
			if ((self->velocity[0] > -1 && self->velocity[0] < 1) && (self->velocity[1] > -1 && self->velocity[1] < 1))
				return;

			//debug_printf("NODE_JUMPPAD\n");

			//VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			//dist[2] = 0;	// Lose the vertical component
			distance = VectorLength(dist);	// Get the absolute length

			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			VectorNormalize(dist);
			distance = VectorLength(dist);	// Get the absolute length
			VectorScale(dist, SPEED_RUN * distance, self->velocity);
			if (dist[2] >= 0)
				self->velocity[2] = min(400, self->velocity[2]);
			else
				self->velocity[2] = max(-800, self->velocity[2]);

			/*
			// Jump only when we are moving the correct direction.
			VectorCopy(self->velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);
			VectorNormalize(dist);

			if (DotProduct(velocity, dist) > 0.8)
				ucmd->upmove = SPEED_RUN;

			// Set up a jump move
			if (distance < 128)
				ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 128);
			else
				ucmd->forwardmove = SPEED_RUN;

			self->move_vector[2] *= 2;
			*/

			ACEMV_ChangeBotAngle(self);

			//self->speed = 1000;
			//VectorScale(self->movedir, distance, self->velocity);

			//self->s.angles[YAW]
			//velocity = v_forward * dist + v_up * (dist);

			//VectorScale(dist, distance, self->velocity);


			/*
			vec3_t flydir = { 0.f,0.f,0.f }, kvel = { 0.f,0.f,0.f };
			float accel_scale = 100.f; // the rocket jump hack...
			VectorNormalize2(self->s.angles, flydir);
			flydir[2] += 0.4f;
			VectorScale(flydir, accel_scale, kvel);
			VectorAdd(self->velocity, kvel, self->velocity);
			*/

			/*
			// RiEvEr - re-instate the velocity hack
			if (self->jumphack_timeout < level.framenum)
			{
				VectorCopy(self->move_vector,dist);
				if ((440 * distance / 128) < 440)
					VectorScale(dist,(440 * distance / 128),self->velocity);
				else
					VectorScale(dist,440,self->velocity);
				self->jumphack_timeout = level.framenum + 3.0 * HZ;
			}
			*/
		}

		return;
	}
	//rekkie -- DEV_1 -- e

	////////////////////////////////////////////////////////
	// Jumpto Nodes
	///////////////////////////////////////////////////////
	if(next_node_type == NODE_JUMP)
//		||
//			( current_node_type == NODE_JUMP 
//			&& next_node_type != NODE_ITEM && nodes[self->next_node].origin[2] > self->s.origin[2]+16)
//	  )
	{
		VectorSubtract( nodes[self->next_node].origin, self->s.origin, dist);
		// Lose the vertical component
		dist[2] = 0;
		// Get the absolute length
		distance = VectorLength(dist);
	
		//if (ACEMV_CanJumpInternal(self, MOVE_FORWARD))
		{
			// Jump only when we are moving the correct direction.
			VectorCopy( self->velocity, velocity );
			velocity[2] = 0;
			VectorNormalize( velocity );
			VectorNormalize( dist );
			if( DotProduct( velocity, dist ) > 0.8 )
				ucmd->upmove = SPEED_RUN;

			//Kill running movement
//			self->move_vector[0]=0;
//			self->move_vector[1]=0;
//			self->move_vector[2]=0;
			// Set up a jump move
			if( distance < 256 )
				ucmd->forwardmove = SPEED_RUN * sqrtf( distance / 256 );
			else
				ucmd->forwardmove = SPEED_RUN;

			//self->move_vector[2] *= 2;

			ACEMV_ChangeBotAngle(self);

			//rekkie -- DEV_1 -- s
			// Guide the bot in when close to the target
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			distance = VectorLength(dist);
			if (distance <= 64)
			{
				tr = gi.trace(self->s.origin, tv(-14, -14, -22), tv(14, 14, 2), nodes[self->next_node].origin, g_edicts, MASK_PLAYERSOLID);
				if (tr.fraction > 0.7)
				{
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 200, self->velocity); // Apply it
				}
			}
			//rekkie -- DEV_1 -- e

			/*		
			// RiEvEr - re-instate the velocity hack
			if (self->jumphack_timeout < level.framenum)
			{
				VectorCopy(self->move_vector,dist);
				if ((440 * distance / 128) < 440)
					VectorScale(dist,(440 * distance / 128),self->velocity);
				else
					VectorScale(dist,440,self->velocity);
				self->jumphack_timeout = level.framenum + 3.0 * HZ;
			}
			*/
		}
		/*
		else
		{
				self->goal_node = INVALID;
		}
		*/
		return;
	}
	
	////////////////////////////////////////////////////////
	// Ladder Nodes
	///////////////////////////////////////////////////////

	//rekkie -- DEV_1 -- s
	if (current_node_type == NODE_LADDER_UP || current_node_type == NODE_LADDER_DOWN ||
		next_node_type == NODE_LADDER_UP || next_node_type == NODE_LADDER_DOWN)
	{
		VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist); // Find the distance vector to the next node
		distance = VectorLength(dist);

		VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist_xy); // Find the distance vector to the next node
		dist_xy[2] = 0; // Lose the vertical component
		distance_xy = VectorLength(dist_xy); // Get the absolute length
		
		float speed = VectorLength(self->velocity); // Speed of the player

		// If we are going down the ladder, check for teammates coming up and yield to them.
		trace_t	tr;
		tr = gi.trace(self->s.origin, tv(-16, -16, -8), tv(16, 16, 8), nodes[self->next_node].origin, self, MASK_PLAYERSOLID);
		if ((tr.fraction < 1.0) && tr.ent && (tr.ent != self) && tr.ent->client && (tr.ent->velocity[2] >= 0))
		{
			self->velocity[0] = 0;
			self->velocity[1] = 0;
			self->velocity[2] = 200;
			self->state = STATE_WANDER;
			return;
		}
		
		// If getting off the top of a ladder
		if (current_node_type == NODE_LADDER_UP && next_node_type != NODE_LADDER_DOWN)
		{
			ucmd->forwardmove = SPEED_WALK;

			if ((nodes[self->next_node].origin[2] >= self->s.origin[2])
				&& (nodes[self->next_node].origin[2] >= nodes[self->current_node].origin[2])
				&& (((self->velocity[2] > 0) && !self->groundentity) || OnLadder(self)))
			{
				ucmd->upmove = SPEED_RUN;
				self->velocity[2] = min(200, distance_xy * 10); // Jump higher for farther node
			}

			if (1)
			{
				// Guide the bot in when close to the target
				if (self->s.origin[2] <= nodes[self->next_node].origin[2] + 16 && distance < 64)
				{
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 400, self->velocity); // Apply it
				}
			}

			ACEMV_ChangeBotAngle(self);
			return;
		}


		// If getting off the bottom of a ladder
		if (current_node_type == NODE_LADDER_DOWN && next_node_type != NODE_LADDER_UP)
		{
			// Do nothing here, this is now handled by the next node type
		}

		// Getting onto bottom of the ladder
		if ( current_node_type != NODE_LADDER_UP && next_node_type == NODE_LADDER_DOWN && distance < 192 && nodes[self->next_node].origin[2] > self->s.origin[2])
		{
			// Jump only when we are moving the correct direction.
			VectorCopy(self->velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);
			VectorNormalize(dist_xy);
			float dot = DotProduct(velocity, dist_xy);
			if (dot < 0.8 && dot != 0) // Make bot face ladder
			{
				ACEMV_ChangeBotAngle(self);
				//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
			}
			else // Correct direction, jump up
			{
				ucmd->upmove = SPEED_RUN;
				ucmd->forwardmove = SPEED_WALK;
				//Com_Printf("%s Excellent. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
			}
			

			if (1)
			{
				// Guide the bot in when close to the target
				if (self->s.origin[2] < nodes[self->next_node].origin[2] && distance < 64)
				{
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 200, self->velocity); // Apply it
				}
			}


			if (OnLadder(self) == false)
				return;
		}

		// Getting onto top of the ladder
		if (current_node_type != NODE_LADDER_DOWN && next_node_type == NODE_LADDER_UP)// && self->groundentity)
		{
			// Jump only when we are moving the correct direction.
			VectorCopy(self->velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);
			VectorNormalize(dist_xy);
			float dot = DotProduct(velocity, dist_xy);
			if (dot < 0.8 && dot != 0) // Make bot face ladder
			{
				ACEMV_ChangeBotAngle(self);
				//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
			}
			else // Correct direction, jump up
			{
				if (distance > 64)
					ucmd->forwardmove = SPEED_WALK;
				else if (distance > 48)
					ucmd->forwardmove = 50;
				else if (distance > 24)
					ucmd->forwardmove = 25;
			}

			// Guide the bot in when close to the target
			//VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			//distance = VectorLength(dist);
			if (self->s.origin[2] <= nodes[self->next_node].origin[2] && distance <= 128)
			{
				VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				distance = VectorLength(dist);	// Get the absolute length
				VectorScale(dist, 100, self->velocity); // Apply it
			}
			
			//
			//if (OnLadder(self) == false)
			return;
		}

		// On ladder going up
		if (current_node_type == NODE_LADDER_DOWN && next_node_type == NODE_LADDER_UP)
		{
			if (nodes[self->next_node].origin[2] + NODE_Z_HEIGHT >= self->s.origin[2] && distance_xy < 64)
			{
				VectorCopy(self->velocity, velocity);
				velocity[2] = 0;
				VectorNormalize(velocity);
				VectorNormalize(dist_xy);
				float dot = DotProduct(velocity, dist_xy);
				if (dot < 0.8 && dot != 0) // Make bot face ladder
				{
					ACEMV_ChangeBotAngle(self);
					//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
				}

				if (!self->groundentity)
				{
					// FIXME: Dirty hack so the bots can actually use ladders.
					//vec3_t origin_up;
					//VectorCopy(self->s.origin, origin_up);
					//origin_up[2] += 8;
					//VectorSubtract(origin_up, self->s.origin, dist_xy);
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist_xy);
					VectorNormalize(dist_xy);
					VectorScale(dist_xy, SPEED_RUN, self->velocity);
					if (dist_xy[2] > 0)
						self->velocity[2] = min(200, self->velocity[2]);
				}

				// Guide the bot in when close to the target
				// Useful for the tall ladder found on murder.bsp - helps to deal with ladders that use playerclip
				if (self->s.origin[2] <= nodes[self->next_node].origin[2] + 16 && distance < 64)
				{
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 400, self->velocity); // Apply it
				}
				
				return;
			}
		}

		// If ladder going down
		if (current_node_type == NODE_LADDER_UP && next_node_type == NODE_LADDER_DOWN)
		{
			if (self->s.origin[2] >= nodes[self->next_node].origin[2] && distance_xy < 64) //&& speed <= SPEED_ROAM)
			{				
				ucmd->forwardmove = SPEED_WALK / 2;
				if ((distance_xy < 200) && (nodes[self->next_node].origin[2] <= self->s.origin[2] + 16))
					ucmd->upmove = -SPEED_RUN; //Added by Werewolf to cause crouching

				if (!self->groundentity)
				{
					VectorCopy(self->velocity, velocity);
					velocity[2] = 0;
					VectorNormalize(velocity);
					VectorNormalize(dist_xy);
					float dot = DotProduct(velocity, dist_xy);
					if (dot < 0.8 && dot != 0) // Make bot face ladder
					{
						//ACEMV_ChangeBotAngle(self);
						//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
					}
					
					// FIXME: Dirty hack so the bots can actually use ladders.
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist_xy);
					VectorNormalize(dist_xy);
					VectorScale(dist_xy, SPEED_RUN, self->velocity);
					if (dist_xy[2] < 0)
						self->velocity[2] = max(-200, self->velocity[2]);
				}
				return;
			}
		}
	}

		/*
		// If getting off the top of a ladder
		if ((current_node_type == NODE_LADDER_UP) && (next_node_type != NODE_LADDER_DOWN))
		{
			ucmd->forwardmove = SPEED_WALK;

			if ((nodes[self->next_node].origin[2] >= self->s.origin[2])
				&& (nodes[self->next_node].origin[2] >= nodes[self->current_node].origin[2])
				&& (((self->velocity[2] > 0) && !self->groundentity) || OnLadder(self)))
			{
				ucmd->upmove = SPEED_RUN;
				self->velocity[2] = min(200, distance_xy * 10); // Jump higher for farther node
			}

			ACEMV_ChangeBotAngle(self);
			return;
		}

		// If getting onto the ladder going up
		if ((next_node_type == NODE_LADDER_DOWN) && (distance_xy < 128) && (nodes[self->next_node].origin[2] >= self->s.origin[2]))
		{
			// Jump only when we are moving the correct direction.
			VectorCopy(self->velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);
			VectorNormalize(dist_xy);
			if (DotProduct(velocity, dist_xy) > 0.8)
				ucmd->upmove = SPEED_RUN;

			ucmd->forwardmove = SPEED_WALK / 2;

			ACEMV_ChangeBotAngle(self);
			return;
		}

		// On ladder going up
		if (current_node_type == NODE_LADDER_DOWN && next_node_type == NODE_LADDER_UP)
		{
			if (nodes[self->next_node].origin[2] + NODE_Z_HEIGHT >= self->s.origin[2] && distance_xy < 64)// && speed <= SPEED_ROAM)
			{
				// Jump only when we are moving the correct direction.
				VectorCopy(self->velocity, velocity);
				velocity[2] = 0;
				VectorNormalize(velocity);
				VectorNormalize(dist_xy);
				if (DotProduct(velocity, dist_xy) > 0.8)
					ucmd->upmove = SPEED_RUN;
				ucmd->forwardmove = SPEED_WALK / 2;

				if (!self->groundentity)
				{
					// FIXME: Dirty hack so the bots can actually use ladders.
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist_xy);
					VectorNormalize(dist_xy);
					VectorScale(dist_xy, SPEED_RUN, self->velocity);
					if (dist_xy[2] > 0)
						self->velocity[2] = min(200, self->velocity[2]);
				}
				else
					ACEMV_ChangeBotAngle(self);
				
				return;
			}
		}

		// If ladder going down
		if (current_node_type == NODE_LADDER_UP && next_node_type == NODE_LADDER_DOWN)
		{
			if (self->s.origin[2] >= nodes[self->next_node].origin[2] && distance_xy < 64) //&& speed <= SPEED_ROAM)
			{
				ucmd->forwardmove = SPEED_WALK / 2;
				if ((distance_xy < 200) && (nodes[self->next_node].origin[2] <= self->s.origin[2] + 16))
					ucmd->upmove = -SPEED_RUN; //Added by Werewolf to cause crouching

				if (!self->groundentity)
				{
					// FIXME: Dirty hack so the bots can actually use ladders.
					VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist_xy);
					VectorNormalize(dist_xy);
					VectorScale(dist_xy, SPEED_RUN, self->velocity);
					if (dist_xy[2] < 0)
						self->velocity[2] = max(-200, self->velocity[2]);
				}
				
				//ACEMV_ChangeBotAngle(self);
				return;
			}
		}
		
		// Slow down when near ladder
		if (current_node_type != NODE_LADDER_UP && current_node_type != NODE_LADDER_DOWN)
		{			
			VectorNormalize(dist_xy);
			if (distance_xy > 64)
				VectorScale(dist_xy, SPEED_RUN, self->velocity);
			else if (distance_xy > 48)
				VectorScale(dist_xy, 350, self->velocity);
			else if (distance_xy > 24)
				VectorScale(dist_xy, 300, self->velocity);
			else
				VectorScale(dist_xy, 250, self->velocity);
			ACEMV_ChangeBotAngle(self);
		}

		return;
	}
	*/
	//rekkie -- DEV_1 -- e

	// Find the distance vector to the next node
	VectorSubtract( nodes[self->next_node].origin, self->s.origin, dist);
	// Lose the vertical component
	dist[2] = 0;
	// Get the absolute length
	distance = VectorLength(dist);

	// If on the ladder
	if( (next_node_type == NODE_LADDER) && (distance < 64)
	&&  ((nodes[self->next_node].origin[2] > self->s.origin[2]) || ! self->groundentity) )
	{
		// FIXME: Dirty hack so the bots can actually use ladders.
		VectorSubtract( nodes[self->next_node].origin, self->s.origin, dist );
		VectorNormalize( dist );
		VectorScale( dist, SPEED_RUN, self->velocity );
		if( dist[2] >= 0 )
			self->velocity[2] = min( 200, self->velocity[2] );
		else
			self->velocity[2] = max( -200, self->velocity[2] );
		
		if( self->velocity[2] < 0 )
		{
			// If we are going down the ladder, check for teammates coming up and yield to them.
			trace_t	tr;
			tr = gi.trace( self->s.origin, tv(-16,-16,-8), tv(16,16,8), nodes[self->next_node].origin, self, MASK_PLAYERSOLID );
			if( (tr.fraction < 1.0) && tr.ent && (tr.ent != self) && tr.ent->client && (tr.ent->velocity[2] >= 0) )
			{
				self->velocity[0] = 0;
				self->velocity[1] = 0;
				self->velocity[2] = 200;
			}
		}

		ACEMV_ChangeBotAngle(self);
		return;
	}
	
	// If getting onto the ladder going up
	if( (next_node_type == NODE_LADDER) && (distance < 200)
	&&  (nodes[self->next_node].origin[2] >= self->s.origin[2]) )
	{
		// Jump only when we are moving the correct direction.
		VectorCopy( self->velocity, velocity );
		velocity[2] = 0;
		VectorNormalize( velocity );
		VectorNormalize( dist );
		if( DotProduct( velocity, dist ) > 0.8 )
			ucmd->upmove = SPEED_RUN;

		ucmd->forwardmove = SPEED_WALK / 2;
		
		ACEMV_ChangeBotAngle(self);
		return;
	}
	
	// If getting off the ladder
	if( (current_node_type == NODE_LADDER) && (next_node_type != NODE_LADDER) )
	{
		ucmd->forwardmove = SPEED_WALK;

		if( (nodes[self->next_node].origin[2] >= self->s.origin[2])
		&&  (nodes[self->next_node].origin[2] >= nodes[self->current_node].origin[2])
		&&  ( ((self->velocity[2] > 0) && ! self->groundentity) || OnLadder(self) ) )
		{
			ucmd->upmove = SPEED_RUN;
			self->velocity[2] = min( 200, distance * 10 ); // Jump higher for farther node
		}
		
		ACEMV_ChangeBotAngle(self);
		return;
	}
	
	// If getting onto the ladder going down
	if( (current_node_type != NODE_LADDER) && (next_node_type == NODE_LADDER) )
	{
		ucmd->forwardmove = SPEED_WALK / 2;

		if( (distance < 200) && (nodes[self->next_node].origin[2] <= self->s.origin[2] + 16) )
			ucmd->upmove = -SPEED_RUN; //Added by Werewolf to cause crouching
		
		ACEMV_ChangeBotAngle(self);
		return;
	}

/*	//==============================
	// LEDGES etc..
	// If trying to jump up a ledge
	if(
	   (current_node_type == NODE_MOVE &&
	   nodes[self->next_node].origin[2] > self->s.origin[2]+16 && distance < NODE_DENSITY)
	   )
	{
		ucmd->forwardmove = SPEED_RUN;
		ucmd->upmove = SPEED_RUN;
		ACEMV_ChangeBotAngle(self);
		return;
	}*/
	////////////////////////////////////////////////////////
	// Water Nodes
	///////////////////////////////////////////////////////
	if(current_node_type == NODE_WATER)
	{
		// We need to be pointed up/down
		ACEMV_ChangeBotAngle(self);

		// If the next node is not in the water, then move up to get out.
		if(next_node_type != NODE_WATER && !(gi.pointcontents(nodes[self->next_node].origin) & MASK_WATER)) // Exit water
			ucmd->upmove = SPEED_RUN;
		
		ucmd->forwardmove = SPEED_RUN * 3 / 4;
		return;

	}
	
	// Falling off ledge?
/*	if(!self->groundentity)
	{
		ACEMV_ChangeBotAngle(self);

		self->velocity[0] = self->move_vector[0] * 360;
		self->velocity[1] = self->move_vector[1] * 360;
	
		return;
	}*/
	
	/*
	// Check to see if stuck, and if so try to free us
	// Also handles crouching
	VectorSubtract( self->s.origin, self->lastPosition, dist );
	if( (VectorLength(self->velocity) < 37) || (VectorLength(dist) < FRAMETIME) )
	{
		// Keep a random factor just in case....
		if(random() > 0.5 && ACEMV_SpecialMove(self, ucmd))
			return;
		
		if( self->groundentity )
		{
			// Check if we are obstructed by an oncoming teammate, and if so strafe right.
			trace_t	tr;
			tr = gi.trace( self->s.origin, tv(-16,-16,-8), tv(16,16,8), nodes[self->next_node].origin, self, MASK_PLAYERSOLID );
			if( (tr.fraction < 1.0) && tr.ent && (tr.ent != self) && tr.ent->client
			&&  (DotProduct( self->velocity, tr.ent->velocity ) <= 0) && ACEMV_CanMove( self, MOVE_RIGHT ) )
			{
				ucmd->sidemove = SPEED_RUN;
				ACEMV_ChangeBotAngle(self);
				return;
			}
		}

		self->s.angles[YAW] += random() * 180 - 90; 

		ACEMV_ChangeBotAngle(self);

		ucmd->forwardmove = SPEED_RUN;

		// Raptor007: Only jump when trying to gain altitude, not down ledges.
		if( nodes[self->next_node].origin[2] > self->s.origin[2] + 16 )
			ucmd->upmove = SPEED_RUN;
		
		return;
	}
	*/

	//rekkie -- DEV_1 -- s
	////////////////////////////////////////////////////////
	// NODE_STAND_DROP Nodes
	if (current_node_type == NODE_STAND_DROP || current_node_type == NODE_CROUCH_DROP || current_node_type == NODE_UNSAFE_DROP)
	{
		// If the bot fell below the target (failed the jump down)
		// Try to find another route, otherwise find a new node and go there instead
		if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->next_node].origin[2])
		{
			self->current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
			if (self->current_node == INVALID)
			{
				self->state = STATE_WANDER;
				self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
				return;
			}
			{
				Com_Printf("%s %s failed to drop down, trying alternative path to goal\n", __func__, self->client->pers.netname);
				BOTLIB_SetGoal(self, self->goal_node);
			}
			else
			{
				Com_Printf("%s %s failed to drop down, finding new goal\n", __func__, self->client->pers.netname);
				self->state = STATE_WANDER; // Aquire a new node
			}
			return;
		}

		ACEMV_ChangeBotAngle(self);

		VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
		dist[2] = 0;
		distance = VectorLength(dist);

		if (!self->groundentity)
		{
			// When in air, control speed carefully to avoid overshooting the next node.
			if (distance < 256)
				ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 256);
			else
				ucmd->forwardmove = SPEED_RUN;
		}
		else if (!ACEMV_CanMove(self, MOVE_FORWARD))
		{
			// If we reached a ledge, slow down.
			if (distance < 512)
			{
				// Jump unless dropping down to the next node.
				if (nodes[self->next_node].origin[2] > (self->s.origin[2] + 7) && distance < 500)
					ucmd->upmove = SPEED_RUN;

				ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 512);
			}
			else
				ucmd->forwardmove = SPEED_WALK;
		}
		else // Otherwise move as fast as we can.
			ucmd->forwardmove = SPEED_RUN;

		// Guide the bot in when close to the target
		if (distance <= 128) //NODE_MAX_JUMP_HEIGHT)
		{
			//if (current_node_type == NODE_STAND_DROP) Com_Printf("%s %s: NODE_STAND_DROP\n", __func__, self->client->pers.netname);
			//if (current_node_type == NODE_CROUCH_DROP) Com_Printf("%s %s: NODE_CROUCH_DROP\n", __func__, self->client->pers.netname);
			//if (current_node_type == NODE_UNSAFE_DROP) Com_Printf("%s %s: NODE_UNSAFE_DROP\n", __func__, self->client->pers.netname);
			
			VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
			VectorNormalize(dist);
			distance = VectorLength(dist);	// Get the absolute length
			VectorScale(dist, 200, self->velocity); // Apply it
		}
	}
	///////////////////////////////////////////////////////
	//rekkie -- DEV_1 -- e

	////////////////////////////////////////////////////////
	// Move Nodes
	///////////////////////////////////////////////////////

	//rekkie -- DEV_1 -- s
	// If the bot fell below the target (failed the jump down)
	// Try to find another route, otherwise find a new node and go there instead
	if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->next_node].origin[2])
	{
		// Upgrade target node type to jumppad
		if (targetNodeLink != INVALID)
		{
			//if (debug_mode) Com_Printf("%s %s dropped from a move forward, upgrading move to jumppad node\n", __func__, self->client->pers.netname);
			//nodes[self->current_node].links[targetNodeLink].targetNodeType = NODE_JUMPPAD;
			self->current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
			return;
		}
		else
		{
			if (debug_mode) Com_Printf("%s %s failed to move forward, finding new goal\n", __func__, self->client->pers.netname);
			self->state = STATE_WANDER; // Aquire a new node
		}
	}
	//rekkie -- DEV_1 -- e

	ACEMV_ChangeBotAngle(self);

	VectorSubtract( nodes[self->next_node].origin, self->s.origin, dist );
	dist[2] = 0;
	distance = VectorLength(dist);

	if( ! self->groundentity )
	{
		// When in air, control speed carefully to avoid overshooting the next node.
		if( distance < 256 )
			ucmd->forwardmove = SPEED_RUN * sqrtf( distance / 256 );
		else
			ucmd->forwardmove = SPEED_RUN;

		//*
		//rekkie -- DEV_1 -- s
		// Guide the bot in when close to the target
		VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
		distance = VectorLength(dist);
		if (distance <= 64)
		{
			tr = gi.trace(self->s.origin, tv(-14, -14, -22), tv(14, 14, 2), nodes[self->next_node].origin, g_edicts, MASK_PLAYERSOLID);
			if (tr.fraction > 0.9)
			{
				VectorSubtract(nodes[self->next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				distance = VectorLength(dist);	// Get the absolute length
				VectorScale(dist, 200, self->velocity); // Apply it
			}
		}
		//rekkie -- DEV_1 -- e
		//*/
	}
	//rekkie -- DEV_1 -- s
	else
	{
		if (distance < 256)
			ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 256);
		else
			ucmd->forwardmove = SPEED_RUN;
	}
	//rekkie -- DEV_1 -- e
	/*
	else if (!ACEMV_CanMove(self, MOVE_FORWARD))
	{
		// If we reached a ledge, slow down.
		if( distance < 512 )
		{
			// Jump unless dropping down to the next node.
			if( (nodes[self->next_node].origin[2] > self->s.origin[2] + 7) && (distance < 500) )
				ucmd->upmove = SPEED_RUN;

			ucmd->forwardmove = SPEED_WALK * sqrtf( distance / 512 );
		}
		else
			ucmd->forwardmove = SPEED_WALK;
	}
	else
		// Otherwise move as fast as we can.
		ucmd->forwardmove = SPEED_RUN;
	*/
}
#endif

/*
///////////////////////////////////////////////////////////////////////
// Wandering code (based on old ACE movement code) 
///////////////////////////////////////////////////////////////////////
//
// RiEvEr - this routine is of a very poor standard and has a LOT of problems
// Maybe replace this with the DroneBot or ReDeMpTiOn wander code?
//
void ACEMV_Wander(edict_t *self, usercmd_t *ucmd)
{
	vec3_t  temp;
	int content_head = 0, content_feet = 0;
	float moved = 0;
	
	// Do not move
	if(self->next_move_time > level.framenum)
		return;

	// Special check for elevators, stand still until the ride comes to a complete stop.
	if( self->groundentity && (VectorLength(self->groundentity->velocity) >= 8) )
		if(self->groundentity->moveinfo.state == STATE_UP ||
		   self->groundentity->moveinfo.state == STATE_DOWN) // only move when platform not
		{
			self->velocity[0] = 0;
			self->velocity[1] = 0;
			self->velocity[2] = 0;
			self->next_move_time = level.framenum + 0.5 * HZ;
			return;
		}
	
	
	// Is there a target to move to
	//if (self->movetarget)
	//	ACEMV_MoveToGoal(self,ucmd);
		
	VectorSubtract( self->s.origin, self->lastPosition, temp );
	moved = VectorLength(temp);

	////////////////////////////////
	// Ladder?
	////////////////////////////////
	// To avoid unnecessary falls, don't use ladders when on solid ground.
	if( (! self->groundentity) && OnLadder(self) )
	{
		ucmd->upmove = SPEED_RUN;
		ucmd->forwardmove = 0;
		ucmd->sidemove = 0;
		self->s.angles[PITCH] = -15;

		// FIXME: Dirty hack so the bots can actually use ladders.
		self->velocity[0] = 0;
		self->velocity[1] = 0;
		self->velocity[2] = 200;

		if( moved >= FRAMETIME )
			return;
	}

	////////////////////////////////
	// Swimming?
	////////////////////////////////
	VectorCopy(self->s.origin,temp);
	temp[2] += 22;
	content_head = gi.pointcontents(temp);
	temp[2] = self->s.origin[2] - 8;
	content_feet = gi.pointcontents(temp);

	// Just try to keep our head above water.
	if( content_head & MASK_WATER )
	{
		// If drowning and no node, move up
		if(self->client->next_drown_framenum > 0)
		{
			ucmd->upmove = SPEED_RUN;
			self->s.angles[PITCH] = -45;
		}
		else
			ucmd->upmove = SPEED_WALK;
	}

	// Don't wade in lava, try to get out!
	if( content_feet & (CONTENTS_LAVA|CONTENTS_SLIME) )
		ucmd->upmove = SPEED_RUN;

	// See if we're jumping out of the water.
	if( ! self->groundentity
	&& (content_feet & MASK_WATER)
	&& ACEMV_SpecialMove( self, ucmd ) )
	{
		if( ucmd->upmove > 0 )
			self->velocity[2] = 270;  // FIXME: Is there a cleaner way?
		if( moved >= FRAMETIME )
			return;
	}

//@@	if(ACEMV_CheckEyes(self,ucmd))
//		return;
	
	// Check for special movement if we have a normal move (have to test)
	if( (VectorLength(self->velocity) < 37) || (moved < FRAMETIME) )
	{
		if(random() > 0.1 && ACEMV_SpecialMove(self,ucmd))
			return;

		self->s.angles[YAW] += random() * 180 - 90; 
		self->s.angles[PITCH] = 0;

		if( content_feet & MASK_WATER )  // Just keep swimming.
			ucmd->forwardmove = SPEED_RUN;
		else if(!M_CheckBottom(self) && !self->groundentity) // if there is ground continue otherwise wait for next move
			ucmd->forwardmove = 0;
		else if( ACEMV_CanMove( self, MOVE_FORWARD))
			ucmd->forwardmove = SPEED_WALK;
		
		return;
	}
	
	// Otherwise move as fast as we can
	// If it's safe to move forward (I can't believe ACE didn't check this!
	if( ACEMV_CanMove( self, MOVE_FORWARD )
	|| (content_feet & MASK_WATER) )
	{
		ucmd->forwardmove = SPEED_RUN;
	}
	else
	{
		// Need a "findbestdirection" routine in here
		// Forget about this route!
		ucmd->forwardmove = -SPEED_WALK / 2;
		self->movetarget = NULL;
	}	
}
*/

//rekkie -- DEV_1 -- s
///////////////////////////////////////////////////////////////////////
// Choose the sniper zoom when allowed
///////////////////////////////////////////////////////////////////////
/*
//void	_SetSniper(edict_t* ent, int zoom);
void ACEAI_SetSniper( edict_t *self, int zoom )
{
	if( (self->client->weaponstate != WEAPON_FIRING)
	&&  (self->client->weaponstate != WEAPON_BUSY)
	&& ! self->client->bandaging
	&& ! self->client->bandage_stopped )
		_SetSniper( self, zoom );
}
*/
/*
// Check to make sure we're using a primary weapon where possible
void BOTLIB_CheckCurrentWeapon(edict_t* self)
{
	if (ACEIT_ChangeSniperSpecialWeapon(self, FindItem(SNIPER_NAME)))
	{
		ACEAI_SetSniper(self, 2); // scope in
		return;
	}
	if (ACEIT_ChangeM4SpecialWeapon(self, FindItem(M4_NAME)))
		return;
	if (ACEIT_ChangeMP5SpecialWeapon(self, FindItem(MP5_NAME)))
		return;
	if (ACEIT_ChangeM3SpecialWeapon(self, FindItem(M3_NAME)))
		return;
	if (ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
		return;
	if (ACEIT_ChangeDualSpecialWeapon(self, FindItem(DUAL_NAME)))
		return;
	//if (ACEIT_ChangeWeapon(self, FindItem(GRENADE_NAME)))
	//	return;
	//if (ACEIT_ChangeMK23SpecialWeapon(self, FindItem(MK23_NAME)))
	//	return;
	//if (ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
	//	return;
}
*/

// Locate an item within a set range
edict_t* BOTLIB_LocateRangedItem(edict_t* self, char* classname, float range)
{
	edict_t* item = NULL;

	// look for a target (should make more efficient later)
	item = findradius(NULL, self->s.origin, range);

	while (item)
	{
		if (item->classname == NULL) // end of items found
			return NULL;

		//Com_Printf("%s %s items found: %s  looking for: %s\n", __func__, self->client->pers.netname, item->classname, classname);

		if (Q_stricmp(item->classname, classname) == 0)
		{
			return item; // Found item within range
		}

		// next item
		item = findradius(item, self->s.origin, range);
	}

	return NULL; // found nothing
}

// Locate and pickup a primary weapon if we need one - s
void BOTLIB_GetWeaponsAndAmmo(edict_t* self)
{
	edict_t *item = NULL;

	// If no primary weapon
	if (self->client->weapon != FindItem(SNIPER_NAME)
		&& self->client->weapon != FindItem(M4_NAME)
		&& self->client->weapon != FindItem(MP5_NAME)
		&& self->client->weapon != FindItem(M3_NAME)
		&& self->client->weapon != FindItem(HC_NAME))
	{
		// Try find a random primary weapon
		int rng_wpn = rand() % 5;
		switch (rng_wpn)
		{
		case 0:
			item = BOTLIB_LocateRangedItem(self, "weapon_Sniper", 768);
			break;
		case 1:
			item = BOTLIB_LocateRangedItem(self, "weapon_M4", 768);
			break;
		case 2:
			item = BOTLIB_LocateRangedItem(self, "weapon_MP5", 768);
			break;
		case 3:
			item = BOTLIB_LocateRangedItem(self, "weapon_M3", 768);
			break;
		case 4:
			item = BOTLIB_LocateRangedItem(self, "weapon_HC", 768);
			break;
		default:
			break;
		}

		//if (item)
		//	Com_Printf("%s %s located weapon: %s\n", __func__, self->client->pers.netname, item->classname);
	}
	
	// Check ammo if we have a weapon
	else if (self->client->weapon == FindItem(SNIPER_NAME))
	{
		if (self->client->sniper_rds <= 6) // Rounds
			item = BOTLIB_LocateRangedItem(self, "ammo_sniper", 1024);
	}
	else if (self->client->weapon == FindItem(M4_NAME))
	{
		if (self->client->inventory[ITEM_INDEX(GET_ITEM(M4_ANUM))] < 1) // Clips
			item = BOTLIB_LocateRangedItem(self, "ammo_m4", 1024);
	}
	else if (self->client->weapon == FindItem(MP5_NAME))
	{
		if (self->client->inventory[ITEM_INDEX(GET_ITEM(MP5_ANUM))] < 1) // Mags
			item = BOTLIB_LocateRangedItem(self, "ammo_mag", 1024);
	}
	else if (self->client->weapon == FindItem(M3_NAME))
	{
		if (self->client->inventory[ITEM_INDEX(GET_ITEM(SHELL_ANUM))] <= 7) // Shells
			item = BOTLIB_LocateRangedItem(self, "ammo_m3", 1024);

		//Com_Printf("%s %s got item: %s ammo: %d\n", __func__, self->client->pers.netname, M3_NAME, self->client->inventory[ITEM_INDEX(GET_ITEM(SHELL_ANUM))]);
	}
	else if (self->client->weapon == FindItem(HC_NAME))
	{
		if (self->client->inventory[ITEM_INDEX(GET_ITEM(SHELL_ANUM))] <= 7) // Shells
			item = BOTLIB_LocateRangedItem(self, "ammo_m3", 1024);
	}
	else if (self->client->weapon == FindItem(MK23_NAME) || self->client->weapon == FindItem(DUAL_NAME))
	{
		if (self->client->inventory[ITEM_INDEX(GET_ITEM(MK23_ANUM))] < 1) // Clips
			item = BOTLIB_LocateRangedItem(self, "ammo_clip", 1024);
	}

	// Try to navigate to the item
	// Check we're navigating to a different item->classname to that found in goalentity->classname
	if (item)
	{
		// Check if we're already after this item
		qboolean goalAlreadySet = false;
		if (self->goalentity && Q_stricmp(item->classname, self->goalentity->classname) == 0)
			goalAlreadySet = true;

		// Only grab item if we're not already after it
		if (self->goalentity == NULL || goalAlreadySet == false)
		{
			int item_node = ACEND_FindClosestReachableNode(item, NODE_DENSITY, NODE_ALL);
			if (item_node != INVALID)
			{
				self->goalentity = item;
			}
		}
	}
}
//rekkie -- DEV_1 -- e

//rekkie -- Quake3 -- s
void BOTLIB_Healing(edict_t* self, usercmd_t* ucmd)
{
	if ((self->health != self->old_health || self->client->leg_damage) && self->health < 100)
	{
		//Com_Printf("%s %s is healing\n", __func__, self->client->pers.netname);
		
		// Handle when to bandage
		// 1) Bandage immediately if critical damage (highest priority)
		// 2) Bandage immediately if no nearby enemies
		// 3) Delayed bandaging when no enemy in sight

		if (self->client->bandaging == 0) // Not already bandaging
		{
			// Set bandage delay time
			if (self->bot.bot_bandage_delay_time == 0) // Timer not set yet
			{
				self->bot.bot_bandage_delay_time = self->bot.jumppad_last_time = level.framenum + 5 * HZ; // Base time
				if (self->client->leg_damage)
					self->bot.bot_bandage_delay_time += ((rand() % 15) + 1); // 1) Quicker to call Cmd_Bandage_f if leg damage
				else
					self->bot.bot_bandage_delay_time += ((rand() % 25) + 1); // 2) Slower to call Cmd_Bandage_f for all other damages
			}

			// If nearest enemy is far enough away, just bandage. Ignore line of sight because that is checked before BOTLIB_Healing() is called.
			float closest_enemy = 999999;
			float dist = 0;
			for (int p = 0; p < num_players; p++)
			{
				if (players[p] == self) continue; // Skip self

				if (ACEAI_IsEnemy(self, players[p])) // Are they our enemy
				{
					dist = VectorDistance(self->s.origin, players[p]->s.origin);
					if (dist < closest_enemy)
						closest_enemy = dist;
				}
			}
			//if (closest_enemy > 1536)
			//	self->bot.bot_bandage_delay_time = 0;

			//Com_Printf("%s %s health[%d] enemy dist[%f]\n", __func__, self->client->pers.netname, self->health, closest_enemy);


			// Bandage if out of sight
			if (self->health < 7) // Critical condition, just bandage
			{
				//Com_Printf("%s %s critical bandaging with health:%d\n", __func__, self->client->pers.netname, self->health);
				Cmd_Bandage_f(self);
				self->bot.bot_bandage_delay_time = 0; // Reset timer

				if (self->client->bandaging) self->bot.radioBandaging = true; // If bandaging, flag the bot to radio this in
			}
			//else if ((self->bot.enemy_seen_time + self->bot.bot_bandage_delay_time) <= (level.framenum * HZ))
			else if (self->bot.bot_bandage_delay_time < level.framenum && 
				closest_enemy > 1024 && 
				(self->health < 100 || self->client->bleeding)) // Bot can bandage
			{
				//Com_Printf("%s %s is bandaging with health:%d, bandage delay:%f\n", __func__, self->client->pers.netname, self->health, self->bot_bandage_delay_time);
				Cmd_Bandage_f(self);
				self->bot.bot_bandage_delay_time = 0; // Reset timer

				if (self->client->bandaging) self->bot.radioBandaging = true; // If bandaging, flag the bot to radio this in
			}
		}
		if (self->old_health != self->health)
			self->old_health = self->health;
	}
	// Hold still while bandaging
	if (self->client->bandaging == 1 || self->client->bandage_stopped == 1)
	{
		//self->bot.bi.actionflags |= ACTION_HOLDPOS; // Stop moving while bandaging
		self->bot.bi.actionflags |= ACTION_CROUCH; // Crouch while bandaging

		if (self->old_health != self->health)
			self->old_health = self->health;
	}

	// Update old health
	//self->old_health = self->health;
}
void BOTLIB_Attack(edict_t* self, usercmd_t* ucmd)
{
	if (self->client->weaponstate == WEAPON_RELOADING) return;
	if (self->client->bandaging) return;
	if (self->enemy == NULL) return;
	if (self->enemy && self->enemy->deadflag != DEAD_NO) return;
	if (teamplay->value && lights_camera_action > 0) return;

	//Com_Printf("%s [%d] %s attack enemy %s\n", __func__, level.framenum, self->client->pers.netname, self->enemy->client->pers.netname);

	//self->client->inventory[items[GRENADE_NUM].index] = 2; // Give some ammo
	if (self->client->weapon == FindItemByNum(GRENADE_NUM) && self->client->weaponstate == WEAPON_READY)
	{
		// If pin pulled, let go of attack
		if (self->client->ps.gunframe >= GRENADE_IDLE_FIRST && self->client->ps.gunframe <= GRENADE_IDLE_LAST)
			return;
		else // Pull pin
		{
			self->bot.bi.actionflags |= ACTION_ATTACK;
			return;
		}
	}
	if (self->client->weapon == FindItemByNum(KNIFE_NUM) && self->client->weaponstate == WEAPON_READY)
	{
		self->bot.bi.actionflags |= ACTION_ATTACK;
		return;
	}


	// Always give bots basic ammo -- cheating, but at least they have something.
	if (1)
	{
		// Knives
		if (0 && (rand() % 50) == 0)
		{
			if ((INV_AMMO(self, KNIFE_NUM) < 2))
				self->client->inventory[items[KNIFE_NUM].index] = 1; // Give some ammo
			if (self->client->weapon == FindItem(KNIFE_NAME))
				self->client->pers.knife_mode = 1; // Throwing knives
		}

		// MK23
		//if ((rand() % 10) == 0)
		{
			gitem_t* ammo_item = FindItem(FindItem(MK23_NAME)->ammo);
			int ammo_index = ITEM_INDEX(ammo_item);
			if (self->client->inventory[ammo_index] < 1)
				self->client->inventory[ammo_index] = 1;
		}

		// Grenades
		if (0 && (rand() % 50) == 0)
		{
			if (INV_AMMO(self, BAND_NUM) && (INV_AMMO(self, GRENADE_NUM) < 2))
				self->client->inventory[items[GRENADE_NUM].index] = 1;
		}
	}

	// Sniper bots should zoom in before firing
	if (BOTLIB_SniperZoom(self))
		return; // Wait a frame

	// Pick an appropriate weapon
	if (BOTLIB_ChooseWeapon(self) == false) // If bot has no weapon
	{
		//Com_Printf("%s %s BOTLIB_ChooseWeapon was false\n", __func__, self->client->pers.netname);
		return; // Keep moving
	}

	// Only shoot if the weapon is ready and we can hit the target!
	if (self->client->weaponstate == WEAPON_READY || self->client->weaponstate == WEAPON_FIRING)
	{
		if (self->bot.enemy_in_xhair)
		{
			// Check the need for crouching to increase accuracy. 
			// Only crouch if target is far, doesn't block the shot.
			// Only allow firing of an inacurrate weapon (without a laser) if touching the ground
			if (self->bot.enemy_dist > 512 && INV_AMMO(self, LASER_NUM) == false)
			{
				if ((self->client->m4_rds && (self->client->weapon == FindItem(M4_NAME)))
					|| (self->client->mp5_rds && (self->client->weapon == FindItem(MP5_NAME)))
					|| (self->client->mk23_rds && (self->client->weapon == FindItem(MK23_NAME)))
					|| (self->client->dual_rds && (self->client->weapon == FindItem(DUAL_NAME))))
				{
					if (self->bot.touch_ground.fraction == 1.0) // Don't fire while to far off the ground
						return;

					// Raptor007: Don't crouch if it blocks the shot.
					self->s.origin[2] -= 14;
					if (ACEAI_CheckShot(self))
					{
						self->bot.bi.actionflags |= ACTION_CROUCH;
						//Com_Printf("%s %s crouch shooting\n", __func__, self->client->pers.netname);
					}
					self->s.origin[2] += 14;
				}
			}

			// Jump at enemy if we have a HC
			{
				if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME))// && self->bot.enemy_dist > 128)
				{	// If enemy if far enough away, we're touching the ground, and the high difference is within jumping height
					if (self->bot.enemy_dist > 264 && self->bot.enemy_dist < 300 && self->bot.touch_ground.fraction < 1.0 && self->bot.enemy_height_diff <= 60)
					{
						//BOTLIB_Jump_Takeoff(self, NULL, self->enemy->s.origin, self->viewheight, self->velocity);
						//BOTLIB_DoParabolaJump(self, self->enemy->s.origin);
						// Should probably make the jump less parabola and more head on for a faster attack
					}

					// Only fire the HC when close enough
					if (self->bot.enemy_dist > 264) //self->bot.enemy_dist > 160 //&& (self->bot.touch_ground.fraction == 1.0 || self->bot.enemy_dist > 64))
					{
						if (self->groundentity == NULL)
							return;
						else if (self->bot.enemy_dist > 160)
							return;
					}
				}
			}

			// Attack!
			self->bot.bi.actionflags |= ACTION_ATTACK;
		}
	}
}
//rekkie -- Quake3 -- e

///////////////////////////////////////////////////////////////////////
// Attack movement routine
//
// NOTE: Very simple for now, just a basic move about avoidance.
//       Change this routine for more advanced attack movement.
///////////////////////////////////////////////////////////////////////
void ACEMV_Attack (edict_t *self, usercmd_t *ucmd)
{
	float c;
	vec3_t  target;
	vec3_t	attackvector;
	float	dist;
	qboolean	bHasWeapon;	// Needed to allow knife throwing and kick attacks
	qboolean	enemy_front;

	bHasWeapon = self->grenadewait || ACEAI_ChooseWeapon(self);

	//rekkie -- DEV_1 -- s
	// If player has no weapon, keep moving!
	if (bHasWeapon == false)
	{
		BOTLIB_MOV_Move(self, ucmd); // Keep moving
	}
	//rekkie -- DEV_1 -- e

	// Check distance to enemy
	VectorSubtract( self->s.origin, self->enemy->s.origin, attackvector);
	dist = VectorLength( attackvector);

	enemy_front = infront( self, self->enemy );

  //If we're fleeing, don't bother moving randomly around the enemy and stuff...
//  if (self->state != STATE_FLEE)
  {

//AQ2 CHANGE
	// Don't stand around if all you have is a knife or no weapon.
	if( (self->client->weapon == FindItem(KNIFE_NAME)) || !bHasWeapon/*kick attack*/ )
	{
		// Don't walk off the edge
		if( ACEMV_CanMove( self, MOVE_FORWARD))
		{
			ucmd->forwardmove += SPEED_RUN;
		}
		else
		{
			// Can't get there!
			ucmd->forwardmove = -SPEED_WALK;
			self->enemy=NULL;
			self->state = STATE_WANDER;
			return;
		}

		// Raptor007: Attempt longer knife throws when carrying many knives.
		self->client->pers.knife_mode = 0;
		if( (dist < 2000) && (dist < 400 + 300 * INV_AMMO(self,KNIFE_NUM)) )
		{
			float kick_dist = 200;

			if( bHasWeapon )
			{
				self->client->pers.knife_mode = 1;
				kick_dist = 100;
			}

			if( dist < kick_dist )
			{
				if( dist < 64 )	// Too close
					ucmd->forwardmove = -SPEED_WALK;
				// Kick Attack needed!
				ucmd->upmove = SPEED_RUN;
			}
		}
	}
	else //if(!(self->client->weapon == FindItem(SNIPER_NAME))) // Stop moving with sniper rifle
	{
		// Randomly choose a movement direction
		c = random();		
		if(c < 0.2 && ACEMV_CanMove(self,MOVE_LEFT))
			ucmd->sidemove -= SPEED_RUN; 
		else if(c < 0.4 && ACEMV_CanMove(self,MOVE_RIGHT))
			ucmd->sidemove += SPEED_RUN;

		c = random();
		if(c < 0.6 && ACEMV_CanMove(self,MOVE_FORWARD))
			ucmd->forwardmove += SPEED_RUN;
		else if (c < 0.8 && ACEMV_CanMove(self,MOVE_BACK))
			ucmd->forwardmove -= SPEED_RUN;
	}
//AQ2 END
	if( (dist < 600) && 
		( !(self->client->weapon == FindItem(KNIFE_NAME)) 
)//		&& !(self->client->weapon == FindItem(SNIPER_NAME))) // Stop jumping with sniper rifle
		&&( bHasWeapon )	// Jump already set for kick attack
		)
	{
		// Randomly choose a vertical movement direction
		c = random();

		if ((c < 0.10) && FRAMESYNC) //Werewolf: was 0.15
		{
			if (ACEMV_CanJump(self))
			{
				ucmd->upmove += SPEED_RUN;
				//Com_Printf("%s %s jumping\n", __func__, self->client->pers.netname);
			}
		}
		else if (c > 0.85)	// Only crouch sometimes
		{
			ucmd->upmove -= SPEED_WALK;
			//Com_Printf("%s %s crouching\n", __func__, self->client->pers.netname);
		}
	}

  }	//The rest applies even for fleeing bots

	// Werewolf: Crouch if no laser light
	if( (ltk_skill->value >= 3)
	&& self->groundentity
	&& ! INV_AMMO( self, LASER_NUM )
	&& (  (self->client->m4_rds   && (self->client->weapon == FindItem(M4_NAME)))
	   || (self->client->mp5_rds  && (self->client->weapon == FindItem(MP5_NAME)))
	   || (self->client->mk23_rds && (self->client->weapon == FindItem(MK23_NAME)))
	   || (self->client->dual_rds && (self->client->weapon == FindItem(DUAL_NAME))) ) )
	{
		//rekkie -- DEV_1 -- s
		
		// Try strafing around enemy
		{
			VectorSubtract(self->enemy->s.origin, self->s.origin, self->move_vector);
			ACEMV_ChangeBotAngle(self);


			// Try strafing continually in a general direction, if possible
			static int max_strafe_left = -10;
			static int max_strafe_right = 10;
			if (self->bot_strafe == 0) // Pick new direction
			{
				float strafe_choice = random() < 0.333;
				if (strafe_choice < 0.33) // Left
					self->bot_strafe = -1;
				else if (strafe_choice < 0.66) // Right
					self->bot_strafe = 1;
				else
					self->bot_strafe = 0; // Neither - skip strafing this turn
			}
			if (self->bot_strafe < 0 && random() > 0.15) // Going left 85% of the time
			{
				if (ACEMV_CanMove(self, MOVE_LEFT) && self->bot_strafe >= max_strafe_left) // Can go left with a limit
				{
					//Com_Printf("%s %s strafe [LEFT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
					self->bot_strafe--;
					ucmd->sidemove = -SPEED_RUN;
				}
				else if (ACEMV_CanMove(self, MOVE_RIGHT)) // Cannot go left anymore, so try going right
				{
					self->bot_strafe = 1; // Go right
					ucmd->sidemove = SPEED_RUN;
					//Com_Printf("%s %s strafe [RIGHT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
				}
				else
					self->bot_strafe = 0; // Could not go either direction, so skip strafing this turn and reset back to random choice
			}
			else if (self->bot_strafe > 0 && random() > 0.15) // Going right 85% of the time
			{
				if (ACEMV_CanMove(self, MOVE_RIGHT) && self->bot_strafe <= max_strafe_right) // Can go right with a limit
				{
					//Com_Printf("%s %s strafe [RIGHT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
					self->bot_strafe++;
					ucmd->sidemove = SPEED_RUN;
				}
				else if (ACEMV_CanMove(self, MOVE_LEFT)) // Cannot go left anymore, so try going left
				{
					self->bot_strafe = -1; // Go left
					ucmd->sidemove = -SPEED_RUN;
					//Com_Printf("%s %s strafe [LEFT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
				}
				else
					self->bot_strafe = 0; // Could not go either direction, so skip strafing this turn and reset back to random choice
			}
			else
				self->bot_strafe = 0; // Skip strafing this turn

			// Back off if getting too close (unless we have a HC or knife)
			if (dist < 256)
			{
				if (self->client->weapon == FindItem(HC_NAME) || self->client->weapon == FindItem(KNIFE_NAME))
				{
					// Come in close for the kill
					if (ACEMV_CanMove(self, MOVE_FORWARD))
						ucmd->forwardmove = SPEED_RUN;
				}
				// Try move backwards
				else if (ACEMV_CanMove(self, MOVE_BACK))
					ucmd->forwardmove = -SPEED_RUN;
			}
			// If distance is far, consider crouching to increase accuracy
			else if (dist > 1024)
			{
				if (ACEMV_CanMove(self, MOVE_FORWARD))
					ucmd->forwardmove = SPEED_RUN;

				// Raptor007: Don't crouch if it blocks the shot.
				float old_z = self->s.origin[2];
				self->s.origin[2] -= 14;
				if (ACEAI_CheckShot(self))
				{
					ucmd->upmove = -SPEED_RUN;
					//Com_Printf("%s %s crouch shooting\n", __func__, self->client->pers.netname);
				}
				self->s.origin[2] = old_z;
			}
			else
			{
				// Keep distance with sniper
				if (dist < 1024 && self->client->weapon == FindItem(SNIPER_NAME) && ACEMV_CanMove(self, MOVE_BACK))
					ucmd->forwardmove = -SPEED_RUN;

				// Otherwise move toward target
				else if (ACEMV_CanMove(self, MOVE_FORWARD))
					ucmd->forwardmove = SPEED_RUN;
			}
		}
		 
		// If the bots are close to their target, don't crouch, take other measures.
		//c = random();
		//if (c < 0.10)
		//if (dist < 640)
		if (1)
		{
			//if (ACEMV_CanJump(self))
			//{
			//	ucmd->upmove += SPEED_RUN;
			//	Com_Printf("%s %s jump shooting\n", __func__, self->client->pers.netname);
			//}

			/*
			for (int i = 0; i < MAXLINKS; i++)
			{
				if (self->current_node == INVALID)
				{
					self->current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
					if (self->current_node == INVALID)
						break;
				}

				if (nodes[self->current_node].links[i].targetNode != INVALID)
				{
					c = random();
					if (c < 0.01)
					{
						{
							self->state = STATE_MOVE;
							self->tries = 0; // Reset the count of how many times we tried this goal
							BOTLIB_SetGoal(self, nodes[self->current_node].links[i].targetNode);
							self->wander_timeout = level.framenum + 1.0 * HZ;
							Com_Printf("%s %s avoidance code\n", __func__, self->client->pers.netname);
							return;
						}
					}
					//else
					//	break;
					//else
					//	return;
				}
			}
			*/
		}
		else
		{
		//rekkie -- DEV_1 -- e


			// Raptor007: Don't crouch if it blocks the shot.
			float old_z = self->s.origin[2];
			self->s.origin[2] -= 14;
			if (ACEAI_CheckShot(self))
			{
				ucmd->upmove = -SPEED_RUN;
				//Com_Printf("%s %s crouch shooting\n", __func__, self->client->pers.netname);
			}
			self->s.origin[2] = old_z;


		} //rekkie -- DEV_1
	}

	// Set the attack 
	//@@ Check this doesn't break grenades!
	//Werewolf: I've checked that. Now it doesn't anymore. Behold my power! Muhahahahaha!!!
	if((self->client->weaponstate == WEAPON_READY)||(self->client->weaponstate == WEAPON_FIRING))
	{
		// Only shoot if the weapon is ready and we can hit the target!
		//@@ Removed to try to help RobbieBoy!
		//Reenabled by Werewolf
		if( ACEAI_CheckShot( self ))
		{
			if( (ltk_skill->value >= 0)
			&&  (self->react >= 1.f / (ltk_skill->value + 2.f))             // Reaction time.
			&&  enemy_front
			&&  (FRAMESYNC || (self->client->weaponstate != WEAPON_READY))  // Sync firing with random offsets.
			&&  (self->teamPauseTime == level.framenum - 1) )               // Saw them last frame too.
			{
				ucmd->buttons = BUTTON_ATTACK;

				if(self->client->weapon == FindItem(GRENADE_NAME))
				{
					self->grenadewait = level.framenum + 1.5 * HZ;
					ucmd->forwardmove = -SPEED_RUN; //Stalk back, behold of the holy Grenade!
				}
				else
					self->grenadewait = 0;
			}
		}
		else if (self->state != STATE_FLEE)
		{
			if (ucmd->upmove == -SPEED_RUN)
				ucmd->upmove = 0;
			if (ltk_jumpy->value)
			{
				c = random();
				if(c < 0.50)	//Only jump at 50% probability
				{
					if (ACEMV_CanJump(self))
					ucmd->upmove = SPEED_RUN;
				}
				else if (c < 0.75 && ACEMV_CanMove(self,MOVE_LEFT))
					ucmd->sidemove -= SPEED_WALK;
				else if (ACEMV_CanMove(self,MOVE_RIGHT))
					ucmd->sidemove += SPEED_WALK;
			}
		}
	}

	// Raptor007: Don't immediately shoot friendlies after round.
	if( (team_round_countdown > 40) && ! self->grenadewait )
	{
		if( self->client->weapon == FindItem(KNIFE_NAME) )
			self->client->pers.knife_mode = 1;  // Throwing knives are honorable.
		else if( self->client->weapon != FindItem(HC_NAME) )  // Handcannon is allowed.
			ucmd->buttons &= ~BUTTON_ATTACK;

		if( dist < 128 )
		{
			if( self->groundentity && (self->s.origin <= self->enemy->s.origin) )
				ucmd->upmove = SPEED_RUN;  // Kicking is the most honorable form of combat.
		}
		else
		{
			ucmd->upmove = 0;
			if( ACEMV_CanMove( self, MOVE_FORWARD ) )
				ucmd->forwardmove = SPEED_RUN;  // We need to get closer to kick.
		}
	}

	// Aim
	VectorCopy(self->enemy->s.origin,target);

	// Werewolf: Aim higher if using grenades
	if(self->client->weapon == FindItem(GRENADE_NAME))
		target[2] += 35;

	//AQ2 ADD - RiEvEr
	// Alter aiming based on skill level
	// Modified by Werewolf and Raptor007
	if( 
		( (ltk_skill->value >= 0) && (ltk_skill->value < 10) )
		&& ( bHasWeapon )	// Kick attacks must be accurate
		&& (!(self->client->weapon == FindItem(KNIFE_NAME))) // Knives accurate
		&& (!(self->client->weapon == FindItem(GRENADE_NAME))) // Grenades accurate
		)
	{
		short int sign[3], iFactor = 7;
		sign[0] = (random() < 0.5) ? -1 : 1;
		sign[1] = (random() < 0.5) ? -1 : 1;
		sign[2] = (random() < 0.5) ? -1 : 1;

		// Not that complex. We miss by 0 to 80 units based on skill value and random factor
		// Unless we have a sniper rifle!
		if(self->client->weapon == FindItem(SNIPER_NAME))
			iFactor = 5;

		// Shoot less accurately if we just got hit.
		if( self->client->push_timeout > 45 )
			iFactor += self->client->push_timeout - 45;

		// Shoot more accurately after holding aim, but less accurately when first aiming.
		if( self->react >= 4.f )
			iFactor --;
		else if( (self->react < 0.5f) && (dist > 300.f) )
			iFactor ++;

		// Shoot less accurately at moving targets.
		if (VectorLength(self->enemy->velocity) > 300.f)
			iFactor++;

		target[0] += sign[0] * (10 - ltk_skill->value + ( (  iFactor*(10 - ltk_skill->value)  ) * random() )) * 0.7f;
		target[1] += sign[1] * (10 - ltk_skill->value + ( (  iFactor*(10 - ltk_skill->value)  ) * random() )) * 0.7f;
		target[2] += sign[2] * (10 - ltk_skill->value + ( (  iFactor*(10 - ltk_skill->value)  ) * random() ));
	}
	//Werewolf: Snipers of skill 10 are complete lethal, so I don't use that code down there
/*	else if (ltk_skill->value == 11)
	if(self->client->weapon == FindItem(SNIPER_NAME))
	{
		short int	up, right, iFactor=1;
		up = (random() < 0.5)? -1 :1;
		right = (random() < 0.5)? -1 : 1;
		target[0] += ( right * (10 - 5 +((iFactor*(10 - 5)) *random())) );
		target[2] += ( up * (10 - 5 +((iFactor*(10 - 5)) *random())) );
	}
*/
//AQ2 END

	// Werewolf: release trigger after 1 second for grenades
	if( (self->grenadewait == level.framenum + 1 * HZ) && (self->solid != SOLID_NOT) )
	{
		ucmd->buttons = 0;
	}

	//Werewolf: Wait for grenade to launch before facing elsewhere
	if( level.framenum >= self->grenadewait )
	{
		self->grenadewait = 0;

		// Raptor007: Interpolate angle changes, and set new desired direction at 10fps.
		// Make sure imperfect angles are calculated when first spotting an enemy, too.
		ACEMV_ChangeBotAngle( self );
		if( FRAMESYNC || ! enemy_front || (level.framenum - self->teamPauseTime >= FRAMEDIV) )
			VectorSubtract( target, self->s.origin, self->move_vector );
	}

	// Store time we last saw an enemy
	// This value is used to decide if we initiate a long range search or not.
	self->teamPauseTime = level.framenum;
	
//	if(debug_mode)
//		debug_printf("%s attacking %s\n",self->client->pers.netname,self->enemy->client->pers.netname);
}

//==========================
// AntPathMove
//==========================
//
qboolean	AntPathMove( edict_t *self )
{
	//node_t *temp = &nodes[self->bot.current_node];	// For checking our position

	//if( level.time == (float)((int)level.time) )
	if( level.framenum % HZ == 0 )
	{
		if( 
			!AntLinkExists( self->bot.current_node, SLLfront(&self->pathList) )
			&& ( self->bot.current_node != SLLfront(&self->pathList) )
			)
		{
			// We are off the path - clear out the lists
			AntInitSearch( self );
		}
	}

	// Boot in our new pathing algorithm
	// This will fill ai.pathList with the information we need
	if( 
		SLLempty(&self->pathList)				// We have no path and
		&& (self->bot.current_node != self->bot.goal_node)	// we're not at our destination
		)
	{
		if( !AntStartSearch( self, self->bot.current_node, self->bot.goal_node, true))	// Set up our pathList
		{
			// Failed to find a path
//			gi.bprintf(PRINT_HIGH,"%s: Target at(%i) - No Path \n",
			return false;
		}
		return true;
	}

	// And change our pathfinding to use it
	if ( !SLLempty(&self->pathList) )								// We have a path 
	{
//		if( AtNextNode( self->s.origin, SLLfront(&self->pathList))	// we're at the nextnode
		if( ACEND_FindClosestReachableNode(self,NODE_DENSITY*0.66,NODE_ALL) == SLLfront(&self->pathList)	// we're at the nextnode
			|| self->bot.current_node == SLLfront(&self->pathList)
		)
		{
			self->bot.current_node = SLLfront(&self->pathList);		// Show we're there
			SLLpop_front(&self->pathList);				// Remove the top node
		}
		if( !SLLempty(&self->pathList) )
			self->bot.next_node = SLLfront(&self->pathList);	// Set our next destination
		else
		{
			self->bot.next_node = INVALID;				// We're at the target location
			//Com_Printf("%s We're at the target location!\n", __func__);
		}
		return true;
	}
	return true;	// Pathlist is emptyand we are at our destination
}

