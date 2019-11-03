#include "trainserver.h"
#include "clockserver.h"
#include "uartserver.h"
#include "syscall.h"
#define TRAIN_MAIN_LOOP 0
#define UART1_IN_DELAY 4
#define UART1_OUT_DELAY 1


//-------------------------------server------------------------------
void train_server_initial(train_server * server, int server_tid){
    server->server_tid = server_tid;
    server->mes_send = (TR_message*) server->buf_send;
    server->mes_rcv = (TR_message*) server->buf_rcv;
    server->mes_rpy = (TR_message*) server->buf_rpy;
    server->mes_size = sizeof(TR_message);
    server->tick = 0;
    server->sensor_index = 0;
    server->sensor_change = 0;
    server->sensor_print = 0;
    server->pre_sensor_num = 0;
    server->pre_sensor_char = 'A';

    int i=0;
    //notifier
    for(i=0;i<MAX_DELAY_NOTIFIER;i++){server->delay_notifier[i] = 0;}
    //train (train 0 is track control)
    for(i=0;i<MAX_TRAIN_NUMBER;i++){
        server->train[i].train_speed = 0;
        server->train[i].train_num = 0;
        server->train[i].train_fetch = 0;
        server->train[i].task_start = 0;
        server->train[i].task_end = 0;
        server->train[i].task_count = 0;
        server->train[i].train_pos_num = 0;
        server->train[i].train_pos_char = ' ';
        server->train[i].train_pos_dis = 0;
        server->train[i].train_pos_tick = 0;
        server->train[i].train_next_pos_num = 0;
        server->train[i].train_next_pos_char = ' ';
        server->train[i].train_dest_num = 0;
        server->train[i].train_dest_char = ' ';
        server->train[i].train_dest_dis = 0;
        server->train[i].train_last_distance = 0;
        server->train[i].train_last_velocity = 0;
        server->train[i].train_last_sentick = 0;
        server->train[i].train_safe_dis = 0;
        server->train[i].train_state = TRAIN_NOTHING;
        server->train[i].speed_state = STOPPED;
        server->train[i].train_delay = 0;
        server->train[i].train_cons = 300;
        server->train[i].train_acc = 10;
        server->train[i].train_dec = 18;

        int j;
        for(j=0;j<MAX_TRACK_NUMBER;j++) {
            server->train[i].train_velocity[j] = 0;
        }
        //set_speed1(&server->train[i]);
    }
}

void train_server_boot(){
    //create server
    char train_server_name[] = "TR\000";
    char uart_server_name[] = "US\000";
    RegisterAs(train_server_name);
    uart_server ua_server;
    train_server server;
    int track_id = getTrackID();
    uart_server_initial(&ua_server, WhoIs(uart_server_name));
    train_server_initial(&server, MyTid());
    //debug
    ua_server.debug[0].row = 5;
    ua_server.debug[0].col = 49;
    ua_server.debug[0].index = 0;
    ua_server.debug[0].size = 23;
    ua_server.debug[1].row = 5;
    ua_server.debug[1].col = 76;
    ua_server.debug[1].index = 0;
    ua_server.debug[1].size = 23;
    //create notifier
    Create(7, &train_notifier1_boot);//11
    Create(7, &train_notifier2_boot);//12
    //Create(4, &train_notifier4_boot);//12
    int i;
    for(i=0;i<MAX_DELAY_NOTIFIER;i++)
        Create(7, &train_notifier3_boot);//13-23
    //create track
    train_track track;
    server.track = &track;

    if(track_id == 'A' || track_id == 'a') {
        track_inita(track.track_path);
        for(i = 0; i < MAX_TRACK_NUMBER; i++){
            track.track_path[i].dead = 0;
        }
        for(i = 1;i < MAX_TRAIN_NUMBER;i++){
            set_speed_a(&(server.train[i]));
        }
    }
    else {
        track_initb(track.track_path);
        for(i = 0; i < MAX_TRACK_NUMBER; i++){
            track.track_path[i].dead = 0;
        }
        track.track_path[40].dead = 1;
        for(i = 1;i < MAX_TRAIN_NUMBER;i++){
            set_speed_b(&(server.train[i]));
        }
    }


    for(i=0;i<157;i++)
        server.track->track_switch[i]=34;
    server.track->track_switch[switch_to_track(10)]=33;
    server.track->track_switch[switch_to_track(13)]=33;
    server.track->track_switch[switch_to_track(16)]=33;
    server.track->track_switch[switch_to_track(17)]=33;
    //interupt init
    hwi_initial();
    //sensor init
    uart_server_put(&ua_server, 133, UART_PUT_UART1);
    while (1) {
        if(TRAIN_MAIN_LOOP) debug(&ua_server,1,"DEBUG (MAIN LOOP) FLAG1");
        Receive(&server.client_tid, server.buf_rcv, server.mes_size);

        if(TRAIN_MAIN_LOOP) debug(&ua_server,1,"DEBUG (MAIN LOOP) FLAG2");
        if(server.mes_rcv->type != TR_READY)
            Reply(server.client_tid, server.buf_rcv, server.mes_size);

        if(TRAIN_MAIN_LOOP) debug(&ua_server,1,"DEBUG (MAIN LOOP) FLAG3");
        train_state(&ua_server, &server);

        if(TRAIN_MAIN_LOOP) debug(&ua_server,1,"DEBUG (MAIN LOOP) FLAG4");
        train_block(&ua_server, &server);

        if(TRAIN_MAIN_LOOP) debug(&ua_server,1,"DEBUG (MAIN LOOP) FLAG5");
        train_schedule(&ua_server, &server);

        if(TRAIN_MAIN_LOOP) debug(&ua_server,1,"DEBUG (MAIN LOOP) FLAG6");
        train_execute(&ua_server, &server);
    }
}


