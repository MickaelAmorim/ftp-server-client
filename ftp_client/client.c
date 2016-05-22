/* 
 * File:   client.c
 * Author: Amorim
 *         Aygul
 *
 * Created on 24 juin 2015, 17:01
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "param.h"


/*
 * Fonctions prototypes
 */
static int send_to_server(char cmd[], int s);
static int get_cmd_code(char cmd[]);
static void exec_cmd(int s);
static int validation_message(char buffer[], char word[], int length);
static int authentification(int s);
static int create_client_socket();
static int run_ftp_client();


/*
 * Envoie la commande vers le serveur et récupère la réponse de la commande cmd[]
 * sur le socket D1. Elle retourne true lorsque la fonction s'est terminé correctement.
 */
static int send_to_server(char cmd[], int s){
    char    buffer[512];
    int     read_size;
    
    /*
     * Envoie de la commande vers le serveur
     */
    printf("cmd : %s\n", cmd);
    write(s, cmd, strlen(cmd));
    
    /*
     * initialisation du buffer
     */ 
    bzero(buffer,strlen(buffer));
    
    /*
     * Lecture de la réponse du serveur
     */
     if((read_size=read(s, buffer, 511))==1){
        read_size=read(s, buffer, 511);
     }
    
     if(read_size==0){
        printf ("server is gone :'( \n");
        return false;
     }
    
    /*
     * On affiche le résultat du serveur
     */
    printf("message reçu :\n %s\n", buffer);
    
    return true;
     
}

/*
 * récupère la valeur de stdin et la compare aux commandes autorisées.
 * Cette fonction retourne l'indice n executé par exec_cmd()
 */
static int get_cmd_code(char cmd[]){
  if (validation_message(cmd,"ls",2)){
      return 1;
  }else if(validation_message(cmd,"rls",3)){
      return 2;
  }else if(validation_message(cmd,"pwd",3)){
      return 3;
  }else if(validation_message(cmd,"rpwd",4)){
      return 4;
  }else if(validation_message(cmd,"cd",2)){
      return 5;
  }else if(validation_message(cmd,"rcd",3)){
      return 6;
  }else if(validation_message(cmd,"rm",2)){
      return 7;
  }else if(validation_message(cmd,"upld",4)){
      return 8;
  }else if(validation_message(cmd,"downl",5)){
      return 9;
  }else if(validation_message(cmd,"quit",4)){
      return 0;
  }else {
      return -1;
  }
    
}

/*
 * Cette fonction récupère l'indice de la fonction get_cmd_code()
 * et envoie la commande vers le serveur en fonction de l'indice.
 * Cette fonction regroupe les commandes locales et les commande distantes.
 */
static void exec_cmd(int s){
    
    char        buffer[256];
    int         code;
    
    do{
        printf("\nEFTP > ");
        bzero(buffer,256);
        fgets(buffer, 255, stdin);
        code=get_cmd_code(buffer);
        
        switch(code){
        
            case 1 : 
                printf("Execute ls\n");
                system(buffer);
                 
                break;
            case 2 : 
                printf("Execute rls\n");
                if (buffer[0] == 'r')
                    memmove(buffer,buffer+1,strlen(buffer));
                if(!send_to_server(buffer, s))
                    return false;
                break;
            case 3 : 
                printf("Execute pwd\n");
                system(buffer);
                break;
            case 4 : 
                printf("Execute rpwd\n");
                
                send_to_server("pwd", s);
                break;
            case 5 : 
                printf("Execute cd\n");
                system(buffer);
                break;
            case 6 : 
                printf("Execute rcd\n");
                send_to_server("cd", s);
                break;
            case 7 : 
                printf("Execute rm\n");
                system(buffer);
                break;
            case 8 : 
                printf("Execute upld\n");
                break;
            case 9 : 
                printf("Execute downl\n");
                break;
            case - 1 : 
                printf("Command unknown\n");
                break;
            default :
                send_to_server("quit", s);
                printf("Bye\n");
        }
        
    }while(code);

}

/*
 * Fonction permettant de valider deux chaine de caractère.
 * L'une étant la chaine à comparé et l'autre etant la chaine comparable.
 * Le tout en spécifiant la taille de la chaine comparable
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
 * Processus d'authentification vers le serveur ftp. 
 * Elle retourne false si l'authentification échoue.
 */
