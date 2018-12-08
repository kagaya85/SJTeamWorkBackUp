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

int main(const int argc, const char* argv[])
{
    if(argc != 3)
	{
		printf("Usage:%s [send-filename] [receive-filename]\n", argv[0]);
		return 1;
	}

    ifstream fin(argv[1], ios::in | ios::binary);
    if(!fin.is_open())
    {
        cerr << "File Open Failed: " << strerror(errno) << endl;
        return -1;
    }
    ofstream fout(argv[2], ios::out | ios::binary | ios::trunc);
    if(!fout.is_open())
    {
        cerr << "File Create Failed: " << strerror(errno) << endl;
        fin.close();
        return -1;
    }

    // send
    packet Packet;
    unsigned char FileBuffer[MAX_PKT];
    int bufferLen;
    int EndPackFlag = 0;
    int SndOK = 0;
    // receive
    Network NetworkLayer;
    packet Lastpacket, Curpacket;
    int packetLen;
    memset(Lastpacket.data, 0, MAX_PKT);
    int RecvOK = 0;

    while(TaihouDaisuki)
    {
        if(!SndOK && NetworkLayer.status() == Enable) 
            // send packet to network layer
        {
            if(EndPackFlag) // send EndPacket
            {
                memcpy(Packet.data, EndPacket, MAX_PKT);
                NetworkLayer.to_datalink_layer(&Packet);
                SndOK = 1;
            }

            fin.read((char *)FileBuffer, MAX_PKT);
            bufferLen = fin.gcount();
            if(bufferLen < MAX_PKT) //file read finish
            {
                if(bufferLen) // not single EOF
                {
                    FillPAD(FileBuffer, bufferLen);
                    memcpy(Packet.data, FileBuffer, MAX_PKT);
                    NetworkLayer.to_datalink_layer(&Packet);
                
                    EndPackFlag = 1; // Next Round Sned EndPacket
                }
                else // single EOF, send EndPacket
                {
                    memcpy(Packet.data, EndPacket, MAX_PKT);
                    NetworkLayer.to_datalink_layer(&Packet);
                    SndOK;
                }   
            }
        }
        else
        {
            memcpy(Packet.data, FileBuffer, MAX_PKT);
            NetworkLayer.to_datalink_layer(&Packet);
        }
        }
        if(!RecvOK && NetworkLayer.from_datalink_layer(&Curpacket) == 0) 
            // receive packet from network layer
        {
            if(isEndPacket(Curpacket))
            {
                //write last packet into file
                int LastPackLen = MAX_PKT;
                RemovePAD(Lastpacket, LastPackLen);
                fout.write((char *)Lastpacket.data, LastPackLen);
                SndOK = 1;
            }
            else
            {
                if(!isEndPacket(Lastpacket))
                    fout.write((char *)Lastpacket.data, MAX_PKT);
                Lastpacket = Curpacket;
            }
        }
        if(SndOK && RecvOK)
            break;
    }

    cout << "NetworkLayer: File Exchange Finish." << endl;
    fin.close();
    fout.close();
    return 0;
}


