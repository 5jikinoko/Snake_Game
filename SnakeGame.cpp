/*コンパイルは g++ SnakeGame.cpp -lncursesw -o [任意の実行ファイル名]*/
#include<ncurses.h>
#include<queue>
#include<queue>
#include<cmath>
#include<random>
#include<vector>
#include<locale.h>

//下記の変数は自由に書き換えてOK
//ただし、widthに対してbaby_snakesizeが大きいとバグるかも
const int width = 15;
const int height = 10;
const int maxeggnum=3;
const int baby_snakesize=2;
//ここまで書き換え可


int eggnum=0;
int snakesize;

enum status{ emp=0, body, egg};
enum color{ WALL_COLOR=1, BODY_COLOR, RED, EGG_COLOR};

class Point{
public:
    int x;
    int y;
    Point(int xx, int yy) :
        x(xx), y(yy){};
    Point() :
        x(1), y(1){};
};

void draw_flame(){
    attrset(COLOR_PAIR(WALL_COLOR));
    for(int x=0; x <= width*2; ++x){
        mvprintw(0, x, " ");
        mvprintw(height*2, x, " ");
    }
    for(int y=1; y<height*2; ++y){
        mvprintw(y, 0, " ");
        mvprintw(y, width*2, " ");
    }
    attrset(COLOR_PAIR(0));
    mvprintw(0, width*2+1, "方向転換:矢印キー");
    mvprintw(1, width*2+1, "終了:Shift+Q");
    mvprintw(2, width*2+1, "リスタート:Shift+R");
}

//頭が壁か体に衝突してるならtrueを返す。　そうでなければfalseを返す
bool collision_detection(Point head, char field_status[height][width]){
    if(head.x<=0 || head.x>=width*2 || head.y<=0 || head.y>=height*2)
        return true;
    if(field_status[(head.y-1)/2][(head.x-1)/2]==body) return true;
    else return false; 
}

//卵を食べてたら食べた卵の番号を返す。　そうでなければ-1を返す
int bite_detection(Point head, std::vector<Point>& eggs){
    for(int i=0; i<maxeggnum; ++i){
        if(head.x==(eggs.at(i).x*2 +1) && head.y==(eggs.at(i).y*2 +1)){
            return i;
        }
    }
    return -1;
}

//空いてるマスにランダムで新しい卵を設置
void set_egg(std::vector<Point>& eggs, char field_status[height][width], int target){
    static std::random_device rd;
    static std::mt19937 mt(rd());
    if(eggnum<=0) return;
    int spaces = width*height - snakesize - eggnum;
    if(spaces<=0){
        --eggnum;
        eggs.at(target).x = eggs.at(target).y = -1;
        return;
    }
    int j = mt()%spaces;
    for(int y=0; y<height; ++y){
        for(int x=0; x<width; ++x){
            if(field_status[y][x]==emp){
                if(--spaces==j){
                    eggs.at(target).x = x;
                    eggs.at(target).y = y;
                    field_status[y][x] = egg;
                    attrset(COLOR_PAIR(EGG_COLOR));
                    mvprintw(y*2+1, x*2+1, " ");
                    return;
                }
            }
        }
    }
}

bool move_snake(std::queue<Point>& snake, std::vector<Point>& eggs, char field_status[height][width], int key){
    //移動後の先頭がhead2
    Point head1 = snake.back();
    Point head2 = head1;
    switch(key){
        case KEY_LEFT : head1.x -= 1; head2.x -= 2; break;
        case KEY_RIGHT: head1.x += 1; head2.x += 2; break;
        case KEY_UP   : head1.y -= 1; head2.y -= 2; break;
        case KEY_DOWN : head1.y += 1; head2.y += 2; break;
    }
    int bitten_egg = bite_detection(head2, eggs);

    if(bitten_egg==-1){
        //卵を食べていなかったら尻尾を消去して、該当する位置のfield_statusをempに
        attrset(COLOR_PAIR(0));
        mvprintw(snake.front().y, snake.front().x, " ");
        snake.pop();
        field_status[(snake.front().y-1)/2][(snake.front().x-1)/2] = emp;
        mvprintw(snake.front().y, snake.front().x, " ");
        snake.pop();
        //衝突してたら頭を赤くしてfalseを返す。　ゲームオーバーへ
        if(collision_detection(head2, field_status)){
            //赤
            attrset(COLOR_PAIR(RED));
            mvprintw(head1.y, head1.x, " ");
            return false;
        }
    }else{
        //卵を食べていたら尻尾は消さず、snakesizeをインクリメントして新しい卵を設置
        set_egg(eggs, field_status, bitten_egg);
        ++snakesize;
    }
    snake.push(head1);
    snake.push(head2);
    field_status[(head2.y-1)/2][(head2.x-1)/2] = body;
    attrset(COLOR_PAIR(BODY_COLOR));
    mvprintw(head1.y, head1.x, " ");
    mvprintw(head2.y, head2.x, " ");
    return true;
}

