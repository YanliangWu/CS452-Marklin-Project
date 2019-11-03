/* THIS FILE IS GENERATED CODE -- DO NOT EDIT */


#define TRACK_MAX 144
#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

typedef enum {
    NODE_NONE,
    NODE_SENSOR,
    NODE_BRANCH,
    NODE_MERGE,
    NODE_ENTER,
    NODE_EXIT,
} node_type;

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
    track_edge *reverse;
    track_node *src, *dest;
    int dist;             /* in millimetres */
};

struct track_node {
    const char *name;
    node_type type;
    int num;              /* sensor or switch number */
    track_node *reverse;  /* same location, but opposite direction */
    track_edge edge[2];
    char dead;
};

int track_getpath(track_node *node, int *path, int start, int end);
short track_getnext(track_node *track, short *swither,  char c, short n, char *res_c, short *res_n);
short sensor_to_pre_track(track_node *track, short *switcher, short n, char c);

short sensor_to_track(short num, char c);
short switch_to_track(short swi);
void track_to_sensor(short i, char *c,short *n);


void sensor_reverse(track_node *track, char c, short n, char *res_c, short *res_n);

int is_digit(char i);


void track_inita(track_node *track);
void track_initb(track_node *track);
