#include "../common/datalink.h"
#define MAX_SEQ 1

int main()
{
    Datalink dl;
    seq_nr next_frame_to_send;
    frame s;
    packet buffer;
    event_type event;

    next_frame_to_send = 0;
    dl.from_network_layer(&buffer);
    while(true)
    {
        s.info = buffer;
        s.seq = next_frame_to_send;
        dl.to_physical_layer(&s);
        dl.start_timer(s.seq);
        wait_for_event(&event);
        if (event == frame_arrival)
        {
            dl.from_physical_layer(&s);
            if (s.ack == next_frame_to_send)
            {
                dl.stop_timer(s.ack);
                dl.from_network_layer(&buffer);
                inc(next_frame_to_send);
            }
            else
                continue;
        }
        else if (event == cksum_err)
        {
                continue;
        }
        else if (event == timeout)
        {
                continue;
        }
        else
            cerr << "event error" << endl;
    }

    return 0;
}