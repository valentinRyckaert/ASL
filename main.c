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

typedef struct {
    enum { is_int, is_float, is_str } type;
    union {
        int ival;
        float fval;
        char sval[64];
    } val;
} MultiTypeVar;

MultiTypeVar registers[8];
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
        while ((isalpha(**input) || **input == '?') && length < 1024 - 1) {
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


void raiseError(char message[], int lineIndex) {
    printf("Error line %d: %s\n",++lineIndex, message);
    exit(1);
}


void parse(Token command[], int index) {

    if (command[0].type != TOKEN_INSTRUCTION) {
        raiseError("first token must be an instruction", index);
    }
    if (
        command[1].type != TOKEN_REGISTER
        & command[1].type != TOKEN_INT
        & command[1].type != TOKEN_FLOAT
        & command[1].type != TOKEN_STRING
    ) {
        raiseError("second token must be a register or a value", index);
    }
    if (command[2].type == TOKEN_END_INSTRUCTION) {
        raiseError("third token is missing", index);
    }
    if (
        command[2].type != TOKEN_INT
        && command[2].type != TOKEN_FLOAT
        && command[2].type != TOKEN_STRING
        && command[2].type != TOKEN_REGISTER
    ) {
        raiseError("third token must be register or a value", index);
    }
}

void set_pointers() {
    registers[6].type = is_int;
    registers[6].val.ival = 0;
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
    } else {
        raiseError("unknown register",registers[6].val.ival);
    }
}

void set_register(int registerIndex, MultiTypeVar value, int operation) {
    registers[registerIndex].type =  value.type;
    switch(registers[registerIndex].type) {
        case is_int:
            if (operation == 0)
                registers[registerIndex].val.ival = value.val.ival;
            else if (operation == 1)
                registers[registerIndex].val.ival += value.val.ival;
            else if (operation == 2)
                registers[registerIndex].val.ival -= value.val.ival;
            else if (operation == 3)
                registers[registerIndex].val.ival *= value.val.ival;
            else if (operation == 4)
                registers[registerIndex].val.ival /= value.val.ival;
            break;
        case is_float:
            if (operation == 0)
                registers[registerIndex].val.fval = value.val.fval;
            else if (operation == 1)
                registers[registerIndex].val.fval += value.val.fval;
            else if (operation == 2)
                registers[registerIndex].val.fval -= value.val.fval;
            else if (operation == 3)
                registers[registerIndex].val.fval *= value.val.fval;
            else if (operation == 4)
                registers[registerIndex].val.fval /= value.val.fval;
            break;
        case is_str:
            if (operation == 0)
                strcpy(registers[registerIndex].val.sval,  value.val.sval);
            else if (operation == 1)
                strcat(registers[registerIndex].val.sval,  value.val.sval);
            else if (operation == 2)
                raiseError("cannot do substraction on string", registers[6].val.ival);
            else if (operation == 3)
                raiseError("cannot do multiplication on string", registers[6].val.ival);
            else if (operation == 4)
                raiseError("cannot do division on string", registers[6].val.ival);
            break;
    }
}

void do_operation_on_register(int registerIndex, Token tokenValue, int operationType) {
    MultiTypeVar n;
    if (tokenValue.type == TOKEN_REGISTER) {
        set_register(registerIndex, registers[get_register_index(tokenValue.value)], operationType);
    } else if(tokenValue.type == TOKEN_STRING) {
        n.type = is_str;
        strcpy(n.val.sval, tokenValue.value);
        set_register(registerIndex, n, operationType);
    } else if(tokenValue.type == TOKEN_INT) {
        n.type = is_int;
        n.val.ival = atoi(tokenValue.value);
        set_register(registerIndex, n, operationType);
    } else if(tokenValue.type == TOKEN_FLOAT) {
        n.type = is_float;
        n.val.fval = atof(tokenValue.value);
        set_register(registerIndex, n, operationType);
    }
}

void execute(Token command[]) {


    if (strcmp(command[0].value, "init") == 0) {
        do_operation_on_register(get_register_index(command[1].value), command[2], 0);
    }
    else if (strcmp(command[0].value, "add") == 0) {
        do_operation_on_register(get_register_index(command[1].value), command[2], 1);
    }
    else if (strcmp(command[0].value, "sub") == 0) {
        do_operation_on_register(get_register_index(command[1].value), command[2], 2);
    }
    else if (strcmp(command[0].value, "mul") == 0) {
        do_operation_on_register(get_register_index(command[1].value), command[2], 3);
    }
    else if (strcmp(command[0].value, "div") == 0) {
        do_operation_on_register(get_register_index(command[1].value), command[2], 4);
    }

    else if (strcmp(command[0].value, "go?eq") == 0) {
        if (registers[get_register_index(command[1].value)].val.ival == 0)
            do_operation_on_register(6, command[2], 0);
    }
    else if (strcmp(command[0].value, "go?bi") == 0) {
        if (registers[get_register_index(command[1].value)].val.ival > 0)
            do_operation_on_register(6, command[2], 0);
    }
    else if (strcmp(command[0].value, "go?le") == 0) {
        if (registers[get_register_index(command[1].value)].val.ival < 0)
            do_operation_on_register(6, command[2], 0);
    }

    else if (strcmp(command[0].value, "print") == 0) {
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

    else { raiseError("unknown instruction",registers[6].val.ival); }
    
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
        if (!(strcmp(line, "\n") == 0 || strlen(line) == 0))
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


    set_pointers();
    while (registers[6].val.ival < codeLen) {
        parse(code[registers[6].val.ival], registers[6].val.ival);
        execute(code[registers[6].val.ival]);
        registers[6].val.ival++;
    }

    return 0;

}
