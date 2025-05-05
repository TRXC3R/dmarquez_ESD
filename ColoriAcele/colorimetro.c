//Este c�digo ha sido generado por ChatGPT habiendole dado como imput nuestro programa "prueba_colorimetro.c" que era funcional pero con erroes y con la hoja de especificaciones del sensor de colores.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define I2C_ADDR 0x29 // Direcci�n del TCS34725 en el bus I2C
#define ENABLE_REG 0x00
#define ATIME_REG 0x01
#define CONTROL_REG 0x0F
#define CDATAL 0x14 // Registro de los datos Clear
#define RDATAL 0x16 // Registro de los datos Red
#define GDATAL 0x18 // Registro de los datos Green
#define BDATAL 0x1A // Registro de los datos Blue

int i2c_fd;

// Funci�n para escribir en un registro I2C
void i2c_write_byte(int reg, int value) {
    unsigned char buffer[2] = {reg | 0x80, value}; // 0x80 para indicar comando
    write(i2c_fd, buffer, 2);
}

// Funci�n para leer dos bytes de un registro I2C
int i2c_read_word(int reg) {
    unsigned char buffer[1] = {reg | 0x80};
    write(i2c_fd, buffer, 1);
    
    usleep(10000);
    
    unsigned char data[2];
    read(i2c_fd, data, 2);
    
    //return data[0] | (data[1] << 8); // Convertir a valor de 16 bits
    return (data[1] << 8) | data[0]; // Convertir a valor de 16 bits
}

void init_sensor() {
    // Activar el sensor
    i2c_write_byte(ENABLE_REG, 0x03); // PON (bit 0) y AEN (bit 1)

    // Configurar el tiempo de integraci�n (700ms para m�xima precisi�n)
    i2c_write_byte(ATIME_REG, 0x00);

    // Configurar la ganancia a 16x
    i2c_write_byte(CONTROL_REG, 0x10);
}

int colorimetro(void) {
    // Abrir el bus I2C
    if ((i2c_fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("Error al abrir el bus I2C");
        return 1;
    }

    // Conectar al sensor en la direcci�n 0x29
    if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Error al conectar con el sensor");
        return 1;
    }

    // Inicializar el sensor
    init_sensor();

    // Esperar a que termine la conversi�n de datos
    usleep(700000); // 700 ms

    while (1){
         // Leer valores de color
        int clear = i2c_read_word(CDATAL);
        int red = i2c_read_word(RDATAL);
        int green = i2c_read_word(GDATAL);
        int blue = i2c_read_word(BDATAL);

        // Mostrar los valores en pantalla
        printf("Clear: %d\t", clear);
        printf("Red: %d\t", red);
        printf("Green: %d\t", green);
        printf("Blue: %d\n", blue);
        
        sleep(1);    
    }

    // Cerrar el bus I2C
    close(i2c_fd);
    return 0;
}
