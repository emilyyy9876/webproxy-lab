// csapp.h 헤더 파일을 포함하기
#include "csapp.h"

// echo 함수를 선언하기(이 함수는 클라이언트와의 연결에서 데이터를 수신하고, 다시 클라이언트로 전송함)

void echo(int connfd)
{
    // 읽어들인 데이터의 크기를 저장할 변수
    size_t n;
    // 데이터를 저장할 버퍼
    char buf[MAXLINE];
    // RIO 구조체(버퍼링된 I/O를 관리하는 구조체)
    rio_t rio;

    // RIO 구조체를 초기화하여, connfd로부터 버퍼링된 읽기를 준비하기
    rio_readinitb(&rio, connfd);
    // 클라이언트로부터 한 줄씩 데이터를 읽어오기
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        // 읽어들인 데이터의 크기를 확인하고, 그 크기를 출력하기
        printf("server received %d bytes\n", (int)n);
        // 읽어들인 데이터를 클라이언트에게 다시 전송하기
        Rio_writen(connfd, buf, n);
    }
}
// 서버 소켓을 수신 대기 상태로 설정하고, 클라이언트의 연결을 받아들이는 코드
int main(int argc, char **argv)
{
    // 서버가 수신 대기할 소켓 디스크립터를 저장할 변수
    // 클라이언트와 연결된 소켓 디스크립터를 저장할 변수
    int listenfd, connfd;
    // 클라이언트 주소의 크기를 저장할 변수
    socklen_t clientlen;
    // 클라이언트의 주소 정보를 저장할 구조체
    struct sockaddr_storage clientaddr;

    // 클라이언트의 호스트 이름을 저장할 버퍼
    // 클라이언트의 포트 번호를 저장할 버퍼
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 프로그램 인자가 올바르게 제공되었는지 확인
    if (argc != 2)
    {
        // 사용법이 잘못되었을 때의 에러 메시지
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        // 프로그램을 종료
        exit(0);
    }

    // 지정된 포트에서 수신 대기 소켓을 열기
    listenfd = open_listenfd(argv[1]);
    // 무한 루프를 돌며 클라이언트의 연결을 기다리기
    while (1)
    {
        // 클라이언트 주소 구조체의 크기를 설정하기
        clientlen = sizeof(struct sockaddr_storage);
        // 클라이언트의 연결 요청을 받아들임
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // 클라이언트의 호스트 이름과 포트 번호를 가져오기
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        // 연결된 클라이언트의 정보를 출력하기
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        // 클라이언트와의 연결에서 데이터를 수신하고 다시 전송하는 함수 호출
        echo(connfd);
        // 클라이언트와의 연결을 닫음
        Close(connfd);
    }
    // 프로그램을 정상 종료히기
    exit(0);
}