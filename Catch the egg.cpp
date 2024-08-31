#include "iGraphics.h"
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

char bg[10][50] = {"assets/homescreen.bmp", "assets/gamescreen.bmp", "assets/timeselectscreen.bmp", "assets/quitscreen.bmp", "assets/gobackfromresume.bmp"};
char stickpic[10][50] = {"assets/stick.bmp"};
char henpic[3][50] = {"assets/hen.bmp", "assets/hen.bmp", "assets/hen.bmp"};
char eggpic[4][50] = {"assets/egg.bmp", "assets/regg.bmp", "assets/gegg.bmp", "assets/poop.bmp"};
char basketpic[10][50] = {"assets/basket2.bmp", "assets/basket0.bmp", "assets/basket1.bmp"};
char bonuspic[10][50] = {"assets/plus.bmp", "assets/minus.bmp", "assets/elec.bmp", "assets/time.bmp"};
int points[4] = {1, 5, 10, -1};
int stick_x = 250, stick_y = 500;
int henx[3] = {0, 0, 0}, heny[3] = {510, 0, 0};
int basketx = 650, baskety = 0, basket_idx = 2;
int basket_timer = 0, basket_range = 150; // to change the basket image
int is_paused = 0;
int egg_timer = 0, egg_dx = 0, egg_dy = 20, egg_idx = 0;
int score = 0;
int bonus_timer = 0, bonus_dx = 0, bonus_dy = 20;
int game_time = 120;
char name[80] = "";
int name_len = 0;
int game_over = 0;
int game_level = 1; // 1 for 2 min level 1, 2 for 2 min level 2, 3 for 3 min level 1, 4 for 3 min level 2
int highscore_selected_level = 1;
int score_bonus_timer = 0;

int menu = 0;
int hen_idx = 1;

// pendulum motion variables
double theta = 1;                 // angle
double omega = 0;                 // omega
double alpha = 0;                 // alpha
double length = 400;              // length
double gravity = 4;               // g
int pivot_x = 650, pivot_y = 600; // pivot coord

typedef struct vector
{
    double x;
    double y;
} vector;

typedef struct egg
{
    vector pos;
    int state;
    int type;
} egg;

typedef struct bonus
{
    vector pos;
    int state;
    int type;
} bonus;

typedef struct highscore
{
    char name[80];
    int score;
} highscore;

egg eggs[100];
bonus b;
FILE *hs;

void save_game_over()
{
    FILE *file = fopen("game_over.txt", "w");
    fprintf(file, "%d", game_over);
    fclose(file);
}

void read_game_over()
{
    FILE *file = fopen("game_over.txt", "r");
    fscanf(file, "%d", &game_over);
    fclose(file);
}

void save_resume()
{
    FILE *file = fopen("resume.txt", "w");
    if (file == NULL)
    {
        printf("Error: Could not open file for writing.\n");
        return;
    }

    fprintf(file, "%d %d\n", stick_x, stick_y);
    for (int i = 0; i < 3; i++)
    {
        fprintf(file, "%d %d\n", henx[i], heny[i]);
    }
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", basketx, baskety, basket_idx, basket_timer, basket_range, is_paused, egg_timer, egg_dx, egg_dy, egg_idx, score, bonus_timer, bonus_dx, bonus_dy, game_time, name_len);
    for (int i = 0; i < 80; i++)
    {
        fprintf(file, "%c", name[i]);
    }
    fprintf(file, "\n");
    for (int i = 0; i < 100; i++)
    {
        fprintf(file, "%lf %lf %d\n", eggs[i].pos.x, eggs[i].pos.y, eggs[i].state);
    }
    fprintf(file, "%lf %lf %d\n", b.pos.x, b.pos.y, b.state);
    fprintf(file, "%d %d\n", game_level, score_bonus_timer);
    // write hen_idx
    fprintf(file, "%d\n", hen_idx);

    fclose(file);
}

