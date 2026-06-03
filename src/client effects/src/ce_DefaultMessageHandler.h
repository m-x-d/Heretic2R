//
// ce_DefaultMessageHandler.h
//
// Copyright 1998 Raven Software
//

#pragma once

struct client_entity_s; // Linux port: file-scope forward decl for GCC tag scoping.

#include "ce_Message.h"

extern void CE_DefaultMsgHandler(struct client_entity_s* self, CE_Message_t* msg);
