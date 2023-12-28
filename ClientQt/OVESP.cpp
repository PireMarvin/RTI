#include "OVESP.h"
#include "SGBD.h"

//État du protocole : liste des clients loggés **************************************************
int clients[NB_MAX_CLIENTS];
int nbClients = 0;

typedef struct {
    ARTICLEPANIER articles[PANIER_MAX];
    int nombreArticles;
} PanierClient;

typedef struct {
    int socket;
    char login[50];
    char password[50];
} ClientData;

PanierClient paniersClients[NB_MAX_CLIENTS];
ClientData Loginclients[NB_MAX_CLIENTS];

int estPresent(int socket);
void ajoute(int socket);
void retire(int socket);
void str_replace(char *str, char find, char replace);

pthread_mutex_t mutexClients = PTHREAD_MUTEX_INITIALIZER;

//Parsing de la requet et creation de la reponse **********************************
bool OVESP(char* requete, char* reponse, int socket)
{
	for(int i; i<nbClients; i++)
		printf("\t - Client %d : %s \n",i+1,Loginclients[i].login);

    MYSQL_ROW tuple = NULL;

	//Récupération nom de la requete
	char *ptr = strtok(requete, "#");

    // LOGIN
	if(strcmp(ptr, "LOGIN") == 0)
	{
        if(nbClients >= NB_MAX_CLIENTS)
        {
            sprintf(reponse, "LOGIN#ko#trop de clients!");
            return true;
	    }

	    char login[50];
	    char password[50];
	    strcpy(login, strtok(NULL, "#"));
	    strcpy(password, strtok(NULL, "#"));
		bool nouveauxClient = atoi(strtok(NULL, "#"));

		printf("\t[THREAD %p] LOGIN de %s\n", (void*)pthread_self(), login);

	    if (estPresent(socket) >= 0) 
	    {
	        sprintf(reponse, "LOGIN#ko#Client déjà loggé!");
	    	return true;
	    }

    	if (nouveauxClient ? InsertClient(login, password) : loginRequet(login, password))
		{
            PanierClient nouveauPanier;
            memset(nouveauPanier.articles, 0, sizeof(nouveauPanier.articles));
            nouveauPanier.nombreArticles = 0;

            // Associez le nouveau panier au client
            paniersClients[socket] = nouveauPanier;

            ClientData nouveauClient;
            nouveauClient.socket = socket;
            strcpy(nouveauClient.login, login);
            strcpy(nouveauClient.password, password);

            // Ajoutez le client à la liste
            Loginclients[nbClients] = nouveauClient;
            ajoute(socket);

            sprintf(reponse,"LOGIN#ok");
        }
        else
           sprintf(reponse, "LOGIN#ko#%s", (nouveauxClient) ? "Client déjà enregistré!" : "Mauvais identifiant!");
        return true;
    }

    // CONSULT
	if(strcmp(ptr, "CONSULT") == 0)
	{
	    int idArticle = atoi(strtok(NULL, "#"));
        tuple = getArticleById(idArticle);
        if(!tuple)
            sprintf(reponse,"CONSULT#ko#-1");
        else 
        {
	        // Convertir le prix en chaîne avec une virgule au lieu d'un point
	        char prixStr[256];
	        sprintf(prixStr, "%.2f", atof(tuple[2]));
	        str_replace(prixStr, '.', ','); // Remplacer le point par une virgule

	        // Utiliser la chaîne convertie dans la réponse
	        sprintf(reponse, "CONSULT#ok#%d#%s#%s#%d#%s", atoi(tuple[0]), tuple[1], prixStr, atoi(tuple[3]), tuple[4]);
    	}
	}

	// ACHAT
    if(strcmp(ptr, "ACHAT") == 0)
    {
        int idArticle = atoi(strtok(NULL, "#"));
        int quantite = atoi(strtok(NULL, "#"));

        tuple = getArticleById(idArticle);
        if(!tuple)
            sprintf(reponse,"ACHAT#ko#-1");
        else
        {
            if(atoi(tuple[3]) < quantite)
                sprintf(reponse, "ACHAT#ko#0"); //quantite insuffisante
            else
            {
        		PanierClient* panierDuClient = &paniersClients[socket];
		        int i = 0;
		        bool ok;

				for (i = 0; i < PANIER_MAX-1; i++)
		        {
		            if (panierDuClient->articles[i].id == 0 || panierDuClient->articles[i].id == idArticle)
		            {
		                ok = true;
		                break;
		            }
		        }
		        if (ok)
		        {
		            // Remplir l'emplacement du panier avec les informations de l'article
		            if(panierDuClient->articles[i].id == 0)
		            	panierDuClient->nombreArticles++;
		            panierDuClient->articles[i].id = idArticle;
		            strcpy(panierDuClient->articles[i].intitule, tuple[1]);
		            panierDuClient->articles[i].prix = atof(tuple[2]);
		            panierDuClient->articles[i].stock += quantite;

		            sprintf(reponse, "ACHAT#ok");
		            removeArticle(idArticle, quantite);
		        }
		        else
		            sprintf(reponse, "ACHAT#ko#1");
		    }
        }
    }

	// CADDIE
	if (strcmp(ptr, "CADDIE") == 0)
    {
	    PanierClient* panierDuClient = &paniersClients[socket];

       	sprintf(reponse, "CADDIE#ok#%d",panierDuClient->nombreArticles);
	    for (int i = 0; i < panierDuClient->nombreArticles; i++)
	    {
       		if(panierDuClient->articles[i].id != 0)
	    	{
				char prixStr[7];
	            sprintf(prixStr, "%.2f", panierDuClient->articles[i].prix);
	            str_replace(prixStr, '.', ','); // Remplacer le point par une virgule
	            char art[256];
	            sprintf(art, "#%d#%s#%s#%d", panierDuClient->articles[i].id, panierDuClient->articles[i].intitule, prixStr, panierDuClient->articles[i].stock);
	            strcat(reponse, art);
	    	}
    	}
    }

    // CANCEL
    if (strcmp(ptr, "CANCEL") == 0)
    {
    	int indice = atoi(strtok(NULL, "#"));
	    PanierClient* panierDuClient = &paniersClients[socket];

	    if (indice >= 0 && indice < PANIER_MAX-1 && panierDuClient->articles[indice].id != 0)
	    // if (indice >= 0 && indice < panierDuClient->nombreArticles)
	    {
	        int idArticle = panierDuClient->articles[indice].id;
	        int quantite = panierDuClient->articles[indice].stock;
	        
	        for (int i = indice; i < panierDuClient->nombreArticles; i++)
	            panierDuClient->articles[i] = panierDuClient->articles[i+1];
	        panierDuClient->nombreArticles--;
	        addArticle(idArticle, quantite);
	        sprintf(reponse, "CANCEL#ok");
	    }
	    else
	        sprintf(reponse, "CANCEL#ko");
	}

	// CANCELALL
	if (strcmp(ptr, "CANCELALL") == 0)
	{
	    PanierClient* panierDuClient = &paniersClients[socket];
	    for (int i = 0; i < panierDuClient->nombreArticles; i++)
	    {
	        if (panierDuClient->articles[i].id != 0)
	        {
	            addArticle(panierDuClient->articles[i].id, panierDuClient->articles[i].stock);
	            panierDuClient->articles[i].id = 0;
	            panierDuClient->articles[i].intitule[0] = '\0';
	            panierDuClient->articles[i].prix = 0;
	            panierDuClient->articles[i].stock = 0;
	        }
	    }
	    panierDuClient->nombreArticles = 0;
	    sprintf(reponse, "CANCELALL#ok");
	}

	// CONFIRMER
	if (strcmp(ptr, "CONFIRMER") == 0)
	{
	    PanierClient* panierDuClient = &paniersClients[socket];
	    float total = 0;
	    int idFacture = 0;

	    for (int i = 0; i < panierDuClient->nombreArticles; i++)
	        total += panierDuClient->articles[i].prix * panierDuClient->articles[i].stock;
        for (int i = 0; i < nbClients; i++)
        {
        	if (Loginclients[i].socket == socket)
        	{
			    idFacture = Facture(Loginclients[i].login, total);
	    	}
	    }
	    for (int i = 0; i < panierDuClient->nombreArticles; i++)
	    {
		    ventes(idFacture, panierDuClient->articles[i].id, panierDuClient->articles[i].stock);
	        panierDuClient->articles[i].id = 0;
	        panierDuClient->articles[i].intitule[0] = '\0';
	        panierDuClient->articles[i].prix = 0;
	        panierDuClient->articles[i].stock = 0;
	    }
	    panierDuClient->nombreArticles = 0;
	    sprintf(reponse, "CONFIRMER#ok");
	}

    // LOGOUT
	if(strcmp(ptr, "LOGOUT") == 0)
	{
		printf("\t[THREAD %p] LOGOUT\n",(void*)pthread_self());
		for (int i = 0; i < nbClients; i++)
		{
		    if(Loginclients[i].socket == socket)
		    {
		        // Retirez le client de la liste (dans ce cas, simplement déplacez les éléments ultérieurs vers le haut)
		        for (int j = i; j < nbClients-1; j++)
		            Loginclients[j] = Loginclients[j+1];
				retire(socket);
				sprintf(reponse,"LOGOUT#ok");
		        break;
		    }
		}
		return true;
	}
	return true; //Requête non traité
}

//Gestion de l'état du protocole *********************************************
int estPresent(int socket)
{
	int indice = -1, i;
	pthread_mutex_lock(&mutexClients);
	for(i=0; i<nbClients; i++)
		if(clients[i] == socket)
		{
			indice = i;
			break;
		}
	pthread_mutex_unlock(&mutexClients);
	return indice;	
}

void ajoute(int socket)
{
	pthread_mutex_lock(&mutexClients);
	clients[nbClients] = socket;
	nbClients++;
	pthread_mutex_unlock(&mutexClients);
}

void retire(int socket)
{
	int pos = estPresent(socket);
	if(pos == -1) return;
	pthread_mutex_lock(&mutexClients);
	for(int i = pos; i<=nbClients-2; i++)
		clients[i] = clients[i+1];
	nbClients--;
	pthread_mutex_unlock(&mutexClients);
}

// Fonction pour remplacer un caractère par un autre dans une chaîne
void str_replace(char *str, char find, char replace)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == find) 
            str[i] = replace;
        }
}

//Fin prématurée ******************************************************************
void OVESP_Logout()
{
	pthread_mutex_lock(&mutexClients);
	for(int i=0; i < nbClients; i++)
		close(clients[i]);
	pthread_mutex_unlock(&mutexClients);
}