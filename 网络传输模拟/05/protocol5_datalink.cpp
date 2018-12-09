#include "../common/datalink.h"

#define MAX_SEQ 100 // 窗口大小

int main()
{
    Datalink dl;
    seq_nr next_frame_to_send;
    seq_nr ack_expected;
    seq_nr frame_expected;
    frame r;
    packet buffer[MAX_SEQ + 1];
    seq_nr nbuffered;
    event_type event;
    dl.wait_others();    

    dl.enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0;
    frame_expected = 0;
    nbuffered = 0;

    while(true)
    {
        wait_for_event(&event);
        switch(event)
        {
            case network_layer_ready:
                dl.from_network_layer(&buffer[next_frame_to_send]);
                nbuffered++;
                dl.send_data(next_frame_to_send, frame_expected, buffer, MAX_SEQ);
                inc(next_frame_to_send);
                break;
            case frame_arrival:
                dl.from_physical_layer(&r);
                if (r.seq == frame_expected)
                {
                    dl.to_network_layer(&r.info);
                    inc(frame_expected);
                }
                while(Datalink::between(ack_expected, r.ack, next_frame_to_send))
                {
                    nbuffered = nbuffered -1;
                    stop_timer(ack_expected);
                    inc(ack_expected);
                }
                break;
            case cksum_err:
                break;
            case timeout:   // 回退n
                next_frame_to_send = ack_expected;
                for (seq_nr i = 1; i <= nbuffered; i++)
                {
                    dl.stop_timer(next_frame_to_send);  // 避免重复计时
                    dl.send_data(next_frame_to_send, frame_expected, buffer, MAX_SEQ);
                    inc(next_frame_to_send);
                }
                break;
        }   // end of switch
        
        if (nbuffered < MAX_SEQ)
            dl.enable_network_layer();
        else
            dl.disable_network_layer();
    }

    return 0;
}