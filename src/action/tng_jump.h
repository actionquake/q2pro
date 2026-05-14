#define STAT_SPEEDX					1
#define STAT_HIGHSPEED					2
#define STAT_FALLDMGLAST				3

extern char *jump_statusbar;
extern cvar_t *jump;

void Jmp_EquipClient(edict_t *ent);
void Jmp_SetStats(edict_t *ent);

void Cmd_Jmod_f (edict_t *ent);
void Cmd_PMLCA_f (edict_t *ent);
void Cmd_RHS_f (edict_t *ent);
void Cmd_Goto_f (edict_t *ent);
void Cmd_GotoP_f_compat (edict_t *ent, pmenu_t *p);
void Cmd_GotoPC_f_compat (edict_t *ent, pmenu_t *p);
void Cmd_GotoP_f (edict_t *ent);
void Cmd_GotoPC_f (edict_t *ent);
void Cmd_Clear_f (edict_t *ent);
void Cmd_Reset_f (edict_t *ent);
void Cmd_Store_f (edict_t *ent);
void Cmd_Recall_f (edict_t *ent);
void Cmd_Toggle_f(edict_t *ent, char *toggle);
void PMLaserSight(edict_t *ent);
void PMStealthSlippers(edict_t *ent);