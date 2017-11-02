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
const int stuff_byte_count = 2032;

inline int GetNumBitsForMessage( uint16_t sequence )
{
    static int messageBitsArray[] = { 1, 320, 120, 4, 256, 45, 11, 13, 101, 100, 84, 95, 203, 2, 3, 8, 512, 5, 3, 7, 50 };
    const int modulus = sizeof( messageBitsArray ) / sizeof( int );
    const int index = sequence % modulus;
    return messageBitsArray[index];
}

struct Bar {
    int x;
    int y;
    int z;
    uint8_t stuff[stuff_byte_count];
};

struct BarObject : public Serializable
{
    Bar data;

    void Init()
    {
        data.x = 1;
        data.y = 2;
        data.z = 3;

        for(int i = 0; i < stuff_byte_count; i++) {
            data.stuff[i] = i;
        }
    }

    template <typename Stream> bool Serialize(Stream &stream)
    {
        serialize_int(stream, data.x, 0, 10);
        serialize_int(stream, data.y, 0, 10);
        serialize_bits(stream, data.z, 2);
        serialize_bytes(stream, data.stuff, stuff_byte_count); 
        // Do I need this? --> serialize_align(stream);
        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct TestMessage : public Message
{
    uint16_t sequence;

    TestMessage()
    {
        sequence = 0;
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {        
        serialize_bits( stream, sequence, 16 );

        int numBits = GetNumBitsForMessage( sequence );
        int numWords = numBits / 32;
        uint32_t dummy = 0;
        for ( int i = 0; i < numWords; ++i )
            serialize_bits( stream, dummy, 32 );
        int numRemainderBits = numBits - numWords * 32;
        if ( numRemainderBits > 0 )
            serialize_bits( stream, dummy, numRemainderBits );

        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct FooMessage : public Message 
{
    uint16_t foo;

    FooMessage() 
    {
        foo = 42;
    }

    template <typename Stream> bool Serialize(Stream &stream)
    {
        serialize_bits(stream, foo, 16);
        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct TestBlockMessage : public BlockMessage
{
    uint16_t sequence;

    TestBlockMessage()
    {
        sequence = 0;
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {        
        serialize_bits( stream, sequence, 16 );
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
    TEST_MESSAGE,
    TEST_BLOCK_MESSAGE,
    NUM_FOO_MESSAGE_TYPES
};

YOJIMBO_MESSAGE_FACTORY_START(FooMessageFactory, NUM_FOO_MESSAGE_TYPES);
    YOJIMBO_DECLARE_MESSAGE_TYPE(FOO_MESSAGE, FooMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE(FOO_BLOCK_MESSAGE, FooBlockMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE(TEST_MESSAGE, TestMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE(TEST_BLOCK_MESSAGE, TestBlockMessage );
YOJIMBO_MESSAGE_FACTORY_FINISH();

class FooAdapter : public Adapter
{
    public:
        MessageFactory * CreateMessageFactory(Allocator &allocator)
        {
            return YOJIMBO_NEW(allocator, FooMessageFactory, allocator);
        }

        void OnServerClientConnected(int clientIndex)
        {
            printf("CONNECTED! WEEEEEEEEEE!!!\n");
        }

        void OnServerClientDisconnected(int clientIdex)
        {
            printf("DISCONNECTED! Awww.\n");
        }
};

static FooAdapter foo_adapter;
