#include "physical.h"

using namespace std;

int link_to_DatalinkLayer(const int MSG_KEY)
{
    int msgid = msgget((key_t)MSG_KEY, 0666 | IPC_CREAT);
	if(msgid == -1)
	{
		cerr << "Failed to link to Datalink Layer: " << strerror(errno) << endl;
        return -1;
	}
    return msgid;
}

int data_exchange(const int side, const pid_t pid, const int msgid, const int sockfd)
{
    fd_set sockfds, readfds, writefds;
	FD_ZERO(&sockfds);
	FD_SET(sockfd, &sockfds);

    char buffer_snd[DatapackLen], buffer_rec[DatapackLen];
    int buffer_snd_len, buffer_rec_len;
    int _rs, _ws, ressel;
    int _rcvs, _snds;

    unsigned int read_cnt = 0, write_cnt = 0;

    struct Message msg_data;

    while(TaihouDaisuki)
    {
        readfds = sockfds;
        writefds = sockfds;
        do
        {
            errno = 0;
            ressel = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
        }while(ressel < 0 && errno == EINTR);

        // cout << (side == SENDER ? "SENDER " : "RECEIVER ");
        // cout << "Physical select finish, readable = " << FD_ISSET(sockfd, &readfds) << " , writeable = " << FD_ISSET(sockfd, &writefds) << endl;
        if(ressel < 0)
        {
            cerr << (side == SENDER ? "SENDER " : "RECEIVER ");
            cerr << "Data Exchange Select error: " << strerror(errno) << endl;
            return SOCKET_ERROR;
        }
        else if(ressel == 0)
        {
            do
			{
				errno = 0;
				_rs = read(sockfd, buffer_rec, DatapackLen);
			}while(_rs < 0 && errno == EINTR); //被信号打断， 则重新read
            if(_rs > 0)
			{
                cerr << (side == SENDER ? "SENDER " : "RECEIVER ");
				cerr << "what do you fuxking read??? _r = " << _rs << endl;
                return SOCKET_ERROR;
			}
            if(_rs < 0)
            {
                cerr << (side == SENDER ? "SENDER " : "RECEIVER ");
                cerr << "Data Exchange Select-Read error: " << strerror(errno) << endl;
                return SOCKET_ERROR;
            }
            return SOCKET_CLOSE;
        }

        if(FD_ISSET(sockfd, &readfds))
        {
            int read_res;
            read_res = read_bitstream(side, sockfd, NODatapackLen, buffer_rec);
            if(read_res == READ_CLOSE)
                return SOCKET_CLOSE;
            else if(read_res == READ_ERROR)
                return SOCKET_ERROR;
            if(calc_bitstream(buffer_rec + FramkindLen, SndNoLen) != PureSIGpack) // not pure ACK or NAK pack, continue receving
            {
                read_res = read_bitstream(side, sockfd, DatapackLen - NODatapackLen, buffer_rec + NODatapackLen);
                if(read_res == READ_ERROR)
                    return SOCKET_ERROR;
                buffer_rec_len = DatapackLen;
            }
            else
                buffer_rec_len = NODatapackLen;

            cout << "[" << read_cnt << "]";
            cout << (side == SENDER ? "SENDER " : "RECEIVER ");
            cout << "Physical: receive data " << buffer_rec_len << " byte(s)" << endl;
            ++read_cnt; ////////////////////

            // upload message to DataLink_layer
            msg_data.msg_type = FROM_PHYSICAL;
            // strcpy(msg_data.text, buffer_rec);
            memcpy(msg_data.data, buffer_rec, buffer_rec_len);
            do
            {
                errno = 0;
                _snds = msgsnd(msgid, (void *)&msg_data, MSGBUFF_SIZE, 0);
            }while(_snds == -1 && errno == EINTR);
            if (_snds == -1)
                return TO_DATALINK_ERROR;

            //send sig to DataLink_layer
            int SIG_OK;
            //cout << "SIG to pid = " << pid << endl;
            do
            {
                errno = 0;
                SIG_OK = kill(pid, SIG_FRAME_ARRIVAL);
            }while (SIG_OK && errno == EINTR);
            if(SIG_OK == -1)
            {
                cerr << (side == SENDER ? "SENDER " : "RECEIVER ");
                cerr << "Physical send SIG to datalink layer failed: " << strerror(errno) << endl;
                return SOCKET_ERROR;
            }
        }

        if(FD_ISSET(sockfd, &writefds))
        {
            // loop to send frame that in message queue
            while(TaihouDaisuki)
            {
                do
                {
                    errno = 0;
                    _rcvs = msgrcv(msgid, (void *)&msg_data, MSGBUFF_SIZE, FROM_DATALINK, IPC_NOWAIT);
                    //cout << "Physical Receiving from datalink..." << endl;
                }while(_rcvs == -1 && errno == EINTR);
                
                if(_rcvs == -1)
                {
                    if(errno == ENOMSG)
                    {
                        // cout << (side == SENDER ? "SENDER " : "RECEIVER ");
                        // cout << "Physical get no data from datalink layer" << endl;
                        break;
                    }
                    else
                        return FROM_DATALINK_ERROR; 
                }
                memcpy(buffer_snd, msg_data.data, DatapackLen);

                // cout << (side == SENDER ? "SENDER " : "RECEIVER ");
                // cout << "Physical receive from datalink layer" << endl;

                cout << (side == SENDER ? "SENDER " : "RECEIVER ");
                int write_res;
                if (calc_bitstream(buffer_snd + FramkindLen, SndNoLen) == PureSIGpack)
                {
                    write_res = write_bitstream(side, sockfd, NODatapackLen, buffer_snd);
                    cout << "[" << write_cnt << "]";
                    cout << "Physical: write data " << SndNoLen << " byte(s)" << endl;
                    ++write_cnt; ////////////////////
                }
                else
                {
                    write_res = write_bitstream(side, sockfd, DatapackLen, buffer_snd);
                    cout << "[" << write_cnt << "]";
                    cout << "Physical: write data " << DatapackLen << " byte(s)" << endl;
                    ++write_cnt; ////////////////////
                }
                if (write_res == WRITE_CLOSE)
                    return SOCKET_CLOSE;
                else if (write_res == WRITE_ERROR)
                    return WRITE_ERROR;
            }
        }

        //cout << (side == SENDER ? "SENDER " : "RECEIVER ");
        //cout << "Physical GO FOR NEXT ROUND" << endl;
    }

    return SOCKET_OK; // this will not be run if it works normally
}

