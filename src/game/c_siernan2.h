typedef enum AnimID_e
{	
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_IDLE1,
	NUM_ANIMS
} AnimID_t;

void MG_InitMoods(edict_t *self);

extern animmove_t siernan2_move_c_action1;
extern animmove_t siernan2_move_c_action2;
extern animmove_t siernan2_move_c_idle1; 
