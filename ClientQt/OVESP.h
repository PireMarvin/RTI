#ifndef OVESP_H
#define OVESP_H

#define NB_MAX_CLIENTS 100
#define PANIER_MAX 6  //5

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

typedef struct ARTICLEPANIER
{
  int   id;
  char intitule[20]; 
  double prix;
  int   stock;
} ARTICLEPANIER;

bool OVESP(char* requete, char* reponse, int socket);
void OVESP_Logout();

#endif
