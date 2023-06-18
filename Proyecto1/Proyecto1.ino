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
char letras[] = "ABCDEFGHIJKL";

LiquidCrystal pantalla = LiquidCrystal(22, 23, 24, 25, 26, 27);
LedControl matriz = LedControl(28, 30, 29, 1);

enum estados {
  MENU,
  REGISTRO,
  REGISTRO2,
  ESPERANDO,
  LOGIN,
  LOGS,
  APLICACION,
  SESION,
  SESIONADMIN,
  PANEL,
  RPANEL
} siguiente_estado,
  estado_actual = MENU;
byte opcion_menu = 0;
char nombre_temp[13];
char contra_temp[13];
char numero_temp[9] ;


typedef struct{
    uint8_t id; // de 0 a 99
    char    desc[15];  // descripición del log
}evento; // o log

typedef struct{
    uint8_t celulares;
    uint8_t fallidos;
    uint8_t incidentes;
    uint8_t usuarios;
}admin_stats;

/************************************************************ 
*                 ACCIONES DE USUARIO                        *
*************************************************************/
typedef struct{
    bool available;
    char user[13];
}phone;

typedef phone phone_deposit[9];

struct usuario {
    char nombre[13];
    char contra[13];
    char numero[9] ;
};

typedef enum{
    REMOVE,
    DEPOSIT
}operation;

typedef enum{
    APP_INPUT,
    KB_INPUT
}input_type;

typedef enum{
    CEL_INGRESADOS,
    INT_FALLIDOS,
    INCIDENTES,
    N_USUARIOS
}adm_stat;

input_type tipoEntrada;
#define ADMIN_STATS_START  sizeof(phone_deposit) + 1
#define EEPROM_USERS_START sizeof(phone_deposit) + 1 + sizeof(admin_stats) + 1
#define EEPROM_LOGS_START EEPROM.length() - 100*sizeof(evento)

#define LLAVE1 '5'
#define LLAVE2 '3'
#define ADMIN_NAME "ADMIN*05645"
#define ADMIN_PASS "GRUPO21"

#define SEP    {0,0,1,0,0,1,0,0}
#define FLINE  {1,1,1,1,1,1,1,1}

#define KEYBOARD_DELAY 100

#define MAX_SESSION_TIME 1000*300


void ingresar_telefono(char* user);
void retirar_telefono(char* user);
void update_deposit(uint8_t pos, operation op);
void calcular_casillero_pos(uint8_t pos,uint8_t* x_left, uint8_t* y_top);
bool agregar_usuario(char* username, char* password, char* phone_number);
bool eliminar_cuenta(char* user);
bool validar_credenciales(char* user, char* pass);
boolean iniciar_sesion(char* user, char* pass);
bool find_user(char* user);
bool pedir_password();
char* xor_encode(char* input);
char* xor_decode(char* input);
void tipo_de_input();
bool check_deposit(char type, uint8_t position);
bool check_session_time();
void check_casillero();

//admin
void log_EEPROM(char* description);
void get_log(uint8_t index,char* buffer);
void show_logs();
void show_stats();

const phone default_phone = { .available = true, .user = {'\0'}} ;
phone_deposit casillero = { default_phone,default_phone,default_phone,default_phone,default_phone,default_phone,default_phone,default_phone,default_phone };

long int current_session_time;
admin_stats adm_stats;

uint8_t led_casillero[8][8] = {
    SEP,
    SEP,
    FLINE,
    SEP,
    SEP,
    FLINE,
    SEP,
    SEP
};

char teclado_matricial[9][5] = {
    {'1','*','$','#','!'},
    {'2','A','B','C','0'}, // La tecla 0 se utiliza para otra función, por eso aparece el 0 aquí.
    {'3','D','E','F','\0'},
    {'4','G','H','I','\0'},
    {'5','J','K','L','\0'},
    {'6','M','N','O','\0'},
    {'7','P','Q','R','S'},
    {'8','T','U','V','\0'},
    {'9','W','X','Y','Z'}
};


void tipo_de_input(){
    update_session_time();
    pantalla.clear();
    pantalla.setCursor(0, 0);
    pantalla.println("TIPO DE INPUT:");
    pantalla.setCursor(0, 1);
    pantalla.println("# APLICACION");
    pantalla.setCursor(0, 2);
    pantalla.println("* TECLADO");
    while(true){
        if(estado_actual == SESION && !check_session_time()){loop();}
        char seleccion = leerTecla(); 
        delay(KEYBOARD_DELAY);
        if (seleccion == '*'){
            tipoEntrada = KB_INPUT; 
            if(estado_actual == SESION ){update_session_time();}
            break;
        }else if(seleccion == '#'){
            tipoEntrada = APP_INPUT;
            if(estado_actual == SESION ){update_session_time();}
            break;
        }

    }
    pantalla.clear();
}

