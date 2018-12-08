#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame r;
    event_type event;

    // 等待另外两个进程开启
    pid_t pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("network");
    }
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("physical");
    }

    while(true)
    {
        dl.wait_for_event(&event);
        dl.from_physical_layer(&r);
        dl.to_network_layer(&r.info);
    }

    return 0;
}