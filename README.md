# Spider

## Protocol over TCP
### General information
After device is conected to "PAUK" wi-fi network, host will available on 192.168.88.1:80.
There are three types of requests: to perform movement, set property, get information about current state. Letters 'd', 's', 'i' define these types correspondingly.
Each request ends with symbol '\n' and starts with request type. All parameters are int.

### Movement requests
These requests are added to the internal queue, so they can be performed later or not. Returned value notifies only that, were there mistakes in the request and was it added to the queue.

Identifier of command | Parameters | Range | Example
----------------------|------------|-------|--------
'r' - Set radius of placing of the legs | 1. New radius (mm) | 1. 40 .. depend on current height | "dr60\n" - sets radius to 60 mm
'h' - Change height relative to current position | 1. Shift of height (mm), can be negative | 1. depend on current height and radius | "dh-20\n" - decrases height by 20mm
'f' - Perform turn with step | 1. Angle of turn in degrees| 1. -180 .. 180 | "df30\n" - turn the body 30 degrees clockwise
't' - Perform temporary turn without step | 1. Angle of turn in degrees | 1. -20 .. 20 | "dt10\n"
'b' - Takes the basic position | None | - | "db\n"
'm' - Move in a particular direction | 1. Direction of moving in degrees | 1. -180 .. 180 | "dm100\n"
'm' - Move forward with avoiding obstacles | None | - | "dm\n"
's' - Stop while moving or turning | None | - | "ds\n"

### Set-property requests
Identifier of property | Parameters | Range | Example
-----------------------|------------|-------|--------
'i' - Turn power on/off| New bool value| 1. 0 .. 1| "si0\n" - turns power off
'b' - Turn balancing on/off| New bool value| 1. 0 .. 1| "sb0\n"
'w' - Turn workloads alignment on/off| New bool value| 1. 0 .. 1| "sw0\n"
'h' - Turn height control on/off| New bool value| 1. 0 .. 1| "sh0\n"
'c' - Turn voltage control on/off| New bool value| 1. 0 .. 1| "sc0\n"
'l' - Turn light control on/off| New bool value| 1. 0 .. 1| "sl0\n"
'f' - Turn light on/off. The light control will be automatically turned off | New bool value | 1. 0 .. 1 | "sf1\n"
's' - Set speed of moving| New speed| 1. 0 .. 30| "ss45\n"
'd' - Set critical distance for avoiding while moving | New distance (cm) | 1. 20 .. 150 | "sd30\n"
'p' - Set required position for balancing| 1. Vertical deviation 2. Direction| 1. 0 .. 20 2. -180 .. 180| "sp5 90\n"

### Information requests
One request can contain set of values. Each returned value ends with symbol '\n'. Example of request - "ierhv\n", example of returned value - "OK\n50\n45\n7900\n".

Identifier of value | Range | Example
--------------------|-------|--------
'r' - Get current radius of placing of the legs | 40 .. depend on height | "60\n"
'h' - Height of the body | 0 .. depend on radius | "45\n"
'v' - Voltage of suply battery in mV| 0 .. 9000 | "8100\n"
'w' - Workloads for each leg (6 values), ' ' - is separator | 0 .. 1024 | "123 45 66 56 99 12\n"
'p' - Position in the spherical coordinate system: deviation from the horizon and its direction | 1. 0 .. 50        2. -180 .. 180 | "3.25 150.1\n"
'e' - Error message | - | "OK\n"
'd' - Distance to the obstacle (sm) | 0 .. 150 | "40\n"
'i' - States of controls (on or off): balancing, workloads alignment, height control, voltage control, light control, lightning, power | 0 .. 1 | "0111001\n"
's' - Speed of moving | 1. 0 .. 30| "23\n"
'c' - Critical distance for avoiding while moving | 1. 20 .. 150 | "30\n"
'q' - Required position for balancing: deviation from the horizon and its direction | 1. 0 .. 20  2. -180 .. 180 | "10 100\n"

### Errors handling
There are two types of errors: mistakes in the recieved request and errors during operation of the robot.
If there were mistakes in a request then it is ignored and returned value reports what was wrong.
If operation errors occurs, first, tasks queue and not parsed requests cleared, then robot takes basic position and change error status. You can find out error status only through information request. Movement and set-property requests reset error status.  
