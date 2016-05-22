/* 
 * File:   param.h
 * Author: cisco
 *
 * Created on July 19, 2015, 12:10 PM
 */


#define	PARAM_H

/*
 * On définit les booleans TRUE et FALSE
 */
#undef          true
#undef          false
#define         true (1==1)
#define         false (!true)

/*
 * Variables Globales
 */
static	int     ERROR=-1;
static  int     PORT=50001;
static  int     PORTD1=50002;
static  int     PORTD2=50003;
static  char    DEFAULT_LOGIN[6]="user";
static  char    DEFAULT_PASSWD[9]="cisco";



