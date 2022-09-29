/* EC RAM mapping for MSI Modern 15 A11M-1046 (ImHex pattern)
   Author: Tim Schneeberger <tim.schneeberger(at)outlook.de> */

/* -- MSI Center Pro user scenarios

Super Battery:
	Offset 0x79 set to 01000110; dec: 70 (0x46)
	Offset 0x91 set to 01000110; dec: 70 (0x46)
	Offset 0xD2 set to 11000010; dec: 194 (0xC2)
	Offset 0xD3 set to 10000001; dec: 129 (0x81)
	Offset 0xD4 set to 00001101; dec: 13 (0x0D)
	Offset 0xEB set to 10001111; dec: 143 (0x8F)

Silent:
	Offset 0x79 set to 01001011; dec: 75 (0x4B)
	Offset 0x91 set to 01000110; dec: 70 (0x46)
	Offset 0xD2 set to 11000001; dec: 193 (0xC1)
	Offset 0xD3 set to 10000010; dec: 130 (0x82)
	Offset 0xD4 set to 00011101; dec: 29 (0x1D)
	Offset 0xEB set to 10000000; dec: 128 (0x80)

Balanced:
	Offset 0x79 set to 01001011; dec: 75 (0x4B)
	Offset 0x91 set to 01000110; dec: 70 (0x46)
	Offset 0xD2 set to 11000001; dec: 193 (0xC1)
	Offset 0xD3 set to 10000011; dec: 131 (0x83)
	Offset 0xD4 set to 00001101; dec: 13 (0x0D)
	Offset 0xEB set to 10000000; dec: 128 (0x80)

High Performance:
	Offset 0x79 set to 01010000; dec: 80 (0x50)
	Offset 0x91 set to 01001011; dec: 75 (0x4B)
	Offset 0xD2 set to 11000000; dec: 192 (0xC0)
	Offset 0xD3 set to 10000011; dec: 131 (0x83)
	Offset 0xD4 set to 00001101; dec: 13 (0x0D)
	Offset 0xEB set to 10000000; dec: 128 (0x80)

=> Changed addresses:
	0x79: CPU power?
	0x91: GPU power?
	0xD2: Shift mode
	0xD3: Backlight
	0xD4: Fan flags
	0xEB: (Unknown, lower 4 bits are battery saving flags?)

Note: 0xD4 is only relevant for the silent fan flag. */

u8 cpuRealtimeTemp @0x68;
u8 cpuTemp0 @0x6A;
u8 cpuTemp1 @0x6B;
u8 cpuTemp2 @0x6C;
u8 cpuTemp3 @0x6D;
u8 cpuTemp4 @0x6E;
u8 cpuTemp5 @0x6F;
u8 cpuTemp6 @0x70; // <- unused by MSI Center

u8 cpuRealtimeFanSpeed @0x71;
u8 cpuFanSpeed0 @0x72;
u8 cpuFanSpeed1 @0x73;
u8 cpuFanSpeed2 @0x74;
u8 cpuFanSpeed3 @0x75;
u8 cpuFanSpeed4 @0x76;
u8 cpuFanSpeed5 @0x77;
u8 cpuFanSpeed6 @0x78; // <- see cpuTemp6

u8 gpuRealtimeTemp @0x80;
u8 gpuTemp0 @0x82;
u8 gpuTemp1 @0x83;
u8 gpuTemp2 @0x84;
u8 gpuTemp3 @0x85;
u8 gpuTemp4 @0x86;
u8 gpuTemp5 @0x87;
u8 gpuTemp6 @0x88; // <- unused by MSI Center

u8 gpuRealtimeFanSpeed @0x89;
u8 gpuFanSpeed0 @0x8A;
u8 gpuFanSpeed1 @0x8B;
u8 gpuFanSpeed2 @0x8C;
u8 gpuFanSpeed3 @0x8D;
u8 gpuFanSpeed4 @0x8E;
u8 gpuFanSpeed5 @0x8F;
u8 gpuFanSpeed6 @0x90; // <- see gpuTemp6

u16 cpuRealtimeFanRpm @0xC8;
u16 gpuRealtimeFanRpm @0xCA;

char ecVersion[12] @0xA0;
char ecBuildDate[8] @0xAC;
char ecBuildTime[8] @0xB4;

// Note: You may need to remove right_to_left attributes from bitfields due to bugs with the pattern parser

bitfield x2c {
	padding : 1;
	micMuteLed : 1;
	keyboardLock : 1;
	keyboardLock2 : 1;
	padding : 4;
} [[inline,right_to_left]];
x2c x2c @0x2C;

bitfield x2d {
	padding : 1;
	muteLed : 1;
	padding : 6;
} [[inline,right_to_left]];
x2d x2d @0x2D;

bitfield x2e {
	btEnabled : 1;
	webcamEnabled : 1;
	padding : 1;
	wlanEnabled : 1;
	padding : 4;
} [[inline,right_to_left]];
x2e x2e @0x2E;

bitfield x2f {
	padding : 1;
	webcamHardEnabled : 1; // Blocks regular webcam key
	padding : 6;
} [[inline,right_to_left]];
x2f x2f @0x2F;

bitfield x30 {
    acConnected : 1;
    lidOpen : 1;
    padding : 6;
} [[inline,right_to_left]];
x30 x30 @0x30;

bitfield x98 {
    padding : 7;
    coolerBoost : 1; 
} [[inline,right_to_left]];
x98 x98 @0x98;

// Not used on my model as it has no dedicated GPU
bitfield xd1 {
    graphicsSwitchA : 1;
    graphicsSwitchB : 1;
    padding : 6;
} [[inline,right_to_left]];
xd1 xd1 @0xD1;

/* 
 -- Shift mode
	0xC0 = Overclock to 4.2GHz
	0xC1 = Normal clock speed at 2.8GHz
	0xC2 = Underclock to 2.0GHz
		   Note: Average clock speed won't exceed 2.0GHz, but individual cores still can.
*/
u8 shiftMode @0xD2; 

bitfield xd3 {
	backlight : 4;
    padding : 4;
} [[inline,right_to_left]];
xd3 xd3 @0xD3;

bitfield xd4 {
    padding : 4;
    fanSilent : 1; // <- used by 'silent' scenario
    padding : 1;
    fanUseBasicMode : 1; // <- Auto mode if both false
    fanUseAdvancedMode : 1; //  <-/
} [[inline,right_to_left]];
xd4 xd4 @0xD4;

bitfield xd7 {
    batteryMode : 7; // stop charging at 100; 80 or 60
    padding : 1;
} [[inline,right_to_left]];
xd7 xd7 @0xD7;

bitfield xd9 {
    isScmInstalled : 1;
    isFnLockOn_ReadOnly : 1;
    padding : 6;
} [[inline,right_to_left]];
xd9 xd9 @0xD9;

bitfield xe8 {
    padding : 4;
    isWinFnSwapped : 1;
    padding : 3;
} [[inline,right_to_left]];
xe8 xe8 @0xE8;


