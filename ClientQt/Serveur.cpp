#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <signal.h> 
#include <pthread.h>
#include "TCP.h"
#include "OVESP.h"

void HandlerSIGINT(int s);
void TraitementConnexion(int sService);
void* FctThreadClient(void* p);

int sEcoute;

//Gestion du pool de threads
#define NB_THREADS_POOL 5
#define TAILLE_FILE_ATTENTE 20
int socketAcceptees[TAILLE_FILE_ATTENTE];
int indiceEcriture = 0, indiceLecture = 0;
pthread_mutex_t mutexSocketsAcceptees;
pthread_cond_t condSocketAcceptees;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Erreur...\n");
		printf("USAGE : Serveur portServeur\n");
		exit(1);
	}

	//Initialisation socketsAcceptees
	pthread_mutex_init(&mutexSocketsAcceptees, NULL);
	pthread_cond_init(&condSocketAcceptees, NULL);
	for(int i = 0; i<TAILLE_FILE_ATTENTE; i++)
		socketAcceptees[i] = -1;

	//Armement des signaux
	struct sigaction A;
	A.sa_flags = 0;
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGINT;
	if(sigaction(SIGINT, &A, NULL) == -1)
	{
		perror("Erreur de sigaction");
		exit(1);
	}

	//Création de la socket d'écoute
	if((sEcoute = ServerSocket(atoi(argv[1]))) == -1)
	{
		perror("Erreur de ServerSocket");
		exit(1);
	}

	//Création du pool de threads
	printf("Création du pool de threads.\n");
	pthread_t th;
	for(int i = 0; i<NB_THREADS_POOL; i++)
		pthread_create(&th, NULL, FctThreadClient, NULL);

	//mise en boucle du serveur
	int sService;
	char ipClient[50];
	printf("Demarrage du serveur.\n");
	while(1)
	{
		printf("Attente d'une connexion...\n");
		if((sService = Accept(sEcoute, ipClient)) == -1)
		{
			perror("Erreur de Accept");
			close(sEcoute);
			OVESP_Logout();
			exit(1);
		}
		printf("Connexion accepté : IP = %s socket = %d\n", ipClient, sService);

		//Insertion en liste d'attente et réveil d'un thread du pool
		//(Production d'une tache)
		pthread_mutex_lock(&mutexSocketsAcceptees);
		socketAcceptees[indiceEcriture] = sService;
		indiceEcriture++;
		if(indiceEcriture == TAILLE_FILE_ATTENTE) indiceEcriture = 0;
		pthread_mutex_unlock(&mutexSocketsAcceptees);
		pthread_cond_signal(&condSocketAcceptees);
	}
}

void* FctThreadClient(void* p)
{
	int sService;

	while(1)
	{
		printf("\t [THREAD %p] Attente socket...\n", (void*)pthread_self());

		//Attente d'une tache
		pthread_mutex_lock(&mutexSocketsAcceptees);
		while(indiceEcriture == indiceLecture)
			pthread_cond_wait(&condSocketAcceptees, &mutexSocketsAcceptees);

		sService = socketAcceptees[indiceLecture]; //récupération de la socket de service en local
		socketAcceptees[indiceLecture] = -1;
		indiceLecture++;
		if(indiceLecture == TAILLE_FILE_ATTENTE) indiceLecture = 0;
		pthread_mutex_unlock(&mutexSocketsAcceptees);

		//traitement de la connexion (consommation de la tache)
		printf("\t[THREAD %p] je m'occupe de la socket %d\n", (void*)pthread_self(), sService);
		TraitementConnexion(sService);
	}
}

void TraitementConnexion(int sService)
{
	char requete[200], reponse[200];
	int nbLus, nbEcrits;
	bool onContinue = true;

	while(onContinue)
	{
		printf("\t[THREAD %p] Attente requete...\n", (void*)pthread_self());
		// ********Reception Requete*****************************************
		if((nbLus = Receive(sService, requete)) <0)
		{
			perror("Erreur de Receive");
			close(sService);
			HandlerSIGINT(0);
		}

		//******Fin de connexion? *******************************************
		if(nbLus == 0)
		{
			printf("\t[THREAD %p] Fin de connexion du client.\n", (void*)pthread_self());
			close(sService);
			return;
		}
		requete[nbLus] = 0;
		printf("\t[THREAD %p] Requte recue = %s\n", (void*)pthread_self(), requete);

		//*****Traitement de la requete***************************************
		onContinue = OVESP(requete, reponse, sService);

		//*****Envoie de la reponse*******************************************
		if((nbEcrits = Send(sService, reponse, strlen(reponse))) < 0)
		{
			perror("Erreur de Send");
			close(sService);
			HandlerSIGINT(0);
		}
		printf("\t[THREAD %p] Reponse envoyee = %s\n", (void*)pthread_self(), reponse);

		if(!onContinue)
			printf("\t[THREAD %p] Fin de connexion de la socket %d\n", (void*)pthread_self() , sService);
	}
}

void HandlerSIGINT(int s)
{
	printf("\nArret du serveur.\n");
	close(sEcoute);
	pthread_mutex_lock(&mutexSocketsAcceptees);
	for(int i=0; i<TAILLE_FILE_ATTENTE; i++)
		if(socketAcceptees[i] != -1) close(socketAcceptees[i]);
	pthread_mutex_unlock(&mutexSocketsAcceptees);
	OVESP_Logout();
	exit(0);
}