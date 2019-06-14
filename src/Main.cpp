#include <SFML/Graphics.hpp>
#include <time.h>
using namespace sf;

// 设置宽高
const int M = 20;
const int N = 10;

// 定义游戏网格
int (*field)[N] = new int[M][N];

// 定义方块、区块
struct Point
{
    int x, y;
} a[4], b[4], next[4];

// 定义下落方块的形状
int figures[7][4] =
    {
        1, 3, 5, 7, // I
        2, 4, 5, 7, // Z
        3, 5, 4, 6, // S
        3, 5, 4, 7, // T
        2, 3, 5, 7, // L
        3, 5, 7, 6, // J
        2, 3, 4, 5, // O
};

class ElementText
{
public:
    ElementText(std::string main_str)
    {
        font.loadFromFile("assets/comic.ttf");
        text.setFont(font);
        text.setString(main_str);
    }
    std::string str;
    Text text;
    Font font;
};

class TetrisLogic
{
public:
    // 判断能否移动
    static bool isPlaceable()
    {
        for (int i = 0; i < 4; i++)
        {
            if (a[i].x < 0 || a[i].x >= N || a[i].y >= M)
            {
                return false;
            }

            else if (field[a[i].y][a[i].x])
            {
                return false;
            }
        }
        return true;
    };

    // 移动
    static void move(int dx)
    {
        for (int i = 0; i < 4; i++)
        {
            b[i] = a[i];
            a[i].x += dx;
        }
        if (!isPlaceable())
        {
            for (int i = 0; i < 4; i++)
            {
                a[i] = b[i];
            }
        }
    }

    // 旋转
    static void rotate()
    {
        Point p = a[1]; //旋转中心
        for (int i = 0; i < 4; i++)
        {
            int x = a[i].y - p.y;
            int y = a[i].x - p.x;
            a[i].x = p.x - x;
            a[i].y = p.y + y;
        }
        if (!isPlaceable())
        {
            for (int i = 0; i < 4; i++)
            {
                a[i] = b[i];
            }
        }
    }

    static void init()
    {
        for (int i = 0; i < M; i++)
        {
            for (int j = 0; j < N; j++)
            {
                field[i][j] = 0;
            }
        }
    }
};

