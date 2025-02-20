//Esta prueba está hecha con ayuda de chatgpt, en caso de que funcione tomaremos ideas para nuestro proyecto

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define I2C_BUS "/dev/i2c-1"       // Bus I2C de la Raspberry Pi 4
#define LIS3DH_ADDRESS 0x18        // Dirección I2C del LIS3DH (verifica si es 0x18 o 0x19)

// Registros del LIS3DH
#define CTRL_REG1 0x20             // Registro de configuración
#define OUT_X_L   0x28             // Registro de salida para el eje X (inicio de bloque de 6 bytes)

int main(void) {
    int file;
    
    // Abrir el bus I2C
    if ((file = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Error al abrir el bus I2C");
        exit(1);
    }

    // Configurar el dispositivo como esclavo
    if (ioctl(file, I2C_SLAVE, LIS3DH_ADDRESS) < 0) {
        perror("Error al configurar el dispositivo I2C");
        close(file);
        exit(1);
    }

    // Configurar el sensor escribiendo en CTRL_REG1.
    // 0x57 (01010111): habilita los ejes X, Y, Z y fija una tasa de muestreo de 100 Hz.
    uint8_t config[2] = {CTRL_REG1, 0x57};
    if (write(file, config, 2) != 2) {
        perror("Error al escribir en CTRL_REG1");
        close(file);
        exit(1);
    }

    // Bucle de lectura continua
    while (1) {
        // Para leer en modo auto-incremento se activa poniendo a 1 el bit 7 del registro de inicio.
        uint8_t reg = OUT_X_L | 0x80;
        if (write(file, &reg, 1) != 1) {
            perror("Error al escribir el puntero de registro");
            break;
        }
        
        // Leer 6 bytes: datos de X, Y y Z (cada eje: bajo y alto)
        uint8_t data[6];
        if (read(file, data, 6) != 6) {
            perror("Error al leer datos");
            break;
        }
        
        // Combinar los bytes para obtener enteros de 16 bits (formato little endian)
        int16_t x = (int16_t)(data[0] | (data[1] << 8));
        int16_t y = (int16_t)(data[2] | (data[3] << 8));
        int16_t z = (int16_t)(data[4] | (data[5] << 8));
        
        // Convertir a "g"
        // Suponiendo un rango de ±2g, la sensibilidad es aproximadamente 1 mg/digit (0.001 g/digit)
        float x_g = x * 0.001;
        float y_g = y * 0.001;
        float z_g = z * 0.001;
        
        // Mostrar los datos en pantalla
        printf("X: %.3f g, Y: %.3f g, Z: %.3f g\n", x_g, y_g, z_g);
        
        usleep(100000);  // Pausa de 100 ms entre lecturas
    }

    close(file);
    return 0;
}
