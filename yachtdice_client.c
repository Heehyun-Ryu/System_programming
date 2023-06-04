#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define SUBMIT 4
#define PORT 50000

int diceN[5] = { 0, };
int diceInd[7] = {0,};
int spin[5] = { 1,1,1,1,1};
int scoreboard[12] = {0, };
int scorecheck[12] = {0, };

int turn = 12;
int cnt = 0;
int stop = 0;
int bnstot = 0;


char ipnum[100] = {'\0', };
char intro_1[] = "The Yacht Dice";
char intro_2[] = "-GAME-";
char single[] = "-Single mode";
char multi[] = "-Online mode";
char yacht_board[] = "======================Yacht Dice======================";
char instruction[] = "use 'w' and 's' to move curser, and press 'space' to choose";

char gameover_1[] = "  _____                           ____                 ";
char gameover_2[] = "/ ____|                          / __ \\                ";
char gameover_3[] = "| |  __  __ _ _ __ ___   ___    | |  | |_   _____ _ __ ";
char gameover_4[] = "| | |_ |/ _` | '_ ` _ \\ / _ \\   | |  | \\ \\ / / _ \\ '__|";
char gameover_5[] = "| |__| | (_| | | | | | |  __/   | |__| |\\ V /  __/ |   ";
char gameover_6[] = " \\_____|\\__,_|_| |_| |_|\\___|    \\____/  \\_/ \\___|_|   ";


int row, col;
int trow, tcol;
int ch, board_col;
int choice = 1;

int keyctrl(){
    char temp_k = getch();

    if(temp_k == 'w' || temp_k == 'W'){
        return UP;
    }
    else if(temp_k == 's' || temp_k == 'S'){
        return DOWN;
    }
    else if(temp_k == 'a' || temp_k == 'A'){
        return LEFT;
    }
    else if(temp_k == 'd' || temp_k == 'D'){
        return RIGHT;
    }
    else if(temp_k == ' '){
        return SUBMIT;
    }
    else{
        return 99;
    }
}

void draw(){
    clear();
    getmaxyx(stdscr, row, col);
    trow = row/2;
    tcol = (col - strlen(intro_1))/2;
    move(trow-3, tcol);
    printw("%s", intro_1);
    tcol = (col - strlen(intro_2))/2;
    move(trow-2, tcol);
    printw("%s", intro_2);
    
    tcol = (col - strlen(instruction))/2;
    move(trow+3, tcol);
    printw("%s", instruction);

    tcol = (col - strlen(single))/2;
    move(trow+5, tcol);
    printw("%s", single);
    
    move(trow+6, tcol);
    printw("%s", multi);
    refresh();
}

void game_end(){
    int x = trow;
    int y = (col - strlen(gameover_1))/2- 3;
    int direction_x = 1;
    int direction_y = 1;
    int ccnt = 1;

    while (1) {
        
        clear(); // 화면 지우기

        mvprintw(x-3, y, gameover_1);
        mvprintw(x-2, y, gameover_2);
        mvprintw(x-1, y, gameover_3);
        mvprintw(x, y, gameover_4);
        mvprintw(x+1, y, gameover_5);
        mvprintw(x+2, y, gameover_6);


        refresh(); // 변경된 내용을 화면에 업데이트

        y += direction_y;

        usleep(100000); // 0.1초 대기
        ccnt++;
        if(ccnt > 30){
            break;
        }
    }
    

    
    return;
}


