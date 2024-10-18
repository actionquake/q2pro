#include "g_local.h"

//cvar_t *jump;

char *jump_statusbar =
    "yb -24 "
    "if 9 "
    "xr 0 "
    "yb 0 "
    "xv 0 "
    "yv 0 "
    "pic 9 "
    "endif "
    //  Current Speed
    "xr -105 "
    "yt 2 "
    "string \"Current Speed\" "
    "xr -82 "
    "yt 12 "
    "num 6 1 "

    //  High Speed
    "xr -81 "
    "yt 34 "
    "string \"High Speed\" "
    "xr -82 "
    "yt 44 "
    "num 6 2 "

    //  last fall damage
    "xr -129 "
    "yt 66 "
    "string \"Last Fall Damage\" "
    "xr -82 "
    "yt 76 "
    "num 10 3 "
;

void Jmp_SetStats(edict_t *ent)
{
	vec3_t	velocity;
	vec_t	speed;

	// calculate speed
	VectorClear(velocity);
	VectorCopy(ent->velocity, velocity);
	speed = VectorNormalize(velocity);

	if(speed > ent->client->resp.jmp_highspeed)
		ent->client->resp.jmp_highspeed = speed;

	ent->client->ps.stats[STAT_SPEEDX] = speed;
	ent->client->ps.stats[STAT_HIGHSPEED] = ent->client->resp.jmp_highspeed;
	ent->client->ps.stats[STAT_FALLDMGLAST] = ent->client->resp.jmp_falldmglast;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (ent->health <= 0 || level.intermission_framenum || ent->client->layout)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	if (ent->client->showinventory && ent->health > 0)
		ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	SetIDView (ent);
}

void Jmp_EquipClient(edict_t *ent)
{
	memset(ent->client->inventory, 0, sizeof(ent->client->inventory));
	ent->client->weapon = 0;

	// make client non-solid
	ent->solid = SOLID_TRIGGER;
	AddToTransparentList(ent);
}

void Cmd_Jmod_f (edict_t *ent)
{
	char *cmd = NULL;

	if( ! jump->value ) {
		gi.cprintf(ent, PRINT_HIGH, "The server does not have JumpMod enabled.\n");
		return;
	}

	if(gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "AQ2:TNG Jump mode commands:\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod store - save your current point\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod recall - teleport back to saved point\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod reset - remove saved point\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod clear - reset stats\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod noclip - toggle noclip\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod spawnp <#> - teleport, optional spawnpoint \n");
		gi.cprintf(ent, PRINT_HIGH, " jmod spawnc - teleport closest spawnpoint\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod goto <#> <#> <#> - teleport to x y z coordinates\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod lca - start Lights Camera Action\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod lasersight - toggle lasersight\n");

		return;
	}

	cmd = gi.argv(1);

	if(Q_stricmp(cmd, "store") == 0)
	{
		Cmd_Store_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "recall") == 0)
	{
		Cmd_Recall_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "reset") == 0)
	{
		Cmd_Reset_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "clear") == 0 || Q_stricmp(cmd, "rhs") == 0)
	{
		Cmd_Clear_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "goto") == 0)
	{
		Cmd_Goto_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "spawnp") == 0)
	{
		Cmd_GotoP_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "spawnc") == 0)
	{
		Cmd_GotoPC_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "lca") == 0)
	{
		Cmd_PMLCA_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "noclip") == 0)
	{
		Cmd_Noclip_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "lasersight") == 0)
	{
		PMLaserSight(ent);
		return;
	}
	else if(Q_stricmp(cmd, "slippers") == 0)
	{
		PMStealthSlippers(ent);
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Unknown jmod command\n");
}

edict_t *SelectClosestDeathmatchSpawnPoint (void)
{
	edict_t *bestspot;
	float   bestdistance, bestplayerdistance;
	edict_t *spot;

	spot = NULL;
	bestspot = NULL;
	bestdistance = -1;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
			bestplayerdistance = PlayersRangeFromSpot (spot);
			if ((bestplayerdistance < bestdistance) || (bestdistance < 0))
			{
					bestspot = spot;
					bestdistance = bestplayerdistance;
			}
	}
	return bestspot;
}