void ingresar_telefono(char* user){
    //realizar comprobacion de las condciones del casillero 
    for(int space = 0; space<9; space++){
        if( casillero[space].available ){

            char pos  = space + '0';
            pantalla.clear();
            pantalla.setCursor(0,0);
            pantalla.println("Intoduzca CEL en");
            pantalla.setCursor(0,1);
            pantalla.print(" POS #");
            pantalla.print(pos);
            while(true){
                if(!check_deposit('B',space) ){ // el botón *no está* presionado
                    break;
                }
            }
            pantalla.clear();
            pantalla.setCursor(0,0);
            pantalla.println("MUCHO CALOR en");
            pantalla.setCursor(0,1);
            pantalla.print(" POS #");
            pantalla.print(pos);
            while(true){
                if(!check_deposit('T',space) ){ // no hay una gran temperatura 
                    break;
                }
            }
            if(!pedir_password()){
                log_EEPROM("INGR CEL FAIL");
                return;
            } 
            casillero[space].available = false;
            strncpy(casillero[space].user,user,12);
            update_deposit(space,DEPOSIT);
            update_stats(CEL_INGRESADOS,1);
            log_EEPROM("INGR CEL OK");
            break;
        } 
    }
}

void retirar_telefono(char* user){

    uint8_t user_phone[9] = {255,255,255,255,255,255,255,255,255};
    uint8_t phone_count = 0;

    for(uint8_t space = 0; space<9; space++){
        if( !casillero[space].available && strncmp(user,casillero[space].user,12) == 0){
            user_phone[space] = space;
            phone_count++;
        }
    }
    if (phone_count == 0){
        return;
    }

    uint8_t selected = 255;

    pantalla.clear();
    uint8_t line = 0;
    for(uint8_t p = 0; p<9; p++){
        if(p == 3  || p == 6){line++; pantalla.setCursor(0,line);};
        if(user_phone[p] == 255){continue;}
        char c = user_phone[p] + '0';
        pantalla.println(c);
    }
    line++;
    pantalla.setCursor(0,line);
    pantalla.print("Opcion: ");
    char buf = '\0';
    while(selected == 255){
        pantalla.setCursor(7,line);
        pantalla.print(" ");
        pantalla.setCursor(7,line);
        buf = leerTecla();
        delay(KEYBOARD_DELAY);
        if (buf == ' ' || buf == '*' || buf == '#'){continue;}
        pantalla.print(buf);
        uint8_t index = buf - '0';
        delay(KEYBOARD_DELAY*2);
        if(user_phone[index]==255){
            continue; 
        }
        selected = index;

    }

    if(!pedir_password()){
        log_EEPROM("RM CEL FAIL");
        return;
    } 

    pantalla.clear();
    pantalla.setCursor(0,0);
    pantalla.println("Retire CEL en");
    pantalla.setCursor(0,1);
    pantalla.print(" POS #");
    char pos  = selected + '0';
    pantalla.print(pos);
    while(true){
        if(check_deposit('B',selected) ){ // el botón *no está* presionado
            break;
        }
    }
    char sacado = selected + '0';
    casillero[selected] = default_phone;
    update_deposit(selected,REMOVE);
    pantalla.clear();
    pantalla.setCursor(0,0);
    pantalla.print("Sacado #");
    pantalla.print(sacado);
    update_stats(CEL_INGRESADOS,-1);
    delay(250);

}

void render_casillero(){
    for(uint8_t x = 0; x<9; x++ ){
        for(uint8_t y = 0 ; y<9;y++){
            matriz.setLed(0,x,y,led_casillero[x][y]); 
        }
    }
}

bool eliminar_cuenta(char* user){
    if(!find_user(user)){
        Serial2.println("El usuario no existe");
        return false;
    }
    for(int space = 0; space<9; space++){
        if( !casillero[space].available && strncmp(user,casillero[space].user,12) == 0){
            // No se puede eliminar la cuenta si tiene dispositivos en el casillero
            // Mandar al menú de retiro
            pantalla.clear();
            pantalla.setCursor(0,0);
            pantalla.println("CEL en Sistema");
            pantalla.println("Menu de retiro...");
            delay(400);
            retirar_telefono(user);
            return false;
        }
    }
    // ¿ No pide contraseña para eliminar la cuenta ? 
    // eliminar usuario de la EEPROM
    uint16_t pos;
    struct usuario load;
    char* enc_usr = xor_encode(user);
    for(pos = EEPROM_USERS_START; pos < EEPROM_LOGS_START ; pos+=sizeof(struct usuario) ){
        EEPROM.get(pos,load);
        if(strcmp(load.nombre,enc_usr) == 0){
            for(uint8_t pos_user=0; pos_user < sizeof(struct usuario)-1; pos_user++){
                EEPROM.put(pos+pos_user,'\0');
            }
            Serial2.println("Usuario Borrado");
            free(enc_usr);
            
            log_EEPROM("RMV USR");
            update_stats(N_USUARIOS,-1);
            return true;
        }
    }
    //no se borró
    free(enc_usr);
    return false;
}

