#pragma once

#include "yojimbo.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

using namespace yojimbo;

const uint64_t protocol_id = 0x11223344556677ULL;
const int client_port = 30000;
const int server_port = 40000;

struct FooMessage : public Message 
{
    uint16_t foo;

    FooMessage() 
    {
        foo = 0;
    }

    template <typename Stream> bool Serialize(Stream &stream)
    {
        serialize_bits(stream, foo, 16);
        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct FooBlockMessage : public BlockMessage
{
    uint16_t foo;

    FooBlockMessage()
    {
        foo = 0;
    }

    template <typename Stream> bool Serialize(Stream &stream)
    {
        serialize_bits(stream, foo, 16);
        return true;
    }
    
    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

enum FooMessageType
{
    FOO_MESSAGE,
    FOO_BLOCK_MESSAGE,
    NUM_FOO_MESSAGE_TYPES
};

YOJIMBO_MESSAGE_FACTORY_START(FooMessageFactory, NUM_FOO_MESSAGE_TYPES);
    YOJIMBO_DECLARE_MESSAGE_TYPE(FOO_MESSAGE, FooMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE(FOO_BLOCK_MESSAGE, FooBlockMessage );
YOJIMBO_MESSAGE_FACTORY_FINISH();

class FooAdapter : public Adapter
{
    public:
        MessageFactory * CreateMessageFactory(Allocator &allocator)
        {
            return YOJIMBO_NEW(allocator, FooMessageFactory, allocator);
        }
};

static FooAdapter foo_adapter;
