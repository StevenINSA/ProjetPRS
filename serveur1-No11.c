#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> //en-tête nécessaire pour la création de sockets
#include <sys/socket.h> //en-tête nécessaire pour la création de sockets
#include <netinet/in.h> //pour utiliser struct sockaddr
#include <string.h> //pour la fonction memset()
#include <unistd.h> //pour les fonctions read() et write()
#include <arpa/inet.h> //pour traduire au format réseau/machine
#include <sys/select.h> //fonction select
#include <sys/time.h> //fonction select

int main(int argc, char* argv[]){

  //On vérifie si le nombre d'arguments est bien le bon puis on attribue le numéro de port
  if(argc!=2) {
    perror("Mauvais nombre d'arguments");
    exit(-1);
  }
  int port_public = atoi(argv[1]);
  printf("Port du serveur : %d\n",port_public);

  //Création socket UDP
	int socket_UDP = socket(AF_INET,SOCK_DGRAM,0); //protocole UDP
	printf("Descripteur socket UDP : %d\n",socket_UDP);
  if (socket_UDP<0){
		perror("Erreur socket");
		exit(-1);
  }

  int reuse_UDP = 1 ;
  setsockopt(socket_UDP, SOL_SOCKET, SO_REUSEADDR, &reuse_UDP, sizeof(reuse_UDP)) ;

  //Initialisation structure réseau _
  struct sockaddr_in client1_addr, serveur_addr ; //on déclare aussi le client
  memset((char*)&serveur_addr, 0, sizeof(serveur_addr)) ; //adresse serveur
	memset((char*)&client1_addr, 0, sizeof(client1_addr)) ; //adresse clientUDP
  socklen_t len = sizeof(struct sockaddr);

  int port_data = 1000;

  //Serveur
  serveur_addr.sin_addr.s_addr = INADDR_ANY ;
  printf("Adresse serveur =%d\n",serveur_addr.sin_addr.s_addr);
  serveur_addr.sin_family=AF_INET;
  serveur_addr.sin_port=htons(port_public);
	printf("Port du serveur UDP: %d\n",port_public);

  //Lien socket-structure
  int bind_UDP = bind(socket_UDP,(struct sockaddr*)&serveur_addr,sizeof(struct sockaddr_in));
  printf("Bind UDP = %d\n",bind_UDP);

  char bufferUDP_read_server[100]; //on crée un buffer pour stocker 99 caractères (le dernier étant réservé au \0 pour signaler la fin de la chaîne
  char bufferUDP_write_server[100];
  char port_data_string[10];

  while(1){
    printf("Boucle while n°1.\n");
    //Echange UDP

    int n = recvfrom(socket_UDP,bufferUDP_read_server,sizeof(bufferUDP_read_server),0,(struct sockaddr *)&client1_addr,&len);
    printf("n : %d\n",n);
    bufferUDP_read_server[n]='\0';
    printf("Client 1 :%s\n", bufferUDP_read_server);

    int result = strcmp("SYN",bufferUDP_read_server);
    printf("Resultat de la comparaison : %d\n",result);

     //Si on reçoit un SYN
    if(result==0){
      printf("Le message reçu est bien un SYN\n");
      memset(bufferUDP_write_server,0,sizeof(bufferUDP_write_server));
      memcpy(bufferUDP_write_server,"SYN-ACK",7);

      int data_UDP = socket(AF_INET, SOCK_DGRAM, 0); //quand on reçoit un syn, on créer un nouvelle socket pour les prochains échanges
      struct sockaddr_in my_addr_data;
      memset((char*)&my_addr_data, 0, sizeof(my_addr_data));
      port_data = port_data + 1;
      my_addr_data.sin_family      = AF_INET ;
      my_addr_data.sin_port        = htons(port_data) ;
      my_addr_data.sin_addr.s_addr = INADDR_ANY ;

      int bind_data = bind (data_UDP, (struct sockaddr *)&serveur_addr, sizeof(struct sockaddr_in));
      printf("bind de data : %d\n", bind_data);

      sprintf(port_data_string,"%d",port_data);
      memcpy(bufferUDP_write_server+7, port_data_string, 4);

      sendto(socket_UDP, bufferUDP_write_server, strlen(bufferUDP_write_server), 0, (struct sockaddr *)&client1_addr, len);
      printf("msg envoyé au client : %s\n", bufferUDP_write_server);
    } else {
      printf("le message reçu n'est pas un syn\n");
      exit(-1);
    }

    // reception du ack final de la phase de connection
    memset(bufferUDP_read_server,0,sizeof(bufferUDP_read_server));
    int m = recvfrom(socket_UDP, bufferUDP_read_server, sizeof(bufferUDP_read_server), 0, (struct sockaddr *)&client1_addr, &len);
    bufferUDP_read_server[m]='\0';
    printf("message de l'UDP : %s\n", bufferUDP_read_server);


    printf("*** FIN DU TEST ***\n");
    close(socket_UDP);
    //close(data_UDP); ?? non déclarée
    break;

  }

  return(0);
}
