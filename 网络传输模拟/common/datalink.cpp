#include "Datalink.h"

Datalink::Datalink()
{
    header = NULL;
    datalinkEvent = no_event;
    NetworkDatalinkSeq = 0;
    // DatalinkPhysicalSeq = 0;
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
    TimerNode *p;
    clock_t t;
    if (!ackHeader)
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
    pause();
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
    
    sprintf(fileName, "network_datalink.share.%04d", NetworkDatalinkSeq);
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
    seq_inc(NetworkDatalinkSeq);
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