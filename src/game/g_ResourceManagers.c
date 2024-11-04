void InitMsgMngr();
void ReleaseMsgMngr();
#if	G_NEW_SYSTEM
void InitBoundingFormMngr();
void ReleaseBoundingFormMngr();
#endif

void G_InitResourceManagers()
{
	InitMsgMngr();
#if	G_NEW_SYSTEM
	InitBoundingFormMngr();
#endif
}

void G_ReleaseResourceManagers()
{
	ReleaseMsgMngr();
#if	G_NEW_SYSTEM
	ReleaseBoundingFormMngr();
#endif
}