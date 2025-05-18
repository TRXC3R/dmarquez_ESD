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

#include "accelerometro.h"
#include "colorimetro.h"

#define JSON_FILE "telemetry-data-as-object.json"
#define BUFFER_SIZE 1024
#define PORT_MQTT 1883

typedef struct {
    void (*ejecutar)(const char* server_ip, const char* access_token);
} SensorController;

void ejecutar_sensores(const char* server_ip, const char* access_token) {
    char cmd[BUFFER_SIZE * 2];

    while (1) {
        MeasAcc acc = accelerometro();
        MeasCol col = colorimetro();

        FILE* json = fopen(JSON_FILE, "w");
        if (!json) {
            perror("Error al crear archivo JSON");
            return;
        }

        fprintf(json,
            "{\n"
            "  \"ax\": %.3f,\n"
            "  \"ay\": %.3f,\n"
            "  \"az\": %.3f,\n"
            "  \"clear\": %d,\n"
            "  \"red\": %d,\n"
            "  \"green\": %d,\n"
            "  \"blue\": %d\n"
            "}\n",
            acc.ax, acc.ay, acc.az,
            col.clear, col.red, col.green, col.blue
        );
        fclose(json);

        snprintf(cmd, sizeof(cmd),
            "mosquitto_pub -d -q 1 -h \"%s\" -p %d -t \"v1/devices/me/telemetry\" -u \"%s\" -f \"%s\"",
            server_ip, PORT_MQTT, access_token, JSON_FILE
        );

        int result = system(cmd);
        if (result != 0) {
            fprintf(stderr, "Error al ejecutar: %s\n", cmd);
        } else {
            printf("Datos enviados desde %s\n", JSON_FILE);
        }

        sleep(1);
    }
}

SensorController new_controller() {
    SensorController ctrl;
    ctrl.ejecutar = ejecutar_sensores;
    return ctrl;
}

int main() {
    char ip[BUFFER_SIZE];
    char token[BUFFER_SIZE];

    printf("Introduce la IP del servidor ThingsBoard: ");
    if (fgets(ip, BUFFER_SIZE, stdin) == NULL) {
        perror("Error al leer IP");
        return 1;
    }

    printf("Introduce el TOKEN de acceso del dispositivo: ");
    if (fgets(token, BUFFER_SIZE, stdin) == NULL) {
        perror("Error al leer TOKEN");
        return 1;
    }

    // Eliminar saltos de línea
    ip[strcspn(ip, "\n")] = '\0';
    token[strcspn(token, "\n")] = '\0';

    SensorController controlador = new_controller();
    controlador.ejecutar(ip, token);

    return 0;
}

