# Software design

## Class diagram

### Classes

```mermaid
classDiagram
    class MotorControl {
        +step(Axis ax, Direction dir): void
        +is_at_reference(): bool
    }

    class Mover {
        -pattern: Array~Coordinate~
        -play_move(): void
        +set_pattern(Array~Coordinate~): void
        +play_pattern(): void
    }

    class Planner {
        +theta_speed: real
        +rho_speed: real
        +theta_step_distance
        +rho_step_distance
        +get_move_steps(Coordinate start, Coordinate end): Move
    }

    MotorControl --> Mover
    Planner --> Mover
```

### Structs and Enums

```mermaid
classDiagram
    class Direction {
        <<enumeration>>
        Forwards
        Backwards
    }

    class Axis {
        <<enumeration>>
        Theta
        Rho
    }

    class Move {
        <<struct>>
        +theta_steps: int
        +rho_steps: int
        +theta_direction: Direction
        +rho_direction: Direction
        +move_time: long
    }

    class Coordinate {
        <<struct>>
        +theta_position: real
        +rho_position: real
    }
```


## Example call sequence to play pattern

```mermaid
sequenceDiagram
    note over Mover: New pattern
    Mover->>Mover: set_pattern(pattern)
    loop play() until moves in pattern are complete
        Mover->>Planner: get_move_steps(start, end)
        Planner-->>Mover: return Move
        loop play_move() until move time (and steps) complete
            Mover->>MotorControl: step(axis, direction)
            Mover-->>Mover: Wait for next step due
        end
    end
```

## Pattern files

Patterns can be loaded from Sisyphus style theta-rho (`.thr`) files stored on the ESP32's LittleFS partition.
Put files in `data/` (e.g. `data/patterns/example_rose.thr`) and upload them with `pio run -t uploadfs`.

```
# comment
<theta radians, unwrapped> <rho 0.0 centre .. 1.0 edge>
```

```mermaid
classDiagram
    class AbstractPattern {
        <<abstract>>
        +get_pattern(): Array~Coordinate~
    }

    class PatternFile {
        +PatternFile(FS fs, path, max_points)
        +is_valid(): bool
        +error_count(): int
    }

    class ThrPatternBuilder {
        +feed(text, length): bool
        +finish(): Array~Coordinate~
    }

    AbstractPattern <|-- PatternFile
    ThrPatternBuilder <-- PatternFile
```

- `ThrPatternBuilder` (`Pattern/thr_parser.h`) does the line splitting and parsing with no filesystem dependency, so it is unit tested on the native env (`test/native/test_pattern`). `PatternFile` itself is tested on the ESP32 (`test/embedded/test_filesystem`).
- Rho is clamped to 0..1 and scaled to `AXIS_MIN_R`..`AXIS_MAX_R`, so files work on either motor build.
- Whole rotations are removed from theta so the first point lies in [0, 2pi), avoiding extra spins to reach the start.
- Malformed lines are skipped and counted. Files over `max_points` (default 5000, ~40 KB) are rejected, as the whole pattern is held in memory.
