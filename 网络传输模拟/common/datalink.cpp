#include "Datalink.h"

TimerNode *header;
event_type datalinkEvent;
seq_nr NetworkDatalinkSeq;
seq_nr DatalinkNetworkSeq; // 链路层发向网络层的发送序号
unsigned int arrivedPacketNum;    // 来自网络层已经到达的包数量
unsigned int arrivedFrameNum;    // 来自物理层已经到达的帧数量
seq_nr timeoutSeq;

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
    signal(SIG_NETWORK_LAYER_READY, Datalink::sig_networklayer_ready_handle);
    
    // 设置已毫秒为单位的闹钟
    struct itimerval new_value;    
    new_value.it_value.tv_sec = 0;    
    new_value.it_value.tv_usec = 1000;    
    new_value.it_interval.tv_sec = 0;    
    new_value.it_interval.tv_usec = 1000;    
    setitimer(ITIMER_REAL, &new_value, NULL);
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
        header->fkind = dataFrame;
        header->seq = k;
    }
    else
    {
        p = header;
        t = TIMEOUT_LIMIT

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
        p->fkind = dataFrame;
        p->seq = k;
    }
}

void Datalink::stop_timer(seq_nr k)
{
    TimerNode *p, *q;
    if (!header)
        return;

    if (header->seq == k && header->ftype == dataFrame)
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
        if(q->seq == k && q->ftype == dataFrame)
        {
            p->next = q->next;
            delete q;
            q = p->next;
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
}

void Datalink::disable_network_layer()
{
    pid_t pid;
    pid = getPidByName("network");
    kill(pid, SIG_NETWORKLAYER_DISABLE);
    NetworkStatus = Disable;
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
        header->ftype = ackFrame;
    }
    else
    {
        p = header;
        t = TIMEOUT_LIMIT

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
        p->ftype = ackFrame;
        p->seq = k;
    }
}

void Datalink::stop_ack_timer()
{
    TimerNode *p, *q;
    if (!header)
        return;

    if (header->seq == k && header->ftype == ackFrame)
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
        if(q->seq == k && q->ftype == ackFrame)
        {
            p->next = q->next;
            delete q;
            q = p->next;
            return;
        }
    }    
}

void Datalink::wait_for_event(event_type *event)
{
    if(*event != datalinkEvent) // 在wait_for_event之外有信号中断
    {
        *event = datalinkEvent;
        return;
    }
    do
    {
        pause();
    } while (datalinkEvent == no_event);
    *event = datalinkEvent;
    return;
}

void Datalink::seq_inc(seq_nr k)
{
    if(k < MAX_SHARE_SEQ) 
        k = k + 1; 
    else 
        k = 0;
}

/* 信号处理函数 */
static void Datalink::sigalarm_handle(int signal)
{
    if (!header)
        return;
    
    header->nowTime -= 1;
    if (header->nowTime <= 0)
    {
        if (header->ftype == dataFrame)
        {
            datalinkEvent = timeout;
            timeoutSeq = header->seq;
        }
        else if (header->ftype == ackFrame)
        {
            datalinkEvent = ack_timeout;
        }
        TimerNode *p;
        p = header;
        header = header->next;
        delete p;
    }
    else    // 未超时
        datalinkEvent = no_event;
    signal(SIGALRM, Datalink::sigalarm_handle);    
}

static void Datalink::sig_frame_arrival_handle(int signal)
{
    arrivedFrameNum++;
    datalinkEvent = frame_arrival;
    signal(SIG_FRAME_ARRIVAL, Datalink::sig_frame_arrival_handle);
}

static void Datalink::sig_network_layer_ready_handle(int signal)
{
    arrivedPacketNum++;
    datalinkEvent = network_layer_ready;
    NetworkStatus = Enable;
    signal(SIG_NETWORKLAYER_READY, Datalink::sig_networklayer_ready_handle);    
}

seq_nr Datalink::get_timeout_seq()
{
    return timeoutSeq;
}

/* 层交互函数 */
void Datalink::from_network_layer(packet *pkt)
{
    char fileName[50];
    
    if (NetworkStatus == Disable)
    {
        cerr << "network layer disabled" << endl;
        exit(EXIT_FAILURE);
    }
    
    sprintf(fileName, "network_datalink.share.%04d", NetworkDatalinkSeq);
    int fd;
        // 文件不存在循环等待
    while (access(fileName, F_OK) < 0)
        sleep(1);
        
    do
    {
        errno = 0;
        fd = open(fileName, O_RDONLY);
    } while (fd < 0 && errno = EINTR);
    
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
        ret = read(fd, pkt.data, MAX_PKT);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        cerr << "read " << fileName << " error: " << strerror(errno) << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    seq_inc(NetworkDatalinkSeq);
    return; // ok
}

void Datalink::to_network_layer(packet *pkt)
{
    char fileName[50];
    
    sprintf(fileName, "datalink_network.share.%04d", DatalinkNetworkSeq);


    int fd;
    do
    {
        errno = 0;
        fd = open(fileName, O_WRONLY | O_CREAT);
    } while (fd < 0 && errno = EINTR);

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
        ret = write(fd, pkt.data, MAX_PKT);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        cerr << "write " << fileName << " error: " << strerror(errno) << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    seq_inc(DatalinkNetworkSeq);
    return;
}

void Datalink::from_physical_layer(frame *frm)
{
    int msgid = -1;
    Message msg;

    msgid = msgget(IPC_KEY, 0666 | IPC_CREAT);
    if(msgid < 0) {
        perror("Message get error");
        exit(EXIT_FAILURE);
    }

    // 从队列读取
    int ret;
    do
    {
        errno = 0;
        msgrcv(msgid, &msg, MSGBUFF_SIZE, FROM_PHYSICAL, 0)
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }

    memcpy(&(frm->kind), msg.data, 4);
    memcpy(&(frm->seq), &msg.data[4], 4);
    memcpy(&(frm->ack), &msg.data[8], 4);
    memcpy(frm->info, &msg.data[12], MAX_PKT);
    
    frm->kind = ntohl(frm->kind);
    frm->ack = ntohl(frm->ack);
    frm->seq = ntohl(frm->seq);
    return;
}

void Datalink::to_physical_layer(frame *frm)
{
    int msgid = -1;
    Message msg;

    msgid = msgget(IPC_KEY, 0666 | IPC_CREAT);
    if(msgid < 0) {
        perror("Message get error");
        exit(EXIT_FAILURE);
    }

    frm->kind = htonl(frm->kind);
    frm->ack = htonl(frm->ack);
    frm->seq = htonl(frm->seq);
    
    memcpy(msg.data, &(frm->kind), 4);
    memcpy(&msg.data[4], &(frm->seq), 4);
    memcpy(&msg.data[8], &(frm->ack), 4);
    memcpy(&msg.data[12], frm->info, MAX_PKT);
    msg.msg_type = FROM_DATALINK;
    // 向队列发送
    int ret;
    do
    {
        errno = 0;
        msgsnd(msgid, &msg, MSGBUFF_SIZE, 0)
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    return;
}

static bool Datalink::between(seq_nr a, seq_nr b, seq_nr c)
{
    retutn (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)));
}

void Datalink::send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.info = buffer[frame_nr];
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
    to_physical_layer(&s);
    start_timer(frame_nr);
}

void Datalink::send_data(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.kind = fk;
    if (fk == DataFrame)
        s.info = buffer[frame_nr % NR_BUFS];
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ+1);
    if (fk == nak)
        no_nak = false;
    to_physical_layer(&s);
    if (fk == DataFrame)
        start_timer(frame_nr % NR_BUFS);
    stop_ack_timer();
}

