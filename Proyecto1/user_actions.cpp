#include "user_actions.h"
#include <cstdint>
#include <cstring>

#define DEFAULT_PHONE { .available = true, .user = "\0", .phone_number = "\0" }
#define DEFAULT_PHONE_DEPOSIT { DEFAULT_PHONE, DEFAULT_PHONE, DEFAULT_PHONE, DEFAULT_PHONE, DEFAULT_PHONE, DEFAULT_PHONE }
#define SEP    {0,0,1,0,0,1,0,0}
#define FLINE  {1,1,1,1,1,1,1,1}

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

phone_deposit casillero = DEFAULT_PHONE_DEPOSIT;

void ingresar_telefono(char* user, char* phone_number){
    for(int space = 0; space>6; space++){
        if( casillero[space].available ){
            //pedir contraseña
            casillero[space].available = false;
            strncpy(casillero[space].user,user,12);
            strncpy(casillero[space].phone_number,phone_number,8); 
            update_deposit(space,DEPOSIT);
        } 
    }
}

void retirar_telefono(char* user){
    uint8_t user_phone[6] = {0,0,0,0,0,0};
    uint8_t phone_count = 0;

    for(int space = 0; space>6; space++){
        if( !casillero[space].available && strncmp(user,casillero[space].user,12) == 0){
            user_phone[space] = space;
            phone_count++;
        }
    }
    if (phone_count == 0){
        return;
    }

    uint8_t selected = 0;
    if (phone_count > 1){
        // Pedirle selección de telefono al usuario, si tiene más de uno
    }else{
        selected = user_phone[0];
    }

    // pedir contraseña
    casillero[selected] = DEFAULT_PHONE;
    update_deposit(selected,REMOVE);

}

void eliminar_cuenta(char* user){
    for(int space = 0; space>6; space++){
        if( !casillero[space].available && strncmp(user,casillero[space].user,12) == 0){
            // No se puede eliminar la cuenta si tiene dispositivos en el casillero
            // Mandar al menú de retiro
        }
    }
    // ¿ No pide contraseña para eliminar la cuenta ?
    // eliminar usuario de la EEPROM
}

void update_deposit(uint8_t pos, operation op){
    uint8_t x_left,y_top;
    switch(pos){
        case 0:
            x_left = 0;
            y_top = 0;
            break;
        case 1:
            x_left = 3;
            y_top = 0;
            break;
        case 2:
            x_left = 6;
            y_top = 0;
            break;
        case 3:
            x_left = 0;
            y_top = 4;
            break;
        case 4: 
            x_left = 3;
            y_top = 4;
            break;
        case 5:
            x_left = 6;
            y_top = 4;
            break;
        case 6:
            x_left = 0;
            y_top = 6;
            break;
        case 7:
            x_left = 3;
            y_top = 6;
            break;
        case 8:
            x_left = 6;
            y_top = 6;
            break;
    }

    if( op == DEPOSIT ){
        led_casillero[x_left][y_top]    = 1;
        led_casillero[x_left+1][y_top]  = 1;
        led_casillero[x_left][y_top+1]  = 1;
        led_casillero[x_left][y_top+1]  = 1;
    }else{ //REMOVE
        led_casillero[x_left][y_top]    = 0;
        led_casillero[x_left+1][y_top]  = 0;
        led_casillero[x_left][y_top+1]  = 0;
        led_casillero[x_left][y_top+1]  = 0; 
    }

    // dibujar la matriz
}

int main(){return 0;}
