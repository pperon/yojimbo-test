/*
    Yojimbo Server Example (insecure)

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
#include <signal.h>
#include <time.h>

#include "shared.h"
#include "foo.h"

#define RELIABLE_CHANNEL 0
#define UNRELIABLE_CHANNEL 1

using namespace yojimbo;

static volatile int quit = 0;

void interrupt_handler( int /*dummy*/ )
{
    quit = 1;
}

int ServerMain()
{
    printf( "started server on port %d (insecure)\n", ServerPort );

    double time = 100.0;

    ClientServerConfig config;
    config.numChannels = 2;
    config.channel[0].type = CHANNEL_TYPE_RELIABLE_ORDERED;
    config.channel[1].type = CHANNEL_TYPE_UNRELIABLE_UNORDERED;

    uint8_t privateKey[KeyBytes];
    memset( privateKey, 0, KeyBytes );
    
    Server server( GetDefaultAllocator(), privateKey, Address( "127.0.0.1", ServerPort ), config, adapter, time );
    
    server.Start( MaxClients );
    
    server.SetLatency(250.0f);
    server.SetJitter(250.0f);
    server.SetPacketLoss(50.0f);

    char addressString[256];
    server.GetAddress().ToString( addressString, sizeof( addressString ) );
    printf( "server address is %s\n", addressString );

    const double deltaTime = 0.01f;

    signal( SIGINT, interrupt_handler ); 

    uint16_t  sequence = 0;

    while ( !quit )
    {
        server.SendPackets();
        server.ReceivePackets();
        
        for(int i = 0; i < MaxClients; i++) {
            if(server.IsClientConnected(i)) {
                TestMessage *message = (TestMessage *)server.CreateMessage(i, TEST_MESSAGE);
                if(message) {
                    message->sequence = sequence;
                    printf("sending message with sequence: %d\n", sequence);
                    server.SendMessage(i, UNRELIABLE_CHANNEL, message);
                    sequence++;
                }
            }
        }

        time += deltaTime;
        server.AdvanceTime( time );

        if ( !server.IsRunning() )
            break;

        yojimbo_sleep( deltaTime );
    }

    server.Stop();

    return 0;
}

int main()
{
    printf( "\n" );

    if ( !InitializeYojimbo() )
    {
        printf( "error: failed to initialize Yojimbo!\n" );
        return 1;
    }

    yojimbo_log_level( YOJIMBO_LOG_LEVEL_INFO );

    srand( (unsigned int) time( NULL ) );


    int result = ServerMain();

    ShutdownYojimbo();

    printf( "\n" );

    return result;
}
