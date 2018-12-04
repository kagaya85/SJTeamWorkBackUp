#include <cstdlib>
#include "common.h"

void network_layer_ready()
{
    pid_t pid;
    pid = getPidByName("datalink");
    kill(pid, SIG_NETWORK_LAYER_READY);
}