void Cmd_PMLCA_f(edict_t *ent)
{
	if (ent->client->pers.spectator)
	{
		gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
		ent->client->resp.toggle_lca = 0;
		return;
	}

	// Set this to 1 to prevent damage during LCA
	lights_camera_action = 1;
	if (!ent->client->resp.toggle_lca)
	{
		gi.centerprintf (ent,"LIGHTS...\n");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("atl/lights.wav"), 1, ATTN_STATIC, 0);
		ent->client->resp.toggle_lca = 43;
	}
	else if (ent->client->resp.toggle_lca == 23)
	{
		gi.centerprintf (ent,"CAMERA...\n");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("atl/camera.wav"), 1, ATTN_STATIC, 0);
	}
    else if (ent->client->resp.toggle_lca == 3)
	{
		gi.centerprintf (ent,"ACTION!\n");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("atl/action.wav"), 1, ATTN_STATIC, 0);
	}
	ent->client->resp.toggle_lca--;
	// Set it back to 0 for normal damage to occur
	lights_camera_action = 0;
}

edict_t *PMSelectSpawnPoint (int number)
{
        edict_t 		*spot;
        int             count = 0;
        int             selection;

        spot = NULL;
        while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
			count++;

        if (!count)
			return NULL;

		//if random was selected, pick one
		if (number == 0) {
	        selection = rand() % count + 1;
		} else {
			//if person wanted tele 7 but only 5 found, set to 5
			if (number > count)
				number = count;
			//if person input a negative, set to 0
			else if (number < 0)
				number = 0;
			selection = number;
		}

		spot = NULL;
		do
		{
			spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
			selection--;
		} while (selection > 0);

        return spot;
}

void jmodTeleport (edict_t *ent, edict_t *spot)
{
	vec3_t		teleport_goto, angles;
	int			i;

	VectorCopy (spot->s.origin, teleport_goto);
	teleport_goto[2] += 9;
	VectorCopy (spot->s.angles, angles);

	ent->client->jumping = 0;
	ent->movetype = MOVETYPE_NOCLIP;
	gi.unlinkentity (ent);

	VectorCopy (teleport_goto, ent->s.origin);
	VectorCopy (teleport_goto, ent->s.old_origin);

	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);

	ent->client->ps.pmove.pm_time = 160>>3;		// hold time

	// draw the teleport splash on the player
	ent->s.event = EV_PLAYER_TELEPORT;

	VectorClear (ent->s.angles);
	VectorClear (ent->client->ps.viewangles);
	VectorClear (ent->client->v_angle);

	VectorCopy(angles,ent->s.angles);
	VectorCopy(ent->s.angles,ent->client->v_angle);

	for (i=0;i<2;i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
	if (ent->client->pers.spectator)
		ent->solid = SOLID_BBOX;
	else
		ent->solid = SOLID_TRIGGER;

	ent->deadflag = DEAD_NO;

	gi.linkentity (ent);

	ent->movetype = MOVETYPE_WALK;

	// Run LCA right after spawn
	Cmd_PMLCA_f(ent);
}

void Cmd_Goto_f (edict_t *ent)
{
	int 		i;
	vec3_t		teleport_goto;

	if (!ent->deadflag && !ent->client->pers.spectator)
	{
		// 5 = jmod goto x y z
		if (gi.argc() == 5)
		{
			// Verifying input
			if (Q_stricmp(gi.argv(0), "jmod") == 0 && Q_stricmp(gi.argv(1), "goto") == 0){
				// Hacky shit, set gi.argv(2) as teleport_goto[0], etc..:
				for (i = 0; i < 3; i++){
					teleport_goto[i] = atoi(gi.argv(i+2));
				}
			}
			teleport_goto[2] -= ent->viewheight;

			ent->client->jumping = 0;
			ent->movetype = MOVETYPE_NOCLIP;
			gi.unlinkentity (ent);

			VectorCopy (teleport_goto, ent->s.origin);
			VectorCopy (teleport_goto, ent->s.old_origin);

			// clear the velocity and hold them in place briefly
			VectorClear (ent->velocity);

			ent->client->ps.pmove.pm_time = 160>>3;		// hold time

			// draw the teleport splash on the player
			ent->s.event = EV_PLAYER_TELEPORT;

			VectorClear (ent->s.angles);
			VectorClear (ent->client->ps.viewangles);
			VectorClear (ent->client->v_angle);

			for (i=0;i<2;i++)
				ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
			if (ent->client->pers.spectator)
				ent->solid = SOLID_BBOX;
			else
				ent->solid = SOLID_TRIGGER;

			ent->deadflag = DEAD_NO;

			gi.linkentity (ent);
			
			ent->movetype = MOVETYPE_WALK;
		}
		else
			gi.cprintf(ent,PRINT_HIGH,"Wrong syntax: goto <#> <#> <#>\n");
	}
	else
		gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
	
}

