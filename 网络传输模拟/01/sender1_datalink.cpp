#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame s;
    packet buffer;

    dl.wait_others();    
    
    struct msqid_ds msgbuf;

    while(true)
    {
        // pid = getPidByName("netwo");
        // if(pid < 0)
        //     return 0;
        dl.from_network_layer(&buffer);
        s.info = buffer;
        dl.to_physical_layer(&s);
    }

    return 0;
}