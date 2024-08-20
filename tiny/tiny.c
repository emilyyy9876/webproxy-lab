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

void doit(int fd)
{
  // 1. 변수 선언
  int is_static;
  // sbuf: 파일의 상태정보를 가지고 있음
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  // 2.request 읽어오기
  Rio_readinitb(&rio, fd);           // 구조체 rio를 초기화하기
  Rio_readlineb(&rio, buf, MAXLINE); // 구조체 rio의 buf에, fd로부터 읽어온 값을 저장
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if (strcasecmp(method, "GET")) // GET 메서드가 아니면, "501 Not Implemented" 오류를 반환하고 종료
  {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }

  read_requesthdrs(&rio); // 요청 헤더를 읽기

  // 3. URI를 파싱하여 filename과 CGI 인자(cgiargs) 설정, 그리고 정적/동적 컨텐츠 여부 결정
  is_static = parse_uri(uri, filename, cgiargs);
  // 4.클라이언트에게 정적 컨텐츠를 보내줘야 하는 경우
  if (is_static)
  {
    // 파일이 정규 파일이 아니거나, 읽기 권한이 없으면 clienterror 호출해서, 클라이언트에게 보고
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn’t read the file");
      return;
    }
    // 파일 실행가능...serve_static 호출
    serve_static(fd, filename, sbuf.st_size);
  }
  // 5.클라이언트에게 동적 컨텐츠를 보내줘야 하는 경우
  else
  {
    // 파일이 정규 파일이 아니거나, 읽기 권한이 없으면 clienterror 호출해서, 클라이언트에게 보고
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn’t run the CGI program");
      return;
    }
    // 파일 실행가능...serve_dynamic 호출
    serve_dynamic(fd, filename, cgiargs);
  }
}