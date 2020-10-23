//// DEPENDENCIES
// none needed

//// UTILITY FUNCTIONS
// const clearscreen = () => {}; // not needed in this implementation
const randint = (i) => Math.floor(Math.random() * i);

//// TILE GRAPHICS
const ASCII = {
  clear: ".",
  blocked: "#",
  // P(layer) C(haracter).
  // "Who do you think you are, War?" - NetHack
  pc: "@",
  // only one way out of this mess
  stairs: "<",
};

//// GRID PARAMETERS & HELPERS
// if you climb the stairs on this floor, you've won the game
const TOPFLOOR = 10,
  // bounds for the grid, which is stored as a 1d array
  XMIN = 0,
  XMAX = 15,
  YMIN = 0,
  YMAX = 15,
  FLOORSIZE = XMAX * YMAX;

// helpers for translating 2d coords to flat 1d
const xy = (x, y) => y * XMAX + x;
const boundsok = (x, y) => x >= XMIN && x < XMAX && y >= YMIN && y < YMAX;

//// DATA TYPES
// only two types of terrain, clear and blocked, aka floors and walls
const TILE = {
  clear: Symbol("TILE_CLEAR"),
  blocked: Symbol("TILE_BLOCKED"),
};

const STATE = {
  playing: Symbol("PLAYING"),
  quit: Symbol("QUIT"),
  won: Symbol("WON"),
};

const DIR = {
  N: Symbol("N"),
  W: Symbol("W"),
  S: Symbol("S"),
  E: Symbol("E"),
};

//// GLOBAL STATE
// newfloor() has to be called to properly initialize this
const G = {
  turns: 0,
  px: 0,
  py: 0,
  f: {
    n: 0,
    sx: 0,
    sy: 0,
    tiles: [].fill(TILE.clear, 0, FLOORSIZE - 1),
  },
  state: STATE.playing,
};

//// GAME LOGIC - FLOOR GENERATION
// fill the whole floor with clear tiles
const digfloor_clear = () => {
  G.f.tiles = new Array(FLOORSIZE).fill(TILE.clear, 0, FLOORSIZE - 1);
};

// simple four-quadrant layout
const digfloor_quad = () => {
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
  for (let ix = 0; ix < XMAX; ix++) {
    G.f.tiles[xy(ix, row)] = TILE.blocked;
  }
  for (let iy = 0; iy < YMAX; iy++) {
    G.f.tiles[xy(col, iy)] = TILE.blocked;
  }
  // open up doors in 3 walls
  G.f.tiles[xy(randint(col), row)] = TILE.clear;
  G.f.tiles[xy(col + 1 + randint(XMAX - col - 1), row)] = TILE.clear;
  G.f.tiles[xy(col, randint(row))] = TILE.blocked;
};

// creates a new floor and places the PC on it
const newfloor = () => {
  // we're on a new floor
  G.f.n++;

  // find a random place to start the PC
  G.px = randint(XMAX);
  G.py = randint(YMAX);

  // find a random place to place the stairs, not on the PC
  // or in line with the PC
  do {
    G.f.sx = randint(XMAX);
    G.f.sy = randint(YMAX);
  } while (G.px == G.f.sx || G.py == G.f.sy);

  digfloor_quad();
};

//// GAME LOGIC - RENDERING
// clear the screen, print out the current floor
const draw = () => {
  // this implementation is very different than console
  let out = "";
  for (let iy = 0; iy < YMAX; iy++) {
    for (let ix = 0; ix < XMAX; ix++) {
      if (ix == G.px && iy == G.py) {
        out += ASCII.pc;
      } else if (ix == G.f.sx && iy == G.f.sy) {
        out += ASCII.stairs;
      } else if (G.f.tiles[xy(ix, iy)] == TILE.blocked) {
        out += ASCII.blocked;
      } else {
        out += ASCII.clear;
      }
    }
    out += "\n";
  }
  document.getElementById("screen").innerHTML = out;

  // JS difference: this is in turn() in console
  document.getElementById("status").innerHTML = "FL:" + G.f.n + " T:" + G.turns;
};

//// GAME LOGIC - ACTIONS
// attempt to move the PC in a compass direction
// will fail if the destination is off the grid or blocked
// will always increment the turn counter
const movepc = (dir) => {
  G.turns++;
  let cx = G.px;
  let cy = G.py;

  switch (dir) {
    case DIR.N:
      cy--;
      break;
    case DIR.W:
      cx--;
      break;
    case DIR.S:
      cy++;
      break;
    case DIR.E:
      cx++;
  }

  if (boundsok(cx, cy) && G.f.tiles[xy(cx, cy)] == TILE.clear) {
    // it's an OK place to stand
    G.px = cx;
    G.py = cy;
  }
};

// attempt to climb stairs
// will fail if the PC isn't standing on stairs
// otherwise:
//   if this is not the top floor, generate a new one
//   if this is the top floor, set the game to WON
const climb = () => {
  G.turns++;
  if (G.px != G.f.sx || G.py != G.f.sy) {
    // this isn't a stairwell?
    return;
  }
  if (G.f.n == TOPFLOOR) {
    // we got to the top!
    G.state = STATE.won;
    return;
  }
  newfloor();
};

// process input from the player
// JS/DOM difference -- this is pumped by an event, rather than polling
const turn = (cmd) => {
  switch (cmd) {
    case "Q":
      G.state = STATE.quit;
      break;
    case "h":
      movepc(DIR.W);
      break;
    case "j":
      movepc(DIR.S);
      break;
    case "k":
      movepc(DIR.N);
      break;
    case "l":
      movepc(DIR.E);
      break;
    case "<":
      climb();
      break;
  }
};

//// GAME LOGIC - STARTING AND ENDING
// inform the player as to how they ended the game and in how many turns
const gameover = () => {
  // TODO: actual ending message
  alert("you did it, cool");
};

// this is the equivalent of main() for JS/DOM
window.addEventListener("load", (ev) => {
  newfloor();
  draw();
});

window.addEventListener("keypress", (ev) => {
  turn(ev.key);
  draw();

  if (G.state != STATE.playing) {
    gameover();
  }
});
