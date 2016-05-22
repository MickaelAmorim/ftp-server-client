/* 
 * File:   server.c
 * Author: Amorim
 *
 * Created on 24 juin 2015, 11:56
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include "param.h"


/*
 * fonctions prototype
 */
static void Handler(int sig);
static int get_cmd(int s);
static void kill_the_kids(int sig);
static int validation_message(char buffer[], char word[], int length);
static int authentification(int s);
static int create_server_socket();
static int run_server_ftp();

/*
 * Cette fonction est executer lorsqu'on presse ctrl+c. Cette commande va généré un SIGINT
 * et fermer l'ensemble des sockets avant de s'intérrompre.
 */
static void Handler(int sig) {
        
    if(sig == SIGINT){
        printf("\nLe serveur a été interrompu : Fermeture des sockets\n ");
        close(NewC1);
        close(NewD1);
        close(NewD2);
        close(C1);
        close(D1);
        close(D2);
        exit(0);
    }
	
}

/*
 * Fonction permettant de d'executer les commandes sur le serveur.
 * Elle retroune au client la réponse de sa commande.
 */
static int get_cmd(int s){
    int read_size;
    char buffer[256];
    int code=1;
    char retour[512];
    
    
    while(1){
        char *text = calloc(1,1);
        printf("Wait for command from client\n");
        bzero(buffer,256);
        if((read_size=read(s, buffer, 255))==1){
                read_size=read(s, buffer, 255);
            }
        if(read_size==0){
            printf ("Client is gone :'( \n");
            return false;
            }
        if((validation_message(buffer, "QUIT", 4))){
            printf("Client served QUIT Received.\n");
            return false;
             }
            bzero(retour,512);
            FILE *fp = popen(buffer, "r");
            while (fgets(retour, sizeof(retour), fp) ) {
                     text = realloc( text, strlen(text)+1+strlen(retour) );
                     strcat( text, retour );
                     
            }
            printf("message envoyé : %s\n", text);
            printf("message reçu : %s\n", buffer);
            
            pclose(fp);
            write(s,text,strlen(text));
            
   
    }
    return true;
}

/*
 * Tue les zombies engendrés par les connexions clients.
 */
static void kill_the_kids(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


/*
 * Verifie la validité du message reçu par le client.
 * Cette fonction retourne false si le message ne correspond pas à
 * la chaine de référence inséré en paramètre.
 */
static int validation_message(char buffer[], char word[], int length) {
    int         i;
 
    for(i = 0; i < length; i++) { 
        printf("%i : %c / %c \n", i, buffer[i], word[i]);
        if(buffer[i] != word[i]) {
            return false;
        }
    }
    return true;
}

/*
 * Fonction qui verifie l'intégrité du client auprès du serveur en envoyant une
 * serie de message. Elle retourne false si l'authentification échoue.
 */
static int authentification(int s){
    char        *message, client_message[10], *who;
    int         read_size, verif;
    char        bonj[5]="BONJ";
    char        passwd[7]="PASSWD";
    char        welc[5]="WELC";
    char        buffer[256];
    int         noauth=true;
    int         try=0;
    
    while (noauth){
        who="WHO";
        
        printf("Attempt : %i\n",try);

        bzero(buffer,256);
        /*
         * lecture du premier message du client
         */
        printf("lecture du socket\n");
         if((read_size=read(s, buffer, 255))==1){
                read_size=read(s, buffer, 255);
            }
        if(read_size==0){
            printf ("Client is gone :'( ");
            return false;
        }
       printf ("Read size : %i",read_size);

        /*
         *  Verification du message reçu par le client (BONJ)
         */
        if((verif=validation_message(buffer, bonj, strlen(bonj))) == true){

             /* 
              * Envoie du message WHO au client
              */
             if (try>=3){
                printf("kick client too many fails");
                write(s,"BYE",3);
                return false;
                
             }else{
                 printf("envoie de '%s'\n", who);
                 write(s, "WHO", 3);
             }
             

             /* 
              * Lecture du login
              */ 
             printf("attente d'une réponse ...\n");
             bzero(buffer,256);
             read_size=read(s , buffer , 255);


              /*
               *  Verification du message recu par le client (LOGIN)
               */
             if((verif=validation_message(buffer, DEFAULT_LOGIN, strlen(buffer)-1)) == true){
                /*
                 * Envoie du message PASSWD au client
                 */ 
                printf("envoie de '%s'\n", passwd);
                write(s, "PASSWD", 6);

                /*
                 *  Lecture du mot de passe
                 */
                printf("attente du mot de passe ...\n");
                bzero(buffer,256);
                read_size=read(s , buffer , 255);


                /*
                 * Vérification du message
                 */
                if((verif=validation_message(buffer, DEFAULT_PASSWD, strlen(buffer)-1)) == true){
                    fputs("authentification du client reussit (PASSWD)\n",stdout);

                   /*
                    * Envoie du message WELC au client
                    */ 
                    printf("envoie de '%s'\n", welc );
                    write(s, "WELC", 4);
                    noauth=false;
                  

                }else{
                    printf("verif :  %i\n", verif);
                    fputs("authentification du client à échoué (PASSWD)\n",stdout); 
                    write(s, "NOPASSWD", 8);

                }

             }else{
                printf("verif :  %i\n", verif);
                fputs("authentification du client à échoué (LOGIN)\n",stdout); 
                write(s, "NOLOGI", 7);

             }

        }else{
           printf("verif :  %i\n", verif);
           fputs("authentification du client à échoué\n",stdout);   
            write(s, "NACK", 4);

        }
        try=try+1;
    }
  
    return true;
}

/*
 * Fonction utilisé pour la création des trois socket C1, D1 et D2.
 */
static int create_server_socket(int port){
    int                     s=0;
    struct sockaddr_in      sin;
    int                     reuseaddr = 1;
    
    // creation de la socket TCP(SOCK_STREAM) sur IPv4(AF_INET)
    // recuperation du descripteur de socket listenfd 
    if ((s=socket(AF_INET, SOCK_STREAM, 0)) == -1){
            printf("Unable to create socket");
            return ERROR;
        }
    
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    
    /*
     * initialisation de la structure à 0
     */
    memset((char *) &sin, 0, sizeof(sin));
    
    // port d'ecoute du serveur : l'argument n°1
    // on le range en ordre reseau avec htons
    sin.sin_port = htons(port);
        
    // famille d'adresses : IPv4
    sin.sin_family = AF_INET;
        
    // adresse IP d'ecoute du serveur : INADDR_ANY (adresse wildcard)
    // c'est-a-dire que le serveur ecoutera sur toutes ses interfaces
    // on la range en ordre réseau avec htonl
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s,(struct sockaddr *) &sin, sizeof(sin)) == -1){
	printf("Erreur de bind\n");
        close(s);
	return ERROR;
    }
    
    // on lie la structure servaddr a la socket listenfd
    // pour definir le port et l'adresse d'ecoute du serveur
    
    if (listen(s, 30) == -1){  
	printf("SERVER->LISTEN : Server don't listen.");
        close(s);
        return ERROR;
    }
    
    return(s);
    
}

