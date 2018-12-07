#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame s;
    packet buffer;
    event_type event;

    while(true)
    {
        dl.from_network_layer(&buffer);
        s.info = buffer;
        dl.to_physical_layer(&s);
        wait_for_event(&event);
    }

    return 0;
}