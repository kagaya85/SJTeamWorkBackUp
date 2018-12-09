#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame r;
    event_type event;

    dl.wait_others();    

    while(true)
    {
        dl.wait_for_event(&event);
        dl.from_physical_layer(&r);
        // pid = getPidByName("netwo");
        // if(pid < 0)
        //         return 0;
        dl.to_network_layer(&r.info);
    }

    return 0;
}