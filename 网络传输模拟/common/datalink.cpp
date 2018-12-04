#include "common.h"

void enable_network_layer()
{
    pid_t pid;
    pid = getPidByName("network");
    kill(pid, SIG_NETWORK_LAYER_ENABLE);
}

void disable_network_layer()
{
    pid_t pid;
    pid = getPidByName("network");
    kill(pid, SIG_NETWORK_LAYER_DISABLE);
}

void from_network_layer(packet *pkt)
{
    
}

void start_timer(seq_nr k)
{

}

void stop_timer(seq_nr k)
{

}

void start_ack_timer()
{

}

void stop_ack_timer()
{

}
