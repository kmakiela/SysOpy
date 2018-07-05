

#define PROJECT_NO 1

#define MAX_MSG_SIZE 256

#define MAX_NO_OF_CLIENTS 10

#define START 1
#define MIRROR 2
#define ADD 3
#define MUL 4
#define SUB 5
#define DIV 6
#define TIME 7
#define END 8
#define STOP 9

#define BLUE    "\x1b[34m"
#define YELLOW  "\x1b[33m"
#define DARKRED "\x1b[31m"
#define RED     "\x1b[91m"
#define GREEN   "\x1b[92m"
#define CYAN    "\x1b[96m"
#define MAGENTA "\x1b[35m"
#define DEFAULT "\x1b[0m"

int SERVER_STOP;
int CLIENT_STOP;


struct myMsgBuf{
    long mType;
    pid_t pid;
    char msg[MAX_MSG_SIZE];
};

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};