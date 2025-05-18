# dmarquez_ESD
Repositorio para la asignatura de Embebed Systems with Raspberry PI
El proyecto que funcioa está dentro de la carpeta IOTApp
  1. IOTClient2.c es el archivo que contiene el código del cliente, se conecta al servidor y manda datos cada segundo durante 10 segundos) el ejecutable es client2.
  2. IOTServer2.c es el archivo que contiene el código del servidor, recibe conexiones del cliente lee los ultimas 10 muestras que recibe de cliente cada 10 segundos. Ejecutable server2
  3. IOTServer2.c es una mejora de IOTServer2.c que además calcula la maxima, minima y media de las medidas que recibe de el cliente, se lee del archivo gracias a expresiones regulares. Ejectutable server3
  4. Archivos .c y .h de accelerometro y colorimetro que se implementan en la aplicacion de cliente.
