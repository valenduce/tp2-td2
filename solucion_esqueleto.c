// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) {
    int i=0;
    while (src[i]!=0){
      i++;
    }
    char* res= (char*) malloc(sizeof(char)*(i+1));
    int j=0;
    while (j<i){
      res[j]= src[j];
      j++;
    }
    res[j]= 0;
    return res;
}

int strCompare(char* s1, char* s2) {
    int res= 0;
    int i= 0;
    while (s1[i]==s2[i] && (s1[i]!=0) && (s2[i]!=0)){
      i++;
    }
    if (s1[i]>s2[i]){
      res= -1;
    }
    if (s1[i]<s2[i]){
      res= 1;
    }
    return res;
}

char* strConcatenate(char* src1, char* src2) {
    int len1= 0;
    int len2= 0;
    while (src1[len1]!=0){
      len1++;
    }
    while (src2[len2]!=0){
      len2++;
    }
    char* res= (char*) malloc(sizeof(char)*(len1+len2+1));
    int i=0;
    while (i<len1){
      res[i]= src1[i];
      i++;
    }
    int j=0;
    while (j<len2){
      res[i]= src2[j];
      i++;
      j++;
    }
    res[i]= 0;
    free(src1);
    free(src2);
    return res;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

// gameBoardNew: malloc del tablero y de 20 filas enlazadas con prev/current. Si falla un malloc, libera todo y devuelve NULL.
GameBoard* gameBoardNew() {
    GameBoard* tablero= (GameBoard*) malloc(sizeof(GameBoard));
    if (tablero == NULL){
      return NULL;
    }
    tablero-> queue_head= NULL;
    tablero-> queue_tail= NULL;
    tablero-> first_particle= NULL;
    tablero-> first_row= NULL;
    tablero-> current_piece.col=0;
    tablero-> current_piece.row=0;
    tablero-> current_piece.type=0;
    tablero-> current_piece.rotation=0;
    tablero-> current_piece.active=0;
    tablero-> score= 0;
    tablero-> tick_timer= 0;
    int i=0;
    RowNode* prev = NULL;
    while (i<GRID_ROWS){
      RowNode* current= (RowNode*) malloc(sizeof(RowNode));
      if (current == NULL){
        RowNode* fila= tablero->first_row;
        while(fila!=NULL){
          RowNode* siguiente = fila->next;
          free(fila);
          fila = siguiente;
        }
        free(tablero);
        return NULL;
      }
      for (int j=0; j<GRID_COLS; j++){
        current->cells[j]= 0;
      }
      current->next= NULL;
      if (prev== NULL){ 
        tablero->first_row= current;
      } else {
        prev->next= current;
      }
      prev= current;
      i++;
    }
    return tablero;
}

// gameBoardDelete: libera las 3 listas guardando el next antes de cada free, y al final el tablero.
void gameBoardDelete(GameBoard* board) {
    if (board== NULL){
      return;
    }
    RowNode* current= board->first_row;
    while (current!= NULL){
      RowNode* siguiente= current->next;
      free(current);
      current= siguiente;
    }
    PieceQueueNode* currentQueue= board-> queue_head;
    while (currentQueue!= NULL){
      PieceQueueNode* siguienteQueue = currentQueue->next;
      free(currentQueue);
      currentQueue= siguienteQueue;
    }
    ParticleNode* currentParticle= board->first_particle;
    while (currentParticle!= NULL){
      ParticleNode* siguienteParticle= currentParticle->next;
      free(currentParticle);
      currentParticle= siguienteParticle;
    }
    free(board);
}

// gameBoardAddParticle: malloc de una particula y se inserta al principio de la lista (sin limite fijo).
void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    ParticleNode* newNode= (ParticleNode*) malloc(sizeof(ParticleNode));
    newNode->x= x;
    newNode->y= y;
    newNode->color= color;
    newNode->vel_x= ((rand()%100)/10.0f)- 5.0f;
    newNode->vel_x= ((rand()%50)/-10.0f)- 2.0f;
    newNode->life_timer= 30 + (rand()%30);
    newNode->next= board->first_particle;
    board->first_particle= newNode;
}

