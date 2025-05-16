/*
 * IOTClient2.c
 *
 *  Created on: May 16, 2025
 *      Author: ubuntu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#include "accelerometro.h"
#include "colorimetro.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define TEMP_FILE "datos.txt"

// Simulaciones de sensores
char* dato_accelerometro() {
    static char buffer[1024];
    //snprintf(buffer, sizeof(buffer), "Accel: %s", accelerometro());
    snprintf(buffer, sizeof(buffer), "X: %.3f g, Y: %.3f g, Z: %.3f g", accelerometro().ax, accelerometro().ay, accelerometro().az);

    return buffer;
}

char* dato_colorimetro() {
    static char buffer[1024];
    //snprintf(buffer, sizeof(buffer), "Color: %s", colorimetro());
    snprintf(buffer, sizeof(buffer), "Clear: %d, Red: %d, Green: %d, Blue: %d", colorimetro().clear, colorimetro().red, colorimetro().green, colorimetro().blue);

    return buffer;
}

// "Clase" para recolección de datos
typedef struct {
    void (*ejecutar)(const char* server_ip);
} SensorController;

void ejecutar_sensores(const char* server_ip) {

	// Enviar fichero al servidor
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE];
	while(1){
		FILE* f = fopen(TEMP_FILE, "w");
		FILE* file = fopen(TEMP_FILE, "r");

		if (!f) {
			perror("Error al abrir fichero");
			return;
		}

		for (int i = 0; i < 10; i++) {
			char* acc = dato_accelerometro();
			char* col = dato_colorimetro();
			fprintf(f, "%d: %s | %s\n", i+1, acc, col);
			fflush(f);
			sleep(1);
		}

		fclose(f);


		if (!file) {
			perror("No se pudo abrir el archivo para envío");
			return;
		}

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Error al crear socket");
			fclose(file);
			return;
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(PORT);

		if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
			perror("Dirección IP inválida");
			close(sock);
			fclose(file);
			return;
		}

		if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			perror("Error de conexión");
			close(sock);
			fclose(file);
			return;
		}

		// Enviar línea por línea (puedes también enviar todo de golpe si lo prefieres)
		while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
			send(sock, buffer, strlen(buffer), 0);

		}

		printf("Datos enviados al servidor.\n");
		fclose(file);
	}
    close(sock);

}

// Inicializador de la "clase"
SensorController new_controller() {
    SensorController ctrl;
    ctrl.ejecutar = ejecutar_sensores;
    return ctrl;
}

// MAIN
int main() {
    char ip[BUFFER_SIZE];

    printf("Introduce la IP del servidor: ");
    if (fgets(ip, BUFFER_SIZE, stdin) == NULL) {
        perror("Error al leer IP");
        return 1;
    }

    // Quitar salto de línea
    size_t len = strlen(ip);
    if (ip[len - 1] == '\n') ip[len - 1] = '\0';

    SensorController controlador = new_controller();
    controlador.ejecutar(ip);

    return 0;
}


