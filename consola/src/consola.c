/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <string.h>
 #include <netdb.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>

#define PORT 4000;
#define MAXDATESIZE 100;




int main(int argc, char *argv[]) {
int consola,numBytes;
struct hostent *he;//represento una entrada al host
char buf[MAXDATESIZE];//cadena de cantidad maxima d e caracteres que puedo leer

struct sockaddr_in their_addr;

if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
}

if ((he=gethostbyname(argv[1])) == NULL) {  // obtener información de máquina
    perror("gethostbyname");
    exit(1);
}

if ((consola = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
}
their_addr.sin_family = PF_INET;    // Ordenación de bytes de la máquina
their_addr.sin_port = htons(PORT);  // short, Ordenación de bytes de la red
their_addr.sin_addr = *((struct in_addr *)he->h_addr);
memset(&(their_addr.sin_zero),0, 8);  // poner a cero el resto de la estructura
if (connect(consola, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }
 if ((numBytes=recv(consola, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

  buf[numBytes] = '\0';

  printf("Received: %s",buf);

   close(consola);

return 0;}
