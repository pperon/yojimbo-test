/*
    Yojimbo Client Example (insecure)

    Copyright © 2016 - 2017, The Network Protocol Company, Inc.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
           in the documentation and/or other materials provided with the distribution.

        3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived 
           from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "yojimbo.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <signal.h>
//#include "shared.h"
#include "foo.h"

#define RELIABLE_CHANNEL 0
#define UNRELIABLE_CHANNEL 1

using namespace yojimbo;

static volatile int quit = 0;
const int MaxPacketSize = 16 * 1024;
const int MaxSnapshotSize = 8 * 1024;
const int MaxBlockSize = 64 * 1024;
static const int UNRELIABLE_UNORDERED_CHANNEL = 0;
static const int RELIABLE_ORDERED_CHANNEL = 1;

void interrupt_handler( int /*dummy*/ )
{
    quit = 1;
}

int ClientMain( int argc, char * argv[] )
{   
    printf( "\nconnecting client (insecure)\n" );

    double time = 100.0;

    uint64_t clientId = 0;
    random_bytes( (uint8_t*) &clientId, 8 );
    printf( "client id is %.16" PRIx64 "\n", clientId );

    ClientServerConfig config;
    config.maxPacketSize = MaxPacketSize;
    config.clientMemory = 10 * 1024 * 1024;
    config.serverGlobalMemory = 10 * 1024 * 1024;
    config.serverPerClientMemory = 10 * 1024 * 1024;
    config.numChannels = 2;
    config.channel[RELIABLE_ORDERED_CHANNEL].type = CHANNEL_TYPE_RELIABLE_ORDERED;
    config.channel[RELIABLE_ORDERED_CHANNEL].maxBlockSize = MaxBlockSize;
    config.channel[RELIABLE_ORDERED_CHANNEL].blockFragmentSize = 1024;
    config.channel[UNRELIABLE_UNORDERED_CHANNEL].type = CHANNEL_TYPE_UNRELIABLE_UNORDERED;
    config.channel[UNRELIABLE_UNORDERED_CHANNEL].maxBlockSize = MaxSnapshotSize;

    Client client( GetDefaultAllocator(), Address("0.0.0.0"), config, foo_adapter, time );
    client.SetLatency(0.0f);
    client.SetJitter(0.0f);

    Address serverAddress( "127.0.0.1", server_port );

    if ( argc == 2 )
    {
        Address commandLineAddress( argv[1] );
        if ( commandLineAddress.IsValid() )
        {
            if ( commandLineAddress.GetPort() == 0 )
                commandLineAddress.SetPort( server_port );
            serverAddress = commandLineAddress;
        }
    }

    uint8_t privateKey[KeyBytes];
    memset( privateKey, 0, KeyBytes );

    client.InsecureConnect( privateKey, clientId, serverAddress );

    char addressString[256];
    client.GetAddress().ToString( addressString, sizeof( addressString ) );
    printf( "client address is %s\n", addressString );

    const double deltaTime = 0.01f;

    signal( SIGINT, interrupt_handler );

    while ( !quit )
    {
        client.SendPackets();
        client.ReceivePackets();
        while(true) {
            //* Receive UNRELIABLE messages
            Message *unreliable_message = client.ReceiveMessage(UNRELIABLE_CHANNEL);

            if(!unreliable_message) {
                break;
            }
            printf("Received unreliable message\n");
            switch(unreliable_message->GetType()) {
                case TEST_MESSAGE:
                {
                    TestMessage *testMessage = (TestMessage *)unreliable_message;
                    (void)testMessage;
                    printf("Received unreliable message with sequence\n");
                    break;
                }
                case FOO_MESSAGE:
                {
                    FooMessage *foo_message = (FooMessage *)unreliable_message;
                    (void)foo_message;
                    printf("GOT UNRELIABLE FOO YO \n");
                    break;
                }
                case FOO_BLOCK_MESSAGE:
                {
                    FooBlockMessage *foo_block_message = (FooBlockMessage *)unreliable_message;
                    (void)foo_block_message;
                    printf("GOT UNRELIABLE BLOCK!\n");
                    break;
                }
                break;
            }
            client.ReleaseMessage(unreliable_message);
            //*/ 
            // Receive RELIABLE messages
            Message *reliable_message = client.ReceiveMessage(RELIABLE_CHANNEL);
            if(!reliable_message) {
                break;
            }
            printf("Received reliable message\n");
            switch(reliable_message->GetType()) {
                
                case TEST_MESSAGE:
                {
                    TestMessage *testMessage = (TestMessage *)reliable_message;
                    (void)testMessage;
                    printf("Received message with sequence\n");
                }
                break;

                case FOO_MESSAGE:
                {
                    FooMessage *foo_message = (FooMessage *)reliable_message;
                    (void)foo_message;
                    printf("GOT FOO YO \n");
                }
                break;

                case FOO_BLOCK_MESSAGE:
                {
                    FooBlockMessage *foo_block_message = (FooBlockMessage *)reliable_message;
                    (void)foo_block_message;
                    printf("GOT BLOCK!\n");
                }
                break;

                case TEST_BLOCK_MESSAGE:
                {
                    TestBlockMessage *test_block_message = (TestBlockMessage *)reliable_message;
                    (void)test_block_message;
                    printf("GOT TEST BLOCK MESSAGE\n");
                }
                break;
            }
            client.ReleaseMessage(reliable_message);
        }

        if ( client.IsDisconnected() )
            break;
     
        time += deltaTime;

        client.AdvanceTime( time );

        if ( client.ConnectionFailed() )
            break;

        yojimbo_sleep( deltaTime );
    }

    client.Disconnect();

    return 0;
}

int main( int argc, char * argv[] )
{
    if ( !InitializeYojimbo() )
    {
        printf( "error: failed to initialize Yojimbo!\n" );
        return 1;
    }

    yojimbo_log_level( YOJIMBO_LOG_LEVEL_INFO );

    srand( (unsigned int) time( NULL ) );

    int result = ClientMain( argc, argv );

    ShutdownYojimbo();

    printf( "\n" );

    return result;
}
