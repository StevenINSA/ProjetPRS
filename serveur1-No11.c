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
  struct sockaddr_in client1_addr, serveur_addr; //on déclare aussi le client et pour la socket data
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
  char port_data_string[16];
  char fichier_a_envoyer[32] = ""; //pour le stockage du nom de fichier

  int descripteur_data = 0; //pour récupérer le descripteur de la nouvelle socket

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
      setsockopt(data_UDP, SOL_SOCKET, SO_REUSEADDR, &reuse_UDP, sizeof(reuse_UDP)) ;
      struct sockaddr_in data_addr;
      memset((char*)&data_addr, 0, sizeof(data_addr));
      port_data = port_data + 1;
      data_addr.sin_family      = AF_INET ;
      data_addr.sin_port        = htons(port_data) ;
      data_addr.sin_addr.s_addr = INADDR_ANY ;
      descripteur_data = data_UDP;

      int bind_data = bind (data_UDP, (struct sockaddr *)&data_addr, sizeof(struct sockaddr_in));
      if(bind_data<0){
        perror("Erreur data :\n");
        exit(-1);
      }
      printf("bind de data : %d\n", bind_data);

      sprintf(port_data_string,"%d",port_data);
      memcpy(bufferUDP_write_server+7, port_data_string, 4);

      sendto(socket_UDP, bufferUDP_write_server, sizeof(bufferUDP_write_server), 0, (struct sockaddr *)&client1_addr, len);
      printf("msg envoyé au client : %s\n", bufferUDP_write_server);

    } else {
      printf("le message reçu n'est pas un syn\n");
      exit(-1);
    }

    // reception du ack final de la phase de connection
    memset(bufferUDP_read_server,0,sizeof(bufferUDP_read_server));
    int m = recvfrom(socket_UDP, bufferUDP_read_server, sizeof(bufferUDP_read_server), 0, (struct sockaddr *)&client1_addr, &len);
    bufferUDP_read_server[m]='\0';
    printf("message du client doit etre un ack : %s\n", bufferUDP_read_server);
    printf("Fermeture socket_udp principale\n");
    close(socket_UDP);

    /*
     *  Phase envoi de donnée WARNING : mettre sur la machine du département le projet.pdf
     */

/*
    //le client envoie le nom du fichier qu'il veut recevoir
    memset(bufferUDP_read_server,0,sizeof(bufferUDP_read_server));
    int o = recvfrom(descripteur_data, bufferUDP_read_server, sizeof(bufferUDP_read_server), 0, (struct sockaddr *)&client1_addr, &len);
    bufferUDP_read_server[o]='\0';
    printf("le client veut : %s\n", bufferUDP_read_server);

    //il faut stocker le nom du fichier pour pouvoir le passer dans fopen
    snprintf(fichier_a_envoyer, sizeof(fichier_a_envoyer), "%s", bufferUDP_read_server);
    printf("fichier a envoyer : %s\n", fichier_a_envoyer);

    printf("envoi du fichier pdf\n");
    FILE *fichier = fopen(fichier_a_envoyer, "rb"); //rb : ouvre le pdf au format binaire car on échange des bits sur les sockets
    if (fichier == 0){
     perror("ERREUR OUVERTURE DU FICHIER");
     exit(-1);
    }

    //on calcule ensuite la taille du fichier à envoyer
    fseek(fichier, 0, SEEK_END);          //fseek parcours le fichier et place un curseur à la fin appelée SEEK END
    int taille_fichier = ftell(fichier);  //ftell donne la taille du chemin parcouru par fseek (valeur de la position du curseur)

    printf("taille du fichier en octet : %d\n", taille_fichier);
    fseek(fichier, 0, SEEK_SET);          //on replace le curseur au début;
*/

    printf("*** FIN DU TEST ***\n");

    close(descripteur_data);
    break;

  }

  return(0);
}
