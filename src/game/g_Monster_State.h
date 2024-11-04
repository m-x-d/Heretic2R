//----------------------------------------------------
// Movement State types
//----------------------------------------------------

enum Move_Type_e	// this seems to be redundant with physicInfo->type, except for hover
{
	MOVE_TYPE_STATIC,	// can't move at all
	MOVE_TYPE_TURN,		// change orientation, but not position
	MOVE_TYPE_STEP,		// stuff with legs
	MOVE_TYPE_SWIM,		// anything submereged
	MOVE_TYPE_FLY,		// flying creatures, and any others when falling or being knocked back through the air
	MOVE_TYPE_AIRBORNE,	// nonflying creatures when falling or being knocked back
	MOVE_TYPE_HOVER,
};

// Step State types

enum Step_DOF_e
{
	STEP_DOF_1,		// forward and back only
	STEP_DOF_2_4,	// cardinal directions only
	STEP_DOF_2_8,	// cardinal directions plus, halves	in between
	STEP_DOF_2,		// true 2DOF
};

enum Step_State_e
{
	STEP_STATE_STILL,			// not stepping
	STEP_STATE_PIVOTING,		// turning left or right
	STEP_STATE_MOVING,			// stepping in some form or another
								// valid only if STEP_READINESS_STANDING || STEP_READINESS_CROUCHING is set 
	STEP_STATE_JUMPING,			// some kind of jump
								// valid only if STEP_READINESS_STANDING is set 
};

enum Step_Readiness_e
{
	STEP_READINESS_STANDING,
	STEP_READINESS_CROUCHING,
	STEP_READINESS_SITTING,
	STEP_READINESS_LYING,
};

//----------------------------------------------------
// Action State types
//----------------------------------------------------

enum Action_State_e
{
	ACTION_STATE_IDLE,			// no action, but could be moving
	ACTION_STATE_MISC,			// getting or using an item
	ACTION_STATE_ACTIVATING,	// activating something
	ACTION_STATE_STRIKING,		// making a melee attack
	ACTION_STATE_SHOOTING,		// making a missile attack
	ACTION_STATE_CASTING,		// casting a spell
};

enum Action_Readiness_e
{
	ACTION_READINESS_NORMAL,
	ACTION_READINESS_PAIN,
	ACTION_READINESS_KNOCKBACK,
	ACTION_READINESS_DEAD,
};