bool validar_credenciales(char* user, char* pass){
    uint16_t pos;
    struct usuario load;
    char* enc_usr = xor_encode(user);
    char* enc_pass = xor_encode(pass);
    for(pos = EEPROM_USERS_START; pos < EEPROM_LOGS_START ; pos+=sizeof(struct usuario) ){
        EEPROM.get(pos,load);
        if(strcmp(load.nombre,enc_usr) == 0 && strcmp(load.contra,enc_pass) == 0){
            free(enc_usr);
            free(enc_pass);
            return true;
        }
    }
    free(enc_usr);
    free(enc_pass);
    return false;
}

/*
* Actualiza la matriz del depósito de telefonos
*/ 
void update_deposit(uint8_t pos, operation op){
    uint8_t x_left,y_top;
    calcular_casillero_pos(pos,&x_left,&y_top);
    if( op == DEPOSIT ){
        led_casillero[y_top][x_left]    = 1;
        led_casillero[y_top+1][x_left]  = 1;
        led_casillero[y_top][x_left+1]  = 1;
        led_casillero[y_top+1][x_left+1]  = 1;
    }else{ //REMOVE
        led_casillero[y_top][x_left]    = 0;
        led_casillero[y_top+1][x_left]  = 0;
        led_casillero[y_top][x_left+1]  = 0;
        led_casillero[y_top+1][x_left+1]  = 0; 
    }

    // dibujar la matriz
    render_casillero();
    
    // Guardar estado en la EEPROM
    EEPROM.put(0,'\0');
    EEPROM.put(1,casillero);
}

void calcular_casillero_pos(uint8_t pos,uint8_t* x_left, uint8_t* y_top){
    switch(pos){
        case 0:
            *x_left = 0;
            *y_top = 0;
            break;
        case 1:
            *x_left = 3;
            *y_top = 0;
            break;
        case 2:
            *x_left = 6;
            *y_top = 0;
            break;
        case 3:
            *x_left = 0;
            *y_top = 3;
            break;
        case 4: 
            *x_left = 3;
            *y_top = 3;
            break;
        case 5:
            *x_left = 6;
            *y_top = 3;
            break;
        case 6:
            *x_left = 0;
            *y_top = 6;
            break;
        case 7:
           *x_left = 3;
           *y_top = 6;
            break;
        case 8:
            *x_left = 6;
            *y_top = 6;
            break;
    }
}

void fill_casillero(){
    for(uint8_t space = 0 ; space<9 ; space++){ 
        if( casillero[space].available == false ){
            uint8_t x_left,y_top;
            calcular_casillero_pos(space,&x_left,&y_top);
            led_casillero[y_top][x_left]    = 1;
            led_casillero[y_top+1][x_left]  = 1;
            led_casillero[y_top][x_left+1]  = 1;
            led_casillero[y_top+1][x_left+1]  = 1;
        }
    }
}

/*
* Cifrado XOR, usando las 2 llaves 
*/
char* xor_encode(char* input){
    char keys[2] = {LLAVE1,LLAVE2}; // Falta definir cual es la llave de verdad, ver enunciado xd
    uint8_t len = strlen(input);
    char* ret = (char*)malloc(sizeof(char)*13);
    strncpy(ret,input,12);
    for(uint8_t key = 0; key < 2; key++ ){
        for(uint8_t i = 0; i<len; i++){
            ret[i] = ret[i] ^ keys[key];
        }
    }
    return ret;
}


/*
* Descifrado XOR, usando las 2 llaves
*/
char* xor_decode(char* input){
    char keys[2] = {LLAVE2,LLAVE1};
    uint8_t len = strlen(input);
    char* ret = (char*)malloc(sizeof(char)*13);
    strncpy(ret,input,12);
    for(uint8_t key = 0; key < 2; key++ ){
        for(uint8_t i = 0; i<len; i++){
            ret[i] = ret[i] ^ keys[key];
        }
    }
    return ret;
}


/*
* Agrega un usuario a la eeprom, retorna false si ya existe un usuario con ese nombre 
* No evalua si la memoria está llena
*/
bool agregar_usuario(char* username, char* password, char* phone_number){
    if(find_user(username)){
        Serial2.println("El usuario ya existe");
        return false;
    }
    struct usuario new_user;
    char* enc_usr = xor_encode(username);
    char* enc_pass = xor_encode(password);
    uint16_t pos;
    struct usuario load;
    for(pos = EEPROM_USERS_START ; pos < EEPROM_LOGS_START ; pos+=sizeof(struct usuario) ){
        EEPROM.get(pos,load);
        if(EEPROM.read(pos) == '\0'){
            break;
        }
    }

    strncpy(new_user.nombre,enc_usr,13);
    strncpy(new_user.contra,enc_pass,13);
    strncpy(new_user.numero,phone_number,9);
    EEPROM.put(pos,new_user);

    struct usuario check;
    EEPROM.get(pos+sizeof(struct usuario),check);
    if(!find_user(xor_decode(check.nombre)) || pos + sizeof(struct usuario) < EEPROM_LOGS_START ){
        // Colocar un \0 para marcar que hay un espacio vacio contiguo
        // sólo si no hay otro registro de usuario o si está en la última posición
        EEPROM.put(pos+sizeof(struct usuario),'\0');
    }
    Serial2.println("---Nuevo usuario---");
    Serial2.print(enc_usr);
    Serial2.print(" ");
    Serial2.print(enc_pass);
    Serial2.print(" ");
    Serial2.print(phone_number);
    Serial2.println("");
    Serial2.println("-------------------");
    free(enc_usr);
    free(enc_pass);
    update_stats(N_USUARIOS,1);
    return true;
}
    
