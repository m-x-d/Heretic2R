typedef enum AnimID_e
{	
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	NUM_ANIMS
} AnimID_t;

void MG_InitMoods(edict_t *self);

extern animmove_t tome_move_c_idle1; 
extern animmove_t tome_move_c_idle2; 
