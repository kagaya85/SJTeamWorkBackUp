#ifndef NETWORK
#define NETWORK

#define PADbyte 0

#include <unistd.h>
#include <cstdlib>

#include "common.h"

using namespace std;

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



int isEndPacket(const packet &Packet);
void RemovePAD(packet &Packet, int &len);

void FillPAD(unsigned char * const Packet, const int startPosition)；

#endif // NETWORK
