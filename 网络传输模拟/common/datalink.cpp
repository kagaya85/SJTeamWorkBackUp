#include "common.h"
#include "Datalink.h"

Datalink::Datalink()
{
    header = NULL;
    dataLinkEvent = no_event;
    NetworkDatalinkSeq = 0;
    DatalinkPhysicalSeq = 0;
    networkStatus = Enable; // 默认网络层初始enable
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
    kill(pid, SIG_NETWORK_LAYER_ENABLE);
    networkStatus = Enable;
}

void Datalink::disable_network_layer()
{
    pid_t pid;
    pid = getPidByName("network");
    kill(pid, SIG_NETWORK_LAYER_DISABLE);
    networkStatus = Disable;
}

void Datalink::start_ack_timer()
{
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

int Datalink::from_network_layer(packet *pkt)
{
    char fileName[50];
    
    if (networkStatus == Disable)
    {
        cerr << "network layer disabled" << endl;
        return -1;
    }
    
    sprintf(fileName, "network_datalink.share.%04d", frame_seq);
    int fd = open(fileName);
    if (fd < 0)
    {
        cerr << "open " << fileName << "error" << endl;
        return -1;
    }

    flock(fd, LOCK_EX);
    int readByteNum = read(fd, pkt.data, MAX_PKT);
    seq_inc(NetworkDatalinkSeq);
    return 0;
}

void Datalink::wait_for_event(event_type *event)
{
    sleep();
}

void Datalink::seq_inc(seq_nr k)
{
    if(k < MAX_SHARE_SEQ) 
        k = k + 1; 
    else 
        k = 0;
}

void Datalink::add_a_second(int signal)
{
	alarm(1);
	return;
}