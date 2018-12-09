#include "datalink.h"

using namespace std;

TimerNode * Datalink::header;
event_type Datalink::datalinkEvent;
unsigned int Datalink::arrivedPacketNum;    // 来自网络层已经到达的包数量
unsigned int Datalink::arrivedFrameNum;    // 来自物理层已经到达的帧数量
seq_nr Datalink::timeoutSeq;
Status Datalink::NetworkStatus;
queue<int> Datalink::eventQueue;
// Status Datalink::frameArrivalEvent;
// Status Datalink::cksumErrEvent;
// Status Datalink::timeoutEvent;
// Status Datalink::networkLayerReadyEvent;
// Status Datalink::ackTimeoutEvent;

Datalink::Datalink()
{
    header = NULL;
    no_nak = true;
    datalinkEvent = no_event;
    NetworkDatalinkSeq = 0;
    DatalinkNetworkSeq = 0;
    arrivedPacketNum = 0;
    arrivedFrameNum = 0;
    NetworkStatus = Enable; // 默认网络层初始enable
    
    // 设置信号处理函数
    signal(SIGALRM, Datalink::sigalarm_handle);    
    signal(SIG_FRAME_ARRIVAL, Datalink::sig_frame_arrival_handle);
    signal(SIG_NETWORKLAYER_READY, Datalink::sig_network_layer_ready_handle);
    
    // 设置已毫秒为单位的闹钟
    struct itimerval new_value;    
    new_value.it_value.tv_sec = 0;    
    new_value.it_value.tv_usec = 1000;    
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_usec = 1000;
    setitimer(ITIMER_REAL, &new_value, NULL);

    // 建立消息队列
    msgid = msgget(IPC_KEY, 0666 | IPC_CREAT);
    if (msgid < 0)
    {
        perror("Message get error");
        exit(EXIT_FAILURE);
    }
}

Datalink::~Datalink()
{
    TimerNode *p;
    while (header)
    {
        p = header->next;
        delete header;
        header = p;
    }

    struct msqid_ds msgbuf;
    while (true) 
    {
        // msgctl(msgid, IPC_STAT, &msgbuf);
        // cout << "Datalink: " << "msgbuf.msg_qnum " << msgbuf.msg_qnum << endl;
        if(msgbuf.msg_qnum == 0)
        {
            msgctl(msgid, IPC_RMID, NULL);
            cout << "Datalink: " << "Exit" << endl;
            break;
        }
        else
            sleep(2);
    }
}

void Datalink::start_timer(seq_nr k)
{
    TimerNode *p;
    clock_t t;
    if (!header)
    {
        header = new(nothrow) TimerNode;
        if (header == NULL)
        {
            cerr << "new timerNode error" << endl;
            exit(EXIT_FAILURE);
        }
        header->next = NULL;
        header->nowTime = TIMEOUT_LIMIT;
        header->fkind = DataFrame;
        header->seq = k;
    }
    else
    {
        p = header;
        t = TIMEOUT_LIMIT;

        while (p->next)
        {
            t -= p->nowTime;
            p = p->next;
        }

        t -= p->nowTime;
        p->next = new(nothrow) TimerNode;
        if (p == NULL)
        {
            cerr << "new timerNode error" << endl;
            exit(EXIT_FAILURE);
        }
        p = p->next;
        p->next = NULL;
        if (t == 0)
            t++;
        p->nowTime = t;
        p->fkind = DataFrame;
        p->seq = k;
    }
    cout << "Datalink: " << "start timer seq " << k << endl;
}

void Datalink::stop_timer(seq_nr k)
{
    TimerNode *p, *q;
    if (!header)
        return;

    if (header->seq == k && header->fkind == DataFrame)
    {
        p = header;
        header = header->next;
        delete header;
        cout << "Datalink: " << "stop timer seq " << k << endl;
        return;
    }

    p = header;
    q = header->next;
    while (q)
    {
        if(q->seq == k && q->fkind == DataFrame)
        {
            p->next = q->next;
            delete q;
            q = p->next;
            cout << "Datalink: " << "stop timer seq " << k << endl;
            return;
        }
    }

}

void Datalink::enable_network_layer()
{
    pid_t pid;
    pid = getPidByName("network");
    kill(pid, SIG_NETWORKLAYER_ENABLE);
    NetworkStatus = Enable;
    cout << "Datalink: " << "enable network layer" << endl;
}

void Datalink::disable_network_layer()
{
    pid_t pid;
    pid = getPidByName("network");
    kill(pid, SIG_NETWORKLAYER_DISABLE);
    NetworkStatus = Disable;
    cout << "Datalink: " << "disable network layer" << endl;
}

