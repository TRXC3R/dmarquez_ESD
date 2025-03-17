//Este código ha sido generado por ChatGPT habiendole dado como imput nuestro programa "prueba_colorimetro.c" que era funcional pero con erroes y con la hoja de especificaciones del sensor de colores.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define I2C_ADDR 0x29 // Dirección del TCS34725 en el bus I2C
#define ENABLE_REG 0x00
#define ATIME_REG 0x01
#define CONTROL_REG 0x0F
#define CDATAL 0x14 // Registro de los datos Clear
#define RDATAL 0x16 // Registro de los datos Red
#define GDATAL 0x18 // Registro de los datos Green
#define BDATAL 0x1A // Registro de los datos Blue

int i2c_fd;

// Función para escribir en un registro I2C
void i2c_write_byte(int reg, int value) {
    unsigned char buffer[2] = {reg | 0x80, value}; // 0x80 para indicar comando
    write(i2c_fd, buffer, 2);
}

// Función para leer dos bytes de un registro I2C
int i2c_read_word(int reg) {
    unsigned char buffer[1] = {reg | 0x80};
    write(i2c_fd, buffer, 1);
    
    unsigned char data[2];
    read(i2c_fd, data, 2);
    
    return data[0] | (data[1] << 8); // Convertir a valor de 16 bits
}

void init_sensor() {
    // Activar el sensor
    i2c_write_byte(ENABLE_REG, 0x03); // PON (bit 0) y AEN (bit 1)

    // Configurar el tiempo de integración (700ms para máxima precisión)
    i2c_write_byte(ATIME_REG, 0x00);

    // Configurar la ganancia a 16x
    i2c_write_byte(CONTROL_REG, 0x10);
}

int main() {
    // Abrir el bus I2C
    if ((i2c_fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("Error al abrir el bus I2C");
        return 1;
    }

    // Conectar al sensor en la dirección 0x29
    if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Error al conectar con el sensor");
        return 1;
    }

    // Inicializar el sensor
    init_sensor();

    // Esperar a que termine la conversión de datos
    usleep(700000); // 700 ms

    // Leer valores de color
    int clear = i2c_read_word(CDATAL);
    int red = i2c_read_word(RDATAL);
    int green = i2c_read_word(GDATAL);
    int blue = i2c_read_word(BDATAL);

    // Mostrar los valores en pantalla
    printf("Clear: %d\n", clear);
    printf("Red: %d\n", red);
    printf("Green: %d\n", green);
    printf("Blue: %d\n", blue);

    // Cerrar el bus I2C
    close(i2c_fd);
    return 0;
}
