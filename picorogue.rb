## DEPENDENCIES
# none needed

## UTILITY FUNCTIONS
def clearscreen
  system 'clear'
end
alias randint rand

## TILE GRAPHICS
ASCII = {
  clear: ?.,
  blocked: ?#,
  ## P(layer) C(haracter).
  ## "Who do you think you are, War?" - NetHack
  pc: ?@,
  ## only one way out of this mess
  stairs: ?<,
}

## GRID PARAMETERS & HELPERS
# if you climb the stairs on this floor, you've won the game
TOPFLOOR = 10
# bounds for the grid, which is stored as a 1d array
XMIN, XMAX = 0, 15
YMIN, YMAX = 0, 15
FLOORSIZE = XMAX * YMAX

# helpers for translating 2d coords to flat 1d
def xy(x, y)
  y * XMAX + x
end

def boundsok(x, y)
  x >= XMIN && x < XMAX && y >= YMIN && y < YMAX
end

## DATA TYPES
# only two types of terrain, clear and blocked, aka floors and walls
module Tile
  CLEAR = :TILE_CLEAR
  BLOCKED = :TILE_BLOCKED
end

module State
  PLAYING = :PLAYING
  QUIT = :QUIT
  WON = :WON
end

# compass directions
module Direct
  N = :N
  W = :W
  S = :S
  E = :E
end

## GLOBAL STATE
# newfloor() has to be called to properly initialize this
G = Struct.new(
  :turns,
  :px, :py,
  :f,
  :state
).new(
  0,
  0, 0,
  Struct.new(
    :n,
    :sx, :sy,
    :tiles
  ).new(
    0,
    0, 0,
    [:clear] * FLOORSIZE
  ),
  :playing
)

## GAME LOGIC - FLOOR GENERATION
# creates a new floor and places the PC on it
def newfloor
  # we're on a new floor
  G.f.n += 1

  # find a random place to put the PC
  G.px = rand(XMAX)
  G.py = rand(YMAX)

  # find a random place to place the stairs, not on the PC
  # or in line with the PC
  begin
    G.f.sx = rand(XMAX)
    G.f.sy = rand(YMAX)
  end until (G.px != G.f.sx && G.py != G.f.sy)

  G.f.tiles.map! { Tile::CLEAR }
end

## GAME LOGIC - RENDERING
# clear the screen, print out the current floor
def draw
  clearscreen

  puts "* PICOROGUE *"
  (0...YMAX).each do |iy|
    puts(
      (0...XMAX).map do |ix|
        ASCII[
          # the PC standing on stairs covers it
          if ix == G.px && iy == G.py
            :pc
          elsif ix == G.f.sx && iy == G.f.sy
            :stairs
          elsif G.f.tiles[xy(ix, iy)] == Tile::BLOCKED
            :blocked
          else
            :clear
          end
        ]
      end.join
    )
  end
end

## GAME LOGIC - ACTIONS
# attempt to move the PC in a compass direction
# will fail if the destination is off the grid or blocked
# will always increment the turn counter
def movepc(dir)
  G.turns += 1

  cx, cy = G.px, G.py

  case dir
  when Direct::N then cy -= 1
  when Direct::W then cx -= 1
  when Direct::S then cy += 1
  when Direct::E then cx += 1
  end

  if boundsok(cx, cy) && G.f.tiles[xy(cx, cy)] != Tile::BLOCKED
    # it's an OK place to stand
    G.px, G.py = cx, cy
  end
end

# attempt to climb stairs
# will fail if the PC isn't standing on stairs
# otherwise:
#   if this is not the top floor, generate a new one
#   if this is the top floor, set the game to WON
def climb
  G.turns += 1
  # this isn't a stairwell?
  return if G.px != G.f.sx || G.py != G.f.sy
  if G.f.n == TOPFLOOR
    # we got to the top!
    G.state = State::WON
    return
  end
  newfloor
end

# print the status line and request input from the player
def turn
  printf "FL:%d T:%d | COMMAND? ", G.f.n, G.turns
  STDOUT.flush

  # you can buffer multiple moves before hitting ENTER
  # that makes it almost playable
  case STDIN.getc
  when ?Q then G.state = :quit
  # traditional vi directional keys
  # can't get arrow keys
  when ?h then movepc Direct::W
  when ?j then movepc Direct::S
  when ?k then movepc Direct::N
  when ?l then movepc Direct::E
  when ?< then climb
  end
end

## GAME LOGIC - STARTING AND ENDING
END_REASONS = {
  won: "reached the top in",
  # wow you didn't enjoy this amazing game?
  quit: "gave up after",
}
# should be unreachable
END_REASONS.default = "ascended to demigodhood in"

# inform the player as to how they ended the game and in how many turns
def gameover
  puts %{
Farewell, adventurer!

You #{END_REASONS[G.state]} #{G.turns} turn#{G.turns == 1 ? "" : "s"}.

Thanks for playing PICOROGUE.}
end

if $0 == __FILE__
  # TODO: allow a fixed seed
  srand(0)

  # initialize the first floor
  newfloor

  while G.state == :playing
    draw
    turn
  end

  gameover
end
