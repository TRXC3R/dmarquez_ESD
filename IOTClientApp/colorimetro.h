#ifndef __COLORIMETRO_H
#define __COLORIMETRO_H

typedef struct{
	int clear;
	int red;
	int green;
	int blue;
}MeasCol;

MeasCol colorimetro(void);

//extern MeasCol color;

#endif