//---------------------------------------------------state-----------------------------------------------
void train_state( uart_server *ua_server, train_server *tr_server){
    TR_type type = tr_server->mes_rcv->type;
    switch (type) {
        case TR_CLOCK:{

            int i;
            for (i = 1; i < MAX_TRAIN_NUMBER; i++) {
                train_train *tem_train = &tr_server->train[i];

                //set position
                //train_pos_dis
                tem_train->train_pos_dis += tem_train->train_last_velocity/10;
                //train_predict_pos_dis
                tem_train->train_predict_pos_dis += tem_train->train_last_velocity/10;
                if(tem_train->train_predict_pos_dis > tem_train->train_last_distance){
                    tem_train->train_predict_pos_char = tem_train->train_next_pos_char;
                    tem_train->train_predict_pos_num = tem_train->train_next_pos_num;
                    tem_train->train_predict_pos_dis=tem_train->train_predict_pos_dis%tem_train->train_last_distance;
                }


                tem_train->train_last_sentick = tr_server->tick;

                if(tem_train->train_num) {
                    switch (tem_train->speed_state) {
                        case ACCELERATION: {
                            tem_train->train_last_velocity += tem_train->train_acc;
                            if (tem_train->train_last_velocity >= tem_train->train_cons)
                                tem_train->speed_state = CONSTANT;
                            break;
                        }
                        case DECELERATION: {
                            tem_train->train_last_velocity -= tem_train->train_dec;
                            if (tem_train->train_last_velocity <= 0) {
                                tem_train->speed_state = STOPPED;
                                tem_train->train_last_velocity = 0;
                            }
                            break;
                        }
                        default:{
                            break;
                        }
                    }
                }
            }

            break;
        }
        case TR_SENSOR: {
            //tell which kind of sensor it is
            short n = tr_server->mes_rcv->sensor_num;
            char c = tr_server->mes_rcv->sensor_char;
            tr_server->sensor_change = 0;
            tr_server->sensor_print = 0;
            if ((!((n==0)&&(c=='A')))&&(!((n==tr_server->pre_sensor_num)&&(c==tr_server->pre_sensor_char)))) {
                //kerprintf(COM2,ua_server,"\033[48;30H %x %d       ",&(tr_server->pre_sensor_num),tr_server->pre_sensor_num);
                //debug(ua_server,1,"pre:%c %d|cur:%c %d",tr_server->pre_sensor_char,tr_server->pre_sensor_num,c,n);
                tr_server->pre_sensor_num = n;
                tr_server->pre_sensor_char = c;
                tr_server->sensor_print = 1;
                int i;
                for(i=1;i<MAX_TRAIN_NUMBER;i++){
                    train_train *tem_train = &tr_server->train[i];
                    if(tem_train->train_fetch){
                        if (tem_train->train_next_pos_char==c&&tem_train->train_next_pos_num==n) {
                            tr_server->sensor_change = i;
                            break;
                        }
                    }
                }
                if(!tr_server->sensor_change) {
                    for (i = 1; i < MAX_TRAIN_NUMBER; i++) {
                        train_train *tem_train = &tr_server->train[i];
                        if (!tem_train->train_fetch && tem_train->train_num) {
                            tem_train->train_fetch = 1;
                            tr_server->sensor_change = i;
                            break;
                        }

                    }
                }
            }
            //change the state of train
            if(tr_server->sensor_change){
                train_train *tem_train = &tr_server->train[tr_server->sensor_change];
                if (tem_train->speed_state==CONSTANT) {
                    tem_train->train_last_velocity = tem_train->train_last_distance * 10 / (tr_server->tick - tem_train->train_pos_tick);
                    set_velocity(tem_train);
                }

                tem_train->train_predict_pos_dis = tem_train->train_last_velocity*UART1_IN_DELAY/10;
                //tem_train->train_predict_pos_dis =  read_velocity(tem_train,\
                    sensor_to_track(tem_train->train_pos_num,tem_train->train_pos_num))*UART1_DELAY/10;
                tem_train->train_predict_pos_num = n;
                tem_train->train_predict_pos_char = c;

                tem_train->train_pos_dis = 0;
                tem_train->train_pos_num = n;
                tem_train->train_pos_char = c;
                tem_train->train_pos_tick = tr_server->tick;
                tem_train->train_safe_dis = tem_train->train_last_velocity*4;
                tem_train->train_last_sentick = tr_server->tick;
            }
            break;
        }
        case TR_ADD: {
            int i = 0;
            for (i = 1; i < MAX_TRAIN_NUMBER; i++) {
                if (!tr_server->train[i].train_num) {
                    short next_n;
                    char next_c;
                    tr_server->train[i].train_num = tr_server->mes_rcv->train;
                    tr_server->train[i].train_speed = 0;
                    tr_server->train[i].train_pos_char = tr_server->mes_rcv->sensor_char;
                    tr_server->train[i].train_pos_num = tr_server->mes_rcv->sensor_num;
                    tr_server->train[i].train_last_distance = track_getnext(tr_server->track->track_path, \
                        tr_server->track->track_switch, tr_server->train[i].train_pos_char , tr_server->train[i].train_pos_num , &next_c, &next_n);
                    tr_server->train[i].train_next_pos_num = next_n;
                    tr_server->train[i].train_next_pos_char = next_c;
                    tr_server->train[i].speed_state = STOPPED;
                    break;
                }
            }
            break;
        }
        default:{
            break;
        }
    }
}


//--------------------------------------------block------------------------------------------------
void train_block( uart_server *ua_server, train_server *tr_server){
    /*
    TR_type type = tr_server->mes_rcv->type;
    switch (type){
        case TR_SENSOR:{
            int i;
            for(i=1;i<MAX_TRAIN_NUMBER;i++) {

            }
            break;
        }
        default:{
            break;
        }
    }
     */
}


