#include "../common/physical.h"
#include "../common/common.h"

using namespace std;

int starup_client();

int main(const int argc, char *argv[])
{	
    // 等待其他进程开启
    pid_t pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("datal");
    }
    cout << "Physical: " << "get datalink pid " << pid << endl;

    if(argc != 3)
	{
		printf("Usage:%s [ip] [port]\n", argv[0]);
		return 0;
	}

    pid_t DatalinkLayer_pid = pid;  // here use the datalink proc_name
    int DatalinkLayer_msgid = link_to_DatalinkLayer(IPC_KEY);
	if(DatalinkLayer_msgid == -1)
	{
		cerr << "Failed to Link to Datalink Layer" << endl;
		return -1;
	}

    int sockfd = starup_client();

    fd_set sockfds, writefds;
    FD_ZERO(&sockfds);
    FD_SET(sockfd, &sockfds);

    struct sockaddr_in seraddr;
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(atoi(argv[2]));
	seraddr.sin_addr.s_addr = inet_addr(argv[1]);
	socklen_t len = sizeof(struct sockaddr_in);

    while(TaihouDaisuki)
    {
        connect(sockfd, (struct sockaddr *)&seraddr, len);

        int ret;
        writefds = sockfds;
        do
        {
            errno = 0;
            ret = select(FD_SETSIZE, NULL, &writefds, NULL, NULL);
        }while(ret < 0 && errno == EINTR);

        errno = 0;
        connect(sockfd, (struct sockaddr *)&seraddr, len);
        if (errno == EISCONN)
            cout << "Sender: Physical Connection Complete." << endl;
        else
        {
            cerr << "Sender: Connect error: " << strerror(errno) << endl;
            sleep(1);
            continue;
        }

        int exchgres = data_exchange(SENDER, DatalinkLayer_pid, DatalinkLayer_msgid, sockfd);
        if(exchgres == SOCKET_ERROR)
        {
            close(sockfd);
            cerr << "Physical Layer Sender: Socket Error" << endl;
            break;
        }
		else if(exchgres == FROM_DATALINK_ERROR)
		{
			close(sockfd);
            cerr << "Physical Layer Sender: From Datalink Layer Error" << endl;
            break;
		}
		else if(exchgres == TO_DATALINK_ERROR)
		{
			close(sockfd);
            cerr << "Physical Layer Sender: To Datalink Layer Error" << endl;
            break;
		}
        else if(exchgres == SOCKET_CLOSE) // SOCKET_CLOSE
        {
            cout << "Sender: Physical Connection Disconnected" << endl;
            close(sockfd);
            break;
        }
		else // SOCKET_OK, this will not be run if works correctly
		{
			cout << "Sender: Physical Connection Finish" << endl;
            close(sockfd);
            break;
		}
    }
	
    cout << "Sender Physical Layer Exit" << endl;
    return 0;
}

int starup_client()
{
    int sock = socket(AF_INET,SOCK_STREAM, 0);
	if(sock < 0)
	{
		cerr << "Sender: Create socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
		exit(EXIT_FAILURE);
	}
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    return sock;
}
