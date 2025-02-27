#include <stdio.h>			//Entrada estandar input y output
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>                   // Incluye la macro I2C_SLAVE

struct i2c_rdwr_ioctl_data packets;
struct i2c_msg messages[2];

// ADDRES DEL COLORIMETRO
#define ADDR_COLOR 0x29
#define I2C_BUS "/dev/i2c-1"       // Bus I2C de la Raspberry Pi 4

// REGISTROS COLORIMETRO
#define ENABLE 0x00
#define DATA_COLOR 0x14          // Registro Data Clear hasta Blue Data 0x1B
#define CONFIG 0X0D
#define CONTROL 0x0F
#define ATIME 0x01


int main(){
	int file;

	// Abrir el bus I2C
	if ((file = open(I2C_BUS, O_RDWR)) < 0) {
		perror("Error al abrir el bus I2C");
		exit(1);
	}

	// Configurar el dispositivo como esclavo
	if (ioctl(file, I2C_SLAVE, ADDR_COLOR) < 0) {
		perror("Error al configurar el dispositivo I2C");
		close(file);
		exit(1);
	}
	
	// Despertar al sensor y pasar al ESTADO IDLE
	uint8_t enable[2] = {ENABLE, 0x01};
	if (write(file, enable, 2) != 2) {
		perror("Error al pasar al estado IDLE");
		close(file);
		exit(1);
	}
	
	//uint8_t color_conf[2] = {CONFIG, 0x00};
	//if(write(file, color_conf, 2) !=2){
	//perror("Error de configuracion del acelerometro (CONFIG_ACCEL)");
	//close(file);
	//exit(1);
	//}      
	
	usleep(3000); 
	
	
	// pasar al ESTADO RGBC Init
	uint8_t enable_rgbc[2] = {ENABLE, 0x03};
	if (write(file, enable_rgbc, 2) != 2) {
		perror("Error al pasar al estado RGBC");
		close(file);
		exit(1);
	}
	
	// Configurar la ganancia
	uint8_t control[2] = {CONTROL, 0x01};          // x4 gain
	if (write(file, control, 2) != 2) {
		perror("Error al configurar la ganancia");
		close(file);
		exit(1);
	}
	
	// Configurar el ATIME
	// Despertar al sensor y pasar al ESTADO IDLE
	uint8_t atime[2] = {ATIME, 0XFF};
	if (write(file, atime, 2) != 2) {
		perror("Error al pasar al estado IDLE");
		close(file);
		exit(1);
	}
	
	// Espera para que el sensor se estabilice
	sleep(1);
	
	while(1){
		uint8_t reg = DATA_COLOR;
		if (write(file, &reg, 1) != 1) {
		    perror("Error al escribir el puntero de registro");
		    break;
		}
		
		// Leer 8 bytes: datos de Clear, Red, Green y Blue  (cada eje: low y high)
		uint8_t data[8];
		if (read(file, data, 8) != 8) {
		    perror("Error al leer datos");
		    break;
		}
		
		
		uint16_t clear = (int16_t)((data[0])  | data[1]<<8);
		uint16_t red   = (int16_t)((data[2])  | data[3]<<8);
		uint16_t green = (int16_t)((data[4])  | data[5]<<8);
		uint16_t blue  = (int16_t)((data[6])  | data[7]<<8);
		
		float c = clear;
		float r = red;
		float g = green;
		float b = blue;
		
		printf("data[3]: %X , data[2]: %X , data[1]: %X , data[0]: %X\n", data[3], data[2], data[1], data[0]);
		printf("data[7]: %X , data[6]: %X , data[5]: %X , data[4]: %X\n", data[7], data[6], data[5], data[4]);
		
		// Mostrar los datos en pantalla
                printf("clear: %.3f , red: %.3f , green: %.3f , blue: %.3f\n", c, r, g, b);
		
		sleep(1); 
	}  
	
	
	
	close(file);
	return 0;
	
}
