#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame s;
    packet buffer;

    while(true)
    {
        dl.from_network_layer(&buffer);
        s.info = buffer;
        dl.to_physical_layer(&s);
    }

    return 0;
}