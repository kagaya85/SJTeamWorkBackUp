#include "../common/datalink.h"
#define MAX_SEQ 1

int main()
{
    Datalink dl;
    seq_nr next_frame_to_send;
    frame s;
    packet buffer;
    event_type event;
    
    dl.wait_others();    

    next_frame_to_send = 0;
    dl.from_network_layer(&buffer);
    while(true)
    {
        s.info = buffer;
        s.seq = next_frame_to_send;
        cout << "Datalink: " << "send seq " << s.seq << endl;
        dl.to_physical_layer(&s);
        dl.start_timer(s.seq);
        dl.wait_for_event(&event);
        if (event == frame_arrival)
        {
            dl.from_physical_layer(&s);
            cout << "Datalink: " << "receive ACK " << s.ack << endl;
            if (s.ack == next_frame_to_send)
            {
                dl.stop_timer(s.ack);
                dl.from_network_layer(&buffer);
                inc(next_frame_to_send);
            }
            else
                continue;   // 重发
        }
        else if (event == cksum_err)
        {
            continue;
        }
        else if (event == timeout)
        {
            dl.eventQueue.pop();    // 将seq号pop出来
            continue;
        }
        else
            cerr << "event error" << endl;
    }

    return 0;
}