/*
 * Cette fonction permet de lancer le serveur.
 * Elle est capable de : 
 * - créer la configuration du serveur avec les sockets C1, D1 et D2
 * - créer un fils pour chaque nouvelle connexions client.
 * - authentifier le client.
 * - répondre aux commandes envoyé par le client.
 */
static int run_server_ftp()
{
    //int                     C1,D1,D2;
    struct sockaddr_in      cliaddr, cliaddr1, cliaddr2;
    char                    adresseIP[16], buff[50];
    time_t                  tempo;
    struct sigaction        sa;
    
    
    action.sa_handler=Handler;
    sigemptyset(&action.sa_mask);
    sigaction(SIGQUIT,&action,NULL);
    sigaction(SIGTERM,&action,NULL);
    sigaction(SIGALRM,&action,NULL);
    sigaction(SIGKILL,&action,NULL);
    sigaction(SIGINT,&action,NULL);
    
    if((C1=create_server_socket(PORT)) == ERROR){
        printf("SERVER->SOCKET : Can not start server\n");
        return ERROR;
    }
    
           
    if((D1=create_server_socket(PORTD1)) == ERROR){
        printf("SERVER->SOCKET : Can not start server D1\n");
        return ERROR;
    }
            
    if((D2=create_server_socket(PORTD2)) == ERROR){
        printf("SERVER->SOCKET : Can not start server D2\n");
        return ERROR;
    }
    
    memset((char *) &cliaddr, 0, sizeof(cliaddr));
    memset((char *) &cliaddr1, 0, sizeof(cliaddr1));
    memset((char *) &cliaddr2, 0, sizeof(cliaddr2));
    
    //Kill les petits enfants verts.
    sa.sa_handler = kill_the_kids;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    
    while(true){
        // Variables temporaires
        int         child, port;
        //int        NewC1, NewD1, NewD2;
        socklen_t   len, len1,len2;
        len=sizeof(cliaddr);
        
        len1=sizeof(cliaddr1);
        len2=sizeof(cliaddr2); 
               
        
    	// mise en attente de connexion
	// on recuperera dans cliaddr les coordonnees (adresse et port du client)
	// accept retourne un descripteur de socket connectée avec lequel on lire et ecrira dans la socket
        //cnx=accept(ControlSocket,(struct sockaddr*) &cliaddr, &len);
        
        if((NewC1=accept(C1, (struct sockaddr*)&cliaddr, &len))==ERROR){
           printf("SERVER->SOCKET : Can not accept connection C1 \n");
           return ERROR;
        }

        if((NewD1=accept(D1, (struct sockaddr*)&cliaddr1, &len1))== ERROR){
           printf("SERVER->SOCKET : Can not accept connection D1 \n");
           return ERROR;
        }
            
        if((NewD2=accept(D2, (struct sockaddr*)&cliaddr2, &len2))== ERROR){
           printf("SERVER->SOCKET : Can not accept connection D2 \n");
           return ERROR;
        }
       
      
        // On créer un processus fils pour chaque connection client
        if((child=fork()) == 0)
	{
            int         auth;
            int         token=false;
            int         try=3;
        
            close(C1);
            close(D1);
            close(D2);
          
            
           // authentification(NewC1);
            
           if(authentification(NewC1)){
                get_cmd(NewD1);
           }
            
            // on ferme la connexion
            close(NewC1);
            close(NewD1);
            close(NewD2);
            exit(0);
            
        }
        printf("\n main(): Check the child ; pid=%d\n", child);
        
        close(NewC1);
        close(NewD1);
        close(NewD2);
    }

return true;

}

int main(int argc, char *argv[])
{

    if(run_server_ftp() == true){
        printf("Shutdown FTP server ...");

    }else{
        printf("Error FTP server !");
    }
    
    return 0;
}