// csapp.h 헤더 파일을 포함하기(여기에는 유틸리티 함수들이 선언되어 있음)
#include "csapp.h"

int main(int argc, char **argv)
{
    // 서버와 연결된 소켓 디스크립터를 저장할 변수
    int clientfd;
    // 호스트명, 포트 번호, 그리고 데이터를 담을 버퍼
    char *host, *port, buf[MAXLINE];
    // RIO 구조체. (버퍼링된 I/O를 관리하는 구조체)
    rio_t rio;

    // 프로그램 인자가 올바르게 제공되었는지 확인함
    if (argc != 3)
    {
        // 사용법이 잘못되었을 때의 에러 메시지
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        // 프로그램을 종료
        exit(0);
    }

    // 첫 번째 인자로 호스트명을 받아오기
    host = argv[1];
    // 두 번째 인자로 포트 번호를 받아오기
    port = argv[2];

    // 1. 서버에 연결하고, 소켓 디스크립터를 돌려받기 (주어진 호스트, 포트번호를 가지)
    clientfd = open_clientfd(host, port);
    // RIO 구조체를 초기화하여 버퍼링된 읽기를 준비하기
    Rio_readinitb(&rio, clientfd);

    // 표준 입력(stdin)으로부터 한 줄씩 읽어오기
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        // 서버로 데이터를 전송하기
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버로부터 응답을 한 줄씩 읽어오기
        Rio_readlineb(&rio, buf, MAXLINE);
        // 받은 응답을 표준 출력(stdout)에 출력하기
        Fputs(buf, stdout);
    }

    // 소켓을 닫아 연결을 종료하기
    Close(clientfd);
    // 프로그램을 정상 종료하기
    exit(0);
}