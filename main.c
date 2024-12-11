#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


typedef enum {
    TOKEN_REGISTER,
    TOKEN_INSTRUCTION,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_END_INSTRUCTION,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[1024];
} Token;

struct {
    char name[4];
    enum { is_int, is_float, is_str } type;
    union {
        int ival;
        float fval;
        char sval[64];
    } val;
} registers[8];


int memory[1024];


Token tokenize(const char **input) {
    Token token;
    token.type = TOKEN_INVALID;
    token.value[0] = '\0';

    // Skip whitespace
    while (isspace(**input)) {
        (*input)++;
    }

    // End of instruction
    if (**input == '\0' || **input == '#') {
        token.type = TOKEN_END_INSTRUCTION;
        return token;
    }

    // Instructions and Registers (letter followed by letters)
    if (isalpha(**input)) {
        int length = 0;
        while (isalpha(**input) && length < 1024 - 1) {
            token.value[length++] = **input;
            (*input)++;
        }
        token.value[length] = '\0';
        token.type = token.value[0] == 'r' && (token.value[2] == 'x' || token.value[2] == 'p') ? TOKEN_REGISTER : TOKEN_INSTRUCTION;
        return token;
    }

    // Numbers (digits)
    if (isdigit(**input)) {
        int length = 0;
        while ((isdigit(**input) || **input == '.') && length < 1024 - 1) {
            token.value[length++] = **input;
            (*input)++;
        }
        token.value[length] = '\0';
        token.type = strstr(token.value, ".") != NULL ? TOKEN_FLOAT : TOKEN_INT;
        return token;
    }

    // Strings ("" or '')
    if (strchr("\"", **input) || strchr("'", **input)) {
        int length = 0;
        (*input)++;
        do {
            token.value[length++] = **input;
            (*input)++;
        } while (!(strchr("\"", **input) || strchr("'", **input)) && length < 1024 - 1);
        token.value[length] = '\0';
        token.type = TOKEN_STRING;
        return token;
    }

    // Invalid token
    token.value[0] = **input;
    token.value[1] = '\0';
    token.type = TOKEN_INVALID;
    (*input)++;
    return token;
}



bool parse(Token command[], int index) {
    if(command[0].value[0] == '\0') {
        return true;
    }
    if (command[0].type != TOKEN_INSTRUCTION) {
        printf("Error line %d: first token must be an instruction\n", index);
        return false;
    }
    if (
        command[1].type != TOKEN_REGISTER
        & command[1].type != TOKEN_INT
        & command[1].type != TOKEN_FLOAT
        & command[1].type != TOKEN_STRING
    ) {
        printf("Error line %d: second token must be a register or a value\n", index);
        return false;
    }
    if (command[2].type == TOKEN_END_INSTRUCTION) {
        printf("Error line %d: third token is missing\n", index);
        return false;
    }
    if (
        command[2].type != TOKEN_INT
        && command[2].type != TOKEN_FLOAT
        && command[2].type != TOKEN_STRING
        && command[2].type != TOKEN_REGISTER
    ) {
        printf("Error line %d: third token must be register or a value\n", index);
        return false;
    }

    return true;
}

void set_registers() {
    for (int k = 0; k < 6; k++) {
        registers[k].name[0] = 'r';
        registers[k].name[1] = (char)(k + 97);
        registers[k].name[2] = 'x';
        registers[k].name[3] = '\0';
    }
    strcpy(registers[6].name, "rip");
    strcpy(registers[7].name, "rmp");
    registers[7].type = is_int;
    registers[7].val.ival = 0;
}

int get_register_index(char registerName[]) {
    if(strcmp(registerName, "rax") == 0) {
        return 0;
    } else if (strcmp(registerName, "rbx") == 0) {
        return 1;
    } else if (strcmp(registerName, "rcx") == 0) {
        return 2;
    } else if (strcmp(registerName, "rdx") == 0) {
        return 3;
    } else if (strcmp(registerName, "rex") == 0) {
        return 4;
    } else if (strcmp(registerName, "rfx") == 0) {
        return 5;
    } else if (strcmp(registerName, "rip") == 0) {
        return 6;
    } else if (strcmp(registerName, "rmp") == 0) {
        return 7;
    }
}

void execute(Token command[]) {


    if (strcmp(command[0].value, "init") == 0) {
        if(command[2].type == TOKEN_STRING) {
            registers[get_register_index(command[1].value)].type = is_str;
            strcpy(registers[get_register_index(command[1].value)].val.sval, command[2].value);
        } else if(command[2].type == TOKEN_INT) {
            registers[get_register_index(command[1].value)].type = is_int;
            registers[get_register_index(command[1].value)].val.ival = atoi(command[2].value);
        } else if(command[2].type == TOKEN_FLOAT) {
            registers[get_register_index(command[1].value)].type = is_float;
            registers[get_register_index(command[1].value)].val.fval = atof(command[2].value);
        }
    }


    if (strcmp(command[0].value, "print") == 0) {
        if(command[1].type == TOKEN_REGISTER) {
            switch(registers[get_register_index(command[1].value)].type) {
                case is_str:
                    printf("%s\n",registers[get_register_index(command[1].value)].val.sval);
                    break;
                case is_int:
                    printf("%d\n",registers[get_register_index(command[1].value)].val.ival);
                    break;
                case is_float:
                    printf("%f\n",registers[get_register_index(command[1].value)].val.fval);
                    break;
            }
        } else if(command[1].type == TOKEN_INT) {
            printf("%d\n",atoi(command[1].value));
        } else if (command[1].type == TOKEN_FLOAT) {
            printf("%f\n",atof(command[1].value));
        } else if (command[1].type == TOKEN_STRING) {
            printf("%s\n",command[1].value);
        }
    }
    
}

int main() {
    FILE *file = fopen("test.asl", "r");
    if (!file) {
        printf("Error: Cant read file");
        exit(1);
    }

    char lines[1024][128], line[128];
    int programLength = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        strcpy(lines[programLength++], line);
    }
    fclose(file);

    Token code[1024][4];
    int codeLen = 0;
    for (int i=0;i<programLength;i++) { 
        const char *p = lines[i];
        Token token;
        int tokenIndex = 0;
        while ((token = tokenize(&p)).type != TOKEN_END_INSTRUCTION) {
            code[codeLen][tokenIndex] = token;
            tokenIndex++;
        }
        codeLen++;
    }


    set_registers();
    while (registers[6].val.ival < codeLen){
        if(!parse(code[registers[6].val.ival], registers[6].val.ival)){
            exit(1);
        } else if (code[registers[6].val.ival][0].value == NULL) {
            registers[6].val.ival++;
            continue;
        }
        execute(code[registers[6].val.ival]);
        registers[6].val.ival++;
    }

    return 0;

}
