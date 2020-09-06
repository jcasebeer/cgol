#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>

enum {
    N0=1, N1=2, N2=4, N3=8, N4=16, N5=32, N6=64, N7=128, N8=256                                  
};
int WIDTH = 78;
int HEIGHT = 21;
#define IDX(x,y) ((y)*WIDTH+(x))

char *board = NULL;
char *neighbors = NULL;
unsigned long long generation = 0;

int b_rule = N3;
int s_rule = N2 | N3;

void clear_screen()
{ 
    printf("\e[1;1H\e[2J");
}

void reset_cursor()
{
    printf("\e[1;1H");
}

void print_info()
{
    int i;
    printf("B");
    for (i=0; i<9; i++)
        if (b_rule & (1<<i))
            printf("%d", i);
    printf("/S");
    for (i=0; i<9; i++)
        if (s_rule & (1<<i))
            printf("%d", i);
    printf(" Generation: %d\n", generation);
}
void print_board()
{
    int x, y;
    printf("+");
    for(x=0; x<WIDTH; x++)
        printf("-");
    printf("+\n");

    for(y=0; y<HEIGHT;y++) {
        printf("|");
        for(x=0; x<WIDTH; x++) {
            if (board[IDX(x,y)])
                printf("O");
            else
                printf(" ");
        }
        printf("|\n");
    }
    printf("+");
    for(x=0; x<WIDTH; x++)
        printf("-");
    printf("+\n");
    print_info();
}

int count_neighbors(int x, int y)
{
    int xd, yd;
    int count = 0;
    for(yd=-1; yd<2; yd++)
        for(xd=-1; xd<2; xd++)
            if (xd || yd) {
                int xx = (x+xd+WIDTH) % (WIDTH);
                int yy = (y+yd+HEIGHT) % (HEIGHT);
                count+=board[IDX(xx,yy)];
            }
    return count;
}

void mutate_board()
{
    int x,y;
    for(y=0; y<HEIGHT; y++) 
        for(x=0; x<WIDTH; x++) {
            neighbors[IDX(x,y)] = count_neighbors(x,y);
        }

    for(y=0; y<HEIGHT; y++) 
        for(x=0; x<WIDTH; x++) {
            int index = IDX(x,y);
            int alive = board[index];
            int n = 1<<neighbors[index];
            board[index] = (alive && (n & s_rule)) || (!alive && (n & b_rule));
        }
}

void apply_rule_string(char *rule_string)
{
    b_rule=0;
    s_rule=0;
    char *arg = rule_string;
    int *rule = &b_rule;
    while(arg) {
        switch(*arg) {
            case '\0': return;
            case 'b': 
            case 'B':
                       rule = &b_rule;
                       break;
            case 's':
            case 'S':
                       rule = &s_rule;
                       break;
            default:
                       if (*arg > 47 && *arg < 57) {
                           int n = 1<<((*arg) - 48);
                           (*rule) |= n;
                       }
                       break;
        }
        arg++;
    }
}

void parse_arg(int argc, char **argv)
{
    char *tmp = NULL;
    int i;
    for(i = 1; i<argc; i++) {
        if (strcmp("-w", argv[i]) == 0) {
            int w = i+1<argc ? atoi(argv[++i]) : 0;
            WIDTH = w ? w : WIDTH;
        }
        else
        if (strcmp("-h", argv[i]) == 0) {
            int h = i+1<argc ? atoi(argv[++i]) : 0;
            HEIGHT = h ? h : HEIGHT;
        }
        else {
            apply_rule_string(argv[i]);
        }
    }
}

void set_board_size()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    WIDTH = w.ws_col - 2;
    HEIGHT = w.ws_row - 4;
}

int main(int argc, char **argv)
{ 
    set_board_size();
    parse_arg(argc, argv);
    board = calloc(WIDTH*HEIGHT, sizeof(char));
    neighbors = calloc(WIDTH*HEIGHT, sizeof(char));
    srand((unsigned int)time(NULL));
    int i;
    for(i=0; i<WIDTH*HEIGHT; i++)
        board[i] = rand() & 1;

    clear_screen();
    
    while (1) {
        reset_cursor();
        print_board();
        fflush(stdout);
        mutate_board(); 
        usleep(33000);
        generation++;
    }
    return 0;
}
