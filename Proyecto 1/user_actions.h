#include <cstdint>
typedef struct{
    bool available;
    char user[13];
    char phone_number[9];
}phone;
typedef phone phone_deposit[6];

typedef enum{
    REMOVE,
    DEPOSIT
}operation;

void check_credenciales(char* user, char* pass);
void ingresar_telefono(char* user, char* phone_number);
void retirar_telefono(char* user);
void eliminar_cuenta(char* user);
void update_deposit(uint8_t pos, operation op);