int main()
{
    srand(time(0));
    String title = "My Tetris";
    int wWidth = 600;
    int wHeight = 418;
    RenderWindow window(VideoMode(wWidth, wHeight), title);

    Texture t1, t2, t3;
    t1.loadFromFile("assets/tiles.png");
    t2.loadFromFile("assets/bg.jpg");
    t3.loadFromFile("assets/logo.png");

    Sprite block(t1), bg(t2), logo(t3);
    Font font;
    font.loadFromFile("assets/comic.ttf");

    // x方向上移动的距离
    int dx = 0;
    int colorNum = 1;
    int lineCount = 0;
    int score = 0;
    int level = 1;
    float timer = 0, delay = 0.3;
    float globalTime = 0.0;
    bool rotate = false;
    bool space = false;

    int n = rand() % 7;
    for (int i = 0; i < 4; i++)
    {
        a[i].x = figures[n][i] % 2;
        a[i].y = figures[n][i] / 2;
    }

    int m = rand() % 7;
    for (int i = 0; i < 4; i++)
    {
        next[i].x = figures[m][i] % 2;
        next[i].y = figures[m][i] / 2;
    }

    enum GameState
    {
        MENU = 0,
        RUNNING,
        PAUSE,
        GAMEOVER
    } gameState;
    gameState = MENU;
    Clock clock;

    // GAME LOOP
    while (window.isOpen())
    {
        switch (gameState)
        {

        // 主界面
        case MENU:
        {
            Event evt;
            while (window.pollEvent(evt))
            {
                if (evt.type == Event::Closed)
                {
                    window.close();
                }

                if (evt.type == Event::MouseButtonReleased)
                {
                    gameState = RUNNING;
                }
            }

            // 绘制开始游戏
            window.clear(Color::Black);
            std::string startGameText = "Click to start game!";
            ElementText startGameBtn(startGameText);
            startGameBtn.text.setPosition(155, 200);
            int fadeSpeed = 3;
            static int alpha = 255;
            static bool isFading = true;
            if (clock.getElapsedTime() > milliseconds(16))
            {
                if (isFading)
                {
                    alpha -= fadeSpeed;
                    if (alpha <= 0)
                    {
                        isFading = false;
                    }
                    startGameBtn.text.setFillColor(Color(0, 0, 0, alpha));
                }
                else
                {
                    alpha += fadeSpeed;
                    if (alpha >= UINT8_MAX)
                    {
                        sleep(milliseconds(100));
                        isFading = true;
                    }
                    startGameBtn.text.setFillColor(Color(0, 0, 0, alpha));
                }
            }
            logo.setPosition(110, 36);
            window.draw(bg);
            window.draw(logo);
            window.draw(startGameBtn.text);
            window.display();
            break;
        }
        case RUNNING:
        {
            float time = clock.getElapsedTime().asSeconds();
            clock.restart();
            timer += time;
            globalTime += timer;

            // 事件处理
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                {
                    window.close();
                }

                if (e.type == Event::KeyPressed)
                {
                    if (e.key.code == Keyboard::Up)
                        rotate = true;
                    else if (e.key.code == Keyboard::Left)
                        dx = -1;
                    else if (e.key.code == Keyboard::Right)
                        dx = 1;
                    else if (e.key.code == Keyboard::Space)
                        space = true;
                    else if (e.key.code == Keyboard::Escape)
                    {
                        gameState = PAUSE;
                    }
                }
            }

            // 直接下落
            if (space)
            {
                delay = 0;
            }

            // 加速下落
            if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                delay = 0.05;
            }

            // 移动四联块
            TetrisLogic::move(dx);

            // 旋转四联块
            if (rotate)
            {
                TetrisLogic::rotate();
            }

            // 下落
            if (timer > delay)
            {
                for (int i = 0; i < 4; i++)
                {
                    b[i] = a[i];
                    a[i].y += 1;
                    // 判断到顶 游戏结束
                    if (field[a[i].y][a[i].x] && a[i].y <= 3)
                    {
                        gameState = GAMEOVER;
                        break;
                    }
                }

                // 下落至堆顶 生成新四联块
                if (!TetrisLogic::isPlaceable())
                {
                    space = false;
                    for (int i = 0; i < 4; i++)
                    {
                        field[b[i].y][b[i].x] = colorNum;
                    }

                    colorNum = 1 + rand() % 7;
                    int n = rand() % 7;
                    for (int i = 0; i < 4; i++)
                    {
                        a[i] = next[i];
                        next[i].x = figures[n][i] % 2;
                        next[i].y = figures[n][i] / 2;
                    }
                }
                timer = 0;
            }

            // 消除行
            int lineCountOneTime = 0;
            int k = M - 1;
            for (int i = M - 1; i > 0; i--)
            {
                int count = 0;
                for (int j = 0; j < N; j++)
                {
                    if (field[i][j])
                    {
                        count++;
                    }
                    field[k][j] = field[i][j];
                }
                if (count < N)
                {
                    k--;
                }
                else if (count == N)
                {
                    lineCountOneTime++;
                }
            }

            lineCount += lineCountOneTime;

            switch (lineCountOneTime)
            {
            case 1:
            {
                score += 10;
                break;
            }
            case 2:
            {
                score += 40;
                break;
            }
            case 3:
            {
                score += 90;
                break;
            }
            case 4:
            {
                score += 160;
                break;
            }
            }

            dx = 0;
            rotate = false;
            if (score < 100)
            {
                level = 1;
                delay = 0.4;
            }
            else if (score >= 100 && score < 1000)
            {
                level = 2;
                delay = 0.3;
            }
            else if (score >= 1000 && score < 2000)
            {
                level = 3;
                delay = 0.2;
            }
            else if (score >= 2000 && score < 4000)
            {
                level = 4;
                delay = 0.15;
            }
            else
            {
                level = 5;
                delay = 0.1;
            }

            // 清空window
            window.clear(Color::Black);

            // 绘制消除行数
            auto lineText = "Lines:" + std::to_string(lineCount);
            ElementText lines(lineText);
            lines.text.setPosition(300, 18);
            window.draw(lines.text);

            // 绘制分数
            auto scoreText = "Score:" + std::to_string(score);
            ElementText score(scoreText);
            score.text.setPosition(300, 68);
            window.draw(score.text);

            // 绘制时间
            auto timeText = "Time:" + std::to_string((int)globalTime / 10);
            ElementText gTimer(timeText);
            gTimer.text.setPosition(300, 118);
            window.draw(gTimer.text);

            // 绘制等级
            auto levelText = "Level:" + std::to_string(level);
            ElementText levelElem(levelText);
            levelElem.text.setPosition(300, 168);
            window.draw(levelElem.text);

            // 绘制下一个
            ElementText nextText("Next:");
            nextText.text.setPosition(300, 268);
            window.draw(nextText.text);

            // 绘制下一个四联块
            for (int i = 0; i < 4; i++)
            {
                block.setTextureRect(IntRect(colorNum * 18, 0, 18, 18));
                block.setPosition(next[i].x * 18, next[i].y * 18);
                block.move(300, 312); //偏移
                window.draw(block);
            }

            // 绘制下落中的四联块
            for (int i = 0; i < M; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    if (field[i][j] == 0)
                    {
                        continue;
                    }
                    block.setTextureRect(IntRect(field[i][j] * 18, 0, 18, 18));
                    block.setPosition(j * 18, i * 18);
                    block.move(28, 31); //偏移
                    window.draw(block);
                }
            }

            // 绘制下落后的四联块
            for (int i = 0; i < 4; i++)
            {
                block.setTextureRect(IntRect(colorNum * 18, 0, 18, 18));
                block.setPosition(a[i].x * 18, a[i].y * 18);
                block.move(28, 31); //偏移
                window.draw(block);
            }

            // 绘制网格（Grid）
            for (int i = 0; i < N + 1; i++)
            {
                Vertex lineColumn[] = {Vertex(Vector2f(28 + 18 * i, 31)), Vertex(Vector2f(28 + i * 18, 31 + 20 * 18))};
                window.draw(lineColumn, 2, Lines);
            }

            for (int j = 0; j < M + 1; j++)
            {
                Vertex lineRow[] = {Vertex(Vector2f(28, 31 + j * 18)), Vertex(Vector2f(28 + 10 * 18, 31 + j * 18))};
                window.draw(lineRow, 2, Lines);
            }
            window.display();
            break;
        }

        // 游戏暂停
        case PAUSE:
        {
            Event evt;
            while (window.pollEvent(evt))
            {
                if (evt.type == Event::Closed)
                {
                    window.close();
                }

                if (evt.type == Event::MouseButtonReleased)
                {
                    gameState = RUNNING;
                }
            }

            // 绘制继续游戏
            std::string resumeText = "Click to continue.";
            ElementText resumeBtn(resumeText);
            resumeBtn.text.setPosition(300, 212);
            window.draw(resumeBtn.text);
            window.display();
            break;
        }

        // 游戏结束
        case GAMEOVER:
        {
            Event evt;
            while (window.pollEvent(evt))
            {
                if (evt.type == Event::Closed)
                {
                    window.close();
                }

                if (evt.type == Event::MouseButtonReleased)
                {
                    // auto position = Mouse::getPosition();
                    TetrisLogic::init();
                    field[19][9] = 0;
                    score = 0;
                    lineCount = 0;
                    globalTime = 0;
                    level = 0;
                    gameState = RUNNING;
                }
            }

            // 绘制重新开始游戏
            window.clear(Color::Black);
            std::string restartText = "Click to restart game!";
            std::string scoreStr = "Bravo! You got " + std::to_string(score) + "!";
            ElementText restartBtnText(restartText); 
            ElementText scoreStrText(scoreStr);
            restartBtnText.text.setPosition(36.0, 106.0);
            scoreStrText.text.setPosition(36.0, 36.0);
            window.draw(restartBtnText.text);
            window.draw(scoreStrText.text);
            window.display();
            break;
        }
        }
    }
    return 0;
}
