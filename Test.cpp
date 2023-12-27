#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main()
{
    int sEcoute;
    printf("pid = %d\n",getpid());
    // Creation de la socket
    if ((sEcoute = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Erreur de socket()");
        exit(1);
    }
    printf("socket creee = %d\n",sEcoute);

    // Construction de l'adresse
    struct addrinfo hints; 
    struct addrinfo *results;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;// pour une connexion passive
    if (getaddrinfo(NULL,"50000",&hints,&results) != 0) 
    {
        close(sEcoute);
        exit(1);
    }

    // Affichage du contenu de l'adresse obtenue (optionnel, c’est juste pour info)
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    struct addrinfo* info;
    getnameinfo(results->ai_addr,results->ai_addrlen, host,NI_MAXHOST,port,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST);
    printf("Mon Adresse IP: %s -- Mon Port: %s\n",host,port);
    
    // Liaison de la socket à l'adresse
    if (bind(sEcoute,results->ai_addr,results->ai_addrlen) < 0)
    {
        perror("Erreur de bind()");
        exit(1);
    }
    freeaddrinfo(results);
    printf("bind() reussi !\n");

    //Mise à l'écoute de la socket
    if(listen(sEcoute, SOMAXCONN) == -1)
    {
    	perror("Erreur de listen()");
    	exit(1);
    }
    printf("listen() réussi !\n");
    
    //Attente d'une connexion
    int sService;
    if((sService = accept(sEcoute, NULL, NULL))== -1)
    {
    	perror("Erreur de accept()");
    	exit(1);
    }
    printf("accept () réussi !");
    printf("socket de service = %d\n", sService);

    //Récupération d'information sur le client connecte
    struct sockaddr_in adrClient;
    socklen_t adrClientLen;
    getpeername (sService, (struct sockaddr*)&adrClient, &adrClientLen);
    getnameinfo((struct sockaddr*)&adrClient,adrClientLen,
    			host, NI_MAXHOST,
    			port,NI_MAXSERV,
    			NI_NUMERICSERV | NI_NUMERICHOST);
    printf("Client connecte --> Adresse IP : %s -- Port : %s\n", host, port);

    //Lecture sur la socket
    int nb;
    char buffer1[50];
    if((nb = read(sService, buffer1, 5)) == -1)
    {
    	perror("Erreur de read()");
    	close (sEcoute);
    	close (sService);
    }
    buffer1[nb] = 0;
    printf("nbLus = %d Lu: --%s--\n", nb, buffer1);

    //Seconde lecture sur la socket
    char buffer2[50];
    if((nb = read(sService, buffer2, 5)) == -1)
    {
    	perror("Erreur de read()");
    	close(sEcoute);
    	close(sService);
    }
    buffer2[nb] = 0;
    printf("nbLus = %d Lu: --%s--\n", nb, buffer2);

    //Ecriture sur la socket
    char reponse[50];
    sprintf(reponse,"[Serveur] %s%s", buffer1, buffer2);

    if((nb = write(sService, reponse, strlen(reponse))) == -1)
    {
    	perror("Erreur de write()");
    	close(sEcoute);
    	close(sService);
    }
    printf("nbEcrits = %d Ecrit: --%s--\n", nb, reponse);

    //Fermeture de la connexion cote serveur
    close(sService);
    close(sEcoute);

    exit(0);
    
}