int read_bitstream(const int side, const int fd, const int Len, char* const buffer)
{
    int _rs;
    int buffer_p = 0;
    do
    {
        errno = 0;
        _rs = read(fd, buffer + buffer_p, Len - buffer_p);
        buffer_p += (_rs > 0 ? _rs : 0);
        if(buffer_p == Len)
            break;
    } while ((_rs < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) && (buffer_p < Len));

    if(_rs == 0)
        return READ_CLOSE;
    else if (_rs < 0)
    {
        cerr << (side == SENDER ? "SENDER " : "RECEIVER ");
        cerr << "Data Exchange Read error: " << strerror(errno) << endl;
        return READ_ERROR;
    }
    return READ_OK;
}
int write_bitstream(const int side, const int fd, const int Len, const char* const buffer)
{
    int _ws;
    int buffer_p = 0;
    do
    {
        errno = 0;
        _ws = write(fd, buffer + buffer_p, Len - buffer_p);
        buffer_p += (_ws > 0 ? _ws : 0);
        if(buffer_p == Len)
            break;
    } while((_ws < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) && (buffer_p < Len));

    if(_ws == 0)
        return WRITE_CLOSE;
    else if(_ws < 0)
    {
        cerr << (side == SENDER ? "SENDER " : "RECEIVER ");
        cerr << "Data Exchange Write error: " << strerror(errno) << endl;
        return WRITE_ERROR;
    }
}

unsigned int calc_bitstream(const char* const Bitstream, const int Len)
{
    unsigned int res = 0;
    for(int i = 0; i < Len; ++i)
        res = (res << 4) + Bitstream[i];
    return res;
}
