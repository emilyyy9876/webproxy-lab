// csapp.h 헤더 파일을 포함하기
#include "csapp.h"

// echo 함수를 정의하기(이 함수는 클라이언트와의 연결에서 데이터를 수신하고, 다시 클라이언트로 전송함)
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