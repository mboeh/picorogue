#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// C has no builtin way of doing this and we're not using ncurses (yet)
// this only works on *nix
#define clearscreen() system("clear")
#define randint(n) (rand() % n)

// ASCII tiles
#define T_CLEAR '.'
#define T_BLOCKED '#'
// P(layer) C(haracter).
// "Who do you think you are, War?" - NetHack
#define T_PC '@'
// only one way out of this mess
#define T_STAIRS '<'

// only two types of terrain, clear and blocked, aka floors and walls
typedef enum
{
    TILE_CLEAR,
    TILE_BLOCKED
} tile;

// if you climb the stairs on this floor, you've won the game
#define TOPFLOOR 10
// bounds for the grid, which is stored as a 1d array
#define XMIN 0
#define YMIN 0
#define XMAX 15
#define YMAX 15
#define FLOORSIZE (XMAX * YMAX)
// helpers for translating 2d coords to flat 1d
#define xy(x, y) (y * XMAX + x)
#define boundsok(x, y) (x >= XMIN && x < XMAX && y >= YMIN && y < YMAX)

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
    // number of turns the player has taken
    int turns;
    // current location of the PC
    int px, py;
    floor f;
    enum
    {
        PLAYING,
        QUIT,
        WON
    } state;
} game;

// newfloor() has to be called to properly initialize this
game G = {
    // turns
    0,
    // px, py
    0,
    0,
    // floor
    {0, 0, 0, {0}},
    // won
    PLAYING};


// fill the whole floor with clear tiles
void digfloor_clear() {
    memset(&G.f.tiles, TILE_CLEAR, FLOORSIZE * sizeof(tile));
}

// simple four-quadrant layout
void digfloor_quad() {
    int row, col;
    digfloor_clear();

    // pick a row that doesn't intersect player or stairs
    do {
        row = randint(YMAX);
    } while (row == G.py || row == G.f.sy || row == 0 || row == YMAX - 1);
    // and a column, same deal
    do {
        col = randint(XMAX);
    } while (col == G.px || col == G.f.sx || col == 0 || col == XMAX - 1);
    // fill in the row and column
    for(int ix = 0; ix < XMAX; ix++) {
        G.f.tiles[xy(ix, row)] = TILE_BLOCKED;
    }
    for(int iy = 0; iy < YMAX; iy++) {
        G.f.tiles[xy(col, iy)] = TILE_BLOCKED;
    }
    // open up doors in 3 walls
    G.f.tiles[xy(randint(col), row)] = TILE_CLEAR;
    G.f.tiles[xy(col + 1 + randint(XMAX - col - 1), row)] = TILE_CLEAR;
    G.f.tiles[xy(col, randint(row))] = TILE_CLEAR;
}

// creates a new floor and places the PC on it
void newfloor()
{
    // winning the game is handled in climb()
    assert(G.f.n < TOPFLOOR);

    // we're on a new floor.
    G.f.n++;

    // find a random place to start the PC
    G.px = randint(XMAX);
    G.py = randint(YMAX);
    // find a random place to place the stairs, not on the PC
    // or in line with the PC
    do
    {
        G.f.sx = randint(XMAX);
        G.f.sy = randint(YMAX);
    } while (G.f.sx == G.px || G.f.sy == G.py);

    digfloor_quad();
}

// clear the screen, print out the current floor
void draw()
{
    // print the map line by line
    char line[XMAX + 1];
    memset(line, 0, XMAX + 1);

    clearscreen();

    // the use of printf here is a stopgap until we use ncurses
    printf("* PICOROGUE *\n");
    for (int iy = 0; iy < YMAX; iy++)
    {
        // fill up the next line of the map
        for (int ix = 0; ix < XMAX; ix++)
        {
            // the PC standing on stairs covers it
            if (ix == G.px && iy == G.py)
            {
                line[ix] = T_PC;
            }
            else if (ix == G.f.sx && iy == G.f.sy)
            {
                line[ix] = T_STAIRS;
            }
            else if (G.f.tiles[xy(ix, iy)] == TILE_BLOCKED)
            {
                line[ix] = T_BLOCKED;
            }
            else
            {
                line[ix] = T_CLEAR;
            }
        }
        // print out the next line of the map
        puts(line);
    }
}

// compass directions
typedef enum
{
    N,
    E,
    S,
    W
} dir;

// attempt to move the PC in a compass direction
// will fail if the destination is off the grid or blocked
// will always increment the turn counter
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

    if (boundsok(cx, cy) && G.f.tiles[xy(cx, cy)] == TILE_CLEAR)
    {
        // it's an OK place to stand
        G.px = cx;
        G.py = cy;
    }
}

// attempt to climb stairs
// will fail if the PC isn't standing on stairs
// otherwise:
//   if this is not the top floor, generate a new one
//   if this is the top floor, set the game to WON
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

// print the status line and request input from the player
void turn()
{
    char cmd;

    printf("FL:%d T:%d | COMMAND? ", G.f.n, G.turns);
    fflush(stdout);
    // this sucks but standard C does not have an unbuffered getchar
    // you can buffer multiple moves before hitting ENTER, though
    // that makes it almost playable
    cmd = getchar();

    switch (cmd)
    {
    case 'Q':
        G.state = QUIT;
        break;
    // traditional vi directional keys
    // can't get arrow keys with stdio
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

// inform the player as to how they ended the game and in how many turnns
void gameover()
{
    clearscreen();

    printf("Farewell, adventurer!\n\n");
    printf("You ");
    if (G.state == WON)
    {
        printf("reached the top in");
    }
    else if (G.state == QUIT)
    {
        // wow you didn't enjoy this amazing game?
        printf("gave up after");
    }
    else
    {
        // should be unreachable
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
    // TODO: allow a fixed seed
    time_t t;
    srand((unsigned)time(&t));

    // initialize the first floor
    newfloor();

    while (G.state == PLAYING)
    {
        draw();
        turn();
    }

    gameover();

    return 0;
}