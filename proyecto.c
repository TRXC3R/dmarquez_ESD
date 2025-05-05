#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

//Constantes para el accelerometro
#define I2C_BUS "/dev/i2c-1"       // Bus I2C de la Raspberry Pi 4
#define MPU6050_ADDRESS 0x68        // Dirección I2C del MPU6050 (verifica si es 0x68 o 0x69), ad0 nivel bajo-alto

// Registros del LIS3DH
#define PWR_MGMT_1    0x6B        // Registro de gestión de energía
#define CONFIG    0x1A             // Registro de configuración DLPF
#define CONFIG_ACCEL 0X1C 			//Registro de configuracin del accelerometro
#define OUT_X_L   0x3B             // Registro de salida para el eje X (inicio de bloque de 6 bytes)

//Constantes para el colorimetro
#define I2C_ADDR 0x29 // Dirección del TCS34725 en el bus I2C
#define ENABLE_REG 0x00
#define ATIME_REG 0x01
#define CONTROL_REG 0x0F
#define CDATAL 0x14 // Registro de los datos Clear
#define RDATAL 0x16 // Registro de los datos Red
#define GDATAL 0x18 // Registro de los datos Green
#define BDATAL 0x1A // Registro de los datos Blue

int i2c_fd;


void menu(){
	printf("Selecccionar el programa al ejecutar\n");
	printf("(1) Accelerometro\n");
	printf("(2) Colorimetro\n");
	printf("(3) Salir\n");
	}

int main(){
	int opcion;
	
	while(1){
		menu();
		printf("Ingrese su opcion; ");
		scanf("%d", &opcion);
		getchar();
		
		switch(opcion){
			case 1:
				int file;                       //Fichero de comunicacion I2C de donde se leen las medidas del accelerometro
				uint8_t accel_config;           // Valor en hexadecimal de la configuracion del accelerometro (ejes)
				int fondo_escala;               // Valor entero entre 0-3 del fondo de escala configurado al inicio del programa configurado por el usuario 
				float scale;                    // Sensibilidad, depende del fondo de escala
				
				// Abrir el bus I2C
				if ((file = open(I2C_BUS, O_RDWR)) < 0) {
					perror("Error al abrir el bus I2C");
					exit(1);
				}//Cierra IF
				
				// Configurar el dispositivo como esclavo
				if (ioctl(file, I2C_SLAVE, MPU6050_ADDRESS) < 0) {
					perror("Error al configurar el dispositivo I2C");
					close(file);
					exit(1);
				}//Cierra if

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
					break;
				}//Cierra if
				
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
				}//Cierra Switch
				printf("ESCALA DEL ACELEROMETRO: %f\n", scale);
        
				// Configurar el acelerometro, activar 3 ejes y determinar un fondo de escala +-2g 
				uint8_t accel_conf[2] = {CONFIG_ACCEL, accel_config};
				if(write(file, accel_conf, 2) !=2){
					perror("Error de configuracion del acelerometro (CONFIG_ACCEL)");
					close(file);
					exit(1);
				}//Cierra if      
				
				// Espera para que el sensor se estabilice
				usleep(100000);
				
				// Bucle de lectura continua
				while (1) {
					// Para leer en modo auto-incremento se activa poniendo a 1 el bit 7 del registro de inicio.
					uint8_t reg = OUT_X_L;
					if (write(file, &reg, 1) != 1) {
					    perror("Error al escribir el puntero de registro");
					    break;
					}//Cierra if

					// Leer 6 bytes: datos de X, Y y Z (cada eje: bajo y alto)
				        uint8_t data[6];
				        if (read(file, data, 6) != 6) {
				            perror("Error al leer datos");
				            break;
				        }//Cierra if
			        
				        // Combinar los bytes para obtener enteros de 16 bits (formato little endian)
				        int16_t accel_x = (int16_t)(data[0]<<8 | (data[1]));
				        int16_t accel_y = (int16_t)(data[2]<<8 | (data[3]));
				        int16_t accel_z = (int16_t)(data[4]<<8 | (data[5]));
			        
				        // Conversión a "g"
				        // Para ±2g, la sensibilidad es 16384 LSB/g.
				        float ax = accel_x / scale;
				        float ay = accel_y / scale;
				        float az = accel_z / scale;
			        
				        // Mostrar los datos en pantalla
				        printf("X: %.3f g, Y: %.3f g, Z: %.3f g\n", ax, ay, az);
				        
				        usleep(100000);  // Pausa de 100 ms entre lecturas
			   	 }//Cierra while

				// Cerrar el bus I2C
				close(file);
				return 0;
				break;
			}
				
			case 2: 
				// Función para escribir en un registro I2C
				void i2c_write_byte(int reg, int value) {
				    unsigned char buffer[2] = {reg | 0x80, value}; // 0x80 para indicar comando
				    write(i2c_fd, buffer, 2);
				}//Cierra funcion void
				
				// Función para leer dos bytes de un registro I2C
				int i2c_read_word(int reg) {
				    unsigned char buffer[1] = {reg | 0x80};
				    write(i2c_fd, buffer, 1);
				    
				    usleep(10000);
				    
				    unsigned char data[2];
				    read(i2c_fd, data, 2);
				    
				    //return data[0] | (data[1] << 8); // Convertir a valor de 16 bits
				    return (data[1] << 8) | data[0]; // Convertir a valor de 16 bits
				}//Cierra funcion int
	
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
				break;
			
			case 3:
				printf("Saliendo/n");
				return 0; 
			default: 
				printf("/nOpcion no valida, intente de nuevo/n");
			}

		printf("/nPresione Enter para continuar");
		getchar();		
	}
	return 0;

}
	
