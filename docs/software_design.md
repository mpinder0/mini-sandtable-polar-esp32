# Software design

## Class diagram

### Classes

```plantuml
@startuml
class MotorControl {
+ step(Axis ax, Direction dir): void
+ is_at_reference(): bool
}

class Mover {
- pattern: Array<Coordinate>
- play_move(): void
+ setPattern(Array<Coordinate>): void
+ play_pattern(): void
}

class Planner {
+ theta_speed: real
+ rho_speed: real
+ theta_step_distance
+ rho_step_distance
+ calculateMove(Coordinate start, Coordinate end): Move
}

Mover <-- MotorControl
Mover <-- Planner
@enduml
```

### Structs and Enums

```plantuml
enum Direction {
Forwards
Backwards
}

enum Axis {
Theta
Rho
}

struct Move {
+ theta_steps: int
+ rho_steps: int
+ theta_direction: Direction
+ rho_direction: Direction
+ move_time: long
}

struct Coordinate {
+ theta_position: real
+ rho_position: real
}
```


## Example call sequence to play pattern

```plantuml
@startuml
Note over Mover: New pattern
Mover -> Mover: setPattern(pattern)
loop play() until moves in pattern are complete
Mover -> Planner: getMove(start, end)
Planner --> Mover: return Move
loop play_move() until move time (and steps) complete
Mover -> MotorControl: step(axis, direction)
Mover -> Mover: Wait for next step due
end
end
@enduml
```