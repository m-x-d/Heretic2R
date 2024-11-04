typedef enum AnimID_e
{	
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	ANIM_C_IDLE4,
	NUM_ANIMS
} AnimID_t;

void MG_InitMoods(edict_t *self);

extern animmove_t morcalavin_move_c_action1; 
extern animmove_t morcalavin_move_c_action2; 
extern animmove_t morcalavin_move_c_action3; 
extern animmove_t morcalavin_move_c_idle1; 
extern animmove_t morcalavin_move_c_idle2; 
extern animmove_t morcalavin_move_c_idle3; 
extern animmove_t morcalavin_move_c_idle4; 

