#include "track.h"
#include "uartserver.h"
#define MAX_DELAY_NOTIFIER 10
#define MAX_TRAIN_TASK 40
#define MAX_TRAIN_NUMBER 5
#define MAX_TRACK_NUMBER 144
#define CMD_BUFFER_SIZE 40
#define DOUZI_DEBUG 0

typedef enum TR_type {
    TR_TR = 1,
    TR_SW = 2,
    TR_RV = 3,
    TR_DELAY = 4,
    TR_READY = 5,
    TR_SENSOR =6,
    TR_CLOCK = 7,
    TR_STOP = 8,
    TR_CLEAR = 9,
    TR_ADD = 10,
    TR_SSTOP = 11,
} TR_type;

typedef enum TR_task_type{
    TASK_SENSOR = 1,
    TASK_DELAY = 2,
    TASK_TR = 3,
    TASK_SW = 4,
    TASK_READY = 5,
    TASK_CLOCK = 6,
} TR_task_type;

typedef enum TR_train_state{
    TRAIN_STOP_TRY = 1,
    TRAIN_STOP_END = 2,
    TRAIN_NOTHING = 3,
} TR_train_state;

typedef enum TR_speed_info {
    ACCELERATION = 1,
    DECELERATION = 2,
    CONSTANT = 3,
    STOPPED = 4
} TR_speed_info;


typedef struct TR_message {
    TR_type type;
    int delay;
    int train;
    int switcher;
    int direction;
    int speed;
    int sensor_num;
    int dist_offset;
    char sensor_char;
} TR_message;

typedef struct TR_task{
    TR_task_type type;
    int delay;
    int train;
    int switcher;
    int direction;
    int speed;
    int sensor_num;
    char sensor_char;
} train_task;

typedef struct TR_track {
    track_node track_path[MAX_TRACK_NUMBER];
    short track_block[MAX_TRACK_NUMBER];
    short track_switch[MAX_TRACK_NUMBER];
} train_track;

typedef struct TR_train{
    char train_predict_pos_char;
    short train_predict_pos_num;
    short train_predict_pos_dis;
    short train_num;
    short train_speed;
    short train_fetch;
    char train_dest_char;
    short train_dest_num;
    short train_dest_dis;
    char train_pos_char;
    short train_pos_num;
    short train_pos_dis;
    short train_pos_tick;
    char train_next_pos_char;
    short train_next_pos_num;
    int train_last_sentick;
    int train_last_distance;
    int train_last_velocity;
    short train_safe_dis;
    short train_velocity[MAX_TRACK_NUMBER];
    train_task task[MAX_TRAIN_TASK];
    short task_start;
    short task_end;
    short task_count;
    TR_train_state train_state;
    short train_delay;
    TR_speed_info speed_state;
    short train_acc;
    short train_dec;
    short train_cons;
    int train_dest_offset;
    short train_dest_pre_track;
} train_train;

typedef struct TR_server {
    char buf_send[sizeof(TR_message)];
    char buf_rcv[sizeof(TR_message)];
    char buf_rpy[sizeof(TR_message)];
    TR_message * mes_send;
    TR_message * mes_rcv;
    TR_message * mes_rpy;
    int client_tid;
    int server_tid;
    int mes_size;
    short delay_notifier[MAX_DELAY_NOTIFIER];
    int sensor_index;

    train_track *track;
    train_train train[MAX_TRAIN_NUMBER];
    int tick;
    int pre_sensor_num;
    char pre_sensor_char;
    short sensor_change;
    short sensor_print;
} train_server;


void train_server_boot();
void train_server_initial(train_server * server, int server_tid);
void train_server_clear(train_server * tr_server, uart_server * ua_server);

void train_state(uart_server *ua_server, train_server *tr_server);
void train_block(uart_server *ua_server, train_server *tr_server);
void train_schedule(uart_server *ua_server, train_server *tr_server);
void train_execute(uart_server *ua_server, train_server *tr_server);

void print_time(uart_server *ua_server, int ticks);
void print_switch(uart_server *ua_server, short dir, short swi);
void print_sensor(uart_server *ua_server, train_server *tr_server);
void print_schedule(uart_server *ua_server, train_server *tr_server);

void task_put(train_train *train);
void task_get(train_train *train);

void set_velocity(train_train *train);
short read_velocity(train_train *train, short track);

void train_notifier1_boot();
void train_notifier2_boot();
void train_notifier3_boot();
void train_notifier4_boot();

int sensor_char_analysis(int alpha, char num);

int train_sqrt(int n);
void set_speed_a(train_train *train);
void set_speed_b(train_train *train);

