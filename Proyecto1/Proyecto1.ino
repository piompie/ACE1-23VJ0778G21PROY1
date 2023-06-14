#include <LiquidCrystal.h>
#include "LedControl.h"

LiquidCrystal pantalla = LiquidCrystal(22, 23, 24, 25, 26, 27);
LedControl matriz = LedControl(28, 30, 29, 1);

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
  pantalla.setCursor(0, 0);
  pantalla.println("Ruben 202111835");
  pantalla.setCursor(0, 1);
  pantalla.println("Ruben 202111835");
  pantalla.setCursor(0, 2);
  pantalla.println("Ruben 202111835");
  pantalla.setCursor(0, 3);
  pantalla.println("Ruben 202111835");
  delay(1000);
  pantalla.clear();
  pantalla.setCursor(0, 0);
  pantalla.println("Ruben 202111835");
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("hola");
  delay(400);
  //pantalla.println("hola");
}
