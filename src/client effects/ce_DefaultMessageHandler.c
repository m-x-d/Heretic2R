#include "ce_DefaultMessageHandler.h"
#include "Client Entities.h"

CE_MsgReceiver_t DefaultMessageReceivers[NUM_MESSAGES] =
{
	NULL,
};

void CE_DefaultMsgHandler(client_entity_t *self, CE_Message_t *msg)
{
	CE_MsgReceiver_t receiver;

	receiver = classStatics[self->classID].msgReceivers[msg->ID];

	if(receiver)
	{
		receiver(self, msg);
	}
	else
	{
		// if and when there are a good number of defaults, change the NULL to be an Empty
		// function, overall that should be faster to just always call the function then
		// do the check
		receiver = DefaultMessageReceivers[msg->ID];

		if(receiver)
		{
			DefaultMessageReceivers[msg->ID](self, msg);
		}
	}
}