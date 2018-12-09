#include "../common/datalink.h"

#define MAX_SEQ 1 // 窗口大小

int main()
{
    Datalink dl;
    seq_nr next_frame_to_send;
    seq_nr frame_expected;
    frame r, s;
    packet buffer;
    event_type event;
    
    dl.wait_others();    

    next_frame_to_send = 0;
    frame_expected = 0;
    from_network_layer(&buffer);
    s.info = buffer;
    s.seq = next_frame_to_send;
    s.ack = 1 - frame_expected;
    dl.to_physical_layer(&s);
    dl.start_timer(s.seq);

    while(true)
    {
        wait_for_event(&event);
        if (event == frame_arrival)
        {
            dl.from_physical_layer(&r);
            if (r.seq == frame_expected)
            {
                to_network_layer(&r.info);
                inc(frame_expected);
            }
            if (r.ack == next_frame_to_send)
            {
                dl.stop_timer(r.ack);
                from_network_layer(&buffre);
                inc(next_frame_to_send);
            }
        }
        // 发送新包
        s.info = buffer;
        s.seq = next_frame_to_send;
        s.ack = 1 - frame_expected;
        dl.to_physical_layer(&s);
        dl.start_timer(s.seq);
    }

    return 0;
}