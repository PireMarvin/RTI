#ifndef SGBD_H
#define SGBD_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <mysql.h>
#include <time.h>

int loginRequet(char *login, char *password);
bool InsertClient(char *login, char *password);
MYSQL *ConnexionBD();
MYSQL_ROW getArticleById(int IdArticle);
void removeArticle(int idArticle,int quantite);
void addArticle(int idArticle,int quantite);
int Facture(const char *login, float montantPanier);
int getIdClient(const char *login);
void ventes(int idFacture, int idArticle, int quantite);

#endif