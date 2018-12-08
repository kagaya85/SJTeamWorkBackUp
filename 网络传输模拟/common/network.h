#ifndef NETWORK
#define NETWORK

#include <cstdlib>
#include "common.h"

using namespace std;

class Network {
private:
    static seq_nr NetworkDatalinkSeq;
    static layer_status NetworkStatus;
public:
    Network();
    ~Network();
    void seq_inc(seq_nr k);
    layer_status status();
    void to_datalink_layer(packet *pkt);
    void from_datalink_layer(packet *pkt);
    void network_layer_ready();
    /* 信号处理函数 */
    static void sig_enable_handle(int signal);
    static void sig_disable_handle(int signal);
};
#endif // NETWORK