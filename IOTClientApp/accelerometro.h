#ifndef __ACCELEROMETRO_H
#define __ACCELEROMETRO_H

typedef struct{
	float ax;
	float ay;
	float az;
}MeasAcc;

MeasAcc accelerometro(void);

#endif
