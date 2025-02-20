//Esta prueba está hecha con ayuda de chatgpt, en caso de que funcione tomaremos ideas para nuestro proyecto

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define I2C_BUS "/dev/i2c-1"       // Bus I2C de la Raspberry Pi 4
#define LIS3DH_ADDRESS 0x68        // Dirección I2C del MPU6050 (verifica si es 0x68 o 0x69), ad0 nivel bajo-alto

// Registros del LIS3DH
#define PWR_MGMT_1    0x6B        // Registro de gestión de energía
#define CONFIG    0x1A             // Registro de configuración DLPF
#define CONFIG_ACCEL 0X1C 			//Registro de configuracin del accelerometro
#define OUT_X_L   0x3B             // Registro de salida para el eje X (inicio de bloque de 6 bytes)

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

  // Despertar el MPU6050: escribir 0 en el registro PWR_MGMT_1
  /*l "wakeup" se utiliza para sacar al MPU6050 del modo de suspensión (sleep). 
   * Por defecto, el sensor arranca en este modo para ahorrar energía. 
   * Al escribir 0x00 en el registro PWR_MGMT_1 se "despierta" el dispositivo, permitiéndole comenzar 
   * a realizar mediciones de aceleración y giroscopio.*/
   
    uint8_t wakeup[2] = {PWR_MGMT_1, 0x00};
    if (write(file, wakeup, 2) != 2) {
        perror("Error al escribir en PWR_MGMT_1");
        close(file);
        exit(1);
    }

/*
    // Configurar el sensor escribiendo en CTRL_REG1.
    // 0x57 (01010111): habilita los ejes X, Y, Z y fija una tasa de muestreo de 100 Hz.
    uint8_t config[2] = {CTRL_REG1, 0xE0};
    if (write(file, config, 2) != 2) {
        perror("Error al escribir en CTRL_REG1");
        close(file);
        exit(1);
    }*/
    
    // Configurar el DLPF para reducir el ruido (por ejemplo, valor 0x03)
    uint8_t dlpf[2] = {CONFIG, 0x04};
    if (write(file, dlpf, 2) != 2) {
        perror("Error al configurar DLPF (CONFIG)");
        close(file);
        exit(1);
    }

    // Espera para que el sensor se estabilice
    usleep(100000);

    // Bucle de lectura continua
    while (1) {
        // Para leer en modo auto-incremento se activa poniendo a 1 el bit 7 del registro de inicio.
        uint8_t reg = OUT_X_L;
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
        int16_t accel_x = (int16_t)(data[0]<<8 | (data[1]));
        int16_t accel_y = (int16_t)(data[2]<<8 | (data[3]));
        int16_t accel_z = (int16_t)(data[4]<<8 | (data[5]));
        
        /// Conversión a "g"
        // Para ±2g, la sensibilidad es 16384 LSB/g.
        float ax = accel_x / 2048.0;
        float ay = accel_y / 2048.0;
        float az = accel_z / 2048.0;
        
        // Mostrar los datos en pantalla
        printf("X: %.3f g, Y: %.3f g, Z: %.3f g\n", ax, ay, az);
        
        usleep(100000);  // Pausa de 100 ms entre lecturas
    }

    close(file);
    return 0;
}