/*
* Verifica la existencia de un usuario en la EEPROM
*/
bool find_user(char* user){
    uint16_t pos;
    struct usuario load;
    char* enc_usr = xor_encode(user);
    for(pos = EEPROM_USERS_START ; pos < EEPROM_LOGS_START ; pos+=sizeof(struct usuario) ){
        EEPROM.get(pos,load);
        if(strcmp(load.nombre,enc_usr) == 0 ){
            free(enc_usr);
            return true;
        }
    }
    free(enc_usr);
    return false;
}

/*
* Le pide la contraseña al usuario para hacer verificaciones
* La entrada se hace con el teclado
*/
bool pedir_password(){
    tipo_de_input();
    char buffer[16] = {0};
    while(true){
        pantalla.clear();
        pantalla.setCursor(0, 0);
        bool input;
        if (tipoEntrada == KB_INPUT){pantalla.println("password:"); input  = keyboard_input(buffer,1); } 
        if (tipoEntrada == APP_INPUT){input = bluetooth_input(buffer,"password");}
        if(input){ break; }
    }
    Serial2.println(buffer);
    pantalla.clear();
    bool cred = validar_credenciales(nombre_temp,buffer);
    if(!cred){
        for(uint8_t i = 0; buffer[i]!='\0';i++){buffer[i]='\0';}
        while(true){
            pantalla.clear();
            pantalla.setCursor(0, 0);
            bool input;
            if (tipoEntrada == KB_INPUT){ pantalla.println("error password:"); input  = keyboard_input(buffer,1); } 
            if (tipoEntrada == APP_INPUT){input = bluetooth_input(buffer,"error password"); }
            if(input){ break; }
        }
    }
    cred = validar_credenciales(nombre_temp,buffer);
    if(!cred){
        // si falla 2 veces se bloquea 10 segundos y retorna al mensaje inicia 
        estado_actual = MENU;
        for(uint8_t i= 10 ;i > 0; i--  ){
            pantalla.clear();
            pantalla.setCursor(0,0);
            pantalla.println(i);
            delay(1000);
        }
        mensaje_inicial();
        return false;
    }
    return true;
}

/*
* Función unicamente para hacer pruebas sin tener que pasar por menues xd
*/
boolean iniciar_sesion(char* user, char* pass){
    if (validar_credenciales(user,pass)){
        strncpy(nombre_temp,user,12);
        strncpy(contra_temp,pass,12);
        current_session_time = millis();
        return true;
    }
    return false;
}

void update_session_time(){
    current_session_time = millis();
}

bool check_session_time(){
    if(millis() - current_session_time > MAX_SESSION_TIME){
        Serial2.println("SE ACABO EL TIEMPO");
        estado_actual = MENU;
        return false;
    }
    return true;
}

/*
* Utiliza el teclado matricial para llenar un arreglo de caracteres.
* Retorn true si el usuario da a aceptar, false si da a cancelar
* El parametro line determina la línea en la que se mostrará el input
*/
bool keyboard_input(char* buffer,uint8_t line){
    pantalla.setCursor(0, line);
    uint8_t key_pos = 0;
    uint8_t buffer_index = 0;
    for(;;){
        pantalla.setCursor(0, line);
        char key = leerTecla();
        delay(KEYBOARD_DELAY);
        uint8_t key_index;
        bool next =  false; 
        bool prev = false;
        switch(key){
            case '1':
                key_index = 0;
                key_pos = 0;
                break;
            case '2':
                key_index = 1;
                key_pos = 0;
                break;
            case '3':
                key_index = 2;
                key_pos = 0;
                break;
            case '4':
                key_index = 3;
                key_pos = 0;
                break;
            case '5':
                key_index = 4;
                key_pos = 0;
                break;
            case '6':
                key_index = 5;
                key_pos = 0;
                break;
            case '7':
                key_index = 6;
                key_pos = 0;
                break;
            case '8':
                key_index = 7;
                key_pos = 0;
                break;
            case '9':
                key_index = 8;
                key_pos = 0;
                break;
            case '*':
                prev = true;
                break;
            case '#':
                next = true;
                break;
            case '0':
                if(buffer[buffer_index]!='\0'){
                    buffer_index++;
                }
                continue;
                break;
            default:

                if (digitalRead(2) == HIGH) { // botón aceptar 
                    Serial2.println("aceptar");
                    delay(KEYBOARD_DELAY);
                    update_session_time();
                    return true;
                }

                if(digitalRead(3) == HIGH){ // botón cancelar
                    Serial2.println("cancelar");
                    for(uint8_t i = 0; buffer[i]!='\0';i++){buffer[i]='\0';}
                    delay(KEYBOARD_DELAY);
                    update_session_time();
                    return false;
                }
                if(estado_actual == SESION && !check_session_time()){loop();}

                continue;
                break;
        }
        update_session_time();

        if(next){
            key_pos = ( key_pos == 4 || teclado_matricial[key_index][key_pos + 1 ] == '\0' ) ? 0 : key_pos + 1;
        }else if(prev){
            key_pos = ( key_pos == 0 ) ? 4 : key_pos - 1;
            if(key_pos == 4 && teclado_matricial[key_index][key_pos] == '\0'){
                key_pos--;
            }
        }
        buffer[buffer_index] = teclado_matricial[key_index][key_pos];
        pantalla.println(buffer);
        Serial2.println(buffer);
    }
    return false;
}