void read_resume()
{
    FILE *file = fopen("resume.txt", "r");
    if (file == NULL)
    {
        printf("Error: Could not open file for reading.\n");
        return;
    }

    fscanf(file, "%d %d", &stick_x, &stick_y);
    for (int i = 0; i < 3; i++)
    {
        fscanf(file, "%d %d", &henx[i], &heny[i]);
    }
    fscanf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &basketx, &baskety, &basket_idx, &basket_timer, &basket_range, &is_paused, &egg_timer, &egg_dx, &egg_dy, &egg_idx, &score, &bonus_timer, &bonus_dx, &bonus_dy, &game_time, &name_len);
    for (int i = 0; i < 80; i++)
    {
        fscanf(file, " %c", &name[i]);
    }
    for (int i = 0; i < 100; i++)
    {
        fscanf(file, "%lf %lf %d", &eggs[i].pos.x, &eggs[i].pos.y, &eggs[i].state);
    }
    fscanf(file, "%lf %lf %d", &b.pos.x, &b.pos.y, &b.state);
    fscanf(file, "%d %d", &game_level, &score_bonus_timer);
    // read hen_idx
    fscanf(file, "%d", &hen_idx);

    fclose(file);
}

// mm:ss
char *convert_time(int time)
{
    char *time_str = (char *)malloc(10 * sizeof(char));
    int minutes = time / 60;
    int seconds = time % 60;
    sprintf(time_str, "%02d:%02d", minutes, seconds);
    return time_str;
}

void reset_all()
{
    // reset all
    stick_x = 250, stick_y = 500;
    henx[0] = 0, heny[0] = 510;
    henx[1] = 0, heny[1] = 510;
    henx[2] = 0, heny[2] = 0;
    basketx = 650, baskety = 0, basket_idx = 2;
    basket_timer = 0, basket_range = 150;
    is_paused = false;
    egg_timer = 0, egg_dx = 0, egg_dy = 20, egg_idx = 0;
    score = 0;
    bonus_timer = 0, bonus_dx = 0, bonus_dy = 20;
    game_time = 10;

    for (int i = 0; i < 80; i++)
    {
        name[i] = '\0';
    }
    name_len = 0;
    for (int i = 0; i < 100; i++)
    {
        eggs[i].state = 0;
    }
    b.state = 0;
    menu = 0;
    game_over = 0;
    game_level = 1;
    score_bonus_timer = 0;
}

void henmove()
{
    if (menu == 2 && !is_paused)
    {
        int random_offset = rand() % 300;
        if (rand() % 2 == 0)
        {
            henx[0] = random_offset;
        }
        else
        {
            henx[0] = -random_offset;
        }

        henx[0] = 670 + henx[0];
    }
}

void henmove2()
{
    if (menu == 2 && !is_paused)
    {
        alpha = -(gravity / (length * 1.0)) * sin(theta);
        omega += alpha;
        theta += omega;

        henx[1] = pivot_x + length * sin(theta);
        heny[1] = pivot_y - length * cos(theta);
    }
}

void eggmove()
{
    if (menu == 2)
    {
        if (!is_paused)
        {
            egg_timer += 10;
        }
        else
        {
            egg_timer = 10;
        }

        int interval = 100;
        if (egg_timer % interval == 0)
        {
            egg_idx = (egg_idx + 1) % 100;

            vector hen_pos = {henx[hen_idx], heny[hen_idx]};
            eggs[egg_idx].pos = hen_pos;
            eggs[egg_idx].state = 1;
            eggs[egg_idx].type = rand() % 4;
        }

        // egg_dx = -50 + rand() % 100;

        for (int i = 0; i < 100; i++)
        {
            if (!is_paused && eggs[i].state == 1)
            {
                eggs[i].pos.y -= egg_dy;
                eggs[i].pos.x -= egg_dx;
            }
            if (eggs[i].pos.y < 50)
            {
                eggs[i].state = 0;
            }
            if (eggs[i].state == 1)
            {
                if (eggs[i].pos.x >= basketx - 20 && eggs[i].pos.x <= basketx + basket_range - 20 && eggs[i].pos.y >= baskety && eggs[i].pos.y <= baskety + 100)
                {
                    eggs[i].state = 0;

                    if (score_bonus_timer > 0)
                    {
                        printf("score bonus %d\n", score_bonus_timer);
                        score += 2 * points[eggs[i].type];
                    }
                    else
                    {
                        score += points[eggs[i].type];
                    }
                }
            }
        }
    }
}

