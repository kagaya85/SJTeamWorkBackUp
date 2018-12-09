#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame s;
    packet buffer;
    event_type event;

    dl.wait_others();    

    while(true)
    {
        dl.from_network_layer(&buffer);
        s.info = buffer;
        dl.to_physical_layer(&s);
        dl.wait_for_event(&event);
        dl.from_physical_layer(&s);
    }

    return 0;
}