#include "network.h"

Network::Network(char* const filename)
{
    signal(SIG_NETWORKLAYER_ENABLE, Network::sig_enable_handle);

}

Network::~Network()
{

}

void Network::network_layer_ready()
{
    pid_t pid;
    pid = getPidByName("datalink");
    kill(pid, SIG_NETWORK_LAYER_READY);
}

void Network::seq_inc(seq_nr k)
{
    if(k < MAX_SHARE_SEQ) 
        k = k + 1; 
    else 
        k = 0;
}

static void Network::sig_enable_handle(int signal)
{

}

static void Network::sig_disable_handle(int signal)
{

}

