//Esta prueba está hecha con ayuda de chatgpt, en caso de que funcione tomaremos ideas para nuestro proyecto

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define I2C_BUS "/dev/i2c-1"       // Bus I2C de la Raspberry Pi 4
#define MPU6050_ADDRESS 0x68        // Dirección I2C del MPU6050 (verifica si es 0x68 o 0x69), ad0 nivel bajo-alto

// Registros del LIS3DH
#define PWR_MGMT_1    0x6B        // Registro de gestión de energía
#define CONFIG    0x1A             // Registro de configuración DLPF
#define CONFIG_ACCEL 0X1C 			//Registro de configuracin del accelerometro
#define OUT_X_L   0x3B             // Registro de salida para el eje X (inicio de bloque de 6 bytes)

int main(void) {
    int file;                       //Fichero de comunicacion I2C de donde se leen las medidas del accelerometro
    uint8_t accel_config;           // Valor en hexadecimal de la configuracion del accelerometro (ejes)
    int fondo_escala;               // Valor entero entre 0-3 del fondo de escala configurado al inicio del programa configurado por el usuario 
    float scale;                    // Sensibilidad, depende del fondo de escala
    
    // Abrir el bus I2C
    if ((file = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Error al abrir el bus I2C");
        exit(1);
    }

    // Configurar el dispositivo como esclavo
    if (ioctl(file, I2C_SLAVE, MPU6050_ADDRESS) < 0) {
        perror("Error al configurar el dispositivo I2C");
        close(file);
        exit(1);
    }

  // Despertar el MPU6050: escribir 0 en el registro PWR_MGMT_1
  /*l "wakeup" se utiliza para sacar al MPU6050 del modo de suspensión (sleep). 
   * Por defecto, el sensor arranca en este modo para ahorrar energía. 
   * Al escribir 0x00 en el registro PWR_MGMT_1 se "despierta" el dispositivo, permitiéndole comenzar 
   * a realizar mediciones de aceleración y giroscopio.*/
   /*El bit 6 será el que haga que el acelerómetro salga del modo sueño. Por defecto esta activo*/
   
    uint8_t wakeup[2] = {PWR_MGMT_1, 0x00};
    if (write(file, wakeup, 2) != 2) {
        perror("Error al escribir en PWR_MGMT_1");
        close(file);
        exit(1);
    }


    // Configurar el DLPF para reducir el ruido (por ejemplo, valor 0x03)
    uint8_t dlpf[2] = {CONFIG, 0x04};
    if (write(file, dlpf, 2) != 2) {
        perror("Error al configurar DLPF (CONFIG)");
        close(file);
        exit(1);
    }    
    
    //printf("Ingrese un valor en hexadecimal (E0, E8, F0, F8): ");
    //scanf("%x", &accel_config);  // Leer el valor en formato hexadecimal
    
    printf("Ingrese el valor del fondo de escala\n ");
    printf("0 = +-2g\n");
    printf("1 = +-4g\n");
    printf("2 = +-8g\n");
    printf("3 = +-16g\n");
    scanf("%x", &fondo_escala);  // Leer el valor en formato hexadecimal
    

    switch(fondo_escala){ 
        case 0:
            accel_config =  0xE0; 
            printf("Has ingresado escala +-2g\n");
            scale = 16384.0;
            break;
        case 1:
            accel_config =  0xE8; 
            printf("Has ingresado escala +-4g\n");
            scale = 8192.0;
            break;
        case 2:
            accel_config =  0xF0; 
            printf("Has ingresado escala +-8g\n");
            scale = 4096.0;
            break;
        case 3:
            accel_config =  0xF8; 
            printf("Has ingresado escala +-16g\n");
            scale = 2048.0;
            break;
        default:
            printf("Valor no reconocido.\n");
            break;
        }
        
        printf("ESCALA DEL ACELEROMETRO: %f\n", scale);
        
      // Configurar el acelerometro, activar 3 ejes y determinar un fondo de escala +-2g 
    uint8_t accel_conf[2] = {CONFIG_ACCEL, accel_config};
    if(write(file, accel_conf, 2) !=2){
        perror("Error de configuracion del acelerometro (CONFIG_ACCEL)");
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
        float ax = accel_x / scale;
        float ay = accel_y / scale;
        float az = accel_z / scale;
        
        // Mostrar los datos en pantalla
        printf("X: %.3f g, Y: %.3f g, Z: %.3f g\n", ax, ay, az);
        
        usleep(100000);  // Pausa de 100 ms entre lecturas
    }

    close(file);
    return 0;
}
