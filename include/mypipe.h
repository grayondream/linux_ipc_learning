
/*
 * @brief   半双工管道测试程序
 */
void pipe_client(int readfd, int writefd);
void pipe_server(int readfd, int writefd);

void pipe_test();
void popen_test();
void mkfifo_test();

//单客户端，单服务器
void mkfifo_client_process();
void mkfifo_server_process();

//多客户端，多服务器通信
void mult_fifo_server_process();
void mult_fifo_client_process();

void pipe_main(int argc, char**argv);