static int authentification(int s){
    char        *message, server_message[20];
    int         read_size, verif;
    char        who[4]="WHO";
    char        passwd[7]="PASSWD";
    char        welc[5]="WELC";
    char        buffer[256];
    char        stdin_login[5], stdin_password[7],read_newline[2];
    int         noauth=true;
   
    while(noauth){
            /* 
             * Envoie du message BONJ
             */
            printf("Envoi du message 'BONJ' au serveur\n");
            write(s, "BONJ", 4);

            /*
             * Lecture de la réponse du serveur
             */ 
            printf("Attendre de la réponse du serveur ...\n");
            bzero(buffer,256);
            
            if((read_size=read(s, buffer, 255))==1){
                read_size=read(s, buffer, 255);
            }
             if(read_size==0){
               printf ("Server is gone :'( ");
            return false;
        }
            printf ("Read size : %i",read_size);

            printf("Message du serveur recu : %s\n", buffer);
            if(validation_message(buffer, "BYE", strlen("BYE"))){
            
                printf("Connection closed by server !");
                close(s);
                return false;
            
            
            }else{
            /*
             *  Verification du message reçu par le server (BONJ)
             */
            if((verif=validation_message(buffer, who, strlen(who))) == true){
                printf("BONJ-WHO OK : (%s)\n", buffer);

                printf("Entrez un login : ");
                bzero(buffer,256);
                fgets(buffer, 255, stdin);
               // printf("Chaine: %s \n", stdin_login);

                /*
                 * Envoie du login
                 */

                write(s, buffer, strlen(buffer));
                printf("Envoi du user : %s\n", buffer);
                /* 
                 * lecture de la reponse du server
                 */

                bzero(buffer,256);
                read_size=read(s, buffer, strlen(passwd));

                printf("Message du serveur recu : %s\n", buffer);
                    /*
                     *  Verification du message reçu par le server (PASSWD)
                     */
                if((verif=validation_message(buffer, passwd, strlen(passwd))) == true){
                    printf("Authentification vers le serveur reussit (%s)\n", buffer);

                    bzero(buffer,256);
                    printf("Entrez un password : ");
                    fgets(buffer, 255, stdin);

                    /* 
                     * Envoie du mot de passe
                     */
                    write(s, buffer, 255);


                    /*
                     * attendre de la réponse du serveur
                     */
                    bzero(buffer,256);

                    read_size=read(s, buffer, 255);

                 if((verif=validation_message(buffer, welc, strlen(welc))) == true){
                    printf("Authentification vers le serveur reussit (%s)\n", buffer);
                    noauth=false;


                 }else{
                    printf("verif :  %i\n", verif);
                    fputs("authentification vers le serveur à échoué (WELC)\n",stdout);
                 }

                }else{
                    printf("verif :  %i\n", verif);
                    fputs("authentification vers le serveur à échoué (PASSWD)\n",stdout);
                   
                }

            }else{
                printf("verif :  %i\n", verif);
                fputs("authentification vers le serveur à échoué (WHO)\n",stdout);
              

            }
        }
    }
    return true;
}

/*
 * Fonction permettant de créer les sockets C1, D1, D2
 */
static int create_client_socket(int port){
    int                     s=0;
    struct sockaddr_in      sin;
    
    
    // creation de la socket TCP(SOCK_STREAM) sur IPv4(AF_INET)
    // recuperation du descripteur de socket listenfd 
    if ((s=socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("CLIENT->SOCKET : Unable to create socket.");
        return(ERROR);
    }
    
    /*
     * initialisation de la strcture à 0
     */
    memset((char *) &sin, 0, sizeof(sin));

    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin))< 0){
        printf("CLIENT->CONNECT : Unable to etablish a connexion to server.");
        return(ERROR);
    }
    return(s);
}

/*
 * Cette fonction permet de créer l'ensemble du client.
 * On retrouve la création des trois socket, l'authentification et l'execution des 
 * commande locales + distantes
 */
static int run_ftp_client(){
    int         C1, D1, D2, n, auth;
    char        recu[50];
    int         notoken=true;
    int         try=0;
    
    if((C1=create_client_socket(PORT)) == ERROR){
        printf("CLIENT->SOCKET : Can not start client\n");
        return ERROR;
    }
    
    if((D1=create_client_socket(PORTD1)) == ERROR){
        printf("CLIENT->SOCKET : Can not start client\n");
        return ERROR;
    }
    
    if((D2=create_client_socket(PORTD2)) == ERROR){
        printf("CLIENT->SOCKET : Can not start client\n");
        return ERROR;
    } 
    
    if(authentification(C1)){
        exec_cmd(D1);
    }
    
    
    
  
    return true;  
    
}


int main(int argc, char *argv[])
{
    int         STATUS_CLIENT;
    

    if((STATUS_CLIENT=run_ftp_client()) == true){
        printf("Shutdown FTP Client ...");

    }else{
        printf("Error FTP Client !");
    }
   
    return 0;
}
 
  
