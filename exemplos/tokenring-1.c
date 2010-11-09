/* CLIENTE
   UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/queue.h>

void error(char *msg) {
  perror(msg);
  exit(0);
}

typedef struct t_client_socket {
  int sock;
  struct sockaddr_in server;
} s_client_socket;

s_client_socket* start_client(int port, const char* ip) {
  struct hostent *hp;
  s_client_socket* client_socket;

  client_socket = (s_client_socket*) malloc(sizeof(s_client_socket));

  client_socket->sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (client_socket->sock < 0)
    error("socket");

  client_socket->server.sin_family = AF_INET;
  hp = gethostbyname(ip);

  if (hp == 0)
    error("Unknown host");

  bcopy((char *)hp->h_addr, (char *)&client_socket->server.sin_addr, hp->h_length);
  client_socket->server.sin_port = htons(port);
/*  length = sizeof(struct sockaddr_in);
  printf("Please enter the message: ");
  bzero(buffer, 256);
  fgets(buffer, 255, stdin);
  n = sendto(sock, buffer, strlen(buffer), 0, &server, length);

  if (n < 0)
    error("Sendto");

  n = recvfrom(sock, buffer, 256, 0, &from, &length);

  if (n < 0) 
    error("recvfrom");

  printf(1, "Got an ack: ", 12);
  printf(1, buffer, n); */
  return client_socket;
}

/* SERVIDOR
   Creates a datagram server.  The port 
   number is passed as an argument.  This
   server runs forever */

int start_server(int port) {
  int sock, length;
  struct sockaddr_in server;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    error("Opening socket");

  length = sizeof(server);
  bzero(&server, length);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  
  if (bind(sock, (struct sockaddr *) &server, length) < 0) 
    error("binding");

  return sock;

/*  fromlen = sizeof(struct sockaddr_in);

  while (1) {
    n = recvfrom(sock, buf, 1024, 0, (struct sockaddr *) &from, &fromlen);
    if (n < 0)
      error("recvfrom");
    printf(1, "Received a datagram: ", 21);
    printf(1, buf, n);
    n = sendto(sock, "Got your message\n", 17, 0, (struct sockaddr *) &from, fromlen);

    if (n  < 0) 
      error("sendto");
  }*/
}

int main(int argc, char* argv[]) {
  int port = atoi(argv[1]);
  char* ip = argv[2];
  int server_sock;
  s_client_socket* client_socket;

  int queue_size = 0;
  char** queue;
  queue = (char**) malloc(sizeof(char*));

  server_sock = start_server(port);

  printf ("Enter para iniciar client..\n");
  getchar();

  client_socket = start_client(port, ip);
  printf ("h para criar bastão\n");
  if (getchar() == 'h') {
    sendto(client_socket->sock, "", 0, 0, &(client_socket->server), sizeof(struct sockaddr_in));
    getchar();
  }
  
  #pragma omp parallel num_threads(2)
  {
    struct sockaddr from;
    unsigned int n, from_len;
    char buffer[1024];
    #pragma omp sections
    {
      #pragma omp section
      {
        /* IO */
        while (1) {
          scanf ("%[^\n]", buffer);
          printf ("Chuuulapa!\n");
          queue = realloc(queue, sizeof(char*) * (queue_size + 1));
          queue[queue_size] = (char*) malloc(sizeof(char) * strlen(buffer));
          strcpy(queue[queue_size++], buffer);
        }
      }
      #pragma omp section
      {
        /* Data */
        while (1) {
          n = recvfrom(server_sock, buffer, 1024, 0, (struct sockaddr *) &from, &from_len);
    
          if (n < 0)
            error("recvfrom");

          if (n == 0 && queue_size == 0) {
            system("date +\"%H:%M:%S\"");
           system("sleep 1");
            sendto(client_socket->sock, "", 0, 0, &(client_socket->server), sizeof(struct sockaddr_in));     
          }
          else if (n == 0 && queue_size > 0) {
//            sendto(client_socket->sock, queue[queue_size], strlen(queue[queue_size--]), 0, &(client_socket->server), sizeof(struct sockaddr_in));
          }
          else {
            printf("%s\n", buffer);
            system("sleep 2");
            sendto(client_socket->sock, buffer, n, 0, &(client_socket->server), sizeof(struct sockaddr_in));
          }
        }
      }
    }
  }
}
