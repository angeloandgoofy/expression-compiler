#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26
#define UNARY_OP 8
#define POW_OP 9
#define FLOAT_LIT 12
#define MOD_OP 2

#define MAX_INSTRUCTIONS 100

char *testInput = "(3 + 4) * 2 - 5"; 
int inputIndex = 0;

int charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
int nextToken;
FILE *in_fp;


typedef enum {
    PUSH,
    ADD,
    SUB,
    MULT,
    DIV,
    POW,
    UNARY,
    PUSH_VAR,
    ASSIGN,
    PRINT
} OpCode;

typedef struct {
    OpCode op;   
    double operand; 
    char* identifier;
} Instruction;

Instruction code[MAX_INSTRUCTIONS];

int instructionCount = 0;
void addChar();
void getChar();
void getNonBlank();
int lex();

typedef enum {
    ID,
    OPERATOR,
    INTEGER
} NODETYPE;

typedef struct TNODE {
    NODETYPE type;
    union {
        int intvalue;
        char operator;
        char* identifer;
    } value;

    struct TNODE* left;
    struct TNODE* right;
} NODE;


NODE* expr();
NODE* term();
NODE* factor();
NODE* Exp();

NODE* createNode(NODETYPE type) {
    NODE* newNode = (NODE*)malloc(sizeof(NODE));

    if(newNode == NULL) {
        fprintf(stderr, "Memory allocation failed");
        exit(1);
    }

    newNode->type = type;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;

}

NODE* createInteger(int value) {
    NODE* node = createNode(INTEGER);
    
    node->value.intvalue = value;
    return node;
}

NODE* createOperator(char operator){
    NODE* node = createNode(OPERATOR);
    node->value.operator = operator;
    return node;
}

NODE* createId(char* identifier) {
    NODE* node = createNode(ID);
    node->value.identifer = identifier;
    return node;
}

void genInst(OpCode op, double operand, char* identifier) {
    if (instructionCount < MAX_INSTRUCTIONS) {
        code[instructionCount].op = op;
        code[instructionCount].operand = operand;
        code[instructionCount].identifier = identifier;
        instructionCount++;
    } else {
        printf("Error: Instruction limit exceeded.\n");
        exit(EXIT_FAILURE);
    }
}

void printIntermediateCode() {
    printf("Intermediate Code:\n");
    for (int i = 0; i < instructionCount; i++) {
        switch (code[i].op) {
            case PUSH:
                printf("PUSH %g\n", code[i].operand);
                break;
            case PUSH_VAR:
                printf("PUSH %s\n", code[i].identifier);
                break;
            case ADD:
                printf("ADD\n");
                break;
            case SUB:
                printf("SUB\n");
                break;
            case MULT:
                printf("MULT\n");
                break;
            case DIV:
                printf("DIV\n");
                break;
            case POW:
                printf("POW\n");
                break;
            case UNARY:
                printf("UNARY\n");
                break;
            case PRINT:
                printf("PRINT\n");
                break;
        }
    }
}

static void error();

NODE* expr() 
{
    printf("Enter <expr>\n");

    NODE* node = term();

    while (nextToken == ADD_OP || nextToken == SUB_OP) {
        NODE* newnode = createOperator(lexeme[0]);
        newnode->left = node;
        lex();
        newnode->right = term();
        node = newnode;
    }

    printf("Exit <expr>\n");
    return node;
} 

NODE* term() 
{
    printf("Enter <term>\n");
    NODE* node = factor();

    while (nextToken == MULT_OP || nextToken == DIV_OP || nextToken == MOD_OP) {
            NODE * newnode = createOperator(lexeme[0]);
            newnode->left = node;
            lex();
            newnode->right = factor();
            node = newnode;
    }
    printf("Exit <term>\n");
    return node;
} 
NODE* factor() 
{
    printf("Enter <factor>\n");
    NODE* node = Exp();
    while(nextToken == POW_OP){
        NODE* newNode = createOperator(lexeme[0]);
        newNode->left = node;
        lex();
        newNode->right = Exp();
        node = newNode;
    }
    printf("Exit <factor>\n");
    return node;
} 

NODE* Exp() {
    printf("Enter <exp>\n");
    NODE* node = NULL;

    if (nextToken == INT_LIT || nextToken == FLOAT_LIT) {
        node = createInteger(atof(lexeme)); 
        lex();
    } else if (nextToken == IDENT) {
        node = createId(strdup(lexeme)); 
        lex();
    } else if (nextToken == UNARY_OP) {
        lex();
        NODE* unaryNode = createOperator('-'); 
        unaryNode->right = Exp();
        node = unaryNode;
    } else if (nextToken == LEFT_PAREN) {
        lex();
        node = expr(); 
        if (nextToken == RIGHT_PAREN) {
            lex();
        } else {
            error("Expected closing parenthesis");
        }
    } else {
        error("Invalid expression");
    }

    printf("Exit <exp>\n");
    return node;
}

static void error() 
{
    printf("Error (more is desired, but not implemented).\n");
}

