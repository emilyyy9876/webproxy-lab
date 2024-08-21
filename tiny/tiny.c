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
  Rio_readlineb(&rio, buf, MAXLINE); // 구조체 rio의 buf에, fd로부터한 줄의 데이터(요청라인)를 읽어옴
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version); // 요청 라인 분석, method/uri/version 추출하기

  if (strcasecmp(method, "GET")) // GET 메서드가 아니면, "501 Not Implemented" 오류를 반환하고 종료
  {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }

  read_requesthdrs(&rio); // 요청 헤더를 읽어오기

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

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  // HTTP 응답 body 만들기
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<bodybgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s:%s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s:%s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>TheTinyWebserver</em>\r\n", body);

  // HTTP 응답 출력하기
  // sprintf, Rio_writen을 반복적으로 사용해서, 응답 메시지를 한 줄씩 구성하고, 이를 클라이언트에게 순차적으로 전송함
  // 클라이언트는 이 데이터를 한 줄씩 수신하고 처리함
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type:text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length:%d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];               // 헤더를 저장할 버퍼 선언
  Rio_readlineb(rp, buf, MAXLINE); // 첫 번째 헤더 라인을 읽음
  while (strcmp(buf, "\r\n"))      // strcmp=string compare함수로 두 string을 비교해서 같으면, 0을 반환함
  {
    Rio_readlineb(rp, buf, MAXLINE); // 다음 줄을 buf에 읽어들임
    printf("%s", buf);
  }
  return;
}

// uri를 분석해서 정적콘텐츠인지, 동적콘텐츠인지 구분하고 그에 따라 filename(파일경로)과 cgiargs(cgi인자)를 설정하는 함수
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr; // uri 내 특정 위치를 가리킬 포인터 변수를 선언

  // 정적 컨텐츠인 경우
  if (!strstr(uri, "cgi-bin")) // uri에 cgi-bin이 포함되어 있지 않은 경우
  {
    // 인자..없음
    strcpy(cgiargs, "");
    // 파일경로 지정하기
    strcpy(filename, ".");
    strcat(filename, uri); // string concatenate(string을 이어붙이는 함수).파일경로를 조합하는데 자주 사용됨--->여기서는 file 끝에 uri를 덧붙임
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");
    return 1; // 정적 컨텐츠임을 나타내는 1을 반환함
  }

  // 동적 컨텐츠인 경우
  else
  {
    // 예시uri: "/cgi-bin/script.cgi?name=value"
    ptr = index(uri, '?');
    // ptr이 null이 아닌 경우--->인자 있는 경우
    if (ptr)
    {
      // 포인터 연산....ptr이 가리키는 위치에서 한 칸 다음의 메모리 주소를 가리키도록 함

      // strcpy는 두번째 인자인 ptr+1이 가리키는 문자열의 시작 주소부터 문자열의 끝('\0문자)까지 "모든 문자"를 첫 번째 인자인 cgiargs에 복사함
      // 즉, ptr + 1은 'n'을 가리키며, 이 위치에서 시작하여 문자열의 끝까지(name=value\0)가 cgiargs에 복사함
      strcpy(cgiargs, ptr + 1);

      // '?'를 널 문자('\0')로 대체하여 "name=value"를 잘라냄

      *ptr = '\0'; // "?" 위치에 널 문자를 넣어 문자열을 종료

      // 예시: 변경 전 "/cgi-bin/script.cgi?name=value", 변경 후 "/cgi-bin/script.cgi\0name=value"
      // URI는 "/cgi-bin/script.cgi"가 되고, cgiargs는 "name=value"가 됨
    }

    // ptr이 null인 경우--->인자 없는 경우
    else
      strcpy(cgiargs, ""); // cgiargs를 빈 문자열로 초기화

    // 파일경로 지정하기
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0; // 정적 컨텐츠임을 나타내는 0을 반환함
  }
}