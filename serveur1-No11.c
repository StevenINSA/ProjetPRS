#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> //en-tête nécessaire pour la création de sockets
#include <sys/socket.h> //en-tête nécessaire pour la création de sockets
#include <netinet/in.h> //pour utiliser struct sockaddr
#include <string.h> //pour la fonction memset()
#include <unistd.h> //pour les fonctions read() et write()
#include <arpa/inet.h> //pour traduire au format réseau/machine
#include <sys/select.h> //fonction select
#include <sys/time.h> //pour les timers
#include <math.h> //pour la puissance

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

  int port_data = 1024;

  //Serveur
  serveur_addr.sin_addr.s_addr = INADDR_ANY ;
  printf("Adresse serveur =%d\n",serveur_addr.sin_addr.s_addr);
  serveur_addr.sin_family=AF_INET;
  serveur_addr.sin_port=htons(port_public);
	printf("Port du serveur UDP: %d\n",port_public);

  //Lien socket-structure
  int bind_UDP = bind(socket_UDP,(struct sockaddr*)&serveur_addr,sizeof(struct sockaddr_in));
  if(bind_UDP<0){
    perror("Erreur bind\n");
    exit(-1);
  }
  printf("Bind UDP = %d\n",bind_UDP);

  char bufferUDP_read_server[100]; //on crée un buffer pour stocker 99 caractères (le dernier étant réservé au \0 pour signaler la fin de la chaîne
  char bufferUDP_write_server[100];
  char port_data_string[16];
  char sent_file[100]; //pour le stockage du nom de fichier
  char buffer_sequence[6];
  char buffer_segment[2000];// à redéfinir en fonction du client

  int data_descriptor = 0; //pour récupérer le descripteur de la nouvelle socket

  //pour le timer (retransmission quand perte du ack)
  fd_set set_descripteur_timer;  //pour pouvoir utiliser un timer, il faut utiliser un select, donc un descripteur
  struct timeval time1, time2, timeout, rtt, time_debit, time_debit_start, time_debit_end;
  rtt.tv_usec = 50000;           //on fixe au début un rtt de 50ms

  while(1){
    printf("Boucle while n°1.\n");
    //Echange UDP

    int size_syn = recvfrom(socket_UDP,bufferUDP_read_server,sizeof(bufferUDP_read_server),0,(struct sockaddr *)&client1_addr,&len);
    printf("size syn : %d\n",size_syn);
    printf("Client 1 :%s\n", bufferUDP_read_server);

    int result = strcmp("SYN",bufferUDP_read_server);
    printf("Resultat de la comparaison : %d\n",result);

     //Si on reçoit un SYN
    if(result==0){
      printf("Le message reçu est bien un SYN\n");
      memset(bufferUDP_write_server,0,sizeof(bufferUDP_write_server));
      memcpy(bufferUDP_write_server,"SYN-ACK",7);

      int data_UDP = socket(AF_INET, SOCK_DGRAM, 0); //quand on reçoit un syn, on créer un nouvelle socket pour les prochains échanges
      int reuse_data_UDP = 1 ;
      setsockopt(data_UDP, SOL_SOCKET, SO_REUSEADDR, &reuse_data_UDP, sizeof(reuse_data_UDP)) ;
      struct sockaddr_in data_addr;
      memset((char*)&data_addr, 0, sizeof(data_addr));
      port_data = port_data + 1;
      data_addr.sin_family      = AF_INET ;
      data_addr.sin_port        = htons(port_data) ;
      data_addr.sin_addr.s_addr = INADDR_ANY ;
      data_descriptor = data_UDP;

      int bind_data = bind (data_UDP, (struct sockaddr *)&data_addr, sizeof(struct sockaddr_in));
      if(bind_data<0){
        perror("Erreur data :");
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
    recvfrom(socket_UDP, bufferUDP_read_server, sizeof(bufferUDP_read_server), 0, (struct sockaddr *)&client1_addr, &len);
    printf("message du client doit etre un ack : %s\n", bufferUDP_read_server);
    printf("Fermeture socket_udp principale\n");
    close(socket_UDP);

    /*
     *  Phase envoi de donnée
     */


    //le client envoie le nom du fichier qu'il veut recevoir
    memset(bufferUDP_read_server,0,sizeof(bufferUDP_read_server));
    int size_file_name = recvfrom(data_descriptor, bufferUDP_read_server, sizeof(bufferUDP_read_server), 0, (struct sockaddr *)&client1_addr, &len);
    printf("le client veut : %s\n", bufferUDP_read_server);

    //il faut stocker le nom du fichier pour pouvoir le passer dans fopen
    memcpy(sent_file, bufferUDP_read_server, size_file_name);
    printf("fichier a envoyer : %s\n", sent_file);

    printf("envoi du fichier pdf\n");
    FILE *file = fopen(sent_file, "rb"); //rb : ouvre le pdf au format binaire car on échange des bits sur les sockets
    if (file == 0){
     perror("ERREUR OUVERTURE DU FICHIER");
     exit(-1);
    }

    //on calcule ensuite la taille du fichier à envoyer
    fseek(file, 0, SEEK_END);          //fseek parcours le fichier et place un curseur à la fin appelée SEEK END
    int size_file = ftell(file);  //ftell donne la taille du chemin parcouru par fseek (valeur de la position du curseur)

    printf("taille du fichier en octet : %d\n", size_file);
    fseek(file, 0, SEEK_SET);          //on replace le curseur au début;
    char file_buffer[size_file];
    //les octets lus sont stockés dans buffer_fichier
    //size_t read_blocks = fread(file_buffer,1,500,fichier); //500 blocs de 1 octet
    size_t read_blocks = fread(file_buffer,size_file,1,file); //on lit le fichier en un coup

    if(read_blocks!=1){
      perror("erreur lecture fichier");
      ferror(file);
    }
    int packets_size = 1494; //pour arriver à une taille de 1500 octets avec les 6 du n° de séquence
    int packets_number = size_file/packets_size;
    int seq = 1;
    int window_size = 20; //on fixe une fenêtre de 50 segments à envoyer sans attendre de ack (en sachant que le client 1 drop à partir de 100)
    int window = window_size; //cette valeur va servir de seuil pour fixer le nombre de segment qu'on envoit

    gettimeofday(&time_debit_start, NULL); //pour le calcul du débit, on lance le chrono quand on commence la transmission du fichier

    while (1){ //phase envoie de segments
      //printf("For i = %d\n",seq);
      //printf("On copie à partir de file_buffer[%d]\n",packets_size*(seq-1));

      while (seq <= window && seq <= packets_number+1) { //si le n° de seq est inférieur à la taille de la fenêtre (ou inférieur au nombre de ack à recevoir), on envoit
        //Remise à zéro des buffers
        memset(buffer_segment,0,sizeof(buffer_segment));
        memset(buffer_sequence,0,sizeof(buffer_sequence));

        sprintf(buffer_sequence,"%d",seq);
        printf("Sequence number (from buffer_sequence) : %s\n",buffer_sequence);

        //Segment auquel on rajoute en-tête
        memcpy(buffer_segment,buffer_sequence,6);
        memcpy(buffer_segment+6,file_buffer+packets_size*(seq-1),packets_size);

        sendto(data_descriptor,buffer_segment,packets_size+6,0,(struct sockaddr *)&client1_addr,len);
        seq++;

      }
      //on lance le timer une fois qu'on a envoyé notre salve
      gettimeofday(&time1, NULL); //on place la valeur de gettimeofday dans un timer dans le but de récupurer le rtt plus tard
      //on a fini d'envoyer toute les données, on attend le ack

      //partie mise en place du timer pour la retransmission
      FD_ZERO(&set_descripteur_timer);
      FD_SET(data_descriptor, &set_descripteur_timer);
      timeout.tv_usec = 10* rtt.tv_usec; //on sécurise le temps d'attente de retransmission
      timeout.tv_sec = 0; //bien remettre tv_sec à 0 sinon il prend des valeurs et fausse le timeout
      printf("valeur du timeout en µs : %d\n", timeout.tv_usec);
      //il faut refixer les valeurs de timout à chaque boucle car lors d'un timout, timeout sera fixé à 0. Timeout sera calculé en fct du rtt

      sleep(timeout.tv_usec*pow(10,-6)); //le temps de recevoir les derniers acks envoyés par le client

      int select_value = select(data_descriptor+1, &set_descripteur_timer, NULL, NULL, &timeout); //on écoute sur la socket pendant une durée timeout

      if (select_value == -1)
        perror("select error\n");

      else if (FD_ISSET(data_descriptor, &set_descripteur_timer)){ //si on a une activité sur la socket (i.e on reçoit un ack)

        memset(bufferUDP_read_server, 0, sizeof(bufferUDP_read_server));
        memset(buffer_sequence, 0, sizeof(buffer_sequence));

        int size_seq = recvfrom(data_descriptor, bufferUDP_read_server, sizeof(bufferUDP_read_server), 0, (struct sockaddr *)&client1_addr, &len);
        memcpy(buffer_sequence, bufferUDP_read_server+3, size_seq-3); //+3 car les 3 premières valeurs sont pour le mot ACK
        gettimeofday(&time2, NULL);                                   //on recalcule une timeofday pour faire la différence avec le premier
        rtt.tv_usec = (time2.tv_sec-time1.tv_sec)*pow(10,6) + (time2.tv_usec - time1.tv_usec);         //on estime ainsi le rtt à chaque échange, on rajoute les secondes au cas où

        printf("estimation du RTT : %d\n", rtt.tv_usec);
        printf("message reçu : %s\n", bufferUDP_read_server);
        printf("numéro de seq reçue par le serveur (buffer_check_sequence) : %s\n",buffer_sequence);
        //printf("atoi de buffer_check_sequence %d\n", atoi(buffer_sequence));

        //if (atoi(buffer_sequence) == seq){ //si le numéro de séquence reçu est égale au numéro de séquence envoyé
          //seq++;                           //on peut alors envoyer la séquence suivante
        //} else{
        //  printf("retransmission du n° de seq : %d \n", seq);
        //}
        seq = atoi(buffer_sequence) + 1; //on fait glisser la fenêtre, on va transmettre à partir de la valeur du ACK
        window = seq + window_size;
        printf("on transmet à partir du n° : %d\n", seq);
      }
      else {
        printf("segment perdu - Timeout ! Retransmission\n");
        rtt.tv_usec = 50000; //si un timeout a lieu, on remet notre rtt élevé pour pas attendre trop peu longtemps lors de la retransmission
      }
    if (atoi(buffer_sequence) == packets_number+1) //si le client ack le dernier segment à envoyer, on sort de la boucle
      break;
    } //fin while
    gettimeofday(&time_debit_end, NULL);

    printf("taille du fichier envoyé : %d\n", size_file);

    printf("*** FIN DE TRANSMISSION ***\n");
    memset(bufferUDP_write_server,0,sizeof(bufferUDP_write_server));
    memcpy(bufferUDP_write_server,"FIN",3);

    sendto(data_descriptor,bufferUDP_write_server,sizeof(bufferUDP_write_server),0,(struct sockaddr *)&client1_addr,len);

    time_debit.tv_usec = (time_debit_end.tv_sec-time_debit_start.tv_sec)*pow(10,6) + (time_debit_end.tv_usec - time_debit_start.tv_usec);
    float debit = (float)size_file / time_debit.tv_usec;
    printf("débit lors de la transmission : %f Mo/s\n", debit);
    printf("temps débit en micro sec : %d\n", time_debit.tv_usec);

    close(data_descriptor);
    break;

  }

  return(0);
}
