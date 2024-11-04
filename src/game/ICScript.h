#ifndef ICSCRIPT_H
#define ICSCRIPT_H

typedef int qboolean;

typedef struct ICScript_s
{
	int startFrame;
	char *buf;
	int bufSize;
	int count;
} ICScript_t;

void ICScript_Con(ICScript_t *this_ptr, char *name);
void RunICScript();
void KillICScript();


#endif