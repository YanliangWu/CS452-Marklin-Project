//#include <stdio>
#include "rps.h"




//-------------------------------Server-----------------------------
void rps_server_boot(){

    //assgin nameserver
    bwprintf(COM2, "SERVER SIGN NS\n\r");
    char server_name[3];
    server_name[0]='N';
    server_name[1]='S';
    server_name[2]='\000';
    RegisterAs(server_name);
     
    //initial server
    bwprintf(COM2, "SERVER INITIAL\n\r");
    RPS_server server;
    int message_size = sizeof(RPS_message);
    char buffer_receive[message_size];
    char buffer_reply[message_size];
    rps_server_initial(&server, buffer_receive, buffer_reply);

    //run server
    bwprintf(COM2, "SERVER LOOP\n\r");
    while(1){
        if(server.state == 5)
            break;
        
        bwgetc(COM2);
        
        RPS_message *message_receive;
        int tid=0;
        Receive(&tid, server.buffer_receive, message_size);
        message_receive = (RPS_message*) server.buffer_receive;
        
        switch (message_receive->type){
            case SIGNUP:{
                bwprintf(COM2, "SIGNUP\n\r");
                rps_server_signup(&server,message_receive, tid);
                break;
            }
            case PLAY:{
                bwprintf(COM2, "PLAY\n\r");
                rps_server_play(&server,message_receive, tid);
                break;
            }
            case QUIT:{
                bwprintf(COM2, "QUIT\n\r");
                rps_server_quit(&server,message_receive, tid);
                break;
            }
            default:
                break;
        }
        rps_server_startgame(&server);
        rps_server_checkgame(&server);
        rps_server_resultgame(&server);

    }
    //end server
    EndAs(server_name);
    Exit();
    bwprintf(COM2, "SERVER FINISH\n\r");
}

void rps_server_initial(RPS_server *server, char* buf_rcv, char* buf_rpy){
    server->server_tid = MyTid();
    server->player1_tid = 0;
    server->player2_tid = 0;
    server->player1_score = 0;
    server->player2_score = 0;
    //server->player1_dec = NO_CHOICE;
    //server->player2_dec = NO_CHOICE;
    server->buffer_receive = buf_rcv;
    server->buffer_reply = buf_rpy;
    server->state = 0;
    server->player_count = 0;
    queue_initial(&server->player);
    int i;
    for(i=1;i<50;i++)
        server->signed_player[i]=0;
}

void rps_server_signup(RPS_server *server, RPS_message *message, int tid){
    queue_push(&(server->player),tid);
    server->signed_player[tid] = 1;
    server->player_count++;
}

void rps_server_play(RPS_server *server, RPS_message *message, int tid){
    RPS_message *message_reply = (RPS_message *) server->buffer_reply;
    if(server->state == 1) {
        message_reply->type = PLAY_DONE;
        Reply(tid, server->buffer_reply, sizeof(RPS_message));
        if(server->player1_tid == tid){
            server->state = 2;
            server->player1_dec = message->decision;
        }else{
            server->state = 3;
            server->player2_dec = message->decision;
        }
    }else if(server->state == 2){
        if(server->player1_tid == tid){
            message_reply->type = PLAY_WAIT;
            Reply(tid, server->buffer_reply, sizeof(RPS_message));  
        }else{
            message_reply->type = PLAY_DONE;
            server->player2_dec = message->decision;
            Reply(server->player2_tid, server->buffer_reply, sizeof(RPS_message));
            server->state = 4;  
        }
    }else if(server->state == 3){
        if(server->player2_tid == tid){
            message_reply->type = PLAY_WAIT;
            Reply(tid, server->buffer_reply, sizeof(RPS_message));  
        }else{
            message_reply->type = PLAY_DONE;
            server->player1_dec = message->decision;
            Reply(server->player1_tid, server->buffer_reply, sizeof(RPS_message));
            server->state = 4;
        }
    }else if(server->state == 4){
        message_reply->type = PLAY_WAIT;
        Reply(tid, server->buffer_reply, sizeof(RPS_message));  
    }else if(server->state == 5){
        message_reply->type = QUIT;
        Reply(tid, server->buffer_reply, sizeof(RPS_message));  
        server->state = 0;
    }
    //bwprintf(COM2, "rps_server_play() end\n\r");
}

void rps_server_quit(RPS_server *server, RPS_message *message, int tid){
    //bwprintf(COM2, "rps_server_quit() start\n\r");
    server->signed_player[server->player1_tid] = 0;
    server->signed_player[server->player2_tid] = 0;
    server->state = 5;
    server->player1_tid = 0;
    server->player2_tid = 0;
    server->player_count -= 2;

    RPS_message *message_reply = (RPS_message *) server->buffer_reply;
    message_reply->type = QUIT_DONE;
    Reply(tid, server->buffer_reply, sizeof(RPS_message));  
    //bwprintf(COM2, "rps_server_quit() end\n\r");
}

