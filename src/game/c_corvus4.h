typedef enum AnimID_e
{	
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_ACTION4,
	ANIM_C_ACTION5,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_WALKSTART,
	ANIM_C_WALK1,
	ANIM_C_WALK2,
	ANIM_C_WALKSTOP1,
	ANIM_C_WALKSTOP2,
	ANIM_C_PIVOTLEFTGO,
	ANIM_C_PIVOTLEFT,
	ANIM_C_PIVOTLEFTSTOP,
	ANIM_C_PIVOTRIGHTGO,
	ANIM_C_PIVOTRIGHT,
	ANIM_C_PIVOTRIGHTSTOP,
	NUM_ANIMS
} AnimID_t;

void MG_InitMoods(edict_t *self);



extern animmove_t corvus4_move_c_action1;
extern animmove_t corvus4_move_c_action2;
extern animmove_t corvus4_move_c_action3;
extern animmove_t corvus4_move_c_action4;
extern animmove_t corvus4_move_c_action5;
extern animmove_t corvus4_move_c_idle1; 
extern animmove_t corvus4_move_c_idle2; 
extern animmove_t corvus4_move_c_walkstart; 
extern animmove_t corvus4_move_c_walk1; 
extern animmove_t corvus4_move_c_walk2; 
extern animmove_t corvus4_move_c_walkstop1; 
extern animmove_t corvus4_move_c_walkstop2; 
extern animmove_t corvus4_move_c_pivotleftgo;
extern animmove_t corvus4_move_c_pivotleft;
extern animmove_t corvus4_move_c_pivotleftstop;
extern animmove_t corvus4_move_c_pivotrightgo;
extern animmove_t corvus4_move_c_pivotright;
extern animmove_t corvus4_move_c_pivotrightstop;
