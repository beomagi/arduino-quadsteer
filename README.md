# arduino-quadsteer

This project takes steering, throttle input and modifier inputs for each.
The outputs are a rear steering output and rear ESC output.
The steering output takes the modifier and returns the steering, nothing or the reverse.
The throttle output takes the modifier and returns the input or nothing,
If the modifier is a continuous dial, the output will be a continuous transition.

On startup, leave the modifiers and inputs at neutral position. This is checked in the first few seconds to determine any needed offsets.
