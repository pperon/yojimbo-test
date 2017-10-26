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

//#include "shared.h"
#include "foo.h"

#define UNRELIABLE_CHANNEL 0
#define RELIABLE_CHANNEL 1

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

int ServerMain()
{
    printf( "started server on port %d (insecure)\n", server_port );

    double time = 100.0;

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

    uint8_t privateKey[KeyBytes];
    memset( privateKey, 0, KeyBytes );
    
    Server server( GetDefaultAllocator(), privateKey, Address( "127.0.0.1", server_port ), config, foo_adapter, time );
    
    server.Start( MaxClients );
    
    server.SetLatency(0.0f);
    server.SetJitter(0.0f);
    server.SetPacketLoss(0.0f);

    char addressString[256];
    server.GetAddress().ToString( addressString, sizeof( addressString ) );
    printf( "server address is %s\n", addressString );

    const double deltaTime = 0.01f;

    signal( SIGINT, interrupt_handler ); 

    uint16_t  sequence = 0;
    (void)sequence;
    
    /*
    const int bytesWritten = writeStream.GetBytesProcessed();

    BarObject barObjectRead;
    ReadStream readStream(GetDefaultAllocator(), buffer, bytesWritten);
    barObjectRead.Serialize(readStream);
    bar = barObjectRead.data;
    printf("Derp: %d %d %d\n", bar.x, bar.y, bar.z);
    */

    while ( !quit )
    {
        server.SendPackets();
        server.ReceivePackets();
        
        for(int i = 0; i < MaxClients; i++) {
            if(server.IsClientConnected(i)) {

                if(!server.CanSendMessage(i, RELIABLE_ORDERED_CHANNEL)) {
                    break;
                }
                
                //*
                TestBlockMessage * message = (TestBlockMessage*) server.CreateMessage(i, TEST_BLOCK_MESSAGE );
                if(message) {
                    message->sequence = sequence;
                    const int blockSize = 1 + ( ( i * 901 ) % 3333 );
                    uint8_t * blockData = server.AllocateBlock(i, blockSize);//(uint8_t*) YOJIMBO_ALLOCATE( GetDefaultAllocator(), blockSize );
                    if(blockData) {
                        for ( int j = 0; j < blockSize; ++j )
                            blockData[j] = i + j;
                        server.AttachBlockToMessage(i, message, blockData, blockSize);
                        //message->AttachBlock( GetDefaultAllocator(), blockData, blockSize );
                        printf("Sending test block message with length: %d\n", blockSize);
                        server.SendMessage(i, RELIABLE_CHANNEL, message );
                        sequence++;
                    }
                }
                //*/

                /*
                FooMessage *foo_message = (FooMessage *)server.CreateMessage(i, FOO_MESSAGE);
                if(foo_message) {
                    foo_message->foo = 42;
                    printf("sending foo message\n");
                    server.SendMessage(i, RELIABLE_CHANNEL, foo_message);
                }
                //*/ 

                
                /*
                FooBlockMessage *foo_block_message = (FooBlockMessage *)server.CreateMessage(i, FOO_BLOCK_MESSAGE);
                if(foo_block_message) {
                    //foo_block_message->foo = 42;
                    //printf("sending foo block message\n");
                    //const int blockSize = 1000;
                    //uint8_t * blockData = (uint8_t*) YOJIMBO_ALLOCATE( GetDefaultAllocator(), blockSize );
                    //for ( int j = 0; j < blockSize; ++j )
                    //    blockData[j] = i + j;
                    //foo_block_message->AttachBlock( GetDefaultAllocator(), blockData, blockSize );
                    //server.SendMessage(i, RELIABLE_CHANNEL, foo_block_message);

                    // testing object serialization
                    const int BufferSize = 16;
                    uint8_t buffer[BufferSize];
                    WriteStream writeStream(GetDefaultAllocator(), buffer, BufferSize);
                    BarObject barObjectWrote;
                    barObjectWrote.Init();
                    barObjectWrote.Serialize(writeStream);
                    writeStream.Flush();
                    int bytes_processed = writeStream.GetBytesProcessed();
                    (void)bytes_processed;
                    const uint8_t *stream_data = writeStream.GetData();
                    (void)stream_data;
                    
                    // TODO: EXAMPLE Deserialize stream_data into a BarObject
                    BarObject barObjectRead;
                    ReadStream readStream(GetDefaultAllocator(), stream_data, bytes_processed);
                    barObjectRead.Serialize(readStream);
                    
                    //uint8_t *ptr = (uint8_t *)malloc(bytes_processed);
                    uint8_t *ptr = server.AllocateBlock(i, bytes_processed);
                    if(ptr) {
                        memcpy(ptr, stream_data, bytes_processed);
                        server.AttachBlockToMessage(i, foo_block_message, ptr, bytes_processed);
                        server.SendMessage(i, RELIABLE_CHANNEL, foo_block_message);
                    }
                }
                //*/
                
                if(!server.CanSendMessage(i, UNRELIABLE_UNORDERED_CHANNEL)) {
                    break;
                }

                /*
                TestMessage *unreliable_message = (TestMessage *)server.CreateMessage(i, TEST_MESSAGE);
                if(unreliable_message) {
                    //unreliable_message->sequence = sequence;
                    printf("sending unreliable message\n");
                    server.SendMessage(i, UNRELIABLE_CHANNEL, unreliable_message);
                    sequence++;
                }
                //*/
               
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