void bonus_effect()
{

    if (b.type == 0 && b.pos.x >= basketx - 20 && b.pos.x <= basketx + basket_range - 20 && b.pos.y >= baskety && b.pos.y <= baskety + 100 && b.state == 1)
    {
        basket_idx = 0;
        basket_range = 200;
        b.state = 0;
    }
    if (b.type == 1 && b.pos.x >= basketx - 20 && b.pos.x <= basketx + basket_range - 20 && b.pos.y >= baskety && b.pos.y <= baskety + 100 && b.state == 1)
    {
        basket_idx = 1;
        basket_range = 100;
        b.state = 0;
    }

    // change the basket_idx to 2 after a certain time
    if (basket_idx == 0 || basket_idx == 1)
    {
        basket_timer += 10;
        if (basket_timer >= 10000)
        {
            basket_idx = 2;
            basket_range = 150;
            basket_timer = 0;
        }
    }

    // score bonus
    if (b.type == 2 && b.pos.x >= basketx - 20 && b.pos.x <= basketx + basket_range - 20 && b.pos.y >= baskety && b.pos.y <= baskety + 100 && b.state == 1)
    {
        b.state = 0;
        score_bonus_timer += 10;
    }

    if (score_bonus_timer > 0)
    {
        score_bonus_timer += 10;
        if (score_bonus_timer >= 10000)
        {
            score_bonus_timer = 0;
            b.state = 1;
        }
    }

    // time bonus
    if (b.type == 3 && b.pos.x >= basketx - 20 && b.pos.x <= basketx + basket_range - 20 && b.pos.y >= baskety && b.pos.y <= baskety + 100 && b.state == 1)
    {
        game_time += 2;
        b.state = 0;
    }
}

void bonusmove()
{
    if (!is_paused && menu == 2)
    {
        bonus_timer += 10;
    }
    int interval = 5000;
    if (bonus_timer % interval == 0)
    {
        vector bonus_pos = {300 + rand() % 600, 750 + rand() % 200};
        b.pos = bonus_pos;
        b.state = 1;
        b.type = rand() % 4;
    }
    if (b.state == 1 && !is_paused)
    {
        b.pos.y -= bonus_dy;
    }
}

// check if score is highscore
int check_highscore(int current_score, int game_level)
{
    FILE *file;
    if (game_level == 1)
    {
        file = fopen("hs1.txt", "r");
    }
    else if (game_level == 2)
    {
        file = fopen("hs2.txt", "r");
    }
    else if (game_level == 3)
    {
        file = fopen("hs3.txt", "r");
    }
    else if (game_level == 4)
    {
        file = fopen("hs4.txt", "r");
    }
    if (file == NULL)
    {
        return 1;
    }

    highscore hs[5];
    int count = 0;

    while (fscanf(file, "%s %d", hs[count].name, &hs[count].score) != EOF)
    {
        count++;
    }
    fclose(file);

    if (count < 5)
    {
        return 1;
    }

    if (current_score > hs[count - 1].score)
    {
        return 1;
    }

    return 0;
}

void save_highscore(const char *name, int current_score, int game_level)
{
    highscore hs[6];
    int count = 0;

    FILE *file;

    if (game_level == 1)
    {
        file = fopen("hs1.txt", "r");
    }
    else if (game_level == 2)
    {
        file = fopen("hs2.txt", "r");
    }
    else if (game_level == 3)
    {
        file = fopen("hs3.txt", "r");
    }
    else if (game_level == 4)
    {
        file = fopen("hs4.txt", "r");
    }
    if (file != NULL)
    {
        while (fscanf(file, "%s %d", hs[count].name, &hs[count].score) != EOF)
        {
            count++;
        }
        fclose(file);
    }
    strcpy(hs[count].name, name);
    hs[count].score = current_score;
    count++;

    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (hs[j].score < hs[j + 1].score)
            {
                highscore temp = hs[j];
                hs[j] = hs[j + 1];
                hs[j + 1] = temp;
            }
        }
    }
    if (game_level == 1)
    {
        file = fopen("hs1.txt", "w");
    }
    else if (game_level == 2)
    {
        file = fopen("hs2.txt", "w");
    }
    else if (game_level == 3)
    {
        file = fopen("hs3.txt", "w");
    }
    else if (game_level == 4)
    {
        file = fopen("hs4.txt", "w");
    }
    for (int i = 0; i < count && i < 5; i++)
    {
        fprintf(file, "%s %d\n", hs[i].name, hs[i].score);
    }
    fclose(file);
}