void rps_server_startgame(RPS_server *server){
    if(server->state==0&&server->player_count==2){
        int tid1 = queue_pop(&(server->player));
        int tid2 = queue_pop(&(server->player));
        server->player1_tid = tid1;
        server->player2_tid = tid2;
        server->state = 1;
        RPS_message *message_reply = (RPS_message *) server->buffer_reply;
        message_reply->type = SIGNUP_DONE;
        Reply(tid1, server->buffer_reply, sizeof(RPS_message)); 
        Reply(tid2, server->buffer_reply, sizeof(RPS_message));
    }
}

void rps_server_checkgame(RPS_server *server){
    if(server->state==4){
        int res = rps_who_won(server->player1_dec,server->player2_dec);
        if (res == 1){
            server->player1_score++;
        }else if(res ==2 ){
            server->player2_score++;
        }
        server->state = 1;
        rps_print(server->player1_dec,server->player2_dec);
        bwprintf(COM2, "     %d    SCORES    %d\n\r",server->player1_score, server->player2_score);
    }
}

void rps_server_resultgame(RPS_server *server){
    if(server->state==5){
        bwprintf(COM2, "GAME OVER\n\r");
        bwprintf(COM2, "PLAYER1: %d  VS  PLAYER2: %d\n\r",server->player1_score, server->player2_score);
        if(server->player1_score > server->player2_score)
            bwprintf(COM2, "PLAYER1 WON\n\r");
        else if(server->player1_score < server->player2_score)
            bwprintf(COM2, "PLAYER2 WON\n\r");
        else
            bwprintf(COM2, "DRAW\n\r");
    }
}

//-------------------------------Client------------------------------
void rps_client_boot(){
    //bwprintf(COM2, "CLIENT INITIAL\n\r");
    RPS_client client;
    int message_size = sizeof(RPS_message);
    char buffer_reply[message_size];
    char buffer_send[message_size];
    rps_client_initial(&client, buffer_send, buffer_reply);

    //sign game
    //bwprintf(COM2, "CLIENT SIGN GAME\n\r");
    RPS_message *message_send;
    RPS_message *message_reply;
    message_send = (RPS_message *) client.buffer_send;
    message_send->type = SIGNUP;
    Send(client.server_tid,client.buffer_send,message_size,client.buffer_reply,message_size);
    message_reply = (RPS_message *) client.buffer_reply;

    //run server
    //bwprintf(COM2, "CLIENT LOOP\n\r");
    while(1){

        if(client.round<5){
            message_send->type = PLAY;
            message_send->decision = rps_decision(rps_random_gen(3));
            Send(client.server_tid,client.buffer_send,message_size,client.buffer_reply,message_size);
            message_reply = (RPS_message *) client.buffer_reply;
            client.round++;
        }else{
            message_send->type = QUIT;
            Send(client.server_tid,client.buffer_send,message_size,client.buffer_reply,message_size);
            message_reply = (RPS_message *) client.buffer_reply;
        }
        if(message_reply->type == PLAY_QUIT)
            break;
    }

    bwprintf(COM2, "CLIENT FINISH\n\r");
}


void rps_client_initial(RPS_client *client,char* buf_send, char* buf_reply){
    char server_name[3];
    server_name[0]='N';
    server_name[1]='S';
    server_name[2]='\000';
    client->server_tid = WhoIs(server_name);
    client->tid = MyTid();
    client->buffer_send = buf_send;
    client->buffer_reply = buf_reply;
    client->state = 0;
    client->round = 1;
}






//----------------------------------common--------------------------------
int rps_random_gen(int mod){
    int * timer_val = (int *)(TIMER3_BASE + VAL_OFFSET);
    int seed = *timer_val;
    return seed%mod;
}

RPS_decision rps_decision(int num) {
    switch(num){
        case 0:
            return ROCK;
        case 1:
            return PAPER;
        case 2:
            return SCISSORS;
        default:
            return -1;
    }
}

void rps_print(RPS_decision player_1_choice, RPS_decision player_2_choice) {
        bwprintf(COM2," PLAYER1    VS    PLAYER2\n\r");
    if (player_1_choice == ROCK)
        bwprintf(COM2,"   ROCK          ");
    if (player_1_choice == PAPER)
        bwprintf(COM2,"  PAPER          ");
    if (player_1_choice == SCISSORS)
        bwprintf(COM2," SCISSORS        ");
    if (player_2_choice == ROCK)
        bwprintf(COM2,"  ROCK    ");
    if (player_2_choice == PAPER)
        bwprintf(COM2," PAPER    ");
    if (player_2_choice == SCISSORS)
        bwprintf(COM2,"SCISSORS  ");
    bwprintf(COM2,"\n\r");
}

int rps_who_won(RPS_decision player_1_choice, RPS_decision player_2_choice) {
    if (player_1_choice == ROCK) {
        switch(player_2_choice) {
            case ROCK:
                return 0;
            case PAPER:
                return 2;
            case SCISSORS:
                return 1;
        }
    } else if (player_1_choice == PAPER) {
        switch(player_2_choice) {
            case ROCK:
                return 1;
            case PAPER:
                return 0;
            case SCISSORS:
                return 2;
        }
    } else if (player_1_choice == SCISSORS) {
        switch(player_2_choice) {
            case ROCK:
                return 2;
            case PAPER:
                return 1;
            case SCISSORS:
                return 0;
        }
    } else {}
    return -1;
}


