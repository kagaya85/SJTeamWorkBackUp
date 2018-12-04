#include "common.h"

void send_frame_arrival()
{
    pid_t pid;
    pid = getPidByName("datalink");
    kill(pid, SIG_FRAME_ARRIVAL);
}