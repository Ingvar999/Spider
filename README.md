# Spider

## Protocol over TCP
### General information
Identifier of command | Parameters | Range | Example
----------------------|------------|-------|--------
'r' - Set radius of placing of the legs | 1. New radius (mm) | 1. 40 .. depend on current height | "dr60\n" - sets radius to 60 mm
'h' - Change height relative to current position | 1. Shift of height (mm), can be negative | 1. depend on current height and radius | "dh-20\n" - decrases height by 20mm
'f' - Perform turn with step | 1. Angle of turn in degrees| 1. -180 .. 180 | "df30\n" - turn the body 30 degrees clockwise
't' - Perform temporary turn without step | 1. Angle of turn in degrees | 1. -20 .. 20 | "dt10\n"
'b' - Takes the basic position | None | - | "db\n"
'm' - Move in a particular direction or default | 1. Direction of moving in degrees or None | 1. -180 .. 180 | "dm100\n"

Identifier of property | Parameters | Range | Example
-----------------------|------------|-------|--------
'i' - Turn power on/off| New bool value| 1. 0 .. 1| "si0\n" - turns power off

Identifier of value | Range | Example
--------------------|-------|--------
'r' - Get current radius of placing of the legs | 40 .. depend on height | "60\n"
'h' - Height of the body | 0 .. depend on radius | "45\n"
'v' - Voltage of suply battery in mV| 0 .. 9000 | "8100\n"
'w' - Workloads for each leg (6 values), ' ' - is separator | 0 .. 1024 | "123 45 66 56 99 12\n"
 
