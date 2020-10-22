const TILES = {
  clear: ".",
  blocked: "#",
  pc: "@",
  stairs: "<",
};

const TOPFLOOR = 10,
  XMIN = 0,
  XMAX = 15,
  YMIN = 0,
  YMAX = 15,
  FLOORSIZE = XMAX * YMAX;

const xy = (x, y) => y * XMAX + x;
const boundsok = (x, y) => x >= XMIN && x < XMAX && y >= YMIN && y < YMAX;

const TILE_CLEAR = Symbol("TILE_CLEAR");
const TILE_BLOCKED = Symbol("TILE_BLOCKED");

const PLAYING = Symbol("PLAYING");
const QUIT = Symbol("QUIT");
const WON = Symbol("WON");

const G = {
  turns: 0,
  px: 0,
  py: 0,
  f: {
    n: 0,
    sx: 0,
    sy: 0,
    tiles: [].fill(TILE_CLEAR, 0, FLOORSIZE - 1),
  },
  state: PLAYING,
};

const randint = (i) => Math.floor(Math.random() * i);

const digfloor_clear = () => {
  G.f.tiles = new Array(FLOORSIZE).fill(TILE_CLEAR, 0, FLOORSIZE - 1);
};

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
  for(let ix = 0; ix < XMAX; ix++) {
      G.f.tiles[xy(ix, row)] = TILE_BLOCKED;
  }
  for(let iy = 0; iy < YMAX; iy++) {
      G.f.tiles[xy(col, iy)] = TILE_BLOCKED;
  }
  // open up doors in 3 walls
  G.f.tiles[xy(randint(col), row)] = TILE_CLEAR;
  G.f.tiles[xy(col + 1 + randint(XMAX - col - 1), row)] = TILE_CLEAR;
  G.f.tiles[xy(col, randint(row))] = TILE_CLEAR;
};

const newfloor = () => {
  G.f.n++;
  G.px = randint(XMAX);
  G.py = randint(YMAX);
  do {
    G.f.sx = randint(XMAX);
    G.f.sy = randint(YMAX);
  } while (G.px == G.f.sx || G.py == G.f.sy);
  digfloor_quad();
};

const draw = () => {
  let out = "";
  for (let iy = 0; iy < YMAX; iy++) {
    for (let ix = 0; ix < XMAX; ix++) {
      if (ix == G.px && iy == G.py) {
        out += TILES.pc;
      } else if (ix == G.f.sx && iy == G.f.sy) {
        out += TILES.stairs;
      } else if (G.f.tiles[xy(ix, iy)] == TILE_BLOCKED) {
        out += TILES.blocked;
      } else {
        out += TILES.clear;
      }
    }
    out += "\n";
  }
  document.getElementById("screen").innerHTML = out;

  // JS difference
  document.getElementById("status").innerHTML = "FL:" + G.f.n + " T:" + G.turns;
};

const DIR = {
  N: Symbol("N"),
  W: Symbol("W"),
  S: Symbol("S"),
  E: Symbol("E"),
};

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

  if (boundsok(cx, cy) && G.f.tiles[xy(cx, cy)] == TILE_CLEAR) {
    // it's an OK place to stand
    G.px = cx;
    G.py = cy;
  }
};

const climb = () => {
  G.turns++;
  if (G.px != G.f.sx || G.py != G.f.sy) {
    // this isn't a stairwell?
    return;
  }
  if (G.f.n == TOPFLOOR) {
    // we got to the top!
    G.state = WON;
    return;
  }
  newfloor();
};

const turn = (cmd) => {
  switch (cmd) {
    case "Q":
      G.state = QUIT;
      break;
    // traditional vi directional keys
    // can't get arrow keys with stdio
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

const gameover = () => {
  alert("you did it, cool");
};

window.addEventListener("load", (ev) => {
  newfloor();
  draw();
});

window.addEventListener("keypress", (ev) => {
  turn(ev.key);
  draw();

  if (G.state != PLAYING) {
    gameover();
  }
});