void Datalink::start_ack_timer()
{
    TimerNode *p;
    clock_t t;
    if (!header)
    {
        header = new(nothrow) TimerNode;
        if (header == NULL)
        {
            cerr << "new timerNode error" << endl;
            exit(EXIT_FAILURE);
        }
        header->next = NULL;
        header->nowTime = TIMEOUT_LIMIT;
        header->fkind = AckFrame;
    }
    else
    {
        p = header;
        t = TIMEOUT_LIMIT;

        while (p->next)
        {
            t -= p->nowTime;
            p = p->next;
        }
        t -= p->nowTime;

        p->next = new(nothrow) TimerNode;
        if (p == NULL)
        {
            cerr << "new timerNode error" << endl;
            exit(EXIT_FAILURE);
        }
        p = p->next;
        p->next = NULL;
        if (t == 0)
            t++;
        p->nowTime = t;
        p->fkind = AckFrame;
    }
    cout << "Datalink: " << "start ack timer" << endl;
}

void Datalink::stop_ack_timer()
{
    TimerNode *p, *q;
    if (!header)
        return;

    if (header->fkind == AckFrame)
    {
        p = header;
        header = header->next;
        delete header;
        return;
    }

    p = header;
    q = header->next;
    while (q)
    {
        if(q->fkind == AckFrame)
        {
            p->next = q->next;
            delete q;
            q = p->next;
            return;
        }
    }
    cout << "Datalink: " << "stop ack timer" << endl;
}

void Datalink::wait_for_event(event_type *event)
{
    cout << "Datalink: " << "wait for event" << endl;

    while(eventQueue.empty())    
        pause();

    *event = (event_type)eventQueue.front();
    eventQueue.pop();

    cout << "Datalink: handle event ";
    switch(*event)
    {
        case no_event:
            cout << "no_event" << endl;
            break;
        case frame_arrival:
            cout << "frame_arrival" << endl;
            break;
        case cksum_err:
            cout << "cksum_err" << endl;
            break;
        case timeout:
            cout << "timeout" << endl;
            break;
        case network_layer_ready:
            cout << "network_layer_ready" << endl;
            break;
        case ack_timeout:
            cout << "ack_timeout" << endl;
            break;
    }

    return;
}

void Datalink::seq_inc(seq_nr &k)
{
    if(k < MAX_SHARE_SEQ) 
        k = k + 1; 
    else 
        k = 0;
}

/* 信号处理函数 */
void Datalink::sigalarm_handle(int signal)
{
    if (!header)
        return;
    
    header->nowTime -= 1;
    if (header->nowTime <= 0)
    {
        if (header->fkind == DataFrame)
        {
            eventQueue.push(timeout);
            eventQueue.push(header->seq);   // 将超时的seq号紧随其后
            cout << "Datalink: " << "seq "<< timeoutSeq <<" timeout" << endl;
        }
        else if (header->fkind == AckFrame)
        {
            eventQueue.push(ack_timeout);
            cout << "Datalink: " << "ack timeout" << endl;
        }
        TimerNode *p;
        p = header;
        header = header->next;
        delete p;
    }
    else    // 未超时
        datalinkEvent = no_event;
}

void Datalink::sig_frame_arrival_handle(int signal)
{
    arrivedFrameNum++;
    eventQueue.push(frame_arrival);
    cout << "Datalink: get signal SIG_FRAME_ARRIVAL" << endl;
}

void Datalink::sig_network_layer_ready_handle(int signal)
{
    arrivedPacketNum++;
    eventQueue.push(network_layer_ready);
    NetworkStatus = Enable;
    cout << "Datalink: get signal SIG_NETWORKLAYER_READY" << endl;
}

seq_nr Datalink::get_timeout_seq()
{
    return timeoutSeq;
}