void set_color(){
    start_color();
    init_pair(WALL_COLOR, COLOR_WHITE, COLOR_WHITE);
    init_pair(BODY_COLOR, COLOR_GREEN, COLOR_GREEN);
    init_pair(RED, COLOR_RED, COLOR_RED);
    init_pair(EGG_COLOR, COLOR_YELLOW, COLOR_YELLOW);
}

int main(){
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, true);
    if(COLS<=18+width*2 || LINES <= height*2){
        endwin();
        puts("画面の大きさが足りません");
        return 0;
    }
    set_color();
    int key, before_key;
    int frequency;
    char field_status[height][width];
START:
{   
    erase();
    draw_flame();
    frequency = 100000;
    timeout(frequency);
    snakesize = baby_snakesize;
    eggnum = maxeggnum;
    for(int y=0; y<height; ++y){
        for(int x=0; x<width; ++x) field_status[y][x]=emp;
    }
    std::vector<Point> eggs(maxeggnum,Point(-1,-1));
    std::queue<Point> snake;
    attrset(COLOR_PAIR(BODY_COLOR));
    int s = baby_snakesize%2;
    int w = (width/2)%2;
    int h = (height/2)%2;
    //ヘビの実態は奇数の部分
    int loop_count=0;
    for(int i=-(baby_snakesize+s)+2; i<baby_snakesize-s+2; ++i){
        snake.push(Point((width/2)-w+i, (height/2)-h+1));
        if(loop_count++%2==1) field_status[((height/2)-h+1-1)/2][((width/2)-w+i-1)/2]=body;
        mvprintw(snake.back().y, snake.back().x, " ");
    }
    for(int i=0; i<maxeggnum; ++i){
        set_egg(eggs, field_status, i);
    }
    //矢印キーを受け取るまでループ
    while(true){
        key = before_key = getch();
        if(key==KEY_DOWN || key==KEY_UP || key==KEY_RIGHT || key==KEY_LEFT)
            break;
    }
    while(true){
        switch(key){
            case 'Q' : goto QUIT;
            case 'R' : goto START;
            case KEY_LEFT : if(before_key==KEY_RIGHT) key = before_key; 
                            else before_key = key;
                            break; 
            case KEY_RIGHT: if(before_key==KEY_LEFT) key = before_key;
                            else before_key = key;
                            break;
            case KEY_UP   : if(before_key==KEY_DOWN) key = before_key;
                            else before_key = key;
                            break;
            case KEY_DOWN : if(before_key==KEY_UP) key = before_key;
                            else before_key = key;
                            break;
            default : key = before_key; break;
        }
        if(!move_snake(snake, eggs, field_status, key)){
            attrset(COLOR_PAIR(0));
            mvprintw(7, width*2+1, "ゲームオーバー");
            timeout(100000);
            while(true){
                key = getch();
                if(key=='Q') goto QUIT;
                else if(key=='R') goto START;
            }
        }
        attrset(COLOR_PAIR(0));
        mvprintw(5, width*2+1, "得点:%d", snakesize -baby_snakesize);
        if(width*height==snakesize){
            attrset(COLOR_PAIR(0));
            mvprintw(7, width*2+1, "ゲームクリア！");
            timeout(100000);
            while(true){
                key = getch();
                if(key=='Q') goto QUIT;
                else if(key=='R') goto START;
            }
        }
        key = getch();
        if(frequency>=100) frequency -= 1;
        timeout(frequency);
    }
}
QUIT:
    endwin();
}