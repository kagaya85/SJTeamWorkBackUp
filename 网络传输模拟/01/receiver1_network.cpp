#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include <errno.h>
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

#include "../common/network.h"

using namespace std;

int isEndPacket(const packet &Packet);
void RemovePAD(packet &Packet, int &len);

int main(const int argc, const char* argv[])
{
    if(argc != 2)
	{
		printf("Usage:%s [reeive-filename]\n", argv[0]);
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
    memset(Lastpacket.data, 0, MAX_PKT);

    while(TaihouDaisuki)
    {
        int res = NetworkLayer.from_datalink_layer(&Curpacket);
        if(res == -1) // from datalink timeout
        {
            unsigned int LastSec = 1000000;
            while(LastSec)
                LastSec = usleep(LastSec);
            continue;
        }
        if(isEndPacket(Curpacket))
        {
            //write last packet into file
            int LastPackLen = MAX_PKT;
            RemovePAD(Lastpacket, LastPackLen);
            fout.write((char *)Lastpacket.data, LastPackLen);
            break;
        }
        else
        {
            if(!isEndPacket(Lastpacket))
                fout.write((char *)Lastpacket.data, MAX_PKT);
            Lastpacket = Curpacket;
        }
    }

    cout << "NetworkLayer Receiver: File Exchange Finish." << endl;
    fout.close();
    return 0;
}
