#include "../common/network.h"

using namespace std;

int main(const int argc, const char* argv[])
{
    // 等待其他进程开启
    pid_t pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("datal");
    }
    cout << "Network: " << "get datalink pid " << pid << endl;
    
    if(argc != 2)
	{
		printf("Usage:%s [send-filename]\n", argv[0]);
		return 1;
	}

    ifstream fin(argv[1], ios::in | ios::binary);
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
        if(NetworkLayer.status() == Disable)
        {
            unsigned int LastSec = 1000000;
            while(LastSec)
                LastSec = usleep(LastSec);
            continue;
        }

        // cout << "Sender Network: ready to pick packet" << endl;

        if(EndPackFlag) // send EndPacket
        {
            memcpy(Packet.data, EndPacket, MAX_PKT);
            NetworkLayer.to_datalink_layer(&Packet);
            break;
        }

        fin.read((char *)FileBuffer, MAX_PKT);
        bufferLen = fin.gcount();

        cout << "Sender Network: pick packet from file " << bufferLen << " byte(s)" << endl;

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
                break;
            }   
        }
        else
        {
            memcpy(Packet.data, FileBuffer, MAX_PKT);
            NetworkLayer.to_datalink_layer(&Packet);
        }
    }

    cout << "NetworkLayer Sender: File Send Finish." << endl;
    fin.close();
    return 0;
}
