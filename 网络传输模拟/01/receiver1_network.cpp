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

int isEndPacket(const packet &Packet);
void RemovePAD(packet &Packet, int &len);

int main(const int argc, const char* argv[])
{
    if(argc != 2)
	{
		printf("Usage:%s [filename]\n", argv[0]);
		return 1;
	}

    ofstream fout(argv[1], ios::out | ios::binary | ios::trunc);
    if(!fout.is_open())
    {
        cerr << "File Create Failed: " << strerror(errno) << endl;
        return -1;
    }

    Network NetworkLayer;
    packet Lastpacket, Curpacket;
    int packetLen;
    memset(Lastpacket.data(), 0, MAX_PKT);

    while(TaihouDaisuki)
    {
        NetworkLayer.from_datalink_layer(&Curpacket);
        if(isEndPacket(Curpacket))
        {
            //write last packet into file
            int LastPackLen = MAX_PKT;
            RemovePAD(Lastpacket, LastPackLen);
            fout.write(Lastpacket.data, LastPackLen);
            break;
        }
        else
        {
            if(!isEndPacket(Lastpacket))
                fout.write(Lastpacket.data, MAX_PKT);
            Lastpacket = Curpacket;
        }
    }

    cout << "Receiver: File Exchange Finish." << endl;
    fout.close();
    return 0;
}

int isEndPacket(const packet &Packet)
{
    for(int i = 0; i < MAX_PKT; ++i)
        if(Packet.data[i] != '\0')
            return 0;
    return 1;
}
void RemovePAD(packet &Packet, int &len)
{
    for(int i = MAX_PKT - 1; i; --i)
    {
        if(Packet.data[i] == PADbyte)
            --len;
        else
            break;
    }
    return;
}
