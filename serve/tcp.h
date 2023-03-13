#ifndef __TCP_H
#define __TCP_H

int tcp_client_init(char* ip, int port);
int tcp_client_send(unsigned char* buff, int len);
int tcp_client_rcv(unsigned char* buff, int len);
int tcp_client_close();
int tcp_server_init(int port);
int tcp_server_send(char c, unsigned char* buff, int len);
int tcp_server_rcv(char c, unsigned char* buff, int len);
int tcp_server_close();

#endif