bool bluetooth_input(char* buffer,char* message){ 

    limpiarBuffer();
    pantalla.clear();
    pantalla.print(" Esperando  una ");
    pantalla.setCursor(0, 1);
    pantalla.print("   conexion...  ");
    bool alguienPorAhi = false;
    char recibidos[3];
    while(true) {
        while(Serial.available()) {

            if(estado_actual == SESION && !check_session_time()){loop();}
            Serial.readBytes(recibidos, 2);
            alguienPorAhi = true;
            break;
        }
        if(alguienPorAhi){break;}
    }
    update_session_time();
    while(true){

        if(estado_actual == SESION && !check_session_time()){loop();}
        bool seEnvioAlgo = false;
        int indiceBuffer = 0;
        long int t0 = millis();
        long int t1 = millis();
        limpiarBuffer();
        enviarConfirmar(message);
        pantalla.clear();
        pantalla.setCursor(0,0);
        pantalla.println(message);
        while(true){
            while (Serial.available()) {
                seEnvioAlgo = true;
                buffer[indiceBuffer++] = Serial.read();
            }
            if (seEnvioAlgo) {
                t1 = millis();
                if (t1 - t0 >= 500) break;
            } else {
                t0 = millis();
                t1 = millis();
            }
        }
        update_session_time();
        pantalla.setCursor(0,1);
        pantalla.println(buffer);
        pantalla.setCursor(0,2);
        pantalla.print("Aceptar?       ");
        delay(500);
        if (entradaAceptada()){ update_session_time(); return true;}
        for(uint8_t i = 0; buffer[i]!='\0';i++){buffer[i]='\0';}
    }
}

bool check_deposit(char type, uint8_t position){
    char c_pos = position + '0';
    char msg[2] = { type, c_pos }; 
    char rsp[2];
    Serial3.print(msg);
    Serial3.readBytes(rsp, 1);
    if(rsp[0] == '1'){ // si es botón 1 = botón no está presionado, si es sensor de temp 1 = temp alta
        return true;
    }else{
        return false;
    }
}


void check_casillero(){
    for(uint8_t space = 0; space<9; space++){
        char sp = space + '0';
        bool check_temp = check_deposit('T',space);
        bool check_butt = check_deposit('B',space);
        if(!casillero[space].available){
            if(check_butt){
                char butt_log[15]= "NO HAY CEL # ";
                butt_log[strlen(butt_log)-1] = sp;
                Serial2.println(butt_log);
                update_stats(INCIDENTES,1);
                log_EEPROM(butt_log);
            }
            if(check_temp){
                char temp_log[]= "MUCHO CALOR # ";
                temp_log[strlen(temp_log)-1] = sp;
                update_stats(INCIDENTES,1);
                Serial2.println(temp_log);
                log_EEPROM(temp_log);
            }
        }
    /*
    */
    }
}


/*************************************************************/

void log_EEPROM(char* description){
    uint8_t id_counter = 0;
    for(uint16_t pos = EEPROM_LOGS_START; pos < EEPROM.length() ; pos+=sizeof(evento) ){
        uint8_t load;
        load = EEPROM.read(pos);
        if(load == id_counter){
            id_counter++;
        }else{
            evento new_evento = {.id = id_counter}; 
            strncpy(new_evento.desc,description,15);
            EEPROM.put(pos,new_evento);
            break;
        }
    }
}

void get_log(uint8_t index, char* buffer){
    evento load;
    uint16_t pos = EEPROM_LOGS_START + sizeof(evento)*index;
    if(EEPROM.read(pos) != index){return;} // No existe el log
    EEPROM.get(pos,load);
    strncpy(buffer,load.desc,15);
}