int set_ticker(int n_msecs)
{
    struct itimerval new_timeset;
    long n_sec, n_usecs;

    n_sec = n_msecs / 1000;
    n_usecs = (n_msecs % 1000) * 1000L;

    new_timeset.it_interval.tv_sec = n_sec;
    new_timeset.it_interval.tv_usec = n_usecs;
    new_timeset.it_value.tv_sec = n_sec;
    new_timeset.it_value.tv_usec = n_usecs;

    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

void makerand()
{
    for(int i = 0; i < 5; i++)
    {
        if(spin[i] == 1)
        {
            diceN[i] = rand() % 6;
        }
    }

}

int empty(int source[12])
{
    int suml = 0; int sum = 0; int bonus = 0; int total = 0;
    for(int i = 0; i < 12; i++)
    {
        sum += source[i];
        if(i < 6) suml += source[i];
    }
    if(suml > 63) bonus = 35;

    total = sum + bonus;
    printw("===================Yacht Dice====================\n"
        "Aces  (1)| %2d                  4 of a kind(7)| %2d     \n"
        "Twos  (2)| %2d                  Full House (8)| %2d     \n"
        "Threes(3)| %2d                  S_Stright  (9)| %2d     \n"
        "Fours (4)| %2d                  L_Stright (10)| %2d     \n"
        "Fives (5)| %2d                  Yacht     (11)| %2d     \n"
        "Sixes (6)| %2d                  Chance    (12)| %2d     \n",
        source[0], source[6], source[1], source[7], source[2], source[8], source[3], source[9], source[4], source[10], source[5], source[11]);

    if(bnstot == 1)
    {
        printw("Bonus    |%3d                  Total         |%3d      \n", bonus, total);
    }

    refresh();
    return total;
}

void setLIST(int source[12], int nums[])
{

    for(int i = 0; i < 6; i ++)
    {
        source[i] = nums[i+1] * (i+1);
    }

    for (int i = 1; i <= 6; i++)
    {

        if (nums[i] == 3) //FULL HOUSE
        {
            for (int j = 1; j <= 6; j++)
            {
                if (j == i) continue;
                if (nums[j] == 2) source[7] = i * 3 + j * 2;
            }
        }


        if (nums[i] >= 4)
        {
            source[6] = i * 4;  //FOUR OF A KIND

            if (nums[i] == 5) source[10] = 50; //YACHT

        }

        source[11] += nums[i] * i; //CHANCE
        
    }

    if (nums[1] >= 1 && nums[2] >= 1 && nums[3] >= 1 && nums[4] >= 1 ) source[8] = 30; //LITTLE STRAIGHT
    if (nums[5] >= 1 && nums[2] >= 1 && nums[3] >= 1 && nums[4] >= 1 ) source[8] = 30; //LITTLE STRAIGHT
    if (nums[5] >= 1 && nums[6] >= 1 && nums[3] >= 1 && nums[4] >= 1 ) source[8] = 30; //LITTLE STRAIGHT

    if (nums[2] == 1 && nums[3] == 1 && nums[4] == 1 && nums[5] == 1 && nums[6] == 1) source[9] = 40; //BIG STRAIGHT
    if (nums[2] == 1 && nums[3] == 1 && nums[4] == 1 && nums[5] == 1 && nums[1] == 1) source[9] = 40;

}

char dice[6][3][7] =
{
    {"   \n", "  * \n", "   \n"}, 
    {"*  \n", "   \n", "    *\n"}, 
    {"*  \n", "  *\n", "    *\n"}, 
    {"*   *\n", "   \n", "*   *\n"}, 
    {"*   *\n", "  * \n", "*   *\n"}, 
    {"*   *\n", "*   *\n", "*   *\n"},

};


void printdice()
{

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            move(10+j, 0 + (9*i));
            printw(dice[diceN[i]][j]);
        }
        
    }
    move(0,0);
    refresh();
}

void shake(int signum)
{
    makerand();
    signal(SIGALRM, shake);
    printdice();
}

void checknum()
{
    signal(SIGALRM, SIG_IGN);
    while(1)
    {
        if(cnt == 3)
        {
            mvprintw(8, 0, "            ");
            refresh();
            stop = 1;
            return;
        }
        nocbreak();
        char a[11];
        mvprintw(9, 0, "choose the dice:                     ");
        refresh();
        move(9,16);
        echo();
        scanw(" %[^\n]", a);
        noecho();
        if (strcmp(a, "ok") == 0 || strcmp(a, "OK") == 0 || strcmp(a, "Ok") == 0 || strcmp(a, "oK") == 0) 
        {
            stop = 1;
            return;
        }

        if (strcmp(a, "nO") == 0 || strcmp(a, "NO") == 0 || strcmp(a, "No") == 0 || strcmp(a, "no") == 0) 
        {
            spin[0] = 1; spin[1] = 1; spin[2] = 1; spin[3] = 1; spin[4] = 1;
            crmode();
            return;
        }

        char se[11];

        int j = 0;
        for (int i = 0; i < strlen(a); i++) 
        {
            if ( '0' <= a[i] && a[i] <= '9') 
            {
                se[j++] = a[i];
            }
        }
        se[j] = '\0';

        for (int i = 0; i < strlen(se); i++) 
        {
            spin[se[i] - '0' - 1] = 1;
        }

        if(j == 0)
        {
            mvprintw(8, 0, "type right answer                   ");
            refresh();
            sleep(10);
            mvprintw(8, 0, "                 ");
            continue;
        }


        mvprintw(8, 0, "count: %d", cnt);
        refresh();
        crmode();
        return;
    }
    
    //crmode();
}

