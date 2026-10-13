# Idea and Early Concept

I had been looking for a hardware project to help me learn the combination of 3D printing, electronics, and embedded software for some time. Inspired by commercially available sandtables such as [Sisyphus](https://sisyphus-industries.com/) and [Oasis Mini](https://grounded.so/), as well as DIY projects I had seen while reading the [Hackaday blog](https://hackaday.com/blog/?s=sand+table), I set myself the challenge of building one that could be made largely from 3D-printed parts.

The project is primarily a learning exercise: an opportunity to work through the product-development challenges and improve my engineering skills. It is not intended to become a commercial product or to offer a significantly different alternative to existing sandtables.

Sand tables work by moving a magnet below a table covered in sand, rolling a steel ball bearing through the sand to create a residual pattern. The mechanisms are usually XY gantry (rectangular) or polar (circular).

![Sisyphus sand table](./media/Sisyphus%20sand%20table.png)
Example image above taken from the Sisyphus article [here](https://www.thisiscolossal.com/2016/09/new-kinetic-sand-tables-by-bruce-shapiro/).

## Basic Requirements

- Hardware (mechanical and electrical)
    - As I prefer the aesthetic, the aim is to build a circular sand table.
    - Constructed mostly from 3D-printed or off-the-shelf parts. No significant machining or rework of parts is required, and it should be suitable for hand-tool assembly.
    - Given the objective of making the resulting thing mostly 3D-printable, this sets some size constraints based on the 3D printer’s build volume. I have a Bambu Lab A1 Mini, so the horizontal footprint is slightly smaller than a standard 3D print bed, and should be within 180 mm x 180 mm.
    - The device should be cheap, with a final BOM cost of under £100, ideally closer to £50.
    - Power should be supplied from a single off-the-shelf power supply.
    - The device will include some form of self-homing to zero the ball position before starting a pattern.
- Software
    - Basic file-system loading of pattern files.
    - Patterns will be in theta, rho (.thr) format rather than G-code and will also support comments prefixed with &#35;.
        - Note that free online tools such as [Sandify](https://sandify.org/) can be used to help create patterns.
    - When powered on, the sand table will complete its homing routine and then loop through any loaded patterns indefinitely.
    - Out of scope: I have no plan to develop any wireless or app control for the device.

## Subsystems

- Processor: single-board computer or microcontroller
    - Off-the-shelf dev boards rather than custom PCBs
    - Common modules to consider; Raspberry Pi, Arduino, STM32, ESP32
- Moving arm mechanism, including magnet and ball interaction
    - Polar mechanism so it rotates through 360°
    - Should be able to rotate continuously. To avoid cable twisting, no electronics should be installed on top of the mechanism
- Body: housing the processor and other electronics, the rotating mechanism, and the sand tray
- Motors and drivers
    - The mechanism should move at a slow to moderate speed (1–10 mm/s), with enough torque to drag the ball bearing through the sand.
    - Accuracy of the movement mechanism is not especially critical, but within ±2 mm would be acceptable.
    - Stepper or servo (motor with encoder or other position feedback)
- Other electronics
    - Power supply
        - To feed the processor and motor drivers. Voltage step-up or step-down may be required if the processor and drives operate on different voltages.
        - Appropriate isolation or protection in place between the processor and drive circuits, to prevent voltage spikes or drops from the motor circuit interfering with the processor's operation.
    - Homing sensor: some options to consider include
        - Mechanical lever switch
        - Hall-effect sensor (and a magnet) for contactless sensing
        - Motor torque or current feedback detection. Detect an increase when the mechanism becomes stalled at the end of travel; this would require specific drive circuitry