//-----------------------------------------------schedule-------------------------------------------
void train_schedule( uart_server *ua_server, train_server *tr_server){
    TR_type type = tr_server->mes_rcv->type;
    switch (type) {
        case TR_READY: {
            train_train *train = &tr_server->train[0];
            train->task[train->task_end].type = TASK_READY;
            task_put(train);
            break;
        }
        case TR_CLOCK: {
            tr_server->tick++;
            train_train *train = &tr_server->train[0];
            train->task[train->task_end].type = TASK_CLOCK;
            task_put(train);
            break;
        }
        case TR_TR: {
            train_train *train = &tr_server->train[0];
            train->task[train->task_end].type = TASK_TR;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].speed = tr_server->mes_rcv->speed;
            task_put(train);
            break;
        }
        case TR_SW: {
            train_train *train = &tr_server->train[0];
            train->task[train->task_end].type = TASK_SW;
            train->task[train->task_end].switcher = tr_server->mes_rcv->switcher;
            train->task[train->task_end].direction = tr_server->mes_rcv->direction;
            task_put(train);
            break;
        }
        case TR_SSTOP: {
            int i;
            for(i=1;i<MAX_TRAIN_NUMBER;i++) {
                if (tr_server->train[i].train_num == tr_server->mes_rcv->train) break; }

            //calculate delay time..
            train_train *train = &tr_server->train[i];
            int a = train->train_acc * 10;
            int d = train->train_dec * 10;
            int delay_time = train_sqrt((2*tr_server->mes_rcv->dist_offset) / (a + ((a * a) / d)) * 100);
            bwprintf(COM2, "\033[40;10H delay time is: %d", delay_time);
            train->task[train->task_end].type = TASK_TR;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].speed = 10;
            task_put(train);
            train->task[train->task_end].type = TASK_DELAY;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].delay = tr_server->mes_rcv->dist_offset;
            task_put(train);
            train->task[train->task_end].type = TASK_TR;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].speed = 0;
            task_put(train);
            break;
        }
        case TR_STOP: {
            int i;
            for(i=1;i<MAX_TRAIN_NUMBER;i++) {
                if (tr_server->train[i].train_num == tr_server->mes_rcv->train) break; }
            train_train *train = &tr_server->train[i];
            if(train->train_fetch){
                train->task[train->task_end].type = TASK_TR;
                train->task[train->task_end].train = tr_server->mes_rcv->train;
                train->task[train->task_end].speed = 10;
                task_put(train);

                train->train_dest_num = tr_server->mes_rcv->sensor_num;
                train->train_dest_char = tr_server->mes_rcv->sensor_char;
                train->train_dest_offset = tr_server->mes_rcv->dist_offset;
                train->train_state = TRAIN_STOP_TRY;

                train->train_dest_pre_track = sensor_to_pre_track(tr_server->track->track_path,\
                    tr_server->track->track_switch, train->train_dest_num,  train->train_dest_char);
                debug(ua_server,1,"STOP PRE_TRACK:%s SPEED:%d ",tr_server->track->track_path[train->train_dest_pre_track].name,\
                    train->train_velocity[train->train_dest_pre_track]);
            }
            break;
        }
        case TR_RV: {
            int i;
            for(i=1;i<MAX_TRAIN_NUMBER;i++) {
                if (tr_server->train[i].train_num == tr_server->mes_rcv->train) break; }
            train_train *train = &tr_server->train[i];
            train->task[train->task_end].type = TASK_TR;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].speed = 0;
            task_put(train);
            train->task[train->task_end].type = TASK_DELAY;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].delay = 20;
            task_put(train);
            train->task[train->task_end].type = TASK_TR;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].speed = 15;
            task_put(train);
            train->task[train->task_end].type = TASK_DELAY;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].delay = 5;
            task_put(train);
            train->task[train->task_end].type = TASK_TR;
            train->task[train->task_end].train = tr_server->mes_rcv->train;
            train->task[train->task_end].speed = 10;
            task_put(train);
            break;
        }
        case TR_SENSOR: {
            int n = tr_server->mes_rcv->sensor_num;
            char c =  tr_server->mes_rcv->sensor_char;
            train_train *train = &tr_server->train[0];
            train->task[train->task_end].type = TASK_SENSOR;
            train->task[train->task_end].sensor_num = n;
            train->task[train->task_end].sensor_char = c;
            task_put(train);

            track_node *track = tr_server->track->track_path;
            int train_path[150];
            int i;
            for(i=1;i<MAX_TRAIN_NUMBER;i++) {
                train = &tr_server->train[i];
                if(train->train_state == TRAIN_STOP_TRY){
                    //path detection
                    short train_pos = sensor_to_track(train->train_next_pos_num,train->train_next_pos_char);
                    short train_dest = sensor_to_track(train->train_dest_num,train->train_dest_char);
                    short total_dis = track_getpath(track, train_path, train_pos, train_dest) + train->train_dest_offset+\
                        train->train_dest_dis - train->train_pos_dis + train->train_last_distance -\
                            UART1_IN_DELAY*read_velocity(train,sensor_to_track(train->train_pos_num,train->train_pos_char))/10;
                    short k=0, detec_dis=0;
                    //debug(ua_server,1,"%d %s->%s->%s->%s->%s",detec_dis,\
                        tr_server->track->track_path[train_path[0]].name, \
                        tr_server->track->track_path[train_path[1]].name, \
                        tr_server->track->track_path[train_path[2]].name, \
                        tr_server->track->track_path[train_path[3]].name, \
                        tr_server->track->track_path[train_path[4]].name);
                    do {
                        track_node *cur_track = &track[train_path[k]];
                        track_node *next_track = &track[train_path[k+1]];

                        switch (cur_track->type) {
                            case NODE_SENSOR: {
                                detec_dis += cur_track->edge[DIR_AHEAD].dist;
                                break;
                            }
                            case NODE_BRANCH: {
                                short dir;
                                if(cur_track->edge[DIR_STRAIGHT].dest->type == next_track->type &&\
                                    cur_track->edge[DIR_STRAIGHT].dest->num == next_track->num)
                                    {dir = 33;detec_dis += cur_track->edge[DIR_STRAIGHT].dist;}
                                else if(cur_track->edge[DIR_CURVED].dest->type == next_track->type &&\
                                    cur_track->edge[DIR_CURVED].dest->num == next_track->num)
                                    {dir = 34;detec_dis += cur_track->edge[DIR_CURVED].dist;}
                                else{
                                    debug(ua_server,1,"FLAG3 %s s:%s c:%s %d",cur_track->name, cur_track->edge[DIR_STRAIGHT].dest->name,\
                                        cur_track->edge[DIR_CURVED].dest->name,next_track->num);
                                }


                                if(tr_server->track->track_switch[switch_to_track(cur_track->num)]!=dir) {
                                  //  debug(ua_server,0, "SWITCH: %s DIR:%d ", cur_track->name, dir);
                                    train->task[train->task_end].type = TASK_SW;
                                    train->task[train->task_end].switcher = cur_track->num;
                                    train->task[train->task_end].direction = dir;
                                    task_put(train);
                                }
                                break;
                            }
                            default:{
                                detec_dis += cur_track->edge[DIR_AHEAD].dist;
                                break;
                            }
                        }
                    } while(train_path[++k]!=train_dest && detec_dis<train->train_safe_dis);
                    //set stop
                    short speed = read_velocity(train, train->train_dest_pre_track);
                    short stop_dis = (speed*speed)/(train->train_dec*20);
                    //short stop_dis = (train->train_last_velocity*train->train_last_velocity)/(train->train_dec*20);
                    if(total_dis < stop_dis+600){
                        train->train_state = TRAIN_NOTHING;
                        train->train_dest_char = ' ';
                        train->train_dest_num = 0;

                        train->task[train->task_end].type = TASK_DELAY;
                        train->task[train->task_end].train = train->train_num;
                        train->task[train->task_end].delay = 10*(total_dis - stop_dis)/train->train_last_velocity+UART1_OUT_DELAY;
                        task_put(train);
                        train->task[train->task_end].type = TASK_TR;
                        train->task[train->task_end].train = train->train_num;
                        train->task[train->task_end].speed = 0;
                        debug(ua_server,1, "STOP: tot_dis:%d stop_dis:%d",total_dis,stop_dis);
                        task_put(train);
                    }
                }
            }
            if (tr_server->sensor_change) {
                train = &tr_server->train[tr_server->sensor_change];
                short next_n;
                char next_c;
                train->train_last_distance = track_getnext(tr_server->track->track_path, \
                    tr_server->track->track_switch, c, n, &next_c, &next_n);
                debug(ua_server,0,"NEXT SENSOR: %c%d -> %c%d  ",c,n,next_c,next_n);
                train->train_next_pos_num = next_n;
                train->train_next_pos_char = next_c;
            }
            break;
        }

        default:{
            break;
        }
    }
    print_schedule(ua_server, tr_server);
}

