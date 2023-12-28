#include "SGBD.h"

MYSQL* ConnexionBD()
{
    MYSQL* connexion = mysql_init(NULL);

    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0)==NULL)
    {
        fprintf(stderr,"(ACCESBD) Erreur de connexion à la base de données...\n");
        exit(1);  
    }
    return connexion;
}

int loginRequet(char *login, char *password)
{
    MYSQL *connexion = ConnexionBD();
    MYSQL_RES *result;
    char query[256];

    // printf("Login : '%s' Mot de passe : '%s'\n", login, password);
    // Verifier si le client est dans la BD
    memset(query, 0, 256);
    sprintf(query, "SELECT * FROM clients WHERE login like '%s' and password like '%s'", login, password);
    if (mysql_query(connexion, query))
    {
        fprintf(stderr, "Erreur lors de la requête SELECT : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return 0;
    }

    result = mysql_store_result(connexion);

    if(mysql_num_rows(result)==0)
    {
        printf("Client inconnu\n");
        mysql_free_result(result); // Libérez la mémoire du résultat
        mysql_close(connexion);
        return 0;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    printf("Login succes !\n");
    mysql_free_result(result); // Libérez la mémoire du résultat
    mysql_close(connexion);
    return atoi(row[0]);
}

bool InsertClient(char *login, char *password)
{
    MYSQL *connexion = ConnexionBD();
    MYSQL_RES *result;
    char query[256];

    // Verifier si le client est déja inscit
    sprintf(query, "SELECT id FROM clients WHERE login like '%s'", login);
    if (mysql_query(connexion, query))
    {
        fprintf(stderr, "Erreur lors de la requête SELECT : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }
    
    result = mysql_store_result(connexion);

    if (mysql_num_rows(result) > 0) 
    {
        printf("Le client existe déjà. Impossible d'ajouter un doublon.\n");
        mysql_free_result(result); // Libérez la mémoire du résultat
        mysql_close(connexion);
        return false;
    }
    
    // Insérez le nouveau client s'il n'existe pas encore
    memset(query, 0, 256);
    sprintf(query, "INSERT INTO clients (login, password) VALUES ('%s', '%s')", login, password);
    if (mysql_query(connexion, query))
    {
        fprintf(stderr, "Erreur lors de l'insertion dans la base de données : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }
    printf("Nouvel utilisateur ajouté avec succès !\n");
    mysql_close(connexion);
    return true;
}

MYSQL_ROW getArticleById(int IdArticle)
{
    MYSQL *connexion = ConnexionBD();
    char query[256];

    snprintf(query, sizeof(query), "SELECT * FROM articles WHERE id = %d", IdArticle);

    if (mysql_query(connexion, query) != 0)
    {
        fprintf(stderr, "Échec de l'exécution de la requête : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        exit(1);
    }

    MYSQL_RES *result = mysql_store_result(connexion);
    if (result == NULL) {
        fprintf(stderr, "Échec de la récupération du résultat : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        exit(1);
    }

    int num_rows = mysql_num_rows(result);
    if (num_rows == 0)
    {
        // L'article n'a pas été trouvé, renvoyer une erreur
        mysql_free_result(result);
        mysql_close(connexion);
        fprintf(stderr, "L'article avec l'ID %d n'a pas été trouvé.\n", IdArticle);
        return 0; // Vous pouvez choisir de gérer l'erreur de manière différente si nécessaire
    }

    // Récupérer les données de l'article
    MYSQL_ROW row = mysql_fetch_row(result);

    mysql_free_result(result);
    mysql_close(connexion);
    return row;
}

void removeArticle(int idArticle, int quantite)
{
    MYSQL *connexion = ConnexionBD();
    char query[256];

    // Utilisez une requête UPDATE pour mettre à jour la quantité
    snprintf(query, sizeof(query), "UPDATE articles SET stock = stock - %d WHERE id = %d", quantite, idArticle);

    if (mysql_query(connexion, query) != 0) {
        fprintf(stderr, "Échec de l'exécution de la requête : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        exit(1);
    }
    mysql_close(connexion);
}

void addArticle(int idArticle, int quantite)
{
    MYSQL *connexion = ConnexionBD();
    char query[256];

    // Utilisez une requête UPDATE pour mettre à jour la quantité
    snprintf(query, sizeof(query), "UPDATE articles SET stock = stock + %d WHERE id = %d", quantite, idArticle);

    if (mysql_query(connexion, query) != 0) {
        fprintf(stderr, "Échec de l'exécution de la requête : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        exit(1);
    }
    mysql_close(connexion);
}

int Facture(const char *login, float montantPanier) 
{
    MYSQL *connexion = ConnexionBD(); 

    // Récupérer l'ID du client en fonction du login et du mot de passe
    //printf("\nLogin = %s\n", login);
    
    int idClient = getIdClient(login);
    if (idClient == -1)
    {
        fprintf(stderr, "Échec de récupération de l'ID du client. Facture non créée.\n");
        mysql_close(connexion);
        exit(1);
    }
    char query[256];

    // Obtenir la date actuelle au format DATE
    time_t t;
    struct tm *tm_info;
    char dateFacture[11]; // Format "YYYY-MM-DD"

    time(&t);
    tm_info = localtime(&t);
    strftime(dateFacture, 11, "%Y-%m-%d", tm_info);

    // Insérer la nouvelle facture dans la table factures
    snprintf(query, sizeof(query), "INSERT INTO factures (idClient, date, montant) VALUES (%d, '%s', %.2f)",
            idClient, dateFacture, montantPanier);

    if (mysql_query(connexion, query) != 0)
    {
        fprintf(stderr, "Échec de l'insertion de la facture : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        exit(1); 
    }

    // Récupérer l'ID de la dernière facture insérée
    unsigned int lastID = mysql_insert_id(connexion);

    mysql_close(connexion);

    return lastID;
}

int getIdClient(const char *login) 
{
    MYSQL *connexion = ConnexionBD();

    char query[256];
    int idClient = -1; // Initialiser à une valeur d'erreur par défaut.

    sprintf(query, "SELECT id FROM clients WHERE login like '%s'", login);
    // snprintf(query, sizeof(query), "SELECT id FROM clients WHERE login = '%s'", login);
    if (mysql_query(connexion, query) != 0)
    {
        fprintf(stderr, "Échec de la récupération de l'ID du client : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        exit(1); 
    }

    MYSQL_RES *result = mysql_store_result(connexion);
    if (result != NULL)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row != NULL)
            idClient = atoi(row[0]);
        mysql_free_result(result);
    }
    mysql_close(connexion);

    return idClient;
}

void ventes(int idFacture, int idArticle, int quantite)
{
    MYSQL *connexion = ConnexionBD();
            
    char query[256];
    sprintf(query, "INSERT INTO ventes (idFacture, idArticle, quantite) VALUES (%d, %d, %d)", idFacture, idArticle, quantite);
    if (mysql_query(connexion, query) != 0)
    {
        fprintf(stderr, "mysql_query() failed\n");
        mysql_close(connexion);
        exit(1);
    }

    mysql_close(connexion);
}