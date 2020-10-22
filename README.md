# picorogue

Variations on a theme. (This may result in some blog posts.)

Following are my original notes for this idea. The current implementation does not match this in all details.

## Physical structure

* The Game consists of a tower of $FLOORS floors, numbered 1 through $FLOORS.
* Each floor consists of a $XMAXx$YMAX tile grid.
* Each tile is either blocked or clear.
* All clear tiles are connected -- there are no isolated rooms.
* Clear tiles must be directly adjacent (to the north, east, south, or west) to be considered connected -- no diagonals.
* The placement of tiles on each floor is randomized.
* Between 30-70% of tiles should be clear.
* On one clear tile on each floor, an up staircase should be placed.

## Gameplay

* The player character (PC) is placed in a random clear tile on floor 1.
    * If the tile chosen happens to be on the up staircase, pick a new tile.
* Gameplay is divided into turns.
* Each turn starts with the player choosing an action and completes with the resolution of that action.
* The following actions are available:
    * Move (north, east, south, west): Attempt to move one tile in a direction.
        * Moving succeeds if the destination tile is clear, and fails if it is blocked.
    * Climb: Attempt to ascend stairs.
        * Climbing succeeds if the current tile contains a staircase, and fails otherwise.
* When the PC climbs stairs, the next floor up is loaded and the PC is placed on a random tile as stated above.
* If the PC climbs stairs on floor 10, the player has won the game.
    * The player is presented with a message indicating their success and the number of turns it took to win.

## Display and Interface

* The game world is displayed using ASCII characters.
    * A blocked tile is '#'
    * A clear tile is ' '
    * The PC is '@'
    * The up staircase is '<'
* Actions are issued using the keyboard.
    * Up Arrow and k issue Move: North
    * Down Arrow and j issue Move: South
    * Left Arrow and h issue Move: West
    * Right Arrow and l issue Move: East
    * < issues Climb
* Aside from the current floor and PC position, the screen also shows the current floor number and turn count.

## Possible forms of death (elaborations)

* A 'pursuing ghost' (monsters, evasion, simple pathfinding)
* Starvation (items, hunger clock)
* Time runs out (fog of war, turn clock)
* Savage beasts (monsters, line of sight, hit points, equipment)
* Traps (special tiles, 'look', timed events)
* Environmental conditions (special tiles, 'look', equipment)

## Implementation guidelines

* A single, mutable, preferably global game state
* Procedural style to the extent the language permits
* Floors should be generated on demand
    * The design _currently_ only requires storing one floor at once!
* The RNG should be seedable and deterministic -- same floors and start locations for a given seed