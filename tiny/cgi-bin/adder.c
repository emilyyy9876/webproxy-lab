/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void)
{
  // 1. 변수설정
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  // 2. 두 인자를 추출하기
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&'); // string character
    *p = '\0';            // strcpy의 특성 이용해서 인자 나누기 위해, '&'를 '\0'로 바꾸기
    strcpy(arg1, buf);    // buf(query string의 시작)부터 \0까지 --->arg1에 복사
    strcpy(arg2, p + 1);  // \0뒤(두번째인자의 시작)부터 \0까지--->arg2에 복사
    n1 = atoi(arg1);      // ASCII to Integer
    n2 = atoi(arg2);
  }
  // 3. 응답 body(content 변수에 담는 내용) 만들기
  sprintf(content, "QUERY_STRING=%s", buf); // content를 계속 덮어씀
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);

  // 4. http 응답 출력하기->출력하는 곳은 fd -> fd를 통해 클라이언트로 보냄
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content); // 만들어둔 응답 body 출력
  fflush(stdout);

  exit(0);
}
/* $end adder */
