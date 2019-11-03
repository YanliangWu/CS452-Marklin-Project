#include "syscall.h"
#include "bwio.h"
#include "ts7200.h"
#include "nameserver.h"
#include "priority_queue.h"

typedef enum RPS_decision {
    ROCK = 0,
    PAPER = 1,
    SCISSORS = 2,
} RPS_decision;

typedef enum RPS_type {
    SIGNUP = 2,
    PLAY = 3,
    QUIT = 4,
    SIGNUP_DONE = 5,
    QUIT_DONE = 6,
    PLAY_WAIT = 7,
    PLAY_QUIT = 8,
    PLAY_DONE = 9,
} RPS_type;

typedef enum RPS_result {
    WIN = 0,
    LOSE = 1,
    TIE = 2,
} RPS_result;

typedef struct PRS_message{
    RPS_type type;
    RPS_decision decision;
    RPS_result result;
} RPS_message;

//--------------------------------server-----------------------------
typedef struct RPS_server{
    int server_tid;
    int player1_tid;
    int player2_tid;
    int player1_score;
    int player2_score;
    RPS_decision player1_dec;
    RPS_decision player2_dec;
    char *buffer_receive;
    char *buffer_reply;
    int signed_player[50];
    queue player;
    int player_count;
    int state;
} RPS_server;

void rps_server_boot();
void rps_server_initial(RPS_server *server, char*buf_rcv, char*buf_rpy);
void rps_server_signup(RPS_server *server, RPS_message *message, int tid);
void rps_server_play(RPS_server *server, RPS_message *message, int tid);
void rps_server_quit(RPS_server *server, RPS_message *message, int tid);
void rps_server_startgame(RPS_server *server);
void rps_server_checkgame(RPS_server *server);
void rps_server_resultgame(RPS_server *server);
//--------------------------------client------------------------------
typedef struct RPS_client{
    int tid;
    char *buffer_send;
    char *buffer_reply;
    int server_tid;
    int round;
    int state;
} RPS_client;

void rps_client_boot();
void rps_client_initial(RPS_client *client, char* buf_sent, char* buf_receive);

//---------------------------------common-----------------------------
int rps_random_gen(int mod);
RPS_decision rps_decision(int num);
int rps_who_won(RPS_decision player_1_choice, RPS_decision player_2_choice);
void rps_print(RPS_decision player_1_choice, RPS_decision player_2_choice);
