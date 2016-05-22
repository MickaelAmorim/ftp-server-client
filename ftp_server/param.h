/* 
 * File:   param.h
 * Author: cisco
 *
 * Created on July 19, 2015, 12:06 PM
 */


#define	PARAM_H


#undef           true
#undef           false

/*
 * Constantes
 */
#define             true (1==1)
#define             false (!true)
#define             MAX_CLIENT 10;

/*
 * Variables Globales
 */
static  int         ERROR=1; 
static  int         PORT=50001;
static  int         PORTD1=50002;
static  int         PORTD2=50003;
static  char        DEFAULT_LOGIN[5]="user";
static  char        DEFAULT_PASSWD[8]="cisco";
struct  sigaction   action; 
static  int         C1,D1,D2,NewC1,NewD1,NewD2;