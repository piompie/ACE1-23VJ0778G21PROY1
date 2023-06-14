/*
  *Cambien su carnet en las lineas 58-60
*/

#include <LiquidCrystal.h>
#include "LedControl.h"

char teclas[4][3] = { { '1', '2', '3' },
                      { '4', '5', '6' },
                      { '7', '8', '9' },
                      { '*', '0', '#' } };
LiquidCrystal pantalla = LiquidCrystal(22, 23, 24, 25, 26, 27);
LedControl matriz = LedControl(28, 30, 29, 1);
enum estados {
  MENU,
  REGISTRO,
  ESPERANDO,
  LOGIN,
  LOGS,
  APLICACION,
  PANEL
} siguiente_estado,
  estado_actual = MENU;
byte opcion_menu = 0;

char leerTecla() {
  for (int i = 8; i <= 10; i++) {
    digitalWrite(i, HIGH);
    for (int j = 4; j <= 7; j++) {
      if (digitalRead(j) == HIGH) {
        digitalWrite(i, LOW);
        return teclas[j - 4][i - 8];
      }
    }
    digitalWrite(i, LOW);
  }
  return ' ';
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //inicializacion del driver matriz de leds
  matriz.shutdown(0, false);
  matriz.setIntensity(0, 8);
  matriz.clearDisplay(0);
  //inicializacion pantalla
  pantalla.begin(16, 4);
  //inicializacion resto de pines
  for (int i = 2; i <= 7; i++) {
    pinMode(i, INPUT);
  }
  for (int i = 8; i <= 10; i++) {
    pinMode(i, OUTPUT);
  }
  // mensaje inicial
  pantalla.setCursor(0, 0);
  pantalla.println("Ruben 202111835");
  pantalla.setCursor(0, 1);
  pantalla.println("Sergio 202111835");
  pantalla.setCursor(0, 2);
  pantalla.println("Pedro 202111835");
  pantalla.setCursor(0, 3);
  pantalla.println("Jose 202111835");
  delay(500);
  pantalla.clear();
  pantalla.setCursor(0, 0);
  pantalla.println("Sebas 202111835");
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (estado_actual) {
    case MENU:
      {
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Menu principal");
        pantalla.setCursor(0, 1);
        pantalla.print("Presiona aceptar");
        pantalla.setCursor(0, 2);
        pantalla.print("  Login");
        pantalla.setCursor(0, 3);
        pantalla.print("  Registrar");
        pantalla.setCursor(0, opcion_menu + 2);
        pantalla.print(">");
        while (true) { //loop que mueve el cursor o detecta el boton aceptar
          char tecla = leerTecla();
          if (tecla == '2') {
            delay(210);
            opcion_menu--;
            if (opcion_menu > 254) opcion_menu = 0;  //254 porque la variable es byte xd
            break;
          }
          if (tecla == '8') {
            delay(210);
            opcion_menu++;
            if (opcion_menu > 1) opcion_menu = 1;
            break;
          }
          if (digitalRead(2) == HIGH) {  //boton aceptar
            delay(210);
            switch (opcion_menu) {
              case 0:
                //estado_actual = ESPERANDO;
                //siguiente_estado = LOGIN;
                estado_actual = LOGIN;
                break;
              case 1:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                estado_actual = REGISTRO;
                break;
            }
            opcion_menu = 0;
            break;
          }
        }
        break;
      }
    case ESPERANDO:
      {
        break;
      }
    case LOGIN:
      {
        pantalla.clear();
        Serial.println("Estas en login");
        break;
      }
    case REGISTRO:
      {
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Presiona aceptar");
        pantalla.setCursor(0, 1);
        pantalla.print("  Aplicacion");
        pantalla.setCursor(0, 2);
        pantalla.print("  Panel");
        pantalla.setCursor(0, 3);
        pantalla.print("  Salir");
        pantalla.setCursor(0, opcion_menu + 1);
        pantalla.print(">");
        while (true) { //loop que mueve el cursor o detecta el boton aceptar
          char tecla = leerTecla();
          if (tecla == '2') {
            delay(210);
            opcion_menu--;
            if (opcion_menu > 254) opcion_menu = 0;  //254 porque la variable es byte xd
            break;
          }
          if (tecla == '8') {
            delay(210);
            opcion_menu++;
            if (opcion_menu > 2) opcion_menu = 2;
            break;
          }
          if (digitalRead(2) == HIGH) {  //boton aceptar
            delay(210);
            switch (opcion_menu) {
              case 0:
                //estado_actual = ESPERANDO;
                //siguiente_estado = LOGIN;
                estado_actual = APLICACION;
                break;
              case 1:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                estado_actual = PANEL;
                break;
              case 2:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                estado_actual = MENU;
                break;
            }
            opcion_menu = 0;
            break;
          }
        }
        break;
      }
  }
}
