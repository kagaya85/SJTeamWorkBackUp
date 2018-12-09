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
        pid = getPidByName("netwo");
    }
    cout << "Datalink: " << "get network pid " << pid << endl;
    pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("physi");
    }
    cout << "Datalink: " << "get physical pid " << pid << endl;

    while(true)
    {
        //cout << "datalink start wait" << endl;
        dl.wait_for_event(&event);
        //cout << "datalink read from physical" << endl;
        dl.from_physical_layer(&r);
        //cout << "datalink write to network" << endl;
        dl.to_network_layer(&r.info);
    }

    return 0;
}