void diceset()
{
    int c;
     while(1)
        {
            mvprintw(9, 0, "Press spacebar to select random dices");
            c = getch();

            if(c == ' ')
            {
                cnt++;
                nocbreak();
                spin[0] = 0; spin[1] = 0; spin[2] = 0; spin[3] = 0; spin[4] = 0;
                checknum();
                signal(SIGALRM, shake);
                set_ticker(50);
            }

            if (c == 't' || stop)
            {
                mvprintw(9, 0, "dices are chosen                        ");
                for(int i = 0; i < 5; i ++) diceN[i]++;
                move(0, 0);
                break;
            }
        }
}

void scorechoice(int source[12])
{

    int choice = 0;

        
        while(1)
        {
            mvprintw(8, 0, "Make a right choice! : ");
            refresh();

            echo();
            scanw("%d", &choice);
            noecho();

            if(choice < 1 || 12 < choice)
            {
                mvprintw(9, 0, "Choose the right number!");
                refresh();
                sleep(1);
                mvprintw(9, 0, "                         ");
                refresh();
                continue;
            }

            if(!scorecheck[--choice])
            {
                scoreboard[choice] = source[choice];
                scorecheck[choice]++;
                break;
            }
            else
            {
                mvprintw(9, 0, "%d is alreaday taken", choice+1);
                refresh();
                sleep(1);
                mvprintw(9, 0, "                         ");
                continue;
            }

        }
}





void singlegame()
{
    while(turn--)
    {
        cnt = 0; stop = 0; bnstot = 0;
        for(int i = 0; i < 5; i++) spin[i] = 1, diceN[i] = 0;
        for(int i = 0; i < 7; i++) diceInd[i] = 0;
        clear(); refresh(); crmode(); 
        noecho();


        signal(SIGQUIT, SIG_IGN);

        signal(SIGALRM, shake);

        set_ticker(50);

        diceset();

        signal(SIGALRM, SIG_IGN);
        nocbreak();
        
        for(int i = 0; i < 5; i++) diceInd[diceN[i]]++;
        
        int source[12] = { 0, };
        
        setLIST(source, diceInd);

        move(0, 0);
        empty(source);
        
        sleep(2);

        mvprintw(9, 0, "                        ");


        scorechoice(source);


        clear();
        bnstot++;
        move(0, 0);
        empty(scoreboard);
        refresh();
        sleep(3);
    }
    game_end();
    refresh();
}

int intro()
{
    noecho();
    draw();

    while(1){
        int n = keyctrl();
        if(n != UP && n != DOWN && n != SUBMIT){
            draw();
            if(choice == 1){
                move(trow+5, (col - strlen(single))/2);
                printw(">");
                choice = 1;
            }
            else if(choice == 2){
                move(trow+6, (col - strlen(single))/2);
                printw(">");
            }
        }
        switch(n){
            case UP:{
                if(choice == 0){
                    move(trow+5, (col - strlen(single))/2);
                    printw(">");
                    choice = 1;
                    break;
                }
                else{
                    move(trow+6, (col - strlen(single))/2 - 2);
                    printw("  -O");
                    move(trow+5, (col - strlen(single))/2);
                    printw(">");
                    choice = 1;
                    break;
                }
            }
            case DOWN:{
                if(choice == 0){
                    move(trow+6, (col - strlen(single))/2);
                    printw(">");
                    choice = 2;
                    break;
                }
                else{
                    move(trow+5, (col - strlen(single))/2 - 2);
                    printw("  -S");
                    move(trow+6, (col - strlen(single))/2);                    
                    printw(">");
                    choice = 2;
                    break;
                }
            }
            case SUBMIT:{
                if(choice == 10){
                    choice = 0;
                }
                else{
                    choice += 10;
                }
                break;
            }
        }
        if(choice > 10){
            break;
        }
    }
    echo();
    choice -= 10;
    if(choice == 1)
    {
        return 0;
    }
    else if(choice == 2)
    {
        return 1;
    }
}

