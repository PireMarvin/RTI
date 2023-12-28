 #include "TCP.h"

int ServerSocket(int port)
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

    std::string portString = std::to_string(port); //convertir port en une chaine de caractère
    const char* portCharPtr = portString.c_str(); //obtenir un pointeur char* à partir de la chaine

    if (getaddrinfo(NULL,portCharPtr,&hints,&results) != 0) 
    {
        close(sEcoute);
        exit(1);
    }

    // Récupération de l'adresse IP de results
    struct sockaddr_in* addr = (struct sockaddr_in*) results->ai_addr;
    char ipV4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ipV4, INET_ADDRSTRLEN);

    // Maintenant, ipV4 contient l'adresse IP en format lisible
    printf("Adresse IP : %s\n", ipV4);    

    // Liaison de la socket à l'adresse
    if (bind(sEcoute,results->ai_addr,results->ai_addrlen) < 0)
    {
        perror("Erreur de bind()");
        exit(1);
    }
    freeaddrinfo(results);
    printf("bind() reussi !\n");

    return sEcoute;
}

int Accept(int sEcoute, char *ipClient)
{
	//Écoute de la socket
	if(listen(sEcoute, SOMAXCONN) == -1)
    {
    	perror("Erreur de listen()");
    	exit(1);
    }
    printf("listen() réussi !\n");
    //printf("Test après listent de tcp\n");

    //Attente d'une connexion
    int sService;
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);


    if((sService = accept(sEcoute, (struct sockaddr*)&clientAddr, &addrLen))== -1)
    {
    	perror("Erreur de accept()");
    	exit(1);
    }
    printf("accept () réussi !\n");

    //obtenez l'adresse ip du client à partir de la structure sockaddr_in
    char clientIP[INET_ADDRSTRLEN];
    if(inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL)
    {
        perror("Erreur de inet_ntop()");
        exit(1);
    }

    //Copiez l'adresse ip dans la chaine ipClient fourni en argument
    strncpy(ipClient, clientIP, INET_ADDRSTRLEN);
    ipClient[INET_ADDRSTRLEN -1] = '\0';
    printf("socket de service = %d\n", sService);

    return sService;
}

int ClientSocket(char* ipServeur, int portServeur)
{
	int sClient;
	// Creation de la socket
    if ((sClient = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Erreur de socket()");
        exit(1);
    }
    printf("socket creee = %d\n",sClient);

    // Construction de l'adresse
    struct addrinfo hints; 
    struct addrinfo *results;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    std::string portString = std::to_string(portServeur); //convertir port en une chaine de caractère
    const char* portCharPtr = portString.c_str(); //obtenir un pointeur char* à partir de la chaine

    if (getaddrinfo(ipServeur,portCharPtr,&hints,&results) != 0) 
    {
        close(sClient);
        exit(1);
    }

    if (connect(sClient,results->ai_addr,results->ai_addrlen) == -1)
    {
    	perror("Erreur de connect()");
    	exit(1);
    }
    printf("Connect() réussi!");

    return sClient;
}

int Send(int sSocket, char* data, int taille)
{
    if(taille > TAILLE_MAX_DATA)
        return -1;

    //preparation de la charge utile
    char trame[TAILLE_MAX_DATA+2];
    memcpy(trame, data, taille);
    trame[taille] = '#';
    trame[taille+1] = ')';

    //écriture sur la socket
    return write(sSocket, trame, taille+2)-2;
}

int Receive(int sSocket, char* data)
{
    bool fini = false;
    int nbLus, i = 0;
    char lu1, lu2;

    while(!fini)
    {
        if((nbLus = read(sSocket, &lu1, 1)) == -1)
            return -1;

        if(nbLus == 0) return i; //connexion fermee par client

        if(lu1 == '#')
        {
            if((nbLus = read(sSocket, &lu2, 1)) == -1)
                return -1;

            if(nbLus == 0) return i; //connexion fermee par client

            if(lu2 == ')') fini = true;
            else
            {
                data[i] = lu1;
                data[i+1] = lu2;
                i+=2;
            }
        }
        else
        {
            data[i] = lu1;
            i++;
        }
    }
    return i;
}