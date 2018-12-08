#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/common.h"
#include "../common/network.h"

using namespace std;

void FillPAD(packet &Packet, const int startPostition);

int main(const int argc, const char* argv[])
{
    if(argc != 2)
	{
		printf("Usage:%s [filename]\n", argv[0]);
		return 1;
	}

    ifstream fin(argv[1], ios::read | ios::binary | ios::nocreate);
    if(!fin.is_open())
    {
        cerr << "File open failed: " << strerror(errno) << endl;
        return -1;
    }

    Network NetworkLayer;
    packet Packet;
    unsigned char FileBuffer[MAX_PKT];
    int bufferLen;
    int EndPackFlag = 0;

    while(TaihouDaisuki)
    {
        if(NetworkLayer.state() == Disable)
        {
            unsigned int LastSec = 1000000;
            while(LastSec)
                LastSec = usleep(LastSec);
            continue;
        }

        if(EndPackFlag) // send EndPacket
        {
            memcpy(Pocket.data, EndPacket, MAX_PKT);
            NetworkLayer.to_datalink_layer(&Packet, MAX_PKT);
            break;
        }

        fin.read(FileBuffer, MAX_PKT);
        bufferLen = fin.gcount();
        if(bufferLen < MAX_PKT) //file read finish
        {
            if(bufferLen) // not single EOF
            {
                FillPAD(FileBuffer);
                memcpy(Packet.data, FileBuffer, MAX_PKT);
                NetworkLayer.to_datalink_layer(&Packet, MAX_PKT);
                
                EndPackFlag = 1; // Next Round Sned EndPacket
            }
            else // single EOF, send EndPacket
            {
                memcpy(Pocket.data, EndPacket, MAX_PKT);
                NetworkLayer.to_datalink_layer(&Packet, MAX_PKT);
                break;
            }   
        }
        else
        {
            memcpy(Packet.data, FileBuffer, MAX_PKT);
            NetworkLayer.to_datalink_layer(&Packet);
        }
    }

    cout << "Sender: File Send Finish." << endl;
    fin.close();
    return 0;
}

void FillPAD(packet &Packet, const int startPostition)
{
    for(int i = startPosition; i < MAX_PKT; ++i)
        Packet.data[i] = PADbyte;
}