void multiend(int source1[12], int source2[12])
{
    int score1 = 0, score2 = 0;
    move(0, 0);
    score2 = empty(source2);
    mvprintw(8, 0, "<opponent's final scoreboard>");
    move(9, 0);
    score1 = empty(source1);
    mvprintw(17, 0, "<your final scoreboard>");
    refresh();
    sleep(2);

    if(score1 > score2)
    {
        mvprintw(18, 0, "you win!");
    }
    else if(score1 == score2)
    {
        mvprintw(18, 0, "draw!");
    }
    else
    {
        mvprintw(18, 0, "you lose!");
    }
    refresh();
    sleep(5);
    game_end();
}

void multigame()
{
    clear(); //noecho();
    printw("ip: ");
    refresh();
    scanw("%s", ipnum);

    int source2[12];

    int order  = 0;

    int clnt_sock;
    struct sockaddr_in clnt_sock_addr ,recv_sock_addr;
    char msg[30];

    int str_len = 0;

    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&clnt_sock_addr, 0, sizeof(clnt_sock_addr));
    clnt_sock_addr.sin_family = AF_INET;
    clnt_sock_addr.sin_addr.s_addr = inet_addr(ipnum);
    clnt_sock_addr.sin_port = htons(PORT);

    if(connect(clnt_sock, (struct sockaddr*)&clnt_sock_addr, sizeof(clnt_sock_addr)) == -1){
        perror("connct");
        echo();
        exit(1);
    }
    
    int c;
    
    char con;

    read(clnt_sock, &order, sizeof(order));
    mvprintw(2, 0, "waiting for oppenent");
    refresh();
    if(order == 1){
        read(clnt_sock, &con, sizeof(con));
    }
    else if(order == 0)
    {
        printw("full member\n");
        echo();
        exit(1);
    }
    
    crmode();  
    while(turn--)
    {   
        
        cnt = 0; stop = 0; bnstot = 0;
        for(int i = 0; i < 5; i++) spin[i] = 1, diceN[i] = 0;
        for(int i = 0; i < 7; i++) diceInd[i] = 0;
        clear(); refresh();  
        noecho();
        if(order != 1)
        {   

            memset(source2, 0, sizeof(source2));
            clear();
            mvprintw(0, 0, "waiting....");
            refresh();
             str_len = read(clnt_sock, source2, sizeof(source2));
            if(str_len != sizeof(source2)){
                str_len = read(clnt_sock, source2, sizeof(source2));
                //printf("Error \n");
                //exit(1);
            }
            bnstot++;
            move(0, 0);
            empty(source2);
            mvprintw(8, 0, "<opponent's scoreboard>");
            move(9, 0);
            empty(scoreboard);
            mvprintw(17, 0, "<your scoreboard>");
            refresh();
            sleep(3);
            clear();
        }

        signal(SIGQUIT, SIG_IGN);

        signal(SIGALRM, shake);

        set_ticker(50);

        crmode();
        bnstot = 0;
        tcflush(0, TCIFLUSH);
        diceset();

        signal(SIGALRM, SIG_IGN);
        nocbreak();
        
        for(int i = 0; i < 5; i++) diceInd[diceN[i]]++;
        
        int source[12] = { 0, };
        

        setLIST(source, diceInd);

        move(0, 0);
        empty(source);  //print
        

        sleep(2);

        mvprintw(9, 0, "                        ");


        scorechoice(source);    //select

        move(0, 0);
        empty(scoreboard);

        write(clnt_sock, scoreboard, sizeof(scoreboard));
        memset(source, 0, sizeof(source));
        
        
        if(order == 1)
        {

            memset(source2, 0, sizeof(source2));
            clear();
            mvprintw(0, 0, "waiting....");
            refresh();

            str_len = read(clnt_sock, source2, sizeof(source2));
            if(str_len != sizeof(source2)){
                str_len = read(clnt_sock, source2, sizeof(source2));
                //printf("Error \n");
                //exit(1);
            }
            
            clear();
            bnstot++;
            move(0, 0);
            empty(source2);
            mvprintw(8, 0, "<opponent's scoreboard>");
            move(9, 0);
            empty(scoreboard);
            mvprintw(17, 0, "<your scoreboard>");
            refresh();
            sleep(3);
            clear();
        }
        
    }
    clear();

    bnstot = 1;

    multiend(scoreboard, source2);
}



int main(void)
{ 
    srand(time(NULL));

    initscr(); 
    int mode = intro();

    if(mode == 0)
    {
        singlegame();
    }  
    else
    {
        multigame();
    }

    endwin();

    exit(0);
}