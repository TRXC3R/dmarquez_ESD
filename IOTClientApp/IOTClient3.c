#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "accelerometro.h"
#include "colorimetro.h"

#define BUFFER_SIZE 1024
#define TOKEN "A1_TEST_TOKEN"
#define PORT_MQTT 1883

// "Clase" para recolección de datos
typedef struct {
    void (*ejecutar)(const char* server_ip);
} SensorController;

void ejecutar_sensores(const char* server_ip) {
    char cmd[BUFFER_SIZE * 2];

    while (1) {
        // Obtener valores de sensores
        struct AccData acc = accelerometro();
        struct ColorData col = colorimetro();

        // Construir mensaje JSON
        char payload[BUFFER_SIZE];
        snprintf(payload, sizeof(payload),
            "{\"ax\":%.3f, \"ay\":%.3f, \"az\":%.3f, \"clear\":%d, \"red\":%d, \"green\":%d, \"blue\":%d}",
            acc.ax, acc.ay, acc.az, col.clear, col.red, col.green, col.blue
        );

        // Construir comando mosquitto_pub
        snprintf(cmd, sizeof(cmd),
            "mosquitto_pub -d -q 1 -h \"%s\" -p %d -t \"v1/devices/me/telemetry\" -u \"%s\" -m '%s'",
            server_ip, PORT_MQTT, TOKEN, payload
        );

        // Ejecutar comando
        int result = system(cmd);
        if (result != 0) {
            fprintf(stderr, "Error al ejecutar: %s\n", cmd);
        } else {
            printf("Datos enviados: %s\n", payload);
        }

        sleep(1);
    }
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

    printf("Introduce la IP del servidor ThingsBoard: ");
    if (fgets(ip, BUFFER_SIZE, stdin) == NULL) {
        perror("Error al leer IP");
        return 1;
    }

    size_t len = strlen(ip);
    if (ip[len - 1] == '\n') ip[len - 1] = '\0';  // Quitar '\n'

    SensorController controlador = new_controller();
    controlador.ejecutar(ip);

    return 0;
}
