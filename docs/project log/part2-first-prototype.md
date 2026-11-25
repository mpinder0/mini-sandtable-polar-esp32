[← Part 1: Idea and early concept](part1-introduction.md) | [Contents](../../README.md#project-log) | [Part 3: Second prototype →](part3-second-prototype.md)

# Starting to design and first prototype

Having decided that I wanted to build a polar mechanism, I started my CAD design for the 3D-printed parts using [Onshape](https://www.onshape.com/en/), an online parametric CAD design package, and (at the time of writing) [free for non-commercial use](https://www.onshape.com/en/products/free). As I am not an experienced CAD designer, it was also helpful that they provide a number of [tutorials on their website](https://learn.onshape.com/) to help get started.

The first prototype featured a circular sand-table bed, 113 mm in diameter, with a total external diameter of just under 120 mm.

I already had the Raspberry Pi from previous projects, so I purchased the motor HAT and other components to use with it. The Raspberry Pi was also an easy way to get started quickly, allowing me to write the code in Python, a language I was already somewhat familiar with. Having a full operating system on the Pi (Raspberry Pi OS) was also advantageous, providing easy file management and the ability to remotely control the device, interactively starting and stopping scripts while testing and debugging.

## System layout

```mermaid
flowchart LR
    PSU[Power supply] --> PI[Raspberry Pi 4]
    PSU --> HAT
    PI --- HAT[Adafruit Motor HAT]
    HAT -->|coils| MT((28BYJ-48<br/>Theta))
    HAT -->|coils| MR((28BYJ-48<br/>Rho))
    HALL[A3144 Hall sensor] -->|reference| PI
    MT -->|10T → 60T gear| MECH[Polar arm + magnet]
    MR -->|rho slide| MECH
    MECH -. magnet drags .-> BALL((8 mm ball<br/>in sand))
```

## Bill of materials

| Item | Qty | Unit cost | Line cost | Notes / example link |
| --- | --- | --- | --- | --- |
| [Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/) | 1 | £35 | £35 | £33–158 depending on RAM(!). Already owned |
| [Adafruit RPi Motor HAT](https://thepihut.com/products/adafruit-dc-stepper-motor-hat-for-raspberry-pi-mini-kit) | 1 | £22 | £22 | |
| [28BYJ-48](https://www.instructables.com/How-to-Control-28BYJ-48-Stepper-Motor-With-ULN2003/) geared stepper motor | 2 | £3 | £6 | Readily available, e.g. [eBay](https://www.ebay.co.uk/sch/i.html?_nkw=28BYJ-48) |
| A3144 unipolar Hall-effect sensor | 1 | £0.48 | £0.48 | Homing |
| Neodymium magnet, N52 6 × 3 mm disc | 1 | ~£0.30* | £0.30 | |
| Steel ball bearing, 8 mm | 1 | ~£0.10* | £0.10 | |
| Power supply | 1 | £10–15 | £10–15 | |
| 3D-printed parts (PLA) | – | ~£3* | £3 | |
| Sand | – | – | – | Various options from different sources |
| **Total** | | | **~£77–82** | Well over the £50 target, mostly due to the Pi and HAT |

\* Estimated cost.

## Electrical design

The first prototype was electrically very simple, as the motor HAT stacks directly on the Raspberry Pi's 40-pin header and communicates with the Pi over I²C. Each 28BYJ-48 connects to one of the HAT's two stepper ports (M1/M2 and M3/M4), and the Hall-effect sensor connects to a Pi GPIO input.

<!-- TODO: add the GPIO pin used for the Hall-effect sensor and the motor supply voltage used for the HAT -->

## Mechanical design

The first design can be seen in Onshape [here](https://cad.onshape.com/documents/7ad59e3138f1813dd833a7ba/w/e3cdf864d640c5b023916b6a/e/ba935a6014cc0a75bc3e4f22).
![First prototype mechanism](./media/mechanism%20first%20proto.png)

The mechanism uses a 60-tooth gear for the theta axis and 10-tooth drive gears, giving a 6:1 reduction. The larger gear was sized so that the theta motor could be placed away from the centre of the assembly, where the rho motor is installed, while also giving an easily divisible gear ratio between the two axes. Because the linear axis is fixed on top of the rotating assembly, the rho motor must step every time the theta motor steps (1 rho step for 6 in theta, given the larger theta gear ratio) to maintain the correct relative position and avoid rho drifting in or out as the theta axis moves.

## Build photos

![First prototype top mechanism](./media/proto1_mech.jpg)
![First prototype motors](./media/proto1_motors.jpg)
![First prototype sand pattern](./media/proto1_pattern.jpg)

Initial build and testing was undertaken circa June 2025.

## Lessons learned

- Very slow
    - Python — checked and was fully executing around 1 ms, cyclically; however, to drive the steppers fast enough this would need to be improved.
    - Motor HAT update — stepper control uses ICs capable of partial stepping using PWM. Partial stepping can be disabled, but the communication rate to the ICs on the HAT is limited by this.
    - The 64:1 reduction gearbox on the stepper motor requires many steps for small distances moved (2048 steps for a full rotation).
- Jumpy ball movement — not sure whether this is the coarseness of the sand, the uneven 3D-printed surface caused by a monotonic top layer, or the pull of the magnet (too weak or too strong).
- The Raspberry Pi is expensive and too large to realistically fit into the small footprint — the [Pi Zero](https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/) could be considered as an improvement on both counts.

## Next steps

Given the lessons learned, in the next iteration I will make significant design changes — considering an alternative processor and motors, and some experimentation to try to get the ball rolling more smoothly through the sand.

---

[← Part 1: Idea and early concept](part1-introduction.md) | [Contents](../../README.md#project-log) | [Part 3: Second prototype →](part3-second-prototype.md)
