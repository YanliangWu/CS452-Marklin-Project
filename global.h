//
// Created by Yanliang Wu on 2016-07-13.
//

#ifndef TRAIN_PROJECT_GLOBAL_H
#define TRAIN_PROJECT_GLOBAL_H
/* syscall numbers */

#define MAX_TASK_NUM            30

#define SYSCALL_CREATE          1
#define SYSCALL_MYTID           2
#define SYSCALL_MYPARENTTID     3
#define SYSCALL_PASS            4
#define SYSCALL_SEND            5
#define SYSCALL_RECEIVE         6
#define SYSCALL_REPLY           7
#define SYSCALL_AWAITEVENT      8
#define SYSCALL_EXIT            9
#define SYSCALL_GETIDLE         10
#define SYSCALL_GETTRACKID      12
#define SYSCALL_TERMINATE       99
#define HWI_INTERRUPT 100


#endif //TRAIN_PROJECT_GLOBAL_H
