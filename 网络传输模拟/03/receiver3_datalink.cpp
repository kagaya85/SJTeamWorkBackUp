#include "../common/datalink.h"
#define MAX_SEQ 1

int main()
{
    Datalink dl;
    seq_nr frame_expected;
    frame r, s;
    event_type event;

    dl.wait_others();    

    frame_expected = 0;
    while(true)
    {
        dl.wait_for_event(&event);
        if (event = frame_arrival)
        {
            dl.from_physical_layer(&r);
            cout << "Datalink: " << "receive seq " << r.seq << endl;
            if (r.seq == frame_expected)
            {
                dl.to_network_layer(&r.info);
                inc(frame_expected);        
            }
            s.ack = 1 - frame_expected;
            cout << "Datalink: " << "send ACK " << s.ack << endl;
            dl.to_physical_layer(&s);
        }
    }

    return 0;
}