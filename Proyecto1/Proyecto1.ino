/*
  *Cambien su carnet en las lineas 58-60
*/
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "LedControl.h"

char teclas[4][3] = { { '1', '2', '3' },
                      { '4', '5', '6' },
                      { '7', '8', '9' },
                      { '*', '0', '#' } };

#define INICIALIZAR_TECLADO char tecla = ' '
#define LOOP while(true)
#define LINEA_VACIA "                "
#define OFFSET_USUARIOS 1
char letras[] = "ABCDEFGHIJKL";

LiquidCrystal pantalla = LiquidCrystal(22, 23, 24, 25, 26, 27);
LedControl matriz = LedControl(28, 30, 29, 1);

struct usuario {
    char nombre[11];
    char contra[11];
    char numero[9] ;
};

void borrarEEPROM() {
    for (int i = 0; i < EEPROM.length(); i++)
        EEPROM.write(i, 0);
}

enum estados {
  MENU,
  REGISTRO,
  ESPERANDO,
  LOGIN,
  LOGS,
  APLICACION,
  SESION,
  SESIONADMIN,
  PANEL
} siguiente_estado,
  estado_actual = MENU;
byte opcion_menu = 0;
char nombre_temp[11];
char contra_temp[11];
char numero_temp[9] ;

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

INICIALIZAR_TECLADO;

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
  
  //borrarEEPROM();

  EEPROM.begin();

  // Crear una instancia de la estructura usuario
  usuario user;

  // Asignar los valores deseados
  strcpy(user.nombre, "admin1");
  strcpy(user.contra, "1234");
  strcpy(user.numero, "1234");

  EEPROM.put(1000, user);
  // Finalizar la escritura en la memoria EEPROM
  EEPROM.end();




}

boolean entradaAceptada() {
    LOOP {
        if (digitalRead(2)) {
	    delay(210);
	    return true;
	}
        if (digitalRead(3)) {
	    delay(210);
	    return false;
	}
    }
} 

void enviarConfirmar(char* cadena) {
    Serial.println(cadena);
    bool hayAlgo = false;
    char recibidos[3];
    LOOP {
        while(Serial.available()) {
	    Serial.readBytes(recibidos, 2);
            hayAlgo = true;
        }
        if (hayAlgo && !Serial.available()) break;
    }
}

void limpiarBuffer() {
    int t0 = millis();
    int t1 = millis();
    LOOP {
        t1 = millis();
        while(Serial.available()) {
	    Serial.read();
        }
        if ((t1 - t0 >= 1000) && !Serial.available()) break;
    }
}

void imprimirAsteriscos(char* cadena) {
    for (; *cadena; cadena++)
        pantalla.write('*');
}





