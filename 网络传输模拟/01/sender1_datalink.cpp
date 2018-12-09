#include "../common/datalink.h"

int main()
{
    Datalink dl;
    frame s;
    packet buffer;

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
    
    struct msqid_ds msgbuf;

    while(true)
    {
        pid = getPidByName("netwo");
        if(pid < 0)
        {
            msgctl(msqid, IPC_STAT, &msgbuf);
            if(msgbuf.msg_qnum == 0)
                return 0;
            else
                sleep(5);
        }
        dl.from_network_layer(&buffer);
        s.info = buffer;
        dl.to_physical_layer(&s);
    }

    return 0;
}