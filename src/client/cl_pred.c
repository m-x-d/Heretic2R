//
// cl_pred.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

// Sets cl.predicted_origin and cl.predicted_angles
void CL_PredictMovement(void)
{
	//mxd. Skip when unable to render //TODO: more reliable way to check we are ingame (sv.state does this)?
	if (!cl.refresh_prepped)
		return;

	if ((int)cl_paused->value || (int)cl_freezeworld->value || cl.cinematictime > 0 || cl.frame.playerstate.cinematicfreeze)
		return;

	NOT_IMPLEMENTED
}