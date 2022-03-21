#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include "server.h"
#include "WoT.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *win = NULL;
int exit_status = 0;
struct sockaddr_in IP[2];
Tank Player[2];
int win_player = 0;

int check_hit(Tank *tank, int BulletId)
{
    int x1, y1, x2, y2;
    int id = tank->PlayerId;
    x1 = Player[!id].center.x - 1;
    y1 = Player[!id].center.y - 1;
    x2 = x1 + 2;
    y2 = y1 + 2;

    int x = tank->bullets[BulletId].center.x;
    int y = tank->bullets[BulletId].center.y;

    if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
        return (id == 0 ? 1 : 2);

    return 0;
} 

void init_window(void) 
{
	initscr();
    keypad(stdscr, TRUE);
    int row = 40;
    int col = 100;
    noecho();
    curs_set(0);
    start_color();
    init_pair(COLOR1, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR2, COLOR_BLUE, COLOR_BLACK);
    int heightGap = (LINES - row) / 2;
    int widthGap = (COLS - col) / 2;
    win = newwin(row, col, heightGap, widthGap);
    keypad(win, TRUE);
    wtimeout(win, 100);
    touchwin(win);
}

void telo_tankov(void)
{
    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
            wattron(win, COLOR_PAIR(COLOR1));
        else
            wattron(win, COLOR_PAIR(COLOR2));

        if (Player[i].dir == UP || Player[i].dir == DOWN)
        {
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x - 1, '#');
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x, ' ');
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x + 1, '#');
            mvwaddch(win, Player[i].center.y, Player[i].center.x - 1, '#');
            mvwaddch(win, Player[i].center.y, Player[i].center.x, '*');
            mvwaddch(win, Player[i].center.y, Player[i].center.x + 1, '#');
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x - 1, '#');
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x, ' ');
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x + 1, '#');
        }
	
		if (Player[i].dir == LEFT || Player[i].dir == RIGHT)
        {
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x - 1, '#');
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x, '#');
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x + 1, '#');
            mvwaddch(win, Player[i].center.y, Player[i].center.x - 1, ' ');
            mvwaddch(win, Player[i].center.y, Player[i].center.x, '*');
            mvwaddch(win, Player[i].center.y, Player[i].center.x + 1, ' ');
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x - 1, '#');
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x, '#');
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x + 1, '#');
        }
        if (i == 0)
            wattroff(win, COLOR_PAIR(COLOR1));
        else
            wattroff(win, COLOR_PAIR(COLOR2));
    }
	for (int i = 0; i < 2; i++)
    {
        if (i == 0)
            wattron(win, COLOR_PAIR(COLOR1));
        else
            wattron(win, COLOR_PAIR(COLOR2));

        switch (Player[i].tower)
        {
        case UP_LEFT:
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x - 1, '\\');
            break;
        case UP_UP:
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x, '|');
            break;
        case UP_RIGHT:
            mvwaddch(win, Player[i].center.y - 1, Player[i].center.x + 1, '/');
            break;
        case MIDDLE_LEFT:
            mvwaddch(win, Player[i].center.y, Player[i].center.x - 1, '-');
            break;
        case MIDDLE_RIGHT:
            mvwaddch(win, Player[i].center.y, Player[i].center.x + 1, '-');
            break;
        case DOWN_LEFT:
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x - 1, '/');
            break;
        case DOWN_DOWN:
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x, '|');
            break;
        case DOWN_RIGHT:
            mvwaddch(win, Player[i].center.y + 1, Player[i].center.x + 1, '\\');
            break;
        }
        if (i == 0)
            wattroff(win, COLOR_PAIR(COLOR1));
        else
            wattroff(win, COLOR_PAIR(COLOR2));
    }
}