void show_logs(){
    pantalla.clear();
    uint8_t inner_counter;
    char buffer[16] = {0};
    for(uint8_t i = 0; i<100; i++){
        if(inner_counter > 3 ){
            while(true){
                if(digitalRead(2) == HIGH){ delay(KEYBOARD_DELAY); pantalla.clear(); inner_counter = 0; break; }
                if(digitalRead(3) == HIGH){ delay(KEYBOARD_DELAY); return; }
            }
        }
        get_log(i,buffer);
        if(buffer[0] == '\0' ){
            while(true){
                if(digitalRead(3) == HIGH){ delay(KEYBOARD_DELAY); return; }
            }
        }
        pantalla.setCursor(0,inner_counter);
        pantalla.println(buffer);
        for(uint8_t i = 0; buffer[i]!='\0';i++){buffer[i]='\0';}
        inner_counter++;
    }
}

void update_stats(adm_stat type, int step){
    switch(type){
        case CEL_INGRESADOS:
            adm_stats.celulares = adm_stats.celulares + step;
            break;
        case INT_FALLIDOS:
            adm_stats.fallidos = adm_stats.fallidos + step;
            break;
        case INCIDENTES:
            adm_stats.incidentes = adm_stats.incidentes +step;
            break;
        case N_USUARIOS:
            adm_stats.usuarios = adm_stats.usuarios + step;
            break;
    }

    EEPROM.put(ADMIN_STATS_START+1,adm_stats);
}

void show_stats(){
    pantalla.clear();
    pantalla.setCursor(0,0);
    pantalla.print("CEL #      ");
    pantalla.print(adm_stats.celulares);
    pantalla.setCursor(0,1);
    pantalla.print("FAILED #   ");
    pantalla.print(adm_stats.fallidos);
    pantalla.setCursor(0,2);
    pantalla.print("INCIDENT # ");
    pantalla.print(adm_stats.incidentes);
    pantalla.setCursor(0,3);
    pantalla.print("ACT USER # ");
    pantalla.print(adm_stats.usuarios);
    while(true){
        if(digitalRead(3) == HIGH){ delay(KEYBOARD_DELAY); return; }
    }
}


bool validar_numero(char* numero){
    uint8_t len = strlen(numero);
    if(len != 8){return false;}
    char c;
    for(uint8_t ci = 0;ci<len;ci++){
        c = numero[ci];
        if(!isdigit(c)){return false;}
    }
    return true;
}

bool validar_nombre(char* nombre){
    uint8_t len = strlen(nombre);
    if(len > 12 || len < 8){return false;}
    char c;
    for(uint8_t ci = 0;ci<len;ci++){
        c = nombre[ci];
        if(!isdigit(c) && !isalpha(c)){return false;}
    }
    return true;
}

bool validar_contra(char* pass){
    uint8_t len = strlen(pass);
    if(len > 12 || len < 8){return false;}
    bool flag_digit = false;
    bool flag_alpha = false;
    bool flag_especial= false;
    char c;
    for(uint8_t ci = 0;ci<len;ci++){
        c = pass[ci];
        if(isdigit(c)){
            flag_digit = true; 
        }
        if(isalpha(c)){
            flag_alpha = true;
        }
        if(c == '*' || c == '#' || c == '$' || c == '!'){
            flag_especial = true;
        }
    }
    return flag_digit && flag_alpha && flag_especial;
    
}

/************************************************************* 
**************************************************************
*************************************************************/

void borrarEEPROM() {
  for (int i = 0; i < EEPROM.length(); i++)
    EEPROM.write(i, 0);
}

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

void mensaje_inicial() {
  pantalla.clear();
  pantalla.setCursor(0, 0);
  pantalla.println("Ruben 202111835");
  pantalla.setCursor(0, 1);
  pantalla.println("Sergio 202100154");
  pantalla.setCursor(0, 2);
  pantalla.println("Pedro 201020600");
  pantalla.setCursor(0, 3);
  pantalla.println("Jose 201901756");
  delay(500);
  pantalla.clear();
  pantalla.setCursor(0, 0);
  pantalla.println("Sebas 201906085");
  delay(500);
}

void setup() {
  // put your setup code here, to run once:
  Serial2.begin(9600);
  // put your setup code here, to run once:
  Serial2.begin(9600);
  Serial.begin(9600);
  Serial3.begin(9600);  // Comunicación serial con el arduino de los sensores
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
  mensaje_inicial();


  if (!find_user(ADMIN_NAME)) {
    Serial2.println("Agregar Admin");
    EEPROM.put(EEPROM_USERS_START, '\0');  // Se pone un 0 en la primera posición, para marcar que está vacía
    agregar_usuario(ADMIN_NAME, ADMIN_PASS, "0");
  } else {
    Serial2.println("Hay admin");
  }

  if (EEPROM.read(0) == '\0') {  // Existe casillero
    EEPROM.get(1, casillero);
    fill_casillero();
  }
  render_casillero();
 
    //agregar_usuario("B","1","1");
    //iniciar_sesion("B","1");
    //estado_actual =SESION;
    //Para no pasar por menus xd 
    //agregar_usuario("A","1","1");
    //estado_actual = SESIONADMIN;
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
    while (Serial.available()) {
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
    while (Serial.available()) {
      Serial.read();
    }
    if ((t1 - t0 >= 1000) && !Serial.available()) break;
  }
}

