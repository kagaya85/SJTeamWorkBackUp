#ifndef NETWORK
#define NETWORK

#define PADbyte 0

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <cstdio>

#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "common.h"

const char EndPacket[MAX_PKT] = {0};

class Network {
private:
    seq_nr NetworkDatalinkSeq;
    seq_nr DatalinkNetworkSeq;
    static layer_status NetworkStatus;
public:
    Network();
    ~Network();
    void seq_inc(seq_nr k);
    layer_status status();
    void to_datalink_layer(packet *pkt);
    int from_datalink_layer(packet *pkt);
    void network_layer_ready();
    /* 信号处理函数 */
    static void sig_enable_handle(int signal);
    static void sig_disable_handle(int signal);
};


//----------common function----------
int isEndPacket(const packet &Packet);
void RemovePAD(packet &Packet, int &len);
void FillPAD(unsigned char * const Packet, const int startPosition);

const char* To_Datalink_Dir = "./toDatalinkCache";
const char* To_Network_Dir = "./toNetworkCache";

#endif // NETWORK
