/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
  // 1. 변수 설정
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  // 2. 명령줄 인자 확인하기
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  // 3. open_listenfd를 호출해서, 듣기 소켓을 열기
  listenfd = Open_listenfd(argv[1]);

  // 4. 무한 루프로 듣기 소켓이 연결 요청을 기다렸다가
  while (1)
  {
    clientlen = sizeof(clientaddr);
    // 5. 요청이 오면 client의 소켓주소를 clientaddr에 채우고, 연결된 소켓 디스크립터를 반환함
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    // 6. 연결성공 시. client의 정보를 읽어옴
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    // 7. client의 정보를 프린트하기
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    // 8. doit함수를 호출해서, http 트랜잭션을 처리함(요청을 받고, 응답까지 처리)
    doit(connfd);
    // 9. 그 이후에 연결소켓을 닫음
    Close(connfd);
  }
}