#include "../common/physical.h"
#include "../common/common.h"

using namespace std;

int startup_server(int, const char*);

int main(const int argc, char *argv[])
{
    // 等待其他进程开启
    pid_t pid = -1;
    while(pid < 0)
    {
        sleep(1);
        pid = getPidByName("datalink");
    }

	if(argc != 2)
	{
		printf("Usage:%s [port]\n", argv[0]);
		return 1;
	}

	int DatalinkLayer_pid = get_pid_by_name("receiver1_datalink");  // here use the datalink proc_name
	int DatalinkLayer_msgid = link_to_DatalinkLayer(IPC_KEY);
	if(DatalinkLayer_msgid == -1)
	{
		cerr << "Failed to Link to Datalink Layer" << endl;
		return -1;
	}

	const char* ip = "0.0.0.0";
	const int port = atoi(argv[1]);

	int listenfd = startup_server(port, ip);
	
	struct sockaddr_in cliaddr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	
    fd_set sockfds, readfds;
	FD_ZERO(&sockfds);
	FD_SET(listenfd, &sockfds);

	while(TaihouDaisuki)
	{
        readfds = sockfds;
        int ret;
		do
        {
            errno = 0;
            ret = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        }while(ret < 0 && errno == EINTR);

        if(ret < 0)
		{
			cerr << "Receiver: Select error: " << strerror(errno) << endl;
			exit(EXIT_FAILURE);
		}
		if (!FD_ISSET(listenfd, &readfds))
            continue;

        int sockfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
        if(sockfd < 0)
        {
            cerr << "Receiver: Accept error: " << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Receiver: Physical Connection Complete." << endl;
        int flag = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);

		int exchgres = data_exchange(RECEIVER, DatalinkLayer_pid, DatalinkLayer_msgid, sockfd);
        if(exchgres == SOCKET_ERROR)
        {
            close(sockfd);
            cerr << "Physical Layer Receiver: Socket Error" << endl;
            break;
        }
		else if(exchgres == FROM_DATALINK_ERROR)
		{
			close(sockfd);
            cerr << "Physical Layer Receiver: From Datalink Layer Error" << endl;
            break;
		}
		else if(exchgres == TO_DATALINK_ERROR)
		{
			close(sockfd);
            cerr << "Physical Layer Receiver: To Datalink Layer Error" << endl;
            break;
		}
        else if(exchgres == SOCKET_CLOSE) // SOCKET_CLOSE
        {
            cout << "Receiver: Physical Connection Disconnected" << endl;
            close(sockfd);
            break;
        }
		else // SOCKET_OK, this will not be run if works correct
		{
			cout << "Receiver: Physical Connection Finish" << endl;
            close(sockfd);
            break;
		}
    }
	
	close(listenfd);

    cout << "Receiver Physical Layer Exit" << endl;
	return 0;
}

int startup_server(int _port, const char* _ip)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		cerr << "Receiver: Create socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
		exit(EXIT_FAILURE);
	}
	
    int flag = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flag | O_NONBLOCK);
    int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);
	local.sin_addr.s_addr = inet_addr(_ip);
	socklen_t len = sizeof(local);
	
	if(bind(sock, (struct sockaddr *)&local, len) < 0)
	{
		cerr << "Receiver: Bind socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
		exit(2);
	}
	
	if(listen(sock, 5) < 0)
	{
		cerr << "Receiver: Listen socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
		exit(3);
	}
	
	return sock;
}
