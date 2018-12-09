#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame r, s;
    event_type event;

    dl.wait_others();    
    s.kind = AckFrame;

    while(true)
    {
        dl.wait_for_event(&event);
        dl.from_physical_layer(&r);
        dl.to_network_layer(&r.info);
        dl.to_physical_layer(&s);
    }

    return 0;
}