/*
2018110881 정소희
2017115991 김미정
2018112842 이단비
*/

#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <termios.h>

#define INPUT_UP 65
#define INPUT_DOWN 66
#define INPUT_RIGHT 67
#define INPUT_LEFT 68
#define INPUT_ENTER 13
#define EXIT 113

static struct termios initial_settings, new_settings;

static int peek_character = -1;

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int _kbhit()
{
    unsigned char ch;
    int nread;

    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int _getch()
{
    char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

int _putch(int c)
{
    putchar(c);
    fflush(stdout);
    return c;
}

void background()
{
        int i;

        mvprintw(0,0,"+-------------------+\n");

        for(i=1; i<22; i++)
        {
                mvprintw(i,0,"!                   !\n");
        }
        mvprintw(22,0,"+-------------------+\n");

        for(i=23; i<26; i++)
        {
                mvprintw(i,0,"!                   !\n");
        }
       mvprintw(26,0,"+-------------------+\n");
}

int input_key()
{
        int point = 0;
        init_keyboard();

        if (_kbhit()) point = _getch();

        close_keyboard();

        return point;
}

int menu()
{
        int input =  input_key();
        static int pic = 12;

        noecho();

        background();

        mvprintw(8, 3, "WALLS AVOIDANCE");

        mvprintw(12, 5, "START");
        mvprintw(14, 5, "EXIT");
        mvprintw(24, 3, "MADE BY Primae");
        mvprintw(pic, 11, "<-");

        switch(input)
        {
                case INPUT_UP :
                        clear();
                        if(pic > 12) pic-=2;
                        break;
                case INPUT_DOWN :
                        clear();
                        if(pic < 14) pic+=2;
                        break;
                case INPUT_ENTER :
                        return pic;
                        break;
                default :
                        break;
        }

        refresh();
}

int menu_end()
{
        int input = input_key();
        static int pic = 14;

        noecho();

        background();

        mvprintw(8, 3, "WALLS AVOIDANCE");
        mvprintw(14, 5, "EXIT");
        mvprintw(24, 5, "MADE BY Primae");
        mvprintw(pic, 11, "<-");

        switch (input)
        {
                case INPUT_UP :
                        if (pic > 14) pic -= 2;
                        break;
                case INPUT_DOWN :
                        if (pic < 16) pic += 2;
                        break;
                case INPUT_ENTER :
                         return pic;
                         break;
                 default:
                        break;
        }

        refresh();
}

int main()
{
        int end = 0;
        int num;

        initscr();

        while(1004)
        {
                if(end != -1) num = menu();
                else if(end == -1) num = menu_end();

                if(num == 12)
                {
                        clear();
                }
                else if(num == 14)
                {
                        clear();
                        background();
                        mvprintw(11, 2, "Thank you for play");
                        mvprintw(24, 3, "MADE BY Primae");
                        refresh();
                        sleep(1);
                        break;
                }
        }

        endwin();
}