//-------------------------------------------execute---------------------------------------------------
void train_execute(uart_server *ua_server, train_server *tr_server){
    int k;
    for(k=MAX_TRAIN_NUMBER-1;k>=0;k--) {
        train_train *train = &tr_server->train[k];
        if(((tr_server->mes_rcv->type!=TR_DELAY)&&!train->train_delay)||\
        ((tr_server->mes_rcv->type==TR_DELAY)&&(tr_server->mes_rcv->train==train->train_num))) {
            train->train_delay = 0;
            while (train->task_count) {
                train_task *next_task = &(train->task[train->task_start]);
                switch (next_task->type) {
                    case TASK_SENSOR: {
                        print_sensor(ua_server, tr_server);
                        uart_server_put(ua_server, 133, UART_PUT_UART1);
                        task_get(train);
                        break;
                    }
                    case TASK_CLOCK: {
                        print_time(ua_server, tr_server->tick);
                        task_get(train);
                        break;
                    }
                    case TASK_READY: {
                        int i = 0;
                        for (i = 1; i < MAX_DELAY_NOTIFIER; i++) {
                            if (!tr_server->delay_notifier[i]) {
                                tr_server->delay_notifier[i] = tr_server->client_tid;
                                break;
                            }
                        }
                        task_get(train);
                        return;
                    }
                    case TASK_DELAY: {
                        train->train_delay = 1;
                        int i;
                        for (i = 0; i < MAX_TRAIN_NUMBER; i++) {
                            if (tr_server->delay_notifier[i]) {
                                tr_server->mes_rpy->delay = next_task->delay;
                                tr_server->mes_rpy->train = next_task->train;
                                Reply(tr_server->delay_notifier[i], tr_server->buf_rpy, tr_server->mes_size);
                                tr_server->delay_notifier[i] = 0;
                                break;
                            }
                        }
                        task_get(train);
                        debug(ua_server,1,"DELAY: train:%d time:%d",next_task->train, next_task->delay);
                        return;
                    }
                    case TASK_SW: {
                        uart_server_put(ua_server, next_task->direction, UART_PUT_UART1);
                        uart_server_put(ua_server, next_task->switcher, UART_PUT_UART1);
                        uart_server_put(ua_server, 32, UART_PUT_UART1);
                        task_get(train);

                        //feedback changing
                        tr_server->track->track_switch[switch_to_track(next_task->switcher)] = next_task->direction;
                        print_switch(ua_server, next_task->direction, next_task->switcher);
                        debug(ua_server,1,"SW: switch:%d dir:%d",next_task->switcher, next_task->direction);
                        break;
                    }
                    case TASK_TR: {
                        uart_server_put(ua_server, next_task->speed, UART_PUT_UART1);
                        uart_server_put(ua_server, next_task->train, UART_PUT_UART1);
                        task_get(train);
                        debug(ua_server,1,"TR: train:%d speed:%d",next_task->train, next_task->speed);

                        //feedback changing
                        int i = 0;
                        for (i = 1; i < MAX_TRAIN_NUMBER; i++) {
                            train_train *tem_train = &tr_server->train[i];
                            if (tem_train->train_num == next_task->train) {//fetch the train
                                if(next_task->speed==15) { //reverse case (tr xx 15)
                                    char cur_c;
                                    short cur_n;
                                    char next_c;
                                    short next_n;
                                    sensor_reverse(tr_server->track->track_path,tem_train->train_predict_pos_char,tem_train->train_predict_pos_num,&next_c,&next_n);
                                    sensor_reverse(tr_server->track->track_path,tem_train->train_next_pos_char,tem_train->train_next_pos_num,&cur_c,&cur_n);
                                    tem_train->train_pos_char = cur_c;
                                    tem_train->train_pos_num = cur_n;
                                    tem_train->train_next_pos_char = next_c;
                                    tem_train->train_next_pos_num = next_n;
                                    tem_train->train_pos_dis = tem_train->train_last_distance - tem_train->train_pos_dis;
                                    tem_train->train_predict_pos_char = cur_c;
                                    tem_train->train_predict_pos_num = cur_n;
                                    tem_train->train_predict_pos_dis = tem_train->train_pos_dis;
                                }
                                else { // normal case (tr xx 0-14)
                                    if (tem_train->train_speed > next_task->speed) {
                                        //debug(ua_server, 1, "Train %d is started Decelerate", tem_train->train_num);
                                        tem_train->speed_state = DECELERATION;
                                    }
                                    else if (tem_train->train_speed < next_task->speed) {
                                        //debug(ua_server, 1, "Train %d is started Accelerate", tem_train->train_num);
                                        tem_train->speed_state = ACCELERATION;
                                    }
                                    tem_train->train_speed = next_task->speed;
                                }
                                break;
                            }
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }
    }
}


//--------------------------------------------print------------------------------------------------
//print_time
void print_time(uart_server *ua_server, int ticks){
    int idle_rate = getIdle();
    kerprintf(COM2, ua_server, "\033[4;40H %d  \000", idle_rate);
    unsigned int ticks_in_hour = 60 * 60 * 10;
    unsigned int ticks_in_minute = 60 * 10;
    unsigned int ticks_in_second = 10;
    unsigned int hours = ticks / ticks_in_hour;
    unsigned int minutes = (ticks - (hours * ticks_in_hour)) / ticks_in_minute;
    unsigned int seconds = (ticks - (hours * ticks_in_hour) - \
    (minutes * ticks_in_minute)) / ticks_in_second;
    unsigned int tenth_seconds = ticks % ticks_in_second;
    kerprintf(COM2, ua_server, "\033[4;19H %d:%d:%d    \000", minutes, seconds, tenth_seconds);
}
//print_switch
void print_switch(uart_server *ua_server, short dir, short swi){
    char switch_char;
    if(dir == 34) switch_char = 'C';
    else switch_char = 'S';
    if(swi < 12 && swi > 0)
        kerprintf(COM2, ua_server, "\033[8;%dH%c\000", ((swi * 4)-1),switch_char);
    else if(swi >= 12 && swi<20)
        kerprintf(COM2, ua_server, "\033[11;%dH%c\000",(((swi-11) * 4)-1),switch_char);
    else
        kerprintf(COM2, ua_server, "\033[11;%dH%c\000",(((swi-145) * 4)-1),switch_char);
}
//print_sensor
void print_sensor(uart_server *ua_server, train_server *tr_server){
    if(tr_server->sensor_print) {
        train_train *train = &tr_server->train[0];
        train_task *next_task = &(train->task[train->task_start]);
        kerprintf(COM2,ua_server, "\033[15;%dH%c%d \000",tr_server->sensor_index*4+7,\
                        next_task->sensor_char, next_task->sensor_num );
        tr_server->sensor_index = (tr_server->sensor_index+1)%9;
    }
}
//print_schedule
void print_schedule(uart_server *ua_server, train_server *tr_server){
    int i;
    for(i=1;i<MAX_TRAIN_NUMBER;i++) {
        if(tr_server->train[i].train_num) {
            kerprintf(COM2, ua_server, "\033[2%d;0H 0%c%c  %d  0%c%c\000", i, \
                tr_server->train[i].train_num / 10 + '0', \
                tr_server->train[i].train_num % 10 + '0', \
                tr_server->train[i].train_fetch, \
                tr_server->train[i].train_speed / 10 + '0', \
                tr_server->train[i].train_speed % 10 + '0');
        }
    }

    for(i=1;i<MAX_TRAIN_NUMBER;i++) {
        if(tr_server->train[i].train_num) {
            kerprintf(COM2, ua_server, "\033[2%d;13H %c%c%c %c%c%c\000", i, \
                tr_server->train[i].train_dest_char, \
                tr_server->train[i].train_dest_num / 10 + '0', \
                tr_server->train[i].train_dest_num % 10 + '0', \
                tr_server->train[i].train_pos_char, \
                tr_server->train[i].train_pos_num / 10 + '0', \
                tr_server->train[i].train_pos_num % 10 + '0');
        }
    }
    for(i=1;i<MAX_TRAIN_NUMBER;i++) {
        if(tr_server->train[i].train_num) {
            kerprintf(COM2, ua_server, "\033[2%d;21H %c%c%c%c %c%c%c\000", i, \
                tr_server->train[i].train_pos_dis/1000 + '0', \
                (tr_server->train[i].train_pos_dis/100)%10 + '0', \
                (tr_server->train[i].train_pos_dis/10)%10 + '0', \
                tr_server->train[i].train_pos_dis % 10 + '0', \
                tr_server->train[i].train_last_velocity/100 + '0',\
                (tr_server->train[i].train_last_velocity/10)%10 + '0',\
                tr_server->train[i].train_last_velocity%10+'0');
        }
    }
    for(i=1;i<MAX_TRAIN_NUMBER;i++) {
        if(tr_server->train[i].train_num) {
            kerprintf(COM2, ua_server, "\033[2%d;30H %c%c%c %c%c%c%c\000", i, \
                tr_server->train[i].train_predict_pos_char, \
                tr_server->train[i].train_predict_pos_num / 10 + '0', \
                tr_server->train[i].train_predict_pos_num % 10 + '0', \
                tr_server->train[i].train_predict_pos_dis/1000 + '0', \
                (tr_server->train[i].train_predict_pos_dis/100)%10 + '0', \
                (tr_server->train[i].train_predict_pos_dis/10)%10 + '0', \
                tr_server->train[i].train_predict_pos_dis % 10 + '0');
        }
    }
    //kerprintf(COM2, ua_server, "\033[41;10H  sensor41:%d\000",tr_server->track->track_path[41].num);
}








//----------------------------velocity------------------------
void set_velocity(train_train *train){
   short last_v = train->train_velocity[sensor_to_track(train->train_next_pos_num, train->train_pos_char)];
    if(train->train_last_velocity<2000) {
        if (!last_v) {
            train->train_velocity[sensor_to_track(train->train_pos_num, train->train_pos_char)] = train->train_last_velocity;
        } else {
            train->train_velocity[sensor_to_track(train->train_pos_num,train->train_pos_char)] = \
                0.6*last_v+0.4*train->train_last_velocity;
        }
    }
    else{
        bwprintf(COM2,"velocity over 2000mm/second");
    }
}

short read_velocity(train_train *train, short track){
    if(!train->train_velocity[track]){
        return train->train_last_velocity;
    }
    else{
        return train->train_velocity[track];
    }
}






//------------------------------notifier--------------------------------
// this is for commands.
void train_notifier1_boot() {
    char uart_server_name[] = "US\000";
    int uart_server_tid = WhoIs(uart_server_name);
    int train_server_tid = MyParentTid();
    uart_server ua_server;
    train_server tr_server;
    char command[40];
    int cmd_end = 0;
    uart_server_initial(&ua_server, uart_server_tid);
    train_server_initial(&tr_server, MyParentTid());
    ua_server.mes_send->type = UART_GET_UART2;

    while (1) {
        Send(uart_server_tid, ua_server.buf_send, ua_server.mes_size,\
            ua_server.buf_rpy, ua_server.mes_size);
        command[cmd_end++] = ua_server.mes_rpy->buffer;
        command[cmd_end] = 0;
        if(ua_server.mes_rpy->buffer == '\b'){
            if( cmd_end>1 ) cmd_end=cmd_end-2;
        }
        if (ua_server.mes_rpy->buffer == '\r' || ua_server.mes_rpy->buffer == '\n') {
            cmd_end = 0;
            if (command[0] == 't' && command[1] == 'r') {
                int t_idx = 2;
                int train = 0, speed = 0, flag = 0;
                while (t_idx < CMD_BUFFER_SIZE && flag != 1) {
                    // parse the numbers
                    if (is_digit(command[t_idx])) {
                        while (is_digit(command[t_idx])) {
                            train *= 10;
                            train += command[t_idx] - '0';
                            t_idx++;
                        }
                        flag = 1;
                    }
                    t_idx++;
                }
                while (t_idx < CMD_BUFFER_SIZE && flag == 1) {
                    // parse the numbers
                    if (is_digit(command[t_idx])) {
                        while (is_digit(command[t_idx])) {
                            speed *= 10;
                            speed += command[t_idx] - '0';
                            t_idx++;
                        }
                        flag = 2;
                    }
                    t_idx++;
                }
                tr_server.mes_send->type = TR_TR;
                tr_server.mes_send->train = train;
                tr_server.mes_send->speed = speed;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            }
                //switching the track
            else if (command[0] == 's' && command[1] == 'w') {
                int t_idx = 2;
                int track = 0, direction = 0;
                int flag = 0;
                while (t_idx < CMD_BUFFER_SIZE) {
                    // parse the numbers
                    if (is_digit(command[t_idx])) {
                        while (is_digit(command[t_idx]) && flag == 0) {
                            track *= 10;
                            track += command[t_idx] - '0';
                            t_idx++;
                        }
                        flag = 1;
                    }
                    if (command[t_idx] == 'C' || command[t_idx] == 'c') {
                        direction = 34;
                        break;
                    }
                    if (command[t_idx] == 'S' || command[t_idx] == 's') {
                        direction = 33;
                        break;
                    }
                    t_idx++;
                }
                tr_server.mes_send->type = TR_SW;
                tr_server.mes_send->switcher = track;
                tr_server.mes_send->direction = direction;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            }
            else if (command[0] == 'r' && command[1] == 'v') {
                int r_idx = 2;
                int train = 0;
                while (r_idx < CMD_BUFFER_SIZE) {
                    if (is_digit(command[r_idx])) {
                        while (is_digit(command[r_idx]) && r_idx < 5) {
                            train *= 10;
                            train += command[r_idx] - '0';
                            r_idx++;
                        }
                    }
                    r_idx++;
                }
                tr_server.mes_send->type = TR_RV;
                tr_server.mes_send->switcher = train;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            }
            else if (command[0] == 's' && command[1] == 't' && command[2] == 'o' && command[3] == 'p'){
                tr_server.mes_send->train = (command[5]-'0')*10 + (command[6]-'0');
                tr_server.mes_send->sensor_char = command[8];
                tr_server.mes_send->sensor_num = (command[9]-'0')*10+(command[10]-'0');
                tr_server.mes_send->dist_offset = (command[12]-'0')*100+(command[13]-'0')*10+(command[14]-'0');
                tr_server.mes_send->type = TR_STOP;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            }

            else if (command[0] == 'c' && command[1] == 'l' && command[2] == 'e' && command[3] == 'a' && command[4] == 'r') {
                tr_server.mes_send->type = TR_CLEAR;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            }
            else if (command[0] == 's' && command[1] == 's' && command[2] == 't' && command[3] == 'o' && command[4] == 'p'){
                tr_server.mes_send->train = (command[6]-'0')*10 + (command[7]-'0');
                int t_idx = 9;
                int offset = 0;
                while (is_digit(command[t_idx])) {
                    offset *= 10;
                    offset += command[t_idx] - '0';
                    t_idx++;
                }
                tr_server.mes_send->type = TR_SSTOP;
                tr_server.mes_send->dist_offset = offset;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            }

            else if (command[0] == 'a' && command[1] == 'd' && command[2] == 'd'){
                tr_server.mes_send->train = (command[4]-'0')*10 + (command[5]-'0');
                tr_server.mes_send->sensor_char = command[7];
                tr_server.mes_send->sensor_num = (command[8]-'0')*10+(command[9]-'0');
                tr_server.mes_send->type = TR_ADD;
                Send(train_server_tid, tr_server.buf_send, tr_server.mes_size, \
                     tr_server.buf_rcv, tr_server.mes_size);
            }
            else if(command[0] == 'q'){
                Terminate();
            }
        }

    }
}
// this is for sensor
void train_notifier2_boot(){
    char uart_server_name[] = "US\000";
    uart_server ua_server;
    train_server tr_server;
    char sensor[10];
    int buf_end = 0;
    uart_server_initial(&ua_server,WhoIs(uart_server_name));
    train_server_initial(&tr_server,MyParentTid());

    while(1){
        sensor[buf_end++] = uart_server_get(&ua_server, UART_GET_UART1);
        if(buf_end == 10){
            int sensor_temp;
            for (sensor_temp = 0; sensor_temp < 10; sensor_temp++) {
                char sensor_int = sensor[sensor_temp];
                if (sensor_int != 0) {
                    int sensor_num = sensor_char_analysis(sensor_temp, sensor_int);
                    char sensor_char = (sensor_temp / 2) + 65;
                    tr_server.mes_send->type = TR_SENSOR;
                    tr_server.mes_send->sensor_char = sensor_char;
                    tr_server.mes_send->sensor_num = sensor_num;
                    break;
                }
            }
            Send(tr_server.server_tid, tr_server.buf_send, tr_server.mes_size, \
                    tr_server.buf_rcv, tr_server.mes_size);
            buf_end = 0;
        }
    }
}
// this is for clock
void train_notifier3_boot(){
    char clock_server_name[] = "CL\000";
    train_server tr_server;
    clock_server cl_server;
    train_server_initial(&tr_server,MyParentTid());
    clock_server_initial(&cl_server,WhoIs(clock_server_name));
    while(1){
        tr_server.mes_send->type = TR_READY;
        Send(tr_server.server_tid, tr_server.buf_send, tr_server.mes_size,\
            tr_server.buf_rpy, tr_server.mes_size);
        Delay(&cl_server,tr_server.mes_rpy->delay);
        tr_server.mes_send->type = TR_DELAY;
        tr_server.mes_send->train = tr_server.mes_rpy->train;
        Send(tr_server.server_tid, tr_server.buf_send, tr_server.mes_size,\
            tr_server.buf_rpy, tr_server.mes_size);
    }
}
void train_notifier4_boot(){
    char clock_server_name[] = "CL\000";
    train_server tr_server;
    clock_server cl_server;
    train_server_initial(&tr_server,MyParentTid());
    clock_server_initial(&cl_server,WhoIs(clock_server_name));
    while(1){
        Delay(&cl_server,10);
        tr_server.mes_send->type = TR_CLOCK;
        Send(tr_server.server_tid, tr_server.buf_send, tr_server.mes_size,\
            tr_server.buf_rpy, tr_server.mes_size);
    }
}

// Problem: C9 + B2 = 96
//----------------------notifier help functions------------------------
int sensor_char_analysis(int alpha, char num){
    int result_num = 0;
    if(num & 128){
        if (alpha % 2 == 1) result_num = 9;
        else result_num = 1;}
    else if(num & 64){
        if (alpha  % 2 == 1) result_num = 10;
        else result_num = 2;}
    else if(num & 32){
        if (alpha  % 2 == 1) result_num = 11;
        else result_num = 3;}
    else if(num & 16){
        if (alpha  % 2 == 1) result_num = 12;
        else result_num = 4;}
    else if(num & 8){
        if (alpha % 2 == 1) result_num = 13;
        else result_num = 5;}
    else if(num & 4){
        if (alpha % 2 == 1) result_num = 14;
        else result_num = 6;}
    else if(num & 2){
        if (alpha % 2 == 1) result_num = 15;
        else result_num = 7;}
    else if(num & 1){
        if (alpha % 2 == 1) result_num = 16;
        else result_num = 8;}
    if(result_num == 0){
        return -1;
    }
    return result_num;
}


//--------------------------task put&get-------------------------
void task_put(train_train *train){
    train->task_end = (train->task_end+1)%MAX_TRAIN_TASK;
    train->task_count++;
    if(train->task_count == MAX_TRAIN_TASK)
        bwprintf(COM2,"train %d task buffer overflow--------------",train->train_num);
}
void task_get(train_train *train){
    if(train->task_count>0){
        train->task_start = (train->task_start+1)%MAX_TRAIN_TASK;
        train->task_count--;
    }
}

//-----------------------square root-------------------------------
int train_sqrt(int n){
    int i;
    for(i = 0; i < n; i++){
        if(i * i >= n){
            if((i * i - n) < (n- (i-1)*(i-1))){
                return i;
            }
            else{
                return i-1;
            }
        }
    }
    bwprintf(COM2, "TRAIN_SPRT ERROR\n\r");
    return -1;
}

//-----------------------set initial velocity----------------------
void set_speed_a(train_train *train){
    short a=320;
    short b=300;
    short c=280;
    //A
    train->train_velocity[0] = b;
    train->train_velocity[1] = a;
    train->train_velocity[2] = c;
    train->train_velocity[3] = b;
    train->train_velocity[4] = b;
    train->train_velocity[5] = a;
    train->train_velocity[6] = a;
    train->train_velocity[7] = c;
    train->train_velocity[8] = a;
    train->train_velocity[9] = c;
    train->train_velocity[10] = c;
    train->train_velocity[11] = a;
    train->train_velocity[12] = c;
    train->train_velocity[13] = a;
    train->train_velocity[14] = a;
    train->train_velocity[15] = c;
    //B
    train->train_velocity[16] = a;
    train->train_velocity[17] = b;
    train->train_velocity[18] = b;
    train->train_velocity[19] = c;
    train->train_velocity[20] = a;
    train->train_velocity[21] = b;
    train->train_velocity[22] = a;
    train->train_velocity[23] = a;
    train->train_velocity[24] = a;
    train->train_velocity[25] = a;
    train->train_velocity[26] = a;
    train->train_velocity[27] = a;
    train->train_velocity[28] = c;
    train->train_velocity[29] = b;
    train->train_velocity[30] = b;
    train->train_velocity[31] = c;
    //C
    train->train_velocity[32] = b;
    train->train_velocity[33] = c;
    train->train_velocity[34] = a;
    train->train_velocity[35] = b;
    train->train_velocity[36] = b;
    train->train_velocity[37] = c;
    train->train_velocity[38] = b;
    train->train_velocity[39] = c;
    train->train_velocity[40] = c;
    train->train_velocity[41] = b;
    train->train_velocity[42] = b;
    train->train_velocity[43] = c;
    train->train_velocity[44] = a;
    train->train_velocity[45] = c;
    train->train_velocity[46] = a;
    train->train_velocity[47] = b;
    //D
    train->train_velocity[48] = a;
    train->train_velocity[49] = b;
    train->train_velocity[50] = b;
    train->train_velocity[51] = a;
    train->train_velocity[52] = b;
    train->train_velocity[53] = c;
    train->train_velocity[54] = c;
    train->train_velocity[55] = b;
    train->train_velocity[56] = b;
    train->train_velocity[57] = c;
    train->train_velocity[58] = a;
    train->train_velocity[59] = b;
    train->train_velocity[60] = a;
    train->train_velocity[61] = b;
    train->train_velocity[62] = b;
    train->train_velocity[63] = c;
    //E
    train->train_velocity[64] = c;
    train->train_velocity[65] = b;
    train->train_velocity[66] = b;
    train->train_velocity[67] = c;
    train->train_velocity[68] = b;
    train->train_velocity[69] = c;
    train->train_velocity[70] = b;
    train->train_velocity[71] = a;
    train->train_velocity[72] = c;
    train->train_velocity[73] = b;
    train->train_velocity[74] = b;
    train->train_velocity[75] = c;
    train->train_velocity[76] = c;
    train->train_velocity[77] = b;
    train->train_velocity[78] = c;
    train->train_velocity[79] = b;
}

void set_speed_b(train_train *train){
    short a=320;
    short b=300;
    short c=280;
    //A
    train->train_velocity[0] = b;
    train->train_velocity[1] = a;
    train->train_velocity[2] = c;
    train->train_velocity[3] = b;
    train->train_velocity[4] = b;
    train->train_velocity[5] = a;
    train->train_velocity[6] = a;
    train->train_velocity[7] = c;
    train->train_velocity[8] = a;
    train->train_velocity[9] = c;
    train->train_velocity[10] = c;
    train->train_velocity[11] = b;
    train->train_velocity[12] = c;
    train->train_velocity[13] = a;
    train->train_velocity[14] = b;
    train->train_velocity[15] = c;
    //B
    train->train_velocity[16] = a;
    train->train_velocity[17] = b;
    train->train_velocity[18] = b;
    train->train_velocity[19] = c;
    train->train_velocity[20] = a;
    train->train_velocity[21] = b;
    train->train_velocity[22] = a;
    train->train_velocity[23] = a;
    train->train_velocity[24] = a;
    train->train_velocity[25] = a;
    train->train_velocity[26] = a;
    train->train_velocity[27] = a;
    train->train_velocity[28] = c;
    train->train_velocity[29] = b;
    train->train_velocity[30] = b;
    train->train_velocity[31] = c;
    //C
    train->train_velocity[32] = b;
    train->train_velocity[33] = c;
    train->train_velocity[34] = a;
    train->train_velocity[35] = b;
    train->train_velocity[36] = b;
    train->train_velocity[37] = c;
    train->train_velocity[38] = b;
    train->train_velocity[39] = c;
    train->train_velocity[40] = c;
    train->train_velocity[41] = b;
    train->train_velocity[42] = b;
    train->train_velocity[43] = c;
    train->train_velocity[44] = a;
    train->train_velocity[45] = c;
    train->train_velocity[46] = a;
    train->train_velocity[47] = b;
    //D
    train->train_velocity[48] = c;
    train->train_velocity[49] = b;
    train->train_velocity[50] = b;
    train->train_velocity[51] = a;
    train->train_velocity[52] = b;
    train->train_velocity[53] = c;
    train->train_velocity[54] = c;
    train->train_velocity[55] = b;
    train->train_velocity[56] = b;
    train->train_velocity[57] = c;
    train->train_velocity[58] = a;
    train->train_velocity[59] = b;
    train->train_velocity[60] = a;
    train->train_velocity[61] = b;
    train->train_velocity[62] = b;
    train->train_velocity[63] = c;
    //E
    train->train_velocity[64] = c;
    train->train_velocity[65] = b;
    train->train_velocity[66] = b;
    train->train_velocity[67] = c;
    train->train_velocity[68] = b;
    train->train_velocity[69] = b;
    train->train_velocity[70] = b;
    train->train_velocity[71] = a;
    train->train_velocity[72] = b;
    train->train_velocity[73] = c;
    train->train_velocity[74] = b;
    train->train_velocity[75] = c;
    train->train_velocity[76] = b;
    train->train_velocity[77] = b;
    train->train_velocity[78] = c;
    train->train_velocity[79] = b;
}






