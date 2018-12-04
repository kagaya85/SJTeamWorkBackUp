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