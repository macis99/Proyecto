/*Proyecto tecnología electrónica*/
// Alvaro Macias
// Sergio Alvarez                                                                                     
// Jaime Castro

#include "mbed.h"
#include "Grove_LCD_RGB_Backlight.h"
#include <cstdio>

//Enumeración de los estados
enum estados {reposo,reposoCalibrado, tara, medirPeso, EsperandoCalibrado1, EsperandoCalibrado2, Calibrado1,Calibrado2} estado;
 
// variables utilizadas
static UnbufferedSerial serial_port(PA_0, PA_1,9600);
int flagtara;
float pesovacio;
float peso100g;
float peso;
float sumapeso=0;
float pesoTara;
float pesoTarado;
float pendiente;
Timer temporizador;

/////////GPIOS UTILIZADOS/////////

Grove_LCD_RGB_Backlight rgbLCD(PB_9,PB_8);
DigitalOut led1(D8);
DigitalOut led2(D7);
DigitalOut led3(D6);
DigitalIn botonTara(D4);
DigitalIn botonMedir(D3);
DigitalIn botonCalibrado(D2);
AnalogIn potenciometro(A3);
AnalogIn Galga(A1);

/////////PROTOTIPOS DE FUNCIONES/////////
void Tarar();
void Medir();
void estados();
void controlEstados();


///////// FUNCION main /////////
int main()
{
    estado=reposo; //empezamos en el estado de reposo
    Galga.set_reference_voltage(3.3); //establecemos la tensión de referencia

    while(1) 
    {
        controlEstados();
        estados();
    }
    return 0;
}

void controlEstados()
{
    if(botonCalibrado==0 && estado== reposo)
    {
        rgbLCD.setRGB(0xff, 0xff, 0xff);
        rgbLCD.locate(0, 0);
        rgbLCD.print("Pulse boton de");
        rgbLCD.locate(0, 1);
        rgbLCD.print("calibrado");
    }
    else if(botonCalibrado==1 && estado== reposo)
    {
        estado=EsperandoCalibrado1;
    }
    else if(botonCalibrado==0 && estado == EsperandoCalibrado1)
    {
        estado=Calibrado1;
        //pesovacio=potenciometro.read();
        pesovacio=Galga.read_voltage(); //se lee la tensión que mide la balanza sin ningún peso
        printf("V peso vacio: %f\n",pesovacio);
        rgbLCD.locate(0, 0);
        rgbLCD.clear();
        rgbLCD.print("Ponga 100 g");
        rgbLCD.locate(0, 1);
        rgbLCD.print("Y pulse boton");
    }
    else if(botonCalibrado==1 && estado==Calibrado1)
    {
        estado=EsperandoCalibrado2;
    }
    else if(botonCalibrado==0 && estado==EsperandoCalibrado2)
    {
        estado=Calibrado2;
        //pesovacio=potenciometro.read();
        peso100g= Galga.read_voltage(); //leemos la tensión que mide la galga con un peso de 100 gramos
        printf("V 100g: %f\n",peso100g);

        pendiente=(peso100g-pesovacio)/(100); //calculamos la pendiente de la recta de calibración
        printf("Pendiente: %f\n",pendiente);
        rgbLCD.locate(0, 0);
        rgbLCD.clear();
        rgbLCD.print("Calibrado");
        rgbLCD.locate(0, 1);
        rgbLCD.print("vuelva a pulsar");
    }
    else if ((botonCalibrado==1 && estado==Calibrado2) || (botonTara==1 && estado==tara) || (botonMedir==1 && estado==medirPeso)) 
    {
        led3=0;
        led2=0;
        led1=0;
        flagtara=0;
        rgbLCD.clear();
        rgbLCD.locate(0, 0);
        rgbLCD.print("Pulse boton de");
        rgbLCD.locate(0, 1);
        rgbLCD.print("Tara o Medir");
        estado=reposoCalibrado;

    }
    else if(botonTara==1 && estado == reposoCalibrado)
    {
        temporizador.reset(); //inicializamos el temporizador
        temporizador.start();
        rgbLCD.locate(0, 0);
        rgbLCD.clear();
        rgbLCD.print("Ponga un peso");
        rgbLCD.locate(0, 1);
        rgbLCD.print("sobre la balanza");
        flagtara = 1;
    }
    else if(flagtara==1 && temporizador.read()>20.0f)
    {
        temporizador.reset();
        temporizador.start();
        //peso =potenciometro.read();
        peso=Galga.read_voltage();
        printf("V peso:%f\n",peso);
        pesoTara=(peso-pesovacio)/pendiente; // Medimos el peso en gramos del primer peso introducido
        printf("Peso tara gramos: %f\n",pesoTara);
        rgbLCD.locate(0, 0);
        rgbLCD.clear();
        rgbLCD.print("Ponga otro peso");
        rgbLCD.locate(0, 1);
        rgbLCD.print("Espere 30 s");
        flagtara=2;
    
    }  
    else if(temporizador.read()>30.0f && flagtara==2)
    {
        estado=tara;
    }
    else if(botonMedir==1 && estado == reposoCalibrado)
    {
        estado =medirPeso;
    }
  

}

void estados()
{

        switch(estado) 
        {
            case tara:
                Tarar();
                break;
            case medirPeso:
                Medir();
                break;
        }
}

void Tarar()
{
         //peso =potenciometro.read();
         peso=Galga.read_voltage(); //medimos la tensión del peso nuevo introducido 
         peso=(peso-pesovacio)/pendiente; //calculamos el peso en gramos
         printf("Peso nuevo gramos: %f\n",peso);
         pesoTarado=peso-pesoTara; //calculamos el peso tarado
         char cadena[10];
         sprintf(cadena,"Peso tarado %f\n",pesoTarado); 
         printf("Peso nuevo gramos: %f\n",pesoTarado);
         rgbLCD.clear();
         rgbLCD.locate(0, 0);
         rgbLCD.print(cadena); //mostramos por la pantalla LCD la cadena
         rgbLCD.locate(0, 1);
         rgbLCD.print("Pulse Tara"); 
}
 
void Medir()
{
    
    rgbLCD.setRGB(0xff, 0xff, 0xff);
    rgbLCD.clear();
    rgbLCD.locate(0, 0);
    rgbLCD.print("Introduzca");
    rgbLCD.locate(0, 1);
    rgbLCD.print("un peso");


    //peso=potenciometro.read();
    peso=Galga.read_voltage(); //medimos la tensión del peso 
    peso=(peso-pesovacio)/pendiente;  // calculamos el peso en gramos
    char cadena1[40];
    int l=sprintf(cadena1,"Peso medido %f\n gramos",peso);
    serial_port.write(cadena1,l);
  
    if(peso<0.25*100)
        {
            led1=0;
            led2=1;
            led3=0;
        }
    if(peso>0.25*100 && peso<0.5*100 )
        {
            led2=0;
            led1=0;
            led3=1;

        }
    if(peso>0.5*100 && peso<100)
        {
            led3=0;
            led2=0;
            led1=1;
        }
}