void print_bullets(void)
{
    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
            wattron(win, COLOR_PAIR(COLOR1));
        else
            wattron(win, COLOR_PAIR(COLOR2));

        for (int j = 0; j < 4; j++)
            mvwaddch(win, Player[i].bullets[j].center.y, Player[i].bullets[j].center.x, 'o');

        if (i == 0)
            wattroff(win, COLOR_PAIR(COLOR1));
        else
            wattroff(win, COLOR_PAIR(COLOR2));
    }
}
void *create_bullet(void *data)
{
    Tank *tank = (Tank *)data;

    tank->amount_bullet++;

    int BulletId = 0;
    for (int i = 0; i < 4; i++)
    {
        if (!tank->bullets[i].alive)
        {
            BulletId = i;
            break;
        }
    }

    tank->bullets[BulletId].alive = 1;

    switch (tank->tower)
    {
    case UP_LEFT:
        tank->bullets[BulletId].vector_x = -1;
        tank->bullets[BulletId].vector_y = -1;
        tank->bullets[BulletId].center.x = tank->center.x - 2;
        tank->bullets[BulletId].center.y = tank->center.y - 2;
        break;
    case UP_UP:
        tank->bullets[BulletId].vector_x = 0;
        tank->bullets[BulletId].vector_y = -1;
        tank->bullets[BulletId].center.x = tank->center.x;
        tank->bullets[BulletId].center.y = tank->center.y - 2;
        break;
    case UP_RIGHT:
        tank->bullets[BulletId].vector_x = 1;
        tank->bullets[BulletId].vector_y = -1;
        tank->bullets[BulletId].center.x = tank->center.x + 2;
        tank->bullets[BulletId].center.y = tank->center.y - 2;
        break;
    case MIDDLE_LEFT:
        tank->bullets[BulletId].vector_x = -1;
        tank->bullets[BulletId].vector_y = 0;
        tank->bullets[BulletId].center.x = tank->center.x - 2;
        tank->bullets[BulletId].center.y = tank->center.y;
        break;
    case MIDDLE_RIGHT:
        tank->bullets[BulletId].vector_x = 1;
        tank->bullets[BulletId].vector_y = 0;
        tank->bullets[BulletId].center.x = tank->center.x + 2;
        tank->bullets[BulletId].center.y = tank->center.y;
        break;
    case DOWN_LEFT:
        tank->bullets[BulletId].vector_x = -1;
        tank->bullets[BulletId].vector_y = 1;
        tank->bullets[BulletId].center.x = tank->center.x - 2;
        tank->bullets[BulletId].center.y = tank->center.y + 2;
        break;
    case DOWN_DOWN:
        tank->bullets[BulletId].vector_x = 0;
        tank->bullets[BulletId].vector_y = 1;
        tank->bullets[BulletId].center.x = tank->center.x;
        tank->bullets[BulletId].center.y = tank->center.y + 2;
        break;
    case DOWN_RIGHT:
        tank->bullets[BulletId].vector_x = 1;
        tank->bullets[BulletId].vector_y = 1;
        tank->bullets[BulletId].center.x = tank->center.x + 2;
        tank->bullets[BulletId].center.y = tank->center.y + 2;
        break;
    }

    while (
        tank->bullets[BulletId].center.x - 1 > 0 &&
        tank->bullets[BulletId].center.x + 1 < 99 &&
        tank->bullets[BulletId].center.y - 1 > 0 &&
        tank->bullets[BulletId].center.y + 1 < 39)
    {
        int res = check_hit(tank, BulletId);
        if (res)
        {
            exit_status = 1;
            win_player = res;
        }

        tank->bullets[BulletId].center.x += tank->bullets[BulletId].vector_x;
        tank->bullets[BulletId].center.y += tank->bullets[BulletId].vector_y;
        usleep(50000);
    }

    tank->bullets[BulletId].alive = 0;
    tank->amount_bullet--;
    tank->bullets[BulletId].center.x = -1;
    tank->bullets[BulletId].center.y = -1;

    return (void *)NULL;
}

int check_collision(int id, int key)
{
    int x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = 0;
    y1 = 0;
    switch (key)
    {
    case KEY_UP:
        x1 = Player[id].center.x - 1;
        y1 = Player[id].center.y - 2;
        break;
    case KEY_DOWN:
        x1 = Player[id].center.x - 1;
        y1 = Player[id].center.y;
        break;
    case KEY_LEFT:
        x1 = Player[id].center.x - 2;
        y1 = Player[id].center.y - 1;
        break;
    case KEY_RIGHT:
        x1 = Player[id].center.x;
        y1 = Player[id].center.y - 1;
        break;
    }
    x2 = x1 + 2;
    y2 = y1 + 2;

    x3 = Player[!id].center.x - 1;
    y3 = Player[!id].center.y - 1;
    x4 = x3 + 2;
    y4 = y3 + 2;

    if ((x1 <= x4) && (x2 >= x3) && (y1 <= y4) && (y2 >= y3))
        return true;
    return false;
}