/* 层交互函数 */
void Datalink::from_network_layer(packet *pkt)
{
    char fileName[50];
    sprintf(fileName, "%s/network_datalink.share.%04d", To_Datalink_Dir, NetworkDatalinkSeq);

    if (NetworkStatus == Disable)
    {
        cerr << "network layer disabled" << endl;
        exit(EXIT_FAILURE);
    }
    
    int fd;
        // 文件不存在循环等待
    //puts("DL: From start");
    while (access(fileName, F_OK) < 0)
        sleep(1);
    //puts("DL: From end");
        
    do
    {
        errno = 0;
        fd = open(fileName, O_RDONLY);
    } while (fd < 0 && errno == EINTR);
    
    if (fd < 0)
    {
        cerr << "open " << fileName << " error" << endl;
        exit(EXIT_FAILURE);
    }

    flock(fd, LOCK_EX);

    int ret;
    do
    {
        errno = 0;
        ret = read(fd, pkt->data, MAX_PKT);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        cerr << "read " << fileName << " error: " << strerror(errno) << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    cout << "Datalink: " << "read " << ret << " byte(s) from "<< fileName << endl;
    close(fd);
    ret = remove(fileName);
    if(ret < 0)
        cerr << "Datalink: " << "delete file \"" << fileName << "\" error" << endl;
    seq_inc(NetworkDatalinkSeq);
    return; // ok
}

void Datalink::to_network_layer(packet *pkt)
{
    char fileName[50];
    sprintf(fileName, "%s/datalink_network.share.%04d", To_Network_Dir, DatalinkNetworkSeq);

    //puts("DL: to start");
    while (access(fileName, F_OK) == 0)
        sleep(2);   // 文件存在，阻塞等待
    //puts("DL: to end");

    mode_t mode = umask(0);
    mkdir(To_Network_Dir, 0777);

    int fd;
    do
    {
        errno = 0;
        fd = open(fileName, O_WRONLY | O_CREAT);
    } while (fd < 0 && errno == EINTR);

    if (fd < 0)
    {
        cerr << "open " << fileName << " error" << endl;
        exit(EXIT_FAILURE);
    }

    flock(fd, LOCK_EX);

    int ret;
    do
    {
        errno = 0;
        ret = write(fd, pkt->data, MAX_PKT);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        cerr << "write " << fileName << " error: " << strerror(errno) << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    cout << "Datalink: " << "write " << ret << " byte(s) to "<< fileName << endl;
    
    close(fd);
    seq_inc(DatalinkNetworkSeq);
    return;
}

void Datalink::from_physical_layer(frame *frm)
{
    Message msg;

    // 从队列读取
    int ret;
    do
    {
        errno = 0;
        ret = msgrcv(msgid, (void *)&msg, MSGBUFF_SIZE, FROM_PHYSICAL, 0);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }

    cout << "Datalink: " << "receive frame from physical layer" << endl;

    int tmpkind;
    memcpy(&tmpkind, msg.data, 4);
    memcpy(&(frm->seq), &msg.data[4], 4);
    memcpy(&(frm->ack), &msg.data[8], 4);
    memcpy(frm->info.data, &msg.data[12], MAX_PKT);
    
    frm->kind = frame_kind(ntohl(tmpkind));
    frm->ack = ntohl(frm->ack);
    frm->seq = ntohl(frm->seq);
    return;
}

void Datalink::to_physical_layer(frame *frm)
{
    Message msg;
    int tmpkind;
    seq_nr tmpack;
    seq_nr tmpseq;

    tmpkind = htonl(frm->kind);
    tmpack = htonl(frm->ack);
    tmpseq = htonl(frm->seq);

    memcpy(msg.data, &tmpkind, 4);
    memcpy(&msg.data[4], &tmpseq, 4);
    memcpy(&msg.data[8], &tmpack, 4);
    memcpy(&msg.data[12], frm->info.data, MAX_PKT);
    msg.msg_type = FROM_DATALINK;
    
    // 向队列发送
    int ret;
    do
    {
        errno = 0;
        ret = msgsnd(msgid, (void *)&msg, MSGBUFF_SIZE, 0);
    } while (ret < 0 && errno == EINTR);
    
    struct msqid_ds msgbuf;
    msgctl(msgid, IPC_STAT, &msgbuf);
    cout << "Datalink: " << "Send frame to physical layer and The msg_qnum is " << msgbuf.msg_qnum << endl;
    
    if (ret < 0)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    return;
}

bool Datalink::between(seq_nr a, seq_nr b, seq_nr c)
{
    return (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)));
}

void Datalink::send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[], int max_seq)
{
    frame s;
    s.info = buffer[frame_nr];
    s.seq = frame_nr;
    s.ack = (frame_expected + max_seq) % (max_seq + 1);
    to_physical_layer(&s);
    start_timer(frame_nr);
}

void Datalink::send_data(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[], int max_seq, int nr_bufs)
{
    frame s;
    s.kind = fk;
    if (fk == DataFrame)
        s.info = buffer[frame_nr % nr_bufs];
    s.seq = frame_nr;
    s.ack = (frame_expected + max_seq) % (max_seq+1);
    if (fk == NakFrame)
        no_nak = false;
    to_physical_layer(&s);
    if (fk == DataFrame)
        start_timer(frame_nr % nr_bufs);
    stop_ack_timer();
}

void Datalink::wait_others()
{
    // 等待另外两个进程开启
    pid_t pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("netwo");
    }
    cout << "Datalink: " << "get network pid " << pid << endl;
    pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("physi");
    }
    cout << "Datalink: " << "get physical pid " << pid << endl;
}

/* 事件判断函数 */
// bool Datalink::is_frameArrivalEvent()
// {
//     if (frameArrivalEvent) // if enable
//     {
//         frameArrivalEvent = Disable;
//         return true;
//     }
//     else
//         return false;
// }

// bool Datalink::is_cksumErrEvent()
// {
//     if (cksumErrEvent) // if enable
//     {
//         cksumErrEvent = Disable;
//         return true;
//     }
//     else
//         return false;
// }

// bool Datalink::is_timeoutEvent()
// {
//     if (timeoutEvent) // if enable
//     {
//         timeoutEvent = Disable;
//         return true;
//     }
//     else
//         return false;
// }

// bool Datalink::is_networkLayerReadyEvent()
// {
//     if (networkLayerReadyEvent) // if enable
//     {
//         networkLayerReadyEvent = Disable;
//         return true;
//     }
//     else
//         return false;
// }

// bool Datalink::is_ackTimeoutEvent()
// {
//     if (ackTimeoutEvent) // if enable
//     {
//         ackTimeoutEvent = Disable;
//         return true;
//     }
//     else
//         return false;
// }