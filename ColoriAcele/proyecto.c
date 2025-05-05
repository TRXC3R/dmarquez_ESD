#include <stdio.h> 
#include <stdlib.h>
#include "proyecto.h"

void menu(){
	printf("Selecccionar el programa al ejecutar\n");
	printf("(1) Accelerometro\n");
	printf("(2) Colorimetro\n");
	printf("(3) Salir\n");
	}

int main(){
	int opcion;
	
	while(1){
		menu();
		printf("Ingrese su opcion; ");
		scanf("%d", &opcion);
		getchar();
		
		switch(opcion){
			case 1:
				//system("gcc accelerometro.c -o accelerometro");
				//system("./accelerometro");
				accelerometro();

				break;
				
			case 2: 
				//system("gcc colorimetro.c -o colorimetro");
				//system("./colorimetro");
				colorimetro();
				break;
				
			case 3:
				printf("Saliendo/n");
				return 0; 
			default: 
				printf("/nOpcion no valida, intente de nuevo/n");
			}
		printf("/nPresione Enter para continuar");
		getchar();		
		}
		return 0;
		
	}
	