int lookup(char ch) {
    switch (ch) {
        case '(':
            addChar();
            nextToken = LEFT_PAREN;
            break;
        case ')':
            addChar();
            nextToken = RIGHT_PAREN;   
            break;
        case '+':
            addChar();
            nextToken = ADD_OP;
            break;
        case '-':
            addChar();
            nextToken = SUB_OP;
            break;
        case '*':
            addChar();
            nextToken = MULT_OP;
            break;
        case '/':
            addChar();
            nextToken = DIV_OP;
            break;
        case '~':
            addChar();
            nextToken = UNARY_OP;
            break;
        case '^':
            addChar();
            nextToken = POW_OP;
            break;
        case '%':
            addChar();
            nextToken = MOD_OP;
        default:
            addChar();
            nextToken = EOF;
            break;
    }
    return nextToken;
}

void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    } else {
        printf("Error - lexeme is too long \n");
    }
}


void getChar() {
    if (testInput[inputIndex] != '\0') {
        nextChar = testInput[inputIndex++];
        if (isalpha(nextChar)) {
            charClass = LETTER;
        } else if (isdigit(nextChar) || nextChar == '.') {
            charClass = DIGIT;
        } else {
            charClass = UNKNOWN;
        }
    } else {
        charClass = EOF;
    }
}


void getNonBlank() {
    while (isspace(nextChar)) getChar();
}


int lex() {
    lexLen = 0;
    getNonBlank();

    switch (charClass) {
        case LETTER:
            addChar();
            getChar();
            
            while (charClass == LETTER || charClass == DIGIT) {
                addChar();
                getChar();
            }

            nextToken = IDENT;
            break;

        case DIGIT:
            if(nextChar == '.'){
                addChar();
                getChar();
                while(charClass == DIGIT){
                    addChar();
                    getChar();
                }
                nextToken = FLOAT_LIT;
            } else {
                addChar();
                getChar();
                
                while (charClass == DIGIT) {
                    addChar();
                    getChar();
                }
                nextToken = INT_LIT;
            }
            //genInst(PUSH, atof(lexeme));
            break;

        case UNKNOWN:
            switch (nextChar)
            {
            case '+':
                    lookup(nextChar);
                    //genInst(ADD, 0); 
                    getChar();
                    break;
                case '-':
                    lookup(nextChar);
                    //genInst(SUB, 0); 
                    getChar();
                    break;
                case '*':
                    lookup(nextChar);
                    //genInst(MULT, 0); 
                    getChar();
                    break;
                case '/':
                    lookup(nextChar);
                    //genInst(DIV, 0); 
                    getChar();
                    break;
                case '^':
                    lookup(nextChar);
                    //genInst(POW, 0); 
                    getChar();
                    break;
                case '~': 
                    lookup(nextChar);
                    //genInst(UNARY, 0);
                    getChar();
                    break;
                case '(':
                case ')':
                    lookup(nextChar); // Handle parentheses
                    getChar();
                    break;
                default:
                    lookup(nextChar); // Handle any unknown character
                    getChar();
                    break;
            }
            break;

        case EOF:
            nextToken = EOF;
            lexeme[0] = 'E';
            lexeme[1] = 'O';
            lexeme[2] = 'F';
            lexeme[3] = 0;
            break;
    } 

    printf("Next token is: %d, Next lexeme is %s\n", nextToken, lexeme);
    return nextToken;
}       
    

void postorder(NODE* node) {
    if (node == NULL) {
        return;
    }

    postorder(node->left);
    postorder(node->right);

    switch (node->type) {
        case INTEGER:
            genInst(PUSH, node->value.intvalue, NULL);
            break;

        case ID:
            genInst(PUSH_VAR, 0, node->value.identifer); 
            break;
        
        case OPERATOR:
            switch (node->value.operator) {
                case '+':
                    genInst(ADD, 0, NULL);
                    break;
                case '-':
                    genInst(SUB, 0, NULL);
                    break;
                case '*':
                    genInst(MULT, 0, NULL);
                    break;
                case '/':
                    genInst(DIV, 0, NULL);
                    break;
                case '^':
                    genInst(POW, 0, NULL);
                    break;
                case '~':
                    genInst(UNARY, 0, NULL);
                    break;
                default:
                    printf("Unknown operator: %c\n", node->value.operator);
                    break;
            }
            break;

        default:
            printf("Unknown node type in postorder traversal\n");
            break;
    }
}

void printTREE (NODE* p) {
    if (p == NULL) {
        return;
    }

    if (p->type == INTEGER) {
        printf(" %d ", p->value.intvalue);
    } else if (p->type == ID) {
        printf(" %s ", p->value.identifer);
    } else if (p->type == OPERATOR) {
      
        printf("(");
        
        printTREE(p->left);

        printf(" %c ", p->value.operator);

        printTREE(p->right);

        printf(")");
    } else {
        printf("Unknown node type\n");
    }
}

void ProjectB() {
    NODE *root;

    inputIndex = 0;
    getChar();
    lex();
    root = expr();

    postorder(root);
    printIntermediateCode();
    printTREE(root);
}

int main() {
    ProjectB();
}