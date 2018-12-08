#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame s;
    packet buffer;

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
        dl.from_network_layer(&buffer);
        s.info = buffer;
        dl.to_physical_layer(&s);
    }

    return 0;
}