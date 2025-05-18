#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4092
#define OUTPUT_FILE "registro_sensores.txt"
#define MAX_MEDIDAS 10

// Estructura para almacenar una medida de sensores
typedef struct {
    float ax, ay, az;
    int clear, red, green, blue;
} Medida;

Medida medidas[MAX_MEDIDAS];
int medida_count = 0;

// Función para analizar una línea y extraer una medida
int parsear_medida(const char* linea, Medida* m) {
    return sscanf(linea,
        "%*d: X: %f g, Y: %f g, Z: %f g | Clear: %d, Red: %d, Green: %d, Blue: %d",
        &m->ax, &m->ay, &m->az, &m->clear, &m->red, &m->green, &m->blue) == 7;
}

// Añadir medida al buffer circular
void agregar_medida(Medida nueva) {
    if (medida_count < MAX_MEDIDAS) {
        medidas[medida_count++] = nueva;
    } else {
        for (int i = 1; i < MAX_MEDIDAS; ++i)
            medidas[i - 1] = medidas[i];
        medidas[MAX_MEDIDAS - 1] = nueva;
    }
}

// Calcular estadísticas de las últimas medidas
void calcular_estadisticas() {
    if (medida_count == 0) return;

    float sum_ax = 0, sum_ay = 0, sum_az = 0;
    int sum_clear = 0, sum_red = 0, sum_green = 0, sum_blue = 0;

    float max_ax = medidas[0].ax, min_ax = medidas[0].ax;
    float max_ay = medidas[0].ay, min_ay = medidas[0].ay;
    float max_az = medidas[0].az, min_az = medidas[0].az;

    int max_clear = medidas[0].clear, min_clear = medidas[0].clear;
    int max_red = medidas[0].red, min_red = medidas[0].red;
    int max_green = medidas[0].green, min_green = medidas[0].green;
    int max_blue = medidas[0].blue, min_blue = medidas[0].blue;

    for (int i = 0; i < medida_count; i++) {
        Medida m = medidas[i];
        sum_ax += m.ax; sum_ay += m.ay; sum_az += m.az;
        sum_clear += m.clear; sum_red += m.red;
        sum_green += m.green; sum_blue += m.blue;

        if (m.ax > max_ax) max_ax = m.ax;
        if (m.ax < min_ax) min_ax = m.ax;
        if (m.ay > max_ay) max_ay = m.ay;
        if (m.ay < min_ay) min_ay = m.ay;
        if (m.az > max_az) max_az = m.az;
        if (m.az < min_az) min_az = m.az;

        if (m.clear > max_clear) max_clear = m.clear;
        if (m.clear < min_clear) min_clear = m.clear;
        if (m.red > max_red) max_red = m.red;
        if (m.red < min_red) min_red = m.red;
        if (m.green > max_green) max_green = m.green;
        if (m.green < min_green) min_green = m.green;
        if (m.blue > max_blue) max_blue = m.blue;
        if (m.blue < min_blue) min_blue = m.blue;
    }

    printf("\n--- Estadísticas (últimas %d medidas) ---\n", medida_count);
    printf("Acelerómetro:\n");
    printf("  X: Media=%.3f  Máx=%.3f  Mín=%.3f\n", sum_ax / medida_count, max_ax, min_ax);
    printf("  Y: Media=%.3f  Máx=%.3f  Mín=%.3f\n", sum_ay / medida_count, max_ay, min_ay);
    printf("  Z: Media=%.3f  Máx=%.3f  Mín=%.3f\n", sum_az / medida_count, max_az, min_az);
    printf("Colorímetro:\n");
    printf("  Clear: Media=%.2f  Máx=%d  Mín=%d\n", sum_clear / (float)medida_count, max_clear, min_clear);
    printf("  Red:   Media=%.2f  Máx=%d  Mín=%d\n", sum_red / (float)medida_count, max_red, min_red);
    printf("  Green: Media=%.2f  Máx=%d  Mín=%d\n", sum_green / (float)medida_count, max_green, min_green);
    printf("  Blue:  Media=%.2f  Máx=%d  Mín=%d\n", sum_blue / (float)medida_count, max_blue, min_blue);
    printf("-----------------------------------------\n\n");
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    socklen_t addrlen = sizeof(address);

    // Limpiar archivo al iniciar el servidor
    FILE* out = fopen(OUTPUT_FILE, "w");
    if (!out) {
        perror("Error al limpiar archivo de salida");
        return -1;
    }
    fclose(out);

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

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Error al aceptar conexión");
            exit(EXIT_FAILURE);
        }

        FILE* out = fopen(OUTPUT_FILE, "a");
        if (!out) {
            perror("Error al abrir archivo de salida");
            close(client_socket);
            close(server_fd);
            return -1;
        }

        printf("Conexión establecida. Recibiendo datos:\n");

        int valread;
        while ((valread = read(client_socket, buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[valread] = '\0';
            printf("%s", buffer);
            fprintf(out, "%s", buffer);
            fflush(out);

            // Procesar línea por línea (por si llegan varias juntas)
            char* linea = strtok(buffer, "\n");
            while (linea != NULL) {
                Medida m;
                if (parsear_medida(linea, &m)) {
                    agregar_medida(m);
                    calcular_estadisticas();
                }
                linea = strtok(NULL, "\n");
            }
        }

        fclose(out);
        close(client_socket);
        printf("\nRecepción finalizada. Datos guardados en '%s'\n", OUTPUT_FILE);
    }

    close(server_fd);
    return 0;
}
