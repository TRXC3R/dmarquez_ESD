/*
 * IOTServer2.c
 *
 *  Created on: May 16, 2025
 *      Author: ubuntu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define OUTPUT_FILE "registro_sensores.txt"

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    socklen_t addrlen = sizeof(address);

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Enlazar socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    printf("Servidor esperando conexiones en el puerto %d...\n", PORT);

    // Aceptar una conexión
    if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("Error al aceptar conexión");
        exit(EXIT_FAILURE);
    }

    // Abrir fichero para guardar los datos
    FILE* out = fopen(OUTPUT_FILE, "w");
    if (!out) {
        perror("Error al abrir archivo de salida");
        close(client_socket);
        close(server_fd);
        return -1;
    }

    printf("Conexión establecida. Recibiendo datos:\n");

    // Leer datos en bucle hasta que el cliente cierre
    int valread;
    while ((valread = read(client_socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[valread] = '\0';  // Asegurar que termina en null
        printf("%s", buffer);    // Mostrar por pantalla
        fprintf(out, "%s", buffer); // Guardar en archivo
        fflush(out);
    }

    printf("\nRecepción finalizada. Datos guardados en '%s'\n", OUTPUT_FILE);

    fclose(out);
    close(client_socket);
    close(server_fd);
    return 0;
}