int move_player(int key, struct sockaddr_in addr)
{

    int check = 0;
    for (int i = 0; i < 2; i++)
    {
        if (IP[i].sin_addr.s_addr == addr.sin_addr.s_addr)
            check = 1;
    }

    if (!check)
        return 0;

    if (key == 'k'){
    	exit_status = 1;
    	return 0;
    }

    int id = 1;
    for (int i = 0; i < 2; i++)
    {
        if (IP[i].sin_addr.s_addr == addr.sin_addr.s_addr)
        {
            id = i;
            break;
        }
    }

    pthread_t bullet;
    pthread_mutex_lock(&mutex);
    switch (key)
    {
    case KEY_UP:
        if (Player[id].center.y - 2 > 0 && !check_collision(id, key))
        {
            Player[id].center.y--;
            Player[id].dir = UP;
        }
        break;
    case KEY_DOWN:
        if (Player[id].center.y + 2 < 39 && !check_collision(id, key))
        {
            Player[id].center.y++;
            Player[id].dir = DOWN;
        }
        break;
    case KEY_LEFT:
        if (Player[id].center.x - 2 > 0 && !check_collision(id, key))
        {
            Player[id].center.x--;
            Player[id].dir = LEFT;
        }
        break;
    case KEY_RIGHT:
        if (Player[id].center.x + 2 < 99 && !check_collision(id, key))
        {
            Player[id].center.x++;
            Player[id].dir = RIGHT;
        }
        break;

    case 'q':
        Player[id].tower = UP_LEFT;
        break;
    case 'w':
        Player[id].tower = UP_UP;
        break;
    case 'e':
        Player[id].tower = UP_RIGHT;
        break;
    case 'a':
        Player[id].tower = MIDDLE_LEFT;
        break;
    case 'd':
        Player[id].tower = MIDDLE_RIGHT;
        break;
    case 'z':
        Player[id].tower = DOWN_LEFT;
        break;
    case 'x':
        Player[id].tower = DOWN_DOWN;
        break;
    case 'c':
        Player[id].tower = DOWN_RIGHT;
        break;

    case ' ':
        if (Player[id].amount_bullet < 4)
            pthread_create(&bullet, NULL, create_bullet, (void *)&Player[id]);
        break;
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}
void init_game(void)
{
	for (int i = 0; i < 2; i++)
    {
        Player[i].dir = UP;
        Player[i].tower = UP_UP;
        Player[i].amount_bullet = 0;
        for (int j = 0; j < 4; j++)
        {
            Player[i].bullets[j].alive = 0;
            Player[i].bullets[j].center.x = -1;
            Player[i].bullets[j].center.y = -1;
        }
        Player[i].PlayerId = i;
    }

    Player[0].center.x = 25;
    Player[0].center.y = 20;
    Player[1].center.x = 75;
    Player[1].center.y = 20;
}

int main(int argc, char **argv)
{
	 if (argc < 3)
    {
        printf("Usage: %s <ip1> <ip2>\n", argv[0]);
        return 0;
    }

    inet_aton(argv[1], &IP[0].sin_addr);
    inet_aton(argv[2], &IP[1].sin_addr);
    init_window();
    init_game();
     pthread_t pid;
    pthread_create(&pid, NULL, udp_server, NULL);

    int sock, ret, yes = 1;
    struct sockaddr_in addr;
    unsigned int addr_len;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("sock");
        exit(1);
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (char *)&yes, sizeof(yes));
    if (ret == -1)
    {
        perror("setsockopt");
        exit(1);
    }
	addr_len = sizeof(struct sockaddr_in);
    memset((void *)&addr, 0, addr_len);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addr.sin_port = htons(PORT);
	int key;
	while (!exit_status)
    {
        werase(win);
        box(win, 0, 0);
        telo_tankov();
        print_bullets();

        wrefresh(win);

        key = wgetch(win);
        switch (key)
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case 'q':
            case 'w':
            case 'e':
            case 'a':
            case 'd':
            case 'z':
            case 'x':
            case 'c':
            case ' ':
            case 'k':
                sendto(sock, &key, sizeof(key), 0, (struct sockaddr *)&addr, addr_len);
                break;
        }
        usleep(1000000 / 120);
    }

    char msg[64];
    if(win_player == 1)
        sprintf(msg, "Player 1 with IP %s WIN!", inet_ntoa(IP[0].sin_addr));
    else if (win_player == 2)
        sprintf(msg, "Player 2 with IP %s WIN!", inet_ntoa(IP[1].sin_addr));
    else
    	sprintf(msg, "GAME CLOSED");

    mvwprintw(win, 20, (100 - strlen(msg)) / 2, msg);

    wtimeout(win, -1);
    wgetch(win);

    pthread_cancel(pid);

    delwin(win);
    endwin();

    return 0;
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    