// gameBoardCheckAndClearLines: desenlaza y libera las filas llenas sin copiar celdas; despues agrega arriba una fila vacia por cada una.
int gameBoardCheckAndClearLines(GameBoard* board) {
    int contador= 0;
    int r= 0;
    RowNode* prev = NULL;
    RowNode* current = board->first_row;
    while (current != NULL){
      int completa= 1;
      for (int j=0; j<GRID_COLS; j++){
        if (current->cells[j] == 0){
          completa= 0;
        }
      }
      if (completa== 1){
        contador++;
        for (int j=0; j<GRID_COLS; j++){
            gameBoardAddParticle(board, OFFSET_X +j*CELL_SIZE + CELL_SIZE/2, OFFSET_Y + r*CELL_SIZE + CELL_SIZE/2, current->cells[j]);
         }
        if (prev== NULL){
            board->first_row= current->next;
            free(current);
            current= board->first_row;
        } else {
            prev->next= current->next;
            free(current);
            current= prev->next;
        }
      } else {
        prev = current;
        current= current->next;
        r++;
      }
    }
    
    int i=0;
    while (i<contador){
      RowNode* nuevo= (RowNode*) malloc(sizeof(RowNode));
      if (nuevo== NULL){
        return contador;
      }
      for (int j=0; j<GRID_COLS; j++){
        nuevo->cells[j]= 0;
      }
      nuevo->next= board->first_row;
      board->first_row= nuevo;
      i++;
    }
    return contador;
}

// gameBoardEnqueuePiece: agrega el nodo nuevo al final usando queue_tail, sin recorrer la cola.
void gameBoardEnqueuePiece(GameBoard* board, int type) {
    PieceQueueNode* nuevo= (PieceQueueNode*) malloc(sizeof(PieceQueueNode));
    if (nuevo== NULL){
      return;
    }
    nuevo->type= type;
    nuevo->next= NULL;
    if (board->queue_tail== NULL){
      board->queue_head= nuevo;
      board->queue_tail= nuevo;
    } else {
      board->queue_tail->next= nuevo;
      board->queue_tail= nuevo;
    }
}

// gameBoardDequeuePiece: saca el primero (queue_head). Si la cola queda vacia, queue_tail tambien va a NULL.
int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    if (board->queue_tail== NULL){
      return 0;
    }
    PieceQueueNode* primera= board->queue_head;
    *out_type= primera->type;
    board->queue_head= primera->next;
    if (board->queue_head== NULL){
      board->queue_tail= NULL;
    }
    free(primera);
    return 1;
}

// ========= LÓGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========
void chequear(int condicion) {
    if (condicion) {
        printf("bien\n");
    } else {
        printf("MAL\n");
    }
}

void correrTests() {
    char* a = strDuplicate("");
    char* b = strDuplicate("x");
    char* c = strDuplicate("Hola");
    chequear(strCompare(a, "") == 0);
    chequear(strCompare(b, "x") == 0);
    chequear(strCompare(c, "Hola") == 0);
    free(a);
    free(b);
    free(c);

    chequear(strCompare("", "") == 0);
    chequear(strCompare("a", "a") == 0);
    chequear(strCompare("a", "b") == 1);
    chequear(strCompare("b", "a") == -1);

    char* d = strConcatenate(strDuplicate(""), strDuplicate("abc"));
    char* e = strConcatenate(strDuplicate("hola "), strDuplicate("mundo"));
    chequear(strCompare(d, "abc") == 0);
    chequear(strCompare(e, "hola mundo") == 0);
    free(d);
    free(e);

    GameBoard* t = gameBoardNew();
    int tipo = 0;
    for (int i = 0; i < 5; i++) {
        gameBoardEnqueuePiece(t, i);
    }
    for (int i = 0; i < 5; i++) {
        gameBoardDequeuePiece(t, &tipo);
        chequear(tipo == i);
    }
    chequear(gameBoardDequeuePiece(t, &tipo) == 0);
    chequear(t->queue_head == NULL);
    chequear(t->queue_tail == NULL);
    gameBoardEnqueuePiece(t, 6);
    gameBoardDequeuePiece(t, &tipo);
    chequear(tipo == 6);
    gameBoardDelete(t);

    t = gameBoardNew();
    RowNode* vieja = t->first_row;
    for (int j = 0; j < GRID_COLS; j++) {
        getRowAt(t, 19)->cells[j] = 1;
    }
    chequear(gameBoardCheckAndClearLines(t) == 1);
    chequear(t->first_row->next == vieja);
    chequear(getRowAt(t, 19)->next == NULL);
    gameBoardDelete(t);

    t = gameBoardNew();
    for (int j = 0; j < GRID_COLS; j++) {
        getRowAt(t, 0)->cells[j] = 1;
        getRowAt(t, 1)->cells[j] = 1;
    }
    chequear(gameBoardCheckAndClearLines(t) == 2);
    chequear(getRowAt(t, 19)->next == NULL);
    gameBoardDelete(t);
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
