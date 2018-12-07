#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame r;
    event_type event;

    while(true)
    {
        wait_for_event(&event);
        dl.from_physical_layer(&r);
        dl.to_network_layer(&r.info);
    }

    return 0;
}