void imprimirAsteriscos(char* cadena) {
  for (; *cadena; cadena++)
    pantalla.write('*');
}

/*
  *Agrega a la memoria eeprom un char[15]
*/
bool agregarlogs(char* descripcion) {
    log_EEPROM(descripcion);
    return true;
}



void loop() {
  // put your main code here, to run repeatedly:
  switch (estado_actual) {
    case MENU:
      {
        //Serial.println("MENU");
        check_casillero();
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
        while (true) {  //loop que mueve el cursor o detecta el boton aceptar
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
                //estado_actual = SESIONADMIN;
                estado_actual = REGISTRO2;
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
    case LOGS:
      {
        byte cantidad = 0;
        int i = 0;
        EEPROM.get(EEPROM_LOGS_START, cantidad);
        pantalla.clear();
        Serial2.println(cantidad);
        int siguiente_direccion = EEPROM_LOGS_START + 1;
        int indicePantalla = 0;
        bool salir = false;
        while (i < cantidad && !salir) {
          evento cargado;
          EEPROM.get(siguiente_direccion, cargado);
          pantalla.setCursor(2, indicePantalla);
          Serial2.println(cargado.id);
          pantalla.print(cargado.desc);
          pantalla.setCursor(0, indicePantalla);
          pantalla.print(cargado.id);
          //pantalla.print("Imprime");
          siguiente_direccion += sizeof(evento);
          indicePantalla++;
          i++;
          while (indicePantalla == 4) {
            if (digitalRead(2) == HIGH) {
              delay(230);
              indicePantalla = 0;
              pantalla.clear();
            }
            if (digitalRead(3) == HIGH) {
              delay(230);
              salir = true;
              indicePantalla = 0;
              pantalla.clear();
            }
          }
        }
        while (indicePantalla != 0 && !salir) {
          if (digitalRead(2) == HIGH || digitalRead(3) == HIGH) {
            delay(230);
            break;
          }
        }
        estado_actual = SESIONADMIN;
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
          while (Serial.available()) {
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
        memset(numero_temp, 0, 9);
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
        bool encontrado = validar_credenciales(nombre_temp, contra_temp);
        pantalla.clear();


        if (strcmp(nombre_temp, ADMIN_NAME) == 0 && strcmp(contra_temp, ADMIN_PASS) == 0) {
          pantalla.print("ADMIN");
          delay(50);
          estado_actual = SESIONADMIN;
          agregarlogs("loginAE");
        } else if (encontrado == true) {
          estado_actual = SESION;
          agregarlogs("loginUE");
        } else {
          //pantalla.print(nombre_temp);
          //pantalla.print(contra_temp);
          delay(50);
          estado_actual = MENU;
          agregarlogs("loginFA");
          update_stats(INT_FALLIDOS,1);
        }

        delay(2000);
        break;
        break;
      }
    case SESIONADMIN:
      {
        check_casillero();
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Presiona aceptar");
        pantalla.setCursor(0, 1);
        pantalla.print("  Logs");
        pantalla.setCursor(0, 2);
        pantalla.print("  Estado Sys");
        pantalla.setCursor(0, 3);
        pantalla.print("  Cerrar sesion");
        pantalla.setCursor(0, opcion_menu + 1);
        pantalla.print(">");
        while (true) {  //loop que mueve el cursor o detecta el boton aceptar
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
                //estado_actual = LOGS;
                show_logs();
                break;
              case 1:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                show_stats();
                break;
              case 2:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                estado_actual = MENU;
                agregarlogs("CerradoA");
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
        check_casillero();
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print(" Ingreso Cel");
        pantalla.setCursor(0, 1);
        pantalla.print(" Retiro Cel");
        pantalla.setCursor(0, 2);
        pantalla.print(" Cerrar sesion");
        pantalla.setCursor(0, 3);
        pantalla.print(" Eliminar Cuenta");
        pantalla.setCursor(0, opcion_menu);
        pantalla.print(">");
        while (true) {  //loop que mueve el cursor o detecta el boton aceptar
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
            if (opcion_menu > 3) opcion_menu = 3;
            break;
          }
          if (digitalRead(2) == HIGH) {  //boton aceptar
            delay(210);
            switch (opcion_menu) {
              case 0:
                //estado_actual = ESPERANDO;
                //siguiente_estado = APLICACION;
                ingresar_telefono(nombre_temp);
                break;
              case 1:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                retirar_telefono(nombre_temp);
                break;
              case 2:
                //estado_actual = ESPERANDO;
                //siguiente_estado = REGISTRO;
                estado_actual = MENU;
                agregarlogs("CerradoUS");
                break;
              case 3:
                if (eliminar_cuenta(nombre_temp)) {
                  estado_actual = MENU;
                } else {
                  // menu remover
                }
                break;
            }
            opcion_menu = 0;
            break;
          }
        }
        break;
      }

    case RPANEL:
    {

      char buffer[16] = { 0 };
      char buffer2[16] = { 0 };
      char buffer3[16] = { 0 };
        
      goto nombre_panel;
      err_nombre_panel:{
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Error");
        pantalla.setCursor(0, 1);
        pantalla.print("Nombre no valido");
        for(uint8_t i = 0; buffer[i]!='\0';i++){buffer[i]='\0';}
        delay(300);
      };

      nombre_panel:
      pantalla.clear();
      pantalla.setCursor(0, 0);
      pantalla.print("REGISTRO");
      pantalla.setCursor(0, 1);
      pantalla.print(" - NOMBRE:");

      if (keyboard_input(buffer, 2)) {
        delay(100);
        if(!validar_nombre(buffer)){
            goto err_nombre_panel;
        }
        strcpy(nombre_temp, buffer); 
      }

      goto contra_panel; 
      err_contra_panel:{
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Error");
        pantalla.setCursor(0, 1);
        pantalla.print("Contra no valida");
        for(uint8_t i = 0; buffer2[i]!='\0';i++){buffer[i]='\0';}
        delay(300);
      };

      contra_panel:
      pantalla.clear();
      pantalla.setCursor(0, 0);
      pantalla.print("REGISTRO");
      pantalla.setCursor(0, 1);
      pantalla.print(" - CONTRA:");

      if (keyboard_input(buffer2, 2)) {
        delay(100);
        if(!validar_contra(buffer2)){
            goto err_contra_panel;
        }
        strcpy(contra_temp,buffer2); 
      }

      goto numero_panel;
      err_number_panel:{
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("Error");
        pantalla.setCursor(0, 1);
        pantalla.print("Numero no valido");
        for(uint8_t i = 0; buffer3[i]!='\0';i++){buffer[i]='\0';}
        delay(300);
      };

      numero_panel:
      pantalla.clear();
      pantalla.setCursor(0, 0);
      pantalla.print("REGISTRO");
      pantalla.setCursor(0, 1);
      pantalla.print(" - NUMERO:");
      if (keyboard_input(buffer3, 2)) {
        delay(100);
        if(!validar_numero(buffer3)){
            goto err_number_panel;
        }
        strcpy(numero_temp,buffer3);
        
      }

      bool usuario_creado = agregar_usuario(nombre_temp, contra_temp, numero_temp);
      if (usuario_creado) {
        estado_actual = MENU;
      }
    

      break;

    }
    case PANEL:
      {
        pantalla.clear();
        pantalla.setCursor(0, 0);
        pantalla.print("LOGIN");
        pantalla.setCursor(0, 1);
        pantalla.print(" - NOMBRE:");
        char buffer[16] = { 0 };
        char buffer2[16] = { 0 };

        if (keyboard_input(buffer, 2)) {
          delay(100);
          strcpy(nombre_temp, buffer);
          pantalla.clear();
          pantalla.setCursor(0, 0);
          pantalla.print("Login");
          pantalla.setCursor(0, 1);
          pantalla.print(" - CONTRA:");
          
        }

        

        if (keyboard_input(buffer2, 2)) {
          delay(100);
          strcpy(contra_temp,buffer2);
          
        }

        if(validar_credenciales(buffer, buffer2)){
          //pantalla.clear();
          //pantalla.print(nombre_temp);
          //pantalla.print(contra_temp);
          //delay(500);
          pantalla.clear();
          delay(500);
          if(strcmp(nombre_temp, ADMIN_NAME) == 0 && strcmp(contra_temp, ADMIN_PASS) == 0){
            estado_actual = SESIONADMIN;
            agregarlogs("loginAE");
          }else{
            estado_actual = SESION;
            agregarlogs("loginUE");
          }
          
        }else{
          pantalla.clear();
          estado_actual = MENU;
          agregarlogs("loginFA");
        }


        break;
      }


    case REGISTRO2:
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
        while (true) {  //loop que mueve el cursor o detecta el boton aceptar
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
                siguiente_estado = LOGIN;

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
        while (true) {  //loop que mueve el cursor o detecta el boton aceptar
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
                estado_actual = RPANEL;
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

        memset(nombre_temp, 0, 13);
        memset(contra_temp, 0, 13);
        memset(numero_temp, 0, 9);
        struct usuario nuevo_usuario;
        LOOP {
          limpiarBuffer();
          enviarConfirmar("Nombre:");
          memset(nombre_temp, 0, 13);
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
          memset(contra_temp, 0, 13);
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
            //enviarConfirmar("NADA");
            //
            /*
          memcpy(nuevo_usuario.nombre, nombre_temp, 13);
          memcpy(nuevo_usuario.numero, numero_temp, 9);
          memcpy(nuevo_usuario.contra, contra_temp, 13);
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
            */

        // true -> creado , false -> no creado
        bool usuario_creado = agregar_usuario(nombre_temp, contra_temp, numero_temp);
        if (usuario_creado) {
          estado_actual = MENU;
        }
      }
  }
}
