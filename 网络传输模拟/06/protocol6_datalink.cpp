#include "../common/datalink.h"

#define MAX_SEQ 100 // 窗口大小
#define NR_BUFS ((MAX_SEQ+1)/2)

int main()
{
    Datalink dl;
    seq_nr next_frame_to_send;
    seq_nr ack_expected;
    seq_nr frame_expected, too_far;
    frame r;
    packet out_buf[NR_BUFS], in_buf[NR_BUFS];
    bool arrived[NR_BUFS];
    seq_nr nbuffered;
    event_type event;

    dl.enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0;
    frame_expected = 0;
    too_far = NF_BUFS;  // 初始非法，合法范围（0 ~ NF_BUFS - 1）
    nbuffered = 0;

    for(int i = 0; i < NR_BUFS; i++)
        arrived[i] = false;

    while(true)
    {
        wait_for_event(&event);
        switch(event)
        {
            case network_layer_ready:
                dl.from_network_layer(&buffer[next_frame_to_send]);
                nbuffered++;
                dl.send_data(DataFrame, next_frame_to_send, frame_expected, buffer, MAX_SEQ, NR_BUFS);
                inc(next_frame_to_send);
                break;
            case frame_arrival:
                dl.from_physical_layer(&r);
                if (r.kind == DataFrame)
                {
                    if((r.seq != frame_expected) && dl.no_nak)
                        dl.send_data(NakFrame, 0, frame_expected, out_buf, MAX_SEQ, NR_BUFS); // 发送NAK
                    else
                        start_ack_timer();
                    
                    if (dl.between(frame_expected, r.seq, too_far) && arrived[r.seq%NR_BUFS] == false)
                    {
                        arrived[r.seq%NR_BUFS] = true;
                        in_buf[r.seq%NR_BUFS] = r.info;
                        while (arrived[frame_expected % NR_BUFS])
                        {
                            dl.to_network_layer(&inbuf[arrived[frame_expected] % NR_BUFS]);
                            dl.no_nak = ture;
                            arrived[frame_expected % NR_BUFS] = true;
                            inc(frame_expected);
                            inc(too_far);
                            dl.start_ack_timer();
                        }
                    }
                }   // end of if(r.kind == DaraFrame)
                
                // 如果发送nak，则找出最后一个确认帧序号的下一个
                if ((r.kind == nak) && dl.between(ack_expected, (r.ack + 1) % (MAX_SEQ + 1), next_frame_to_send))
                    dl.send_data(DataFrame, (r.ack + 1) % (MAX_SEQ + 1), frame_expected, out_buf, MAX_SEQ, NR_BUFS);

                // 如果收到ack（独立帧或数据帧捎带）
                while (dl.between(ack_expected, r.ack, next_frame_to_send))
                {
                    nbuffered--;
                    dl.stop_timer(ack_expected % NR_BUFS);
                    inc(ack_expected);
                }
                break;
            case cksum_err:
                if(dl.no_nak)
                    dl.send_data(NakFrame, 0, frame_expected, out_buf, MAX_SEQ, NR_BUFS);
                break;
            case timeout:  
                dl.send_data(DataFrame, dl.get_timeout_seq(), frame_expected, out_buf, MAX_SEQ, NR_BUFS);
                break;
            case ack_timeout:
                dl.send_data(AckFrame, 0, frame_expected, out_buf, MAX_SEQ, NR_BUFS);
                break;
        }   // end of switch

        if (nbuffered < NR_BUFS)
            dl.enable_network_layer();
        else
            dl.disable_network_layer();
    }   // end of while

    return 0;
}