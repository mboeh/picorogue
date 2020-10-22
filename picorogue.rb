TILES = {
  clear: ?.,
  blocked: ?#,
  pc: ?@,
  stairs: ?<,
}

TOPFLOOR = 10
XMIN, XMAX = 0, 15
YMIN, YMAX = 0, 15
FLOORSIZE = XMAX * YMAX

def xy(x, y)
  y * XMAX + x
end

def boundsok(x, y)
  x >= XMIN && x < XMAX && y >= YMIN && y < YMAX
end

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

def newfloor
  G.f.n += 1
  G.f.tiles.map! { :clear }
  G.px = rand(XMAX)
  G.py = rand(YMAX)
  begin
    G.f.sx = rand(XMAX)
    G.f.sy = rand(YMAX)
  end until (G.px != G.f.sx && G.py != G.f.sy)
end

def draw
  system "clear"
  puts "* PICOROGUE *"
  (0...YMAX).each do |iy|
    puts(
      (0...XMAX).map do |ix|
        TILES[
          if ix == G.px && iy == G.py
            :pc
          elsif ix == G.f.sx && iy == G.f.sy
            :stairs
          elsif G.f.tiles[xy(ix, iy)] == :blocked
            :blocked
          else
            :clear
          end
        ]
      end.join
    )
  end
end

def movepc(dir)
  G.turns += 1

  cx, cy = G.px, G.py

  case dir
  when :n then cy -= 1
  when :w then cx -= 1
  when :s then cy += 1
  when :e then cx += 1
  end

  if boundsok(cx, cy) && G.f.tiles[xy(cx, cy)] != :blocked
    G.px, G.py = cx, cy
  end
end

def climb
  G.turns += 1
  return if G.px != G.f.sx || G.py != G.f.sy
  if G.f.n == TOPFLOOR
    G.state = :won
    return
  end
  newfloor
end

def turn
  printf "FL:%d T:%d | COMMAND? ", G.f.n, G.turns
  STDOUT.flush

  case STDIN.getc
  when ?Q then G.state = :quit
  when ?h then movepc :w
  when ?j then movepc :s
  when ?k then movepc :n
  when ?l then movepc :e
  when ?< then climb
  end
end

END_REASONS = {
  won: "reached the top in",
  quit: "gave up after",
}
END_REASONS.default = "ascended to demigodhood in"

def gameover
  puts %{
Farewell, adventurer!

You #{END_REASONS[G.state]} #{G.turns} turn#{G.turns == 1 ? "" : "s"}.

Thanks for playing PICOROGUE.}
end

if $0 == __FILE__
  srand(0)

  newfloor

  while G.state == :playing
    draw
    turn
  end

  gameover
end
