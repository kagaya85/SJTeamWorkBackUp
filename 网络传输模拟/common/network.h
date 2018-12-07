#ifndef NETWORK
#define NETWORK

#include <cstdlib>
#include "common.h"

using namespace std;

class Network {
private:
    static seq_nr NetworkDatalinkSeq;
    static layer_status networkStatus;
public:
    Network(char* const filename);
    ~Network();
    void network_layer_ready();
    void seq_inc(seq_nr k);
    static void sig_enable_handle(int signal);
    static void sig_disable_handle(int signal);
};
#endif // NETWORK