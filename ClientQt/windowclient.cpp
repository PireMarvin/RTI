#include "windowclient.h"
#include "ui_windowclient.h"
#include "TCP.h"

#include <QMessageBox>
#include <string>
#include <signal.h>

using namespace std;

extern WindowClient *w;

int sClient;

void HandlerSIGINT(int s);
void Echange(char* requete, char* reponse);

ARTICLE article;

#define MESSAGE 1400
#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    ui->pushButtonPayer->setText("Confirmer achat");
    setPublicite("!!! Bienvenue sur le Maraicher en ligne !!!");

    struct sigaction A;
    A.sa_flags = 0;
    sigemptyset(&A.sa_mask);
    A.sa_handler = HandlerSIGINT;
    if (sigaction(SIGINT,&A,NULL) == -1)
    {
        perror("Erreur de sigaction");
        exit(1);
    }

    // char * monIP= "192.168.188.128";
    sClient = ClientSocket(NULL,5000);
    printf("Connecte sur le serveur\n");
}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];
  
  sprintf(msgS, "CANCELALL");
  Echange(msgS, msgR);

  sprintf(msgS, "LOGOUT");
  Echange(msgS, msgR);

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{  
  // Récupérer le nom d'utilisateur et le mot de passe entrés par l'utilisateur
  const char* nomLogin = getNom();
  const char* motDePasse = getMotDePasse();
  bool nouveauClient = false;
  
  char msgR[MESSAGE];
  char msgS[MESSAGE];

  if(isNouveauClientChecked())
    nouveauClient = true;

  if(strcmp(nomLogin,"")==0 || strcmp(motDePasse,"")==0)
  {
    dialogueErreur("Erreur d'authentification", "Encoder un utilisateur et/ou un mot de passe");
    return;
  }

  sprintf(msgS, "LOGIN#%s#%s#%d", nomLogin, motDePasse, nouveauClient);

  Echange(msgS, msgR);
  if(strcmp(msgR, "LOGIN#ko#Mauvais identifiant!")==0)
    dialogueErreur("Erreur d'authentification", "Mauvais identifiant !");
  if(strcmp(msgR, "LOGIN#ko#Client déjà loggé!")==0)
    dialogueErreur("Erreur d'authentification", "Client déjà connecté !");
  if(strcmp(msgR, "LOGIN#ko#Client déjà enregistré!")==0)
    dialogueErreur("Erreur d'authentification", "Client déjà enregistré !");
  if(strcmp(msgR, "LOGIN#ko#trop de clients!")==0)
    dialogueErreur("Erreur de connection", "Trop de clients !");
  if(strcmp(msgR, "LOGIN#ok") == 0) 
  {
    if(nouveauClient)
      dialogueMessage("Authentification réussie", "Bienvenu nouveau client !");
    else
      dialogueMessage("Authentification réussie", "Vous êtes connecté !");
    loginOK();
    setPublicite("!!! Bienvenue sur le Maraicher en ligne !!!");
    strcpy(msgS,"");
    sprintf(msgS,"CONSULT#1");
    Echange(msgS, msgR);
    printf("CONSULT#1 reciv\n");
    article = TableArticle(msgR);
    setArticle(article.intitule, article.prix, article.stock, article.image);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
  ui->spinBoxQuantite->setValue(0);

  char msgR[MESSAGE];
  char msgS[MESSAGE];
    
  sprintf(msgS, "CANCELALL");
  Echange(msgS, msgR);

  sprintf(msgS, "LOGOUT#ok");
  Echange(msgS, msgR);
  printf("\nMessage recu : %s\n",msgR);
  logoutOK();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];

  if(article.id<21)
  {
    article.id++;
    sprintf(msgS, "CONSULT#%d",article.id);
    Echange(msgS, msgR);
    article = TableArticle(msgR);
    setArticle(article.intitule, article.prix, article.stock, article.image);
  }
  else
    dialogueErreur("Numero d'article", "Plus d'articles a droit");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];

  if(article.id>1)
  {
    article.id--;
    sprintf(msgS, "CONSULT#%d",article.id);
    Echange(msgS, msgR);
    article = TableArticle(msgR);
    setArticle(article.intitule, article.prix, article.stock, article.image);
  }
  else
    dialogueErreur("Numero d'article", "Plus d'articles a gauche");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];
  int quantite = getQuantite();
  int idArticle = article.id;

  if(quantite == 0)
    dialogueErreur("Erreur quantite","Veuillez selectionner une quantite");
  else
  {
    sprintf(msgS, "ACHAT#%d#%d",idArticle, quantite);
    Echange(msgS, msgR);

    if(strcmp(msgR, "ACHAT#ko#-1")==0)
      dialogueErreur("Erreur article","aucun article trouver");
    if(strcmp(msgR, "ACHAT#ko#0")==0)
      dialogueErreur("Erreur quantite","quantite superieur au stock");
    if(strcmp(msgR, "ACHAT#ko")==0)
      dialogueErreur("Erreur caddie","Vous avez atteint la limite du panier");
    else
    {
      sprintf(msgS, "CADDIE");
      Echange(msgS, msgR);

      MajCaddie(msgR);

      sprintf(msgS, "CONSULT#%d",idArticle);
      Echange(msgS, msgR);
      article = TableArticle(msgR);
      setArticle(article.intitule, article.prix, article.stock, article.image);
     }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];
  int idArticle = article.id;

  if(getIndiceArticleSelectionne() == -1)
    dialogueErreur("Erreur selection", "Aucun article selectionne");
  else
  {
    sprintf(msgS, "CANCEL#%d", getIndiceArticleSelectionne());
    Echange(msgS, msgR);
    if(strcmp (msgR, "CANCEL#ok") == 0)
    {
      sprintf(msgS, "CADDIE");
      Echange(msgS, msgR);
      
      MajCaddie(msgR);

      sprintf(msgS, "CONSULT#%d",idArticle);
      Echange(msgS, msgR);
      article = TableArticle(msgR);
      setArticle(article.intitule, article.prix, article.stock, article.image);
    }
  }
}  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];
  int idArticle = article.id;

  sprintf(msgS, "CANCELALL");
  Echange(msgS,msgR);
  if(strcmp(msgR, "CANCELALL#ok") == 0)
  {
      sprintf(msgS, "CADDIE");
      Echange(msgS, msgR);
      
      MajCaddie(msgR);

      sprintf(msgS, "CONSULT#%d",idArticle);
      Echange(msgS, msgR);
      article = TableArticle(msgR);
      setArticle(article.intitule, article.prix, article.stock, article.image);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
  char msgR[MESSAGE];
  char msgS[MESSAGE];

  sprintf(msgS, "CONFIRMER");
  Echange(msgS, msgR);

  if(strcmp(msgR, "CONFIRMER#ok") == 0)
  {
    printf("\nCONFIRMER\n");
    videTablePanier();
    dialogueMessage("CONFIRMER", "Merci de votre achat");
    setTotal(0);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// nos fonction
void Echange(char* requete, char* reponse)
{
  int nbEcrits, nbLus;
  //Envoi de la requete ****************************
  if((nbEcrits = Send(sClient,requete,strlen(requete))) == -1)
  {
    perror("Erreur de Send");
    close(sClient);
    exit(1);
  }
  //Attente de la reponse **************************
  if((nbLus = Receive(sClient,reponse)) < 0)
  {
    perror("Erreur de Receive");
    close(sClient);
    exit(1);
  }
  if(nbLus == 0)
  {
    printf("Serveur arrete, pas de reponse reçue...\n");
    close(sClient);
    exit(1);
  }
  reponse[nbLus] = 0;
}

//Fin de connexion ********************************************

void HandlerSIGINT(int s)
{
  printf("\nArret du client.\n");
  char msgR[MESSAGE];
  char msgS[MESSAGE];
  strcpy(msgS, "");
  strcpy(msgS, "LOGOUT#ok");
  Echange(msgS, msgR);  
  shutdown(sClient,SHUT_RDWR);
  exit(0);
}

ARTICLE WindowClient::TableArticle(char* msgR)
{
  ARTICLE article;

  char* token = strtok(msgR, "#");
  token = strtok(NULL, "#");      // "avant ok"
  token = strtok(NULL, "#");      // "ok"
  article.id = atoi(token);       // Id de l'article
  token = strtok(NULL, "#");      // Titre de l'article
  strncpy(article.intitule, token, sizeof(article.intitule) - 1); 
  article.intitule[sizeof(article.intitule) - 1] = '\0';
  token = strtok(NULL, "#");      // Prix de l'article 
  article.prix = atof(token);
  token = strtok(NULL, "#");      // Quantité de l'article
  article.stock = atoi(token);
  token = strtok(NULL, "#");      // Image de l'article
  strncpy(article.image, token, sizeof(article.image) - 1);
  article.image[sizeof(article.image) - 1] = '\0';

  return article;
}

void WindowClient::MajCaddie(char* msgR)
{
  float total = 0;
  char* token = strtok(msgR, "#");
  token = strtok(NULL, "#"); // "ok"
  if(strcmp(token, "ok")==0)
  {
    token = strtok(NULL, "#"); // "taille"
    int taille = atoi(token);
    videTablePanier();
    for (int i = 0; i < taille; i++)
    {
      token = strtok(NULL, "#"); // Id de l'article
      int id = atoi(token);
      token = strtok(NULL, "#"); // Titre de l'article
      char* intitule = token;
      token = strtok(NULL, "#"); // Prix de l'article
      float prix = atof(token);
      token = strtok(NULL, "#"); // Quantité de l'article
      int stock = atoi(token);
      
      article.id = id;
      strcpy(article.intitule, intitule);
      article.prix = prix;
      article.stock = stock;

      total += article.prix * article.stock;
      ajouteArticleTablePanier(article.intitule, article.prix, article.stock);
    }
    setTotal(total);
  }
}