void loop() {
  // put your main code here, to run repeatedly:
  switch (estado_actual) {
    case MENU:
      {
        //Serial.println("MENU");
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
                estado_actual = ESPERANDO;
                siguiente_estado = LOGIN;
                //estado_actual = LOGIN;
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
        
	    limpiarBuffer();
	    pantalla.clear();
	    pantalla.print(" Esperando  una ");
	    pantalla.setCursor(0, 1);
	    pantalla.print("   conexion...  ");
	    bool alguienPorAhi = false;
            char recibidos[3];
	    LOOP {
	        while(Serial.available()) {
		    Serial.readBytes(recibidos, 2);
		    alguienPorAhi = true;
		}
		if (alguienPorAhi && !Serial.available()) break;
	    }
	    estado_actual = siguiente_estado;
	    break;
      }
    case LOGIN:
      {
        
            memset(nombre_temp, 0, 11);    
            memset(contra_temp, 0, 11);    
            memset(numero_temp, 0,  9);    
            struct usuario nuevo_usuario;
	    LOOP {
	        limpiarBuffer();
	        enviarConfirmar("Nombre:");
            	memset(nombre_temp, 0, 11);    
                pantalla.clear();
                pantalla.print("L O G I N");
                pantalla.setCursor(0, 1);
                pantalla.print(" - NOMBRE:");
                pantalla.setCursor(0, 2);
                // OBTENER CADENA DE APLICACIÓN -- Nombre
                bool seEnvioAlgo = false;
                int indiceNombre = 0;
                long int t0 = millis();
                long int t1 = millis();
	        limpiarBuffer();
                LOOP {
                    // SI YA SE ENVIO ALGO DESDE LA APLICACION
                    while (Serial.available()) {
                        seEnvioAlgo = true;
                    //   RECIBIRLO
                        nombre_temp[indiceNombre++] = Serial.read();
                    }
                    // CONTROLAR CUANTO HA PASADO DESDE QUE COME...
                    if (seEnvioAlgo) {
                        t1 = millis();
                        if (t1 - t0 >= 500) break;
                    } else {
                        t0 = millis();
                        t1 = millis();
                    }
                }
                pantalla.print(nombre_temp);
                pantalla.setCursor(0, 3);
                pantalla.print("Correcto?       ");
                delay(500);
    	        if (entradaAceptada()) break;
		pantalla.setCursor(0, 2);
		pantalla.print(LINEA_VACIA);
		pantalla.setCursor(0, 3);
		pantalla.print(LINEA_VACIA);
            }
	    LOOP {
	        limpiarBuffer();
	        enviarConfirmar("Contras:");
            	memset(contra_temp, 0, 11);    
                pantalla.clear();
                pantalla.print("L O G I N");
                pantalla.setCursor(0, 1);
                pantalla.print(" - Contras:");
                pantalla.setCursor(0, 2);
                // OBTENER CADENA DE APLICACIÓN -- Nombre
                bool seEnvioAlgo = false;
                int indiceContra = 0;
                long int t0 = millis();
                long int t1 = millis();
	        limpiarBuffer();
                LOOP {
                    // SI YA SE ENVIO ALGO DESDE LA APLICACION
                    while (Serial.available()) {
                        seEnvioAlgo = true;
                    //   RECIBIRLO
                        contra_temp[indiceContra++] = Serial.read();
                    }
                    // CONTROLAR CUANTO HA PASADO DESDE QUE COME...
                    if (seEnvioAlgo) {
                        t1 = millis();
                        if (t1 - t0 >= 500) break;
                    } else {
                        t0 = millis();
                        t1 = millis();
                    }
                }
		imprimirAsteriscos(contra_temp);
                pantalla.setCursor(0, 3);
                pantalla.print("Correcto?       ");
                delay(500);
    	        if (entradaAceptada()) break;
		pantalla.setCursor(0, 2);
		pantalla.print(LINEA_VACIA);
		pantalla.setCursor(0, 3);
		pantalla.print(LINEA_VACIA);
            }
	    enviarConfirmar("NADA");
	    // LEER EEPROM
	    byte usuarios = 0;
	    EEPROM.get(0, usuarios);
	    int siguiente_direccion = OFFSET_USUARIOS;
	    bool encontrado = false;
	    for (int i = 0; i < usuarios; i++) {
	        struct usuario usuario_existente;
		EEPROM.get(siguiente_direccion, usuario_existente);
		if (strcmp(nombre_temp, usuario_existente.nombre) == 0 && \
		    strcmp(contra_temp, usuario_existente.contra) == 0) {
		    encontrado = true;
		}
	        siguiente_direccion += sizeof(struct usuario);
	    }
	    pantalla.clear();


      if(strcmp(nombre_temp, "admin1") == 0 && strcmp(contra_temp, "1234") == 0){
        pantalla.print("ADMIN");
        delay(50);
        estado_actual = SESIONADMIN;
      }else if(encontrado == true){
        estado_actual = SESION;
      }else{
        pantalla.print(nombre_temp);
        pantalla.print(contra_temp);
        delay(50);
        estado_actual = MENU;
        
      }
	    
	    delay(2000);
            break;
        break;
      }
      case SESIONADMIN:
    {
      pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Presiona aceptar");
        pantalla.setCursor(0, 1);
        pantalla.print("  Accion ADMIN");
        pantalla.setCursor(0, 2);
        pantalla.print("  Accion ADMIN2");
        pantalla.setCursor(0, 3);
        pantalla.print("  Cerrar sesion");
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
                //siguiente_estado = APLICACION;
                
                break;
              case 1:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                
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
    case SESION:
    {
      pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Presiona aceptar");
        pantalla.setCursor(0, 1);
        pantalla.print("  Accion Usuario");
        pantalla.setCursor(0, 2);
        pantalla.print("  Accion Usuario2");
        pantalla.setCursor(0, 3);
        pantalla.print("  Cerrar sesion");
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
                //siguiente_estado = APLICACION;
                
                break;
              case 1:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                
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
                estado_actual = ESPERANDO;
                siguiente_estado = APLICACION;
                
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
      case APLICACION:
      {
        
            memset(nombre_temp, 0, 11);    
            memset(contra_temp, 0, 11);    
            memset(numero_temp, 0,  9);    
            struct usuario nuevo_usuario;
	    LOOP {
	        limpiarBuffer();
	        enviarConfirmar("Nombre:");
            	memset(nombre_temp, 0, 11);    
                pantalla.clear();
                pantalla.print("R E G I S T R O");
                pantalla.setCursor(0, 1);
                pantalla.print(" - NOMBRE:");
                pantalla.setCursor(0, 2);
                // OBTENER CADENA DE APLICACIÓN -- Nombre
                bool seEnvioAlgo = false;
                int indiceNombre = 0;
                long int t0 = millis();
                long int t1 = millis();
	        limpiarBuffer();
                LOOP {
                    // SI YA SE ENVIO ALGO DESDE LA APLICACION
                    while (Serial.available()) {
                        seEnvioAlgo = true;
                    //   RECIBIRLO
                        nombre_temp[indiceNombre++] = Serial.read();
                    }
                    // CONTROLAR CUANTO HA PASADO DESDE QUE COME...
                    if (seEnvioAlgo) {
                        t1 = millis();
                        if (t1 - t0 >= 500) break;
                    } else {
                        t0 = millis();
                        t1 = millis();
                    }
                }
                pantalla.print(nombre_temp);
                pantalla.setCursor(0, 3);
                pantalla.print("Correcto?       ");
                delay(500);
    	        if (entradaAceptada()) break;
		pantalla.setCursor(0, 2);
		pantalla.print(LINEA_VACIA);
		pantalla.setCursor(0, 3);
		pantalla.print(LINEA_VACIA);
            }
	    LOOP {
	        limpiarBuffer();
	        enviarConfirmar("Celular:");
            	memset(numero_temp, 0, 9);    
                pantalla.clear();
                pantalla.print("R E G I S T R O");
                pantalla.setCursor(0, 1);
                pantalla.print(" - Celular:");
                pantalla.setCursor(0, 2);
                // OBTENER CADENA DE APLICACIÓN -- Nombre
                bool seEnvioAlgo = false;
                int indiceNumero = 0;
                long int t0 = millis();
                long int t1 = millis();
	        limpiarBuffer();
                LOOP {
                    // SI YA SE ENVIO ALGO DESDE LA APLICACION
                    while (Serial.available()) {
                        seEnvioAlgo = true;
                    //   RECIBIRLO
                        numero_temp[indiceNumero++] = Serial.read();
                    }
                    // CONTROLAR CUANTO HA PASADO DESDE QUE COME...
                    if (seEnvioAlgo) {
                        t1 = millis();
                        if (t1 - t0 >= 500) break;
                    } else {
                        t0 = millis();
                        t1 = millis();
                    }
                }
                pantalla.print(numero_temp);
                pantalla.setCursor(0, 3);
                pantalla.print("Correcto?       ");
                delay(500);
    	        if (entradaAceptada()) break;
		pantalla.setCursor(0, 2);
		pantalla.print(LINEA_VACIA);
		pantalla.setCursor(0, 3);
		pantalla.print(LINEA_VACIA);
            }
	    LOOP {
	        limpiarBuffer();
	        enviarConfirmar("Contras:");
            	memset(contra_temp, 0, 11);    
                pantalla.clear();
                pantalla.print("R E G I S T R O");
                pantalla.setCursor(0, 1);
                pantalla.print(" - Contras:");
                pantalla.setCursor(0, 2);
                // OBTENER CADENA DE APLICACIÓN -- Nombre
                bool seEnvioAlgo = false;
                int indiceContra = 0;
                long int t0 = millis();
                long int t1 = millis();
	        limpiarBuffer();
                LOOP {
                    // SI YA SE ENVIO ALGO DESDE LA APLICACION
                    while (Serial.available()) {
                        seEnvioAlgo = true;
                    //   RECIBIRLO
                        contra_temp[indiceContra++] = Serial.read();
                    }
                    // CONTROLAR CUANTO HA PASADO DESDE QUE COME...
                    if (seEnvioAlgo) {
                        t1 = millis();
                        if (t1 - t0 >= 500) break;
                    } else {
                        t0 = millis();
                        t1 = millis();
                    }
                }
		imprimirAsteriscos(contra_temp);
                pantalla.setCursor(0, 3);
                pantalla.print("Correcto?       ");
                delay(500);
    	        if (entradaAceptada()) break;
		pantalla.setCursor(0, 2);
		pantalla.print(LINEA_VACIA);
		pantalla.setCursor(0, 3);
		pantalla.print(LINEA_VACIA);
            }
	    enviarConfirmar("NADA");
	    //
	    memcpy(nuevo_usuario.nombre, nombre_temp, 11);
	    memcpy(nuevo_usuario.numero, numero_temp, 9);
	    memcpy(nuevo_usuario.contra, contra_temp, 11);
	    // LEER EEPROM
	    byte usuarios = 0;
	    EEPROM.get(0, usuarios);
	    int siguiente_direccion = OFFSET_USUARIOS;
	    for (int i = 0; i < usuarios; i++) {
	        siguiente_direccion += sizeof(struct usuario);
	    }
	    EEPROM.put(siguiente_direccion, nuevo_usuario);
	    usuarios++;
	    EEPROM.put(0, usuarios);
            delay(50);
            estado_actual = REGISTRO;
	    pantalla.clear();
            break;      
      }
  }
}