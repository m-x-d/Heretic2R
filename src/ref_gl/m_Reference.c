#include "Reference.h"

#ifdef _REF_GL
#include "gl_local.h"
#else
#include "r_local.h"
#endif

#include "fmodel.h"

int GetReferencedID(struct model_s *model)
{
	fmdl_t *temp = model->extradata;

	if (model->model_type)
	{
		if(temp->referenceType < NUM_REFERENCED && temp->referenceType > REF_NULL)
		{
			return temp->referenceType;
		}
	}
	return REF_NULL;
}