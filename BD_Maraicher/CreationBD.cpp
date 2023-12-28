#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <time.h>
#include <string.h>

typedef struct
{
  int   id;
  char  intitule[20];
  float prix;
  int   stock;  
  char  image[20];
} ARTICLE;

ARTICLE Elm[] = 
{
  {-1,"carottes",2.16f,29,"carottes.jpg"},
  {-1,"cerises",9.75f,38,"cerises.jpg"},
  {-1,"artichaut",1.62f,45,"artichaut.jpg"},
  {-1,"bananes",2.6f,58,"bananes.jpg"},
  {-1,"champignons",10.25f,44,"champignons.jpg"},
  {-1,"concombre",1.17f,65,"concombre.jpg"},
  {-1,"courgette",1.17f,74,"courgette.jpg"},
  {-1,"haricots",10.82f,57,"haricots.jpg"},
  {-1,"laitue",1.62f,60,"laitue.jpg"},
  {-1,"oranges",3.78f,83,"oranges.jpg"},
  {-1,"oignons",2.12f,44,"oignons.jpg"},
  {-1,"nectarines",10.38f,36,"nectarines.jpg"},
  {-1,"peches",8.48f,51,"peches.jpg"},
  {-1,"poivron",1.29f,53,"poivron.jpg"},
  {-1,"pommes de terre",2.17f,75,"pommesDeTerre.jpg"},
  {-1,"pommes",4.00f,76,"pommes.jpg"},
  {-1,"citrons",4.44f,71,"citrons.jpg"},
  {-1,"ail",1.08f,54,"ail.jpg"},
  {-1,"aubergine",1.62f,67,"aubergine.jpg"},
  {-1,"echalotes",6.48f,43,"echalotes.jpg"},
  {-1,"tomates",5.49f,62,"tomates.jpg"}
};

int main(int argc,char *argv[])
{
  // Connection a MySql
  printf("Connection a la BD...\n");
  MYSQL* connexion = mysql_init(NULL);
  mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

  // Creation d'une table UNIX_FINAL
  printf("Creation de la table articles...\n");
  printf("Creation de la table clients...\n");
  mysql_query(connexion,"SET FOREIGN_KEY_CHECKS = 0;");
  mysql_query(connexion,"drop table articles ;"); // au cas ou elle existerait deja
  mysql_query(connexion,"drop table clients;"); // au cas ou elle existerait deja
  mysql_query(connexion,"drop table factures;"); // au cas ou elle existerait deja
  mysql_query(connexion,"drop table ventes;"); // au cas ou elle existerait deja
  mysql_query(connexion,"drop table employes;"); // au cas ou elle existerait deja
  mysql_query(connexion,"create table articles (id INT(4) AUTO_INCREMENT PRIMARY KEY, intitule varchar(20),prix FLOAT(4),stock INT(4),image varchar(20));");
  mysql_query(connexion,"CREATE TABLE clients (id INT AUTO_INCREMENT PRIMARY KEY,login VARCHAR(255) NOT NULL,password VARCHAR(255) NOT NULL);");
  mysql_query(connexion,"CREATE TABLE factures (Id INT AUTO_INCREMENT PRIMARY KEY,idClient INT,date DATE,montant DECIMAL(10, 2),paye ENUM('impaye', 'paye'),FOREIGN KEY (idClient) REFERENCES clients(id));");
  mysql_query(connexion,"CREATE TABLE ventes (idFacture INT NOT NULL, idArticle INT NOT NULL, quantite INT, FOREIGN KEY (idFacture) REFERENCES factures(Id), FOREIGN KEY (idArticle) REFERENCES articles(id));");
  mysql_query(connexion,"CREATE TABLE employes (id INT AUTO_INCREMENT PRIMARY KEY,login VARCHAR(255) NOT NULL,password VARCHAR(255) NOT NULL);");
  mysql_query(connexion,"INSERT INTO employes (id, login, password) VALUES ('admin', 'admin'");

  mysql_query(connexion,"SET FOREIGN_KEY_CHECKS = 1;");

  // Ajout de tuples dans la table UNIX_FINAL
  printf("Ajout de 21 articles la table articles...\n");
  char requete[256];
  for (int i=0 ; i<21 ; i++)
  {
    sprintf(requete,"insert into articles values (NULL,'%s',%f,%d,'%s');",Elm[i].intitule,Elm[i].prix,Elm[i].stock,Elm[i].image);
    mysql_query(connexion,requete);
  }

  // Deconnection de la BD
  mysql_close(connexion);
  exit(0);
}