void Cmd_GotoP_f_compat (edict_t *ent, pmenu_t *p)
{
	Cmd_GotoP_f(ent);
}

void Cmd_GotoPC_f_compat (edict_t *ent, pmenu_t *p)
{
	Cmd_GotoPC_f(ent);
}

void Cmd_GotoP_f (edict_t *ent)
{
	edict_t 	*spot = NULL;
	char 		*buffer="\0";

	if (ent->deadflag || ent->client->pers.spectator) {
		gi.cprintf(ent, PRINT_HIGH, "This command cannot be used by spectators\n");
		return;
	}

	if (gi.argc() >= 3) {
		buffer = strtok(gi.args()," ");
		buffer = gi.argv(2);
		gi.dprintf("User supplied %s\n", buffer);
		spot = PMSelectSpawnPoint(atoi(buffer));
	} else {
		spot = PMSelectSpawnPoint(0);
	}
	jmodTeleport(ent, spot);
}

void Cmd_GotoPC_f (edict_t *ent)
{
	edict_t 	*spot = NULL;

	if (ent->deadflag || ent->client->pers.spectator) {
		gi.cprintf(ent, PRINT_HIGH, "This command cannot be used by spectators\n");
		return;
	}

	spot = SelectClosestDeathmatchSpawnPoint();
	jmodTeleport(ent, spot);
}

void Cmd_Clear_f(edict_t *ent)
{
	ent->client->resp.jmp_highspeed = 0;
	ent->client->resp.jmp_falldmglast = 0;
	gi.cprintf(ent, PRINT_HIGH, "Statistics cleared\n");
}

void Cmd_Reset_f (edict_t *ent)
{
	VectorClear(ent->client->resp.jmp_teleport_origin);
	VectorClear(ent->client->resp.jmp_teleport_v_angle);
	gi.cprintf(ent, PRINT_HIGH, "Teleport location removed\n");
}

void Cmd_Store_f (edict_t *ent)
{
	if (ent->client->pers.spectator)
	{
		gi.cprintf(ent, PRINT_HIGH, "This command cannot be used by spectators\n");
		return;
	}

	VectorCopy (ent->s.origin, ent->client->resp.jmp_teleport_origin);
	VectorCopy(ent->client->v_angle, ent->client->resp.jmp_teleport_v_angle);

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->resp.jmp_teleport_ducked = true;
	}

	gi.cprintf(ent, PRINT_HIGH, "Location stored\n");
}

void Cmd_Recall_f (edict_t *ent)
{
	int i;

	if (ent->deadflag || ent->client->pers.spectator)
	{
		gi.cprintf(ent, PRINT_HIGH, "This command cannot be used by spectators or dead players\n");
		return;
	}

	if(VectorLength(ent->client->resp.jmp_teleport_origin) == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must first \"store\" a location to teleport to\n");
		return;
	}

	ent->client->jumping = 0;

	ent->movetype = MOVETYPE_NOCLIP;

	/* teleport effect */
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);

	VectorCopy (ent->client->resp.jmp_teleport_origin, ent->s.origin);
	VectorCopy (ent->client->resp.jmp_teleport_origin, ent->s.old_origin);

	VectorClear (ent->velocity);

	ent->client->ps.pmove.pm_time = 160>>3;

	ent->s.event = EV_PLAYER_TELEPORT;

	VectorClear(ent->s.angles);
	VectorClear(ent->client->ps.viewangles);
	VectorClear(ent->client->ps.kick_angles);
	VectorClear(ent->client->v_angle);
	VectorClear(ent->client->ps.pmove.delta_angles);
	VectorClear(ent->client->kick_angles);
	ent->client->fall_time = 0;
	ent->client->fall_value = 0;

	VectorCopy(ent->client->resp.jmp_teleport_v_angle, ent->client->v_angle);
	VectorCopy(ent->client->v_angle, ent->client->ps.viewangles);

	for (i=0;i<2;i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	if (ent->client->resp.jmp_teleport_ducked)
		ent->client->ps.pmove.pm_flags = PMF_DUCKED;

	gi.linkentity (ent);

	/* teleport effect */
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->movetype = MOVETYPE_WALK;
}

void PMLaserSight(edict_t *self)
{
	vec3_t  start,forward,right,end;
	edict_t *lasersight = self->client->lasersight;

	if (lasersight) {  // laser is on
		G_FreeEdict(lasersight);
		self->client->lasersight = NULL;
		return;
	} else {
		AngleVectors (self->client->v_angle, forward, right, NULL);

		VectorSet(end,100 , 0, 0);
		G_ProjectSource (self->s.origin, end, forward, right, start);

		lasersight = G_Spawn();
		self->client->lasersight = lasersight;
		lasersight->owner = self;
		lasersight->movetype = MOVETYPE_NOCLIP;
		lasersight->solid = SOLID_NOT;
		lasersight->classname = "lasersight";
		lasersight->s.modelindex = level.model_lsight;
		lasersight->s.renderfx = RF_TRANSLUCENT;
		lasersight->ideal_yaw = self->viewheight;
		lasersight->count = 0;
		lasersight->think = LaserSightThink;
		lasersight->nextthink = level.framenum + 1;
		LaserSightThink( lasersight );
		VectorCopy( lasersight->s.origin, lasersight->s.old_origin );
		VectorCopy( lasersight->s.origin, lasersight->old_origin );
	}
}

void PMStealthSlippers(edict_t *self)
{
	// Removes stealth slippers
	if (self->client->inventory[ITEM_INDEX(GET_ITEM(SLIP_NUM))]) {
		self->client->inventory[ITEM_INDEX(GET_ITEM(SLIP_NUM))]--;
	} else {
	// Adds stealth slippers
		AddItem(self, GET_ITEM(SLIP_NUM));
	}
}

//PaTMaN - New Toggle Command
void Cmd_Toggle_f(edict_t *ent, char *toggle)
{
	//char	*s;
	char	ACT  [2][12] = { "Deactivated\0","Activated\0" };
	//char	ENA  [2][9]  = { "Disabled\0","Enabled\0" };
	int		spec, val=0;

	//s = strtok(gi.args()," ");

	//if ((gi.argc() == 1) && (s == NULL))
	// if (toggle)
	// {
	// 		gi.cprintf(ent,PRINT_HIGH,"Options to toggle: laser, vest, slippers, silencer, helmet, ir\n");
	// 		return;
	// }

	// if (!Q_stricmp(s,"togglecode"))
	// 	s = strtok(NULL," ");
	// else
	// 	s = strtok(gi.args()," ");

	spec = ent->client->pers.spectator;

	if ( Q_stricmp(toggle, "laser") == 0 )
	{
		if (spec) goto spec;
		if (ent->client->resp.toggles & TG_LASER) {
			ent->client->resp.toggles -= TG_LASER;
			PMLaserSight(ent); 
		} else { 
			ent->client->resp.toggles += TG_LASER;
			val=1;
			PMLaserSight(ent); 
		}
		gi.cprintf(ent, PRINT_HIGH, "Laser %s\n",ACT[val]);
	}
	else if ( Q_stricmp(toggle, "slippers") == 0 )
	{
		if (spec) goto spec;
		if (ent->client->resp.toggles & TG_SLIPPERS) {
			ent->client->resp.toggles -= TG_SLIPPERS;
			PMStealthSlippers(ent);
			//ent->client->inventory[ITEM_INDEX(GET_ITEM(SLIP_NUM))]--;
		}
		else { 
			ent->client->resp.toggles += TG_SLIPPERS;
			val=1;
			PMStealthSlippers(ent);
			//AddItem(ent, GET_ITEM(SLIP_NUM));
		}
		gi.cprintf(ent, PRINT_HIGH, "Slippers %s\n",ACT[val]);
	}
	// else if ( Q_stricmp(s, "kickable") == 0 )
	// {
	// 	if (spec) goto spec;
	// 	if (ent->client->resp.toggles & TG_KICKABLE)
	// 	{
	// 		ent->client->resp.toggles -= TG_KICKABLE;
	// 		gi.cprintf(ent,PRINT_HIGH, "You are now NOT KICKABLE\n");
	// 	}
	// 	else
	// 	{
	// 		ent->client->resp.toggles += TG_KICKABLE;
	// 		gi.cprintf(ent,PRINT_HIGH, "You are now KICKABLE\n");
	// 	}
	// }
	else
		gi.cprintf(ent, PRINT_HIGH, "\"%s\" isn't a valid toggle option\n",toggle);
	return;
spec:
	gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
}