void timer()
{
    if (menu == 2)
    {
        if (!is_paused)
        {
            game_time -= 1;
        }
        if (game_time == 0)
        {

            if (check_highscore(score, game_level))
            {
                menu = 3;
            }
            else
            {
                menu = 4;
            }

            // clear the resume file
            FILE *file = fopen("resume.txt", "w");
            fclose(file);

            // clear the game_over file
            file = fopen("game_over.txt", "w");
            fclose(file);

            game_over = 1;
        }
    }
}

void iDraw()
{
    // place your drawing codes here
    if (menu == 0)
    {
        iClear();
        iShowBMP(0, 0, bg[0]);
    }
    else if (menu == 1)
    {
        iClear();
        iShowBMP(0, 0, bg[2]);
    }
    else if (menu == 2)
    {
        iClear();
        iShowBMP(0, 0, bg[1]);
        iShowBMP2(stick_x, stick_y, stickpic[0], 0);

        iShowBMP2(henx[hen_idx], heny[hen_idx], henpic[0], 0);

        if (hen_idx == 1)
        {
            iSetColor(0, 255, 0);
            for (int i = 0; i < 100; i++)
            {
                iLine(henx[hen_idx] - 50 + 0.1 * i, heny[hen_idx], pivot_x + 0.1 * i, pivot_y - 60);
                iLine(henx[hen_idx] + 150 + 0.1 * i, heny[hen_idx], pivot_x + 0.1 * i, pivot_y - 60);
            }
            iSetColor(255, 0, 0);
            iFilledRectangle(henx[hen_idx] - 50, heny[hen_idx], 200, 5);
        }

        iSetColor(0, 0, 0);
        iText(1100, 620, "SCORE :", GLUT_BITMAP_TIMES_ROMAN_24);
        printf("basket idx %d\n", basket_idx);
        iShowBMP2(basketx, baskety, basketpic[basket_idx], 0);

        for (int i = 0; i < 100; i++)
        {

            if (eggs[i].state == 1)
            {
                iShowBMP2(eggs[i].pos.x, eggs[i].pos.y, eggpic[eggs[i].type], 0);
            }
        }
        char score_str[100];
        sprintf(score_str, "%d", score);
        iText(1200, 620, score_str, GLUT_BITMAP_TIMES_ROMAN_24);

        if (b.state == 1)
        {
            iShowBMP2(b.pos.x, b.pos.y, bonuspic[b.type], 0);
        }
        bonus_effect();
        char *time_str = convert_time(game_time);
        iText(50, 620, time_str, GLUT_BITMAP_TIMES_ROMAN_24);
        if (is_paused)
        {
            iSetColor(255, 0, 0);
            iFilledRectangle(600, 650, 150, 50);
            iSetColor(0, 0, 0);
            iText(620, 670, "RESUME", GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else
        {
            iSetColor(255, 0, 0);
            iFilledRectangle(600, 650, 150, 50);
            iSetColor(0, 0, 0);
            iText(620, 670, "PAUSE", GLUT_BITMAP_TIMES_ROMAN_24);
        }
    }
    else if (menu == 3)
    {
        iClear();
        iShowBMP(0, 0, bg[1]);
        iSetColor(255, 0, 0);
        char score_str[100];
        sprintf(score_str, "%d", score);
        iText(300, 500, "SCORE", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(400, 500, score_str, GLUT_BITMAP_TIMES_ROMAN_24);
        iText(300, 450, "Enter Your Name.It must be less than 80 characters.Press enter to save.", GLUT_BITMAP_TIMES_ROMAN_24);
        iRectangle(300, 300, 600, 50);
        iText(350, 320, name, GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else if (menu == 4)
    {
        iClear();
        iShowBMP(0, 0, bg[1]);
        iSetColor(0, 0, 0);
        iText(600, 400, "You have scored", GLUT_BITMAP_TIMES_ROMAN_24);
        char score_str[100];
        sprintf(score_str, "%d", score);
        iText(600, 300, score_str, GLUT_BITMAP_TIMES_ROMAN_24);
        iText(600, 200, "Press Enter to continue", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else if (menu == 5)
    {
        iClear();
        iShowBMP(0, 0, bg[1]);
        iSetColor(0, 0, 0);
        iText(600, 400, "High Scores", GLUT_BITMAP_TIMES_ROMAN_24);
        highscore hs[5];
        FILE *file;
        if (highscore_selected_level == 1)
        {
            file = fopen("hs1.txt", "r");
        }
        else if (highscore_selected_level == 2)
        {
            file = fopen("hs2.txt", "r");
        }
        else if (highscore_selected_level == 3)
        {
            file = fopen("hs3.txt", "r");
        }
        else if (highscore_selected_level == 4)
        {
            file = fopen("hs4.txt", "r");
        }
        if (file != NULL)
        {
            int count = 0;
            while (fscanf(file, "%s %d", hs[count].name, &hs[count].score) != EOF)
            {
                count++;
            }
            fclose(file);

            for (int i = 0; i < count; i++)
            {
                char score_str[100];
                sprintf(score_str, "%d", hs[i].score);
                iText(600, 300 - i * 50, hs[i].name, GLUT_BITMAP_TIMES_ROMAN_24);
                iText(800, 300 - i * 50, score_str, GLUT_BITMAP_TIMES_ROMAN_24);
            }
        }
    }
    else if (menu == 6)
    {
        iClear();
        iShowBMP(0, 0, bg[3]);
    }
    else if (menu == 7)
    {
        iClear();
        iShowBMP(0, 0, bg[4]);
    }
    else if (menu == 20)
    {
        iClear();
        iShowBMP(0, 0, bg[1]);

        iSetColor(0, 0, 0);

        iText(150, 600, "Game Instructions", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 550, "Chickens are moving on a bark and laying eggs. There is a basket at the bottom. You can move it horizontally", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 520, "using the left and right arrow keys. You have to catch the eggs in the basket.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 490, "There are 4 types of eggs: 1 point egg, 5 point egg, 10 point egg, and -1 point egg.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 460, "There are 4 types of bonuses: +1 basket range, -1 basket range, 2x score, and +5 seconds.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 430, "You have to catch the bonuses to get the effects.", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(100, 370, "Bonus Details", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(100, 340, "+1 basket range: Increases the range of the basket.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 310, "-1 basket range: Decreases the range of the basket.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 280, "2x score: Doubles the score for 10 seconds.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(100, 250, "+2 seconds: Increases the time by 2 seconds.", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(100, 200, "Grab as many eggs as possible in the given time.", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(100, 150, "Press Esc to go back to the main menu.", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    else if (menu == 55)
    {
        iClear();
        iShowBMP(0, 0, bg[2]);
    }
}

/*
    function iMouseMove() is called when the user presses and drags the mouse.
    (mx, my) is the position where the mouse pointer is.
*/

void iMouseMove(int mx, int my)
{
    // place your codes here
}

/*
    function iMouse() is called when the user presses/releases the mouse.
    (mx, my) is the position where the mouse pointer is.
*/
void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (menu == 0)
        {
            if (mx >= 540 && mx <= 725 && my >= 540 && my <= 610)
            {
                // resume the game

                read_game_over();

                if (game_over == 0)
                {
                    read_resume();
                    printf("lol\n");
                    is_paused = 0;
                    menu = 2;
                }
                else
                {
                    menu = 7;
                }
            }
            if (mx >= 520 && mx <= 750 && my >= 450 && my <= 515)
            {
                menu = 1;
            }
            if (mx >= 575 && mx <= 690 && my >= 360 && my <= 430)
            {
                menu = 6;
            }
            if (mx >= 505 && mx <= 760 && my >= 275 && my <= 345)
            {
                menu = 55;
            }

            if (mx >= 490 && mx <= 775 && my >= 185 && my <= 250)
            {
                menu = 20;
            }
        }

        else if (menu == 1)
        {
            if (mx >= 480 && mx <= 785 && my >= 450 && my <= 515)
            {
                reset_all();
                game_time = 120;
                hen_idx = 0;
                menu = 2;
                game_level = 1;
            }

            if (mx >= 480 && mx <= 785 && my >= 360 && my <= 425)
            {
                reset_all();
                game_time = 120;
                hen_idx = 1;
                menu = 2;
                game_level = 2;
            }

            if (mx >= 480 && mx <= 785 && my >= 275 && my <= 345)
            {
                reset_all();
                game_time = 180;
                hen_idx = 0;
                menu = 2;
                game_level = 3;
            }

            if (mx >= 480 && mx <= 785 && my >= 185 && my <= 250)
            {
                reset_all();
                game_time = 180;
                hen_idx = 1;
                menu = 2;
                game_level = 4;
            }
        }
        else if (menu == 2)
        {
            if (mx >= 600 && mx <= 750 && my >= 650 && my <= 700)
            {
                is_paused = !is_paused;
                printf("paused\n");
            }
        }
        else if (menu == 6)
        {
            if (mx >= 370 && mx <= 610 && my >= 325 && my <= 360)
            {
                exit(0);
            }
            if (mx >= 760 && mx <= 845 && my >= 300 && my <= 370)
            {
                menu = 0;
            }
        }
        else if (menu == 7)
        {
            if (mx >= 525 && mx <= 715 && my >= 300 && my <= 365)
            {
                menu = 0;
            }
        }
        else if (menu == 55)
        {
            if (mx >= 480 && mx <= 785 && my >= 450 && my <= 515)
            {
                highscore_selected_level = 1;
                menu = 5;
            }

            if (mx >= 480 && mx <= 785 && my >= 360 && my <= 425)
            {
                highscore_selected_level = 2;
                menu = 5;
            }

            if (mx >= 480 && mx <= 785 && my >= 275 && my <= 345)
            {
                highscore_selected_level = 3;
                menu = 5;
            }

            if (mx >= 480 && mx <= 785 && my >= 185 && my <= 250)
            {
                highscore_selected_level = 4;
                menu = 5;
            }
        }

        printf("x = %d\n", menu);
        printf("x = %d, y= %d\n", mx, my);
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
    }

    /*
        function iKeyboard() is called whenever the user hits a key in keyboard.
        key- holds the ASCII value of the key pressed.
    */
}
void iKeyboard(unsigned char key)
{

    if (menu == 3)
    {
        if (key != 8 && key != 13 && name_len < 80)
        {
            name[name_len] = key;
            name_len++;
        }
        if (key == 8 && name_len > 0)
        {
            name_len--;
            name[name_len] = '\0';
        }
        if (key == 13)
        {
            save_highscore(name, score, game_level);
            save_game_over();
            menu = 0;
        }
    }

    if (menu == 4)
    {
        if (key == 13)
        {
            menu = 0;
        }
    }
    if (key == 27)
    {
        menu = 0;
        save_game_over();
        save_resume();
        is_paused = 1;
    }
}

/*
    function iSpecialKeyboard() is called whenver user hits special keys like-
    function keys, home, end, pg up, pg down, arraows etc. you have to use
    appropriate constants to detect them. A list is:
    GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
    GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
    GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
    GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
*/
void iSpecialKeyboard(unsigned char key)
{

    if (key == GLUT_KEY_END)
    {
    }
    // place your codes for other keys here
    if (key == GLUT_KEY_UP)
    {
    }

    if (key == GLUT_KEY_DOWN)
    {
    }

    if (key == GLUT_KEY_LEFT)
    {
        basketx -= 20;
    }
    if (key == GLUT_KEY_RIGHT)
    {
        basketx += 20;
    }
}

int main()
{
    // place your own initialization codes here.
    iSetTimer(50, henmove2);
    iSetTimer(300, henmove);
    iSetTimer(50, eggmove);
    iSetTimer(50, bonusmove);
    iSetTimer(1000, timer);

    iInitialize(1350, 700, "Catch the egg");

    return 0;
}
