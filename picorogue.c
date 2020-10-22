#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CLS() system("clear")

#define T_CLEAR '.'
#define T_BLOCKED '#'
#define T_PC '@'
#define T_STAIRS '<'

typedef enum
{
    TILE_CLEAR,
    TILE_BLOCKED
} tile;

#define TOPFLOOR 10
#define XMIN 0
#define YMIN 0
#define XMAX 15
#define YMAX 15
#define FLOORSIZE 225
#define XY(x, y) (y * XMAX + x)
#define BOUNDSOK(x, y) (x >= XMIN && x < XMAX && y >= YMIN && y < YMAX)

typedef struct
{
    // floor number
    int n;
    // stair location
    int sx, sy;
    // tiles (flattened 2d array)
    tile tiles[FLOORSIZE];
} floor;

typedef struct
{
    int turns;
    int px, py;
    floor f;
    enum
    {
        PLAYING,
        QUIT,
        WON
    } state;
} game;

game G = {
    // turns
    0,
    // px, py
    0,
    0,
    // floor
    {0, 0, 0, {0}},
    // won
    false};

void newfloor()
{
    // we're on a new floor
    G.f.n++;
    // fill the whole floor with clear tiles
    memset(&G.f.tiles, TILE_CLEAR, FLOORSIZE);
    // find a random place to start the PC
    G.px = rand() % XMAX;
    G.py = rand() % YMAX;
    // find a random place to place the stairs, not on the PC
    // or in line with the PC
    do
    {
        G.f.sx = rand() % XMAX;
        G.f.sy = rand() % YMAX;
    } while (G.f.sx == G.px || G.f.sy == G.py);
    // TODO: place some walls
}

void draw()
{
    char line[XMAX + 1];
    memset(line, 0, XMAX + 1);

    CLS();

    printf("* PICOROGUE *\n");
    for (int iy = 0; iy < YMAX; iy++)
    {
        for (int ix = 0; ix < XMAX; ix++)
        {
            if (ix == G.px && iy == G.py)
            {
                line[ix] = T_PC;
            }
            else if (ix == G.f.sx && iy == G.f.sy)
            {
                line[ix] = T_STAIRS;
            }
            else if (G.f.tiles[XY(ix, iy)] == TILE_BLOCKED)
            {
                line[ix] = T_BLOCKED;
            }
            else
            {
                line[ix] = T_CLEAR;
            }
        }
        puts(line);
    }
}

typedef enum
{
    N,
    E,
    S,
    W
} dir;

void movepc(dir d)
{
    int cx, cy;
    G.turns++;

    cx = G.px;
    cy = G.py;
    switch (d)
    {
    case N:
        cy--;
        break;
    case W:
        cx--;
        break;
    case S:
        cy++;
        break;
    case E:
        cx++;
    }
    if (BOUNDSOK(cx, cy) && G.f.tiles[XY(cx, cy)] == TILE_CLEAR)
    {
        // it's an OK place to stand
        G.px = cx;
        G.py = cy;
    }
}

void climb()
{
    G.turns++;
    if (G.px != G.f.sx || G.py != G.f.sy)
    {
        // this isn't a stairwell?
        return;
    }
    if (G.f.n == TOPFLOOR)
    {
        // we got to the top!
        G.state = WON;
        return;
    }
    newfloor();
}

void turn()
{
    char cmd;

    printf("FL:%d T:%d | COMMAND? ", G.f.n, G.turns);
    fflush(stdout);
    cmd = getchar();

    switch (cmd)
    {
    case 'q':
    case 'Q':
        G.state = QUIT;
        break;
    case 'h':
        movepc(W);
        break;
    case 'j':
        movepc(S);
        break;
    case 'k':
        movepc(N);
        break;
    case 'l':
        movepc(E);
        break;
    case '<':
        climb();
        break;
    }
}

void gameover()
{
    CLS();

    printf("Farewell, adventurer!\n\n");
    printf("You ");
    if (G.state == WON)
    {
        printf("reached the top in");
    }
    else if (G.state == QUIT)
    {
        printf("gave up after");
    }
    else
    {
        printf("ascended to demigodhood in");
    }
    printf(" %d turn", G.turns);
    if (G.turns != 1)
    {
        printf("s");
    }
    printf(".\n\nThanks for playing PICOROGUE.\n");
}

int main()
{
    time_t t;

    srand((unsigned)time(&t));

    newfloor();

    while (G.state == PLAYING)
    {
        draw();
        turn();
    }

    gameover();

    return 0;
}