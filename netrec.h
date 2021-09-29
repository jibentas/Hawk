#ifndef __netrec__
#define __netrec__

#pragma pack(push, xplane)
#pragma pack(1)

struct FlightStruct {		// Flight Conditions
	float	pitch;			// Pitch in deg
	float	roll;			// Roll in deg
	float	yaw;			// Yaw in deg
	float	slip;			// Slip in deg  (for slip ball indicator)
	float	Vind;			// Indicated airspeed in knots
	float	Vtrue;			// True airspeed in m/sec ?
	float	Vgrnd;			// Groundspeed in m/sec ?
	float	heading;		// Heading (True)
	float	magheading;		// Heading (Magnetic)
	float	magvar;			// Local Magnetic Variation (deg)
	float	vs;				// Vertical Speed (ft/minute)
	float	altitude;		// Altitude MSL (metres?)
	float	agl;			// Altitude AGL
	float	baro;			// Barometric Pressure
	float	parkingbrake;	// Parking Brake
	float	turnrate;		// Indicated rate of turn in degrees.
	float	AoA;			// Angle of attack in degrees. +ve = Up.
	float	Mach;			// Out current speed in Mach
	float	G;				// G-Force (nrml)
	int		onground_all;	// Is the aircraft on the ground
	int		depres_slow;	// Slow cabin pressure leak
	int		depres_fast;	// Fast cabin pressure leak
	float	cabin_alt_act;	// Actual cabin pressure in altitude
	float	cabin_alt_set;	// Cabin pressure set for pressurization
	int		antiSkid;		// If AntiSkid is on
};

struct RadioStruct {		// Radio Equipment
	int		com1;
	int		com1_standby;
	int		com2;
	int		com2_standby;
	int		nav1;
	int		nav1_standby;
	int		nav1_gs;		// Nav1 has GS
	float	nav1_hdef;
	float	nav1_vdef;
	float	nav1_distance;	// DME distance (nm)
	float	nav1_time;		// DME time (min)
	int		nav1_flags;		// VOR flags. 0=off 1=To 2=From
	int		nav1_has_vertical;	// Nav1 has a vertical signal?
	int		nav1_has_dme;		// Nav1 has a DME ?
	float	nav1_obs;		// Nav1 OBS (deg mag)
	float	nav1_bearing;	// Relative bearing
	char	nav1_id[5];		// Name of station
	int		nav2;
	int		nav2_standby;
	int		nav2_gs;		// Nav2 has GS
	float	nav2_hdef;
	float	nav2_vdef;
	float	nav2_distance;	// DME distance (nm)
	float	nav2_time;		// DME time (min)
	int		nav2_flags;		// VOR flags. 0=off 1=To 2=From
	int		nav2_has_vertical;	// Nav2 has a vertical signal?
	int		nav2_has_dme;		// Nav2 has a DME ?
	float	nav2_obs;		// Nav2 OBS (deg mag)
	float	nav2_bearing;	// Relative bearing
	float	gps_bearing;
	int		transponder;	// Our transponder code 0000-7777
	int		transponder_lit;	// Pinged?
	int		adf1_freq;		// Freq * 100
	int		adf1_standby;
	float	adf1_bearing;	// Relative Bearing
	float	adf1_card;
	int		adf2_freq;		// Freq * 100
	int		adf2_standby;
	float	adf2_bearing;	// Relative Bearing
	float	adf2_card;
	int		dme_freq;		// DME freq * 100
	float	dme_distance;	// Distance in metres
	int		dme_mode;		// DME Mode. 0=Remote, 1=Freq, 2=groundspeed/time
	float	dme_speed;
	float	dme_time;
	int		audiopanel;		// 1=Com1 2=Com2 4=Nav1 8=Nav2 16=adf1 32=adf2 64=DME 128=Marker (bit field)
	float	radar_alt;		// Radar Altimeter Bug
	float	radar_min;		// Bug (DH) setting for radar altimeter.  In Metres ???
	int		navcomadfmode;	// Mode for the 'all-in-one' radio.
	int		RMILeft;		// RMI Left-side: 1=ADF 0=VOR
	int		RMIRight;		// RMI Right-side: 1=ADF 0=VOR
	float	GPSdmedistance;	// Distance to GPS DME in NM
	int		GPShasdme;		// True if the GPS has a dme signal.
	char	GPSnavid[10];	// String rep of station tuned on GPS.
	float	GPStime;		// Time to DME via GPS.
};

struct EnvStruct {			// Environment/Weather
	float	windspeed;		// Windspeed acting on the aircraft (kts)
	float	winddir;		// Wind Direction.
	float	oat;			// Outside Air Temp (deg C)
	float	localtime;		// Local time
	float	timertime;		// Elapsed timer time
	int		timerrunning;	// Is the timer running?
	int		cockpit_lit;	// 1 = Cockpit lights are on (dark outside)
};

struct NavStruct {			// Navigation
	double	latitude;		// Latitude
	double	longitude;		// Longitude
	double	Xlat[20];		// Other AI aircraft latitude.
	double	Xlon[20];		// Other AI aircraft longitude.
	double	Xalt[20];		// Other AI aircraft altitude in Metres.
	float	Xhdg[20];		// Other AI aircraft heading.
	float	Xvsi[20];		// Other AI aircraft vertical speed in m/s.
};

struct EngineStruct {		// Engine stuff
	int		numengines;		// How many engines?
	float	throttle[8];	// Throttle Position (ratio 0-1)
	float	mixture[8];		// Mixture Control (ratio 0-1)
	float	torque[8];		// Torque NM
	float	egtratio[8];	// EGT ratio, 0 -> 700
	float	egtdegc[8];		// EGT in deg C
	float	ittratio[8];	// ITT ratio, 0 -> 700
	float	ittdegc[8];		// ITT in deg C
	float	ff[8];			// Fuel Flow. Kg/Sec
	float	N1[8];			// N1 as a percentage
	float	N2[8];			// N2 as a percentage
	float	vacuum;			//
	float	oil_pressure[8];// Oil Pressure per engine (psi)
	float	oil_temp[8];	// Oil temperature per engine (degC)
	float	oil_quantity[8];// Oil quantity ratio per engine (0-1)
	float	suction[2];		// Ratio of vacuum indicated, 1 per engine.
	float	rpm[8];			// Engine RPM in revs/minute.
	float	cht[8];			// Cylinder Head Temp ('C)
	float	chtratio[8];	// Cylinder Head Temp as a ratio 0-1
};

struct APStruct {			// Autopilot
	int		status;			// 0=off 1=ATHR 2=HDG 4=WLVL 8=Airspeed 16=VS 32=HOLD 64=FLCH 128=PITCH 256=LNAV(red)
	int		mode;			// status contd... 512=LNAV(green) 1024=VNAV(red) 2048=VNAV(green)
	float	altitude;
	float	vs;
	float	airspeed;
	float	heading;
	int		FDmode;			// Flight Director Mode. 0=Off 1=On 2=On with AP Servos.
	float	FDPitch;
	float	FDRoll;
	float	SyncPitch;		// AP Servo calls.
	float	SyncRoll;
	int		GSstatus;		// 0=Off, 1=Armed, 2=Captured.
};


struct SwStruct {			// Switches
	int		battery[8];		// Battery Switch(es)
	int		avionics;		// Avionics
	int		radios;			// Are radios on?  Bitfield!  1=com1, 2=com2, 4=nav1, 8=nav2, 16=ADF1, 32=ADF2, 64=DME, 128=GPS
	int		beacon;			// Beacon light(s)
	int		strobe;			// Strobe light(s)
	int		pitot;			// Pitot Heat
	int		antiice;		// Anti-Ice
	int		yawdamper;		// Yaw Damper
	int		gearhandle;		// Gear Handle Up or Down
	int		fuelpump[8];	// Fuel Pump - 1 per engine.
	int		generator[8];	// Generators - 1 per engine.
	int		landinglight;	// Landing Light.
	int		taxilight;		// Taxi Light.
	int		navlights;		//
	int		propsync;		// On or Off
	int		maprange;		// Range enum for moving maps.
	int		EFIStcas;		// Is the EFIS tcas on? (bool)
	int		EFISairport;	//
	int		EFISfix;		//
	int		EFISvor;		//
	int		EFISndb;		//
	int		EFISwxr;
	int		BUOXYON;		// BackUpOxy Switch Selected
	int		APUgen;
	int		APUstarter;
};	

struct ElecStruct {			// Electrical
	float	hobbs;			// Time on the Hobbs meter (secs)
	int		stall;			// Stall Warning
	int		gpws;			// Ground Proximity Warning
	int		OM;				// Outer Marker is lit
	int		MM;				// Middle Marker is lit
	int		IM;				// Inner Marker is lit
	float	batteryvolts[8];// Volts for each battery.
	float	batteryamps[8];	// Amps per battery
	float	genamps[8];		// Amps per battery.
	int		inverter[2];	// Are inverters on?
	int		igniter[8];		// Are igniters on?
	int		autoignition[8];// Auto Ignition?
	int		ignitionkey[8];	// 0 = off, 1 = left, 2 = right, 3 = both, 4 = starting
	int		cockpitlights;	// 0=off, 1=on
	float	cockpitred;		// Level of RED cockpit lighting.
	float	cockpitgreen;	// Level of GREEN cockpit lighting.
	float	cockpitblue;	// Level of BLUE cockpit lighting.
	int		APUrunning;		// Is the APU running?  0=No, 1=Yes.
	int		NVGs;			// Is Night Vision on
};

struct AirframeStruct {			// Anything Airframe Related.
	float	gear[5];			// Landing gear. 0.0 = Up.
	float	speedbrakeratio;
	float	trim_elevator;		// Elevator Trim. -1=down +1=up  NOTE! These are -0.5 - +0.5
	float	trim_aileron;		// Aileron Trim. -1 -> +1
	float	trim_rudder;		// Rudder Trim. -1 -> +1
	float	flap[2];			// Flap deployment (degrees) +ve is trailing edge down.
	float	flap_ratio;			// Ratio of flap deployed [0..1]
	int		nws;				// Is nose wheel steering on
};

struct AnnStruct {				// Annunciators
	int		ap_disconnect;		//
	int		low_vacuum;			//
	int		low_voltage;		//
	int		fuel_quantity;		//
	int		fuel_pressure[8];	// array of 8
	int		hydraulic_pressure;	//
	int		oil_pressure[8];	// array of 8
	int		oil_temperature[8];	// array of 8
	int		inverter[2];		// array of 2
	int		generator[8];		// array of 8
	int		enginefire[8];
	int		autoignition[8];
	int		speedbrake;
	int		mastercaution;
	int		DHlit;				// Radio Altimeter DH lamp is lit.
	int		ice;				// Ice Detected
	int		chipdetect[8];		// Chip detect, per engine.
};

struct FuelStruct {
	int		numtanks;			// No of fuel tanks in use.
	float	capacity;			// Total Capacity for fuel across ALL tanks.
	float	tank[9];			// Weight of fuel in each possible tank.
	float	ratio[9];			// Ratio of total fuel capacity for each tank.
	int		selected;			// Selected Fuel Tank.
	int		fuelcap;			// Fuel cap left off
	int		fadec[8];			// Is FADEC on (one per engine)
};

struct CameraStruct {
	float	head_x;				// Positions of Pilot's head in x-axis
	float	head_y;				// Positions of Pilot's head in y-axis
	float	head_z;				// Positions of Pilot's head in z-axis
	float	head_yaw;			// Degree of Yaw of Pilot's head
	float	head_pitch;			// Degree of Pitch of Pilot's head
	float	head_roll;			// Degree of Roll of Pilot's head
};

struct NR {						// Our combined Network Structure.
	int  	ID;					// 16-bit Plugin Identifier.
	int		Version;			// X-Plane version.
	FlightStruct	flight;
	RadioStruct		radio;
	EnvStruct		env;
	SwStruct		switches;
	NavStruct		nav;
	ElecStruct		elec;
	AirframeStruct	airframe;
	EngineStruct	engine;
	AnnStruct		ann;
	APStruct		autopilot;
	FuelStruct		fuel;
	CameraStruct	cam;
};
#pragma pack(pop, xplane)


// COMMANDS FOR SENDING TO THE PLUGIN.
#define pcParkingBrakeON	1
#define pcParkingBrakeOFF	2
#define pcAvionicsON		3
#define pcAvionicsOFF		4
#define pcLandingLightON	5
#define pcLandingLightOFF	6
#define pcBeaconON			7
#define pcBeaconOFF			8
#define pcStrobeON			9
#define pcStrobeOFF			10
#define pcNavLightON		11
#define pcNavLightOFF		12
#define pcPitotHeatON		13
#define pcPitotHeatOFF		14
#define pcPropSyncON		15
#define pcPropSyncOFF		16
#define pcADF1_1DOWN		17
#define pcADF1_1UP			18
#define pcADF1_2DOWN		19
#define pcADF1_2UP			20
#define pcADF1_3DOWN		21
#define pcADF1_3UP			22
#define pcYawDamperON		23
#define pcYawDamperOFF		24
#define pcAntiIceON			25
#define pcAntiIceOFF		26
#define pcFuelPumpON		27
#define pcFuelPumpOFF		28
#define pcInverterON		29
#define pcInverterOFF		30
#define pcBatteryON			31
#define pcBatteryOFF		32
#define pcTransponderUP		33			// Digit to change is in inbuf.value
#define pcTransponderDOWN	34
#define pcRadioAltUP		35
#define pcRadioAltDOWN		36
#define pcCom1MajUP			37
#define pcCom1MajDOWN		38
#define pcCom1MinUP			39
#define pcCom1MinDOWN		40
#define pcNav1MajUP			41
#define pcNav1MajDOWN		42
#define pcNav1MinUP			43
#define pcNav1MinDOWN		44
#define pcCom2MajUP			45
#define pcCom2MajDOWN		46
#define pcCom2MinUP			47
#define pcCom2MinDOWN		48
#define pcNav2MajUP			49
#define pcNav2MajDOWN		50
#define pcNav2MinUP			51
#define pcNav2MinDOWN		52
#define pcGeneratorON		53
#define pcGeneratorOFF		54
#define pcMapRangeUP		55
#define pcMapRangeDOWN		56
#define pcIgniterON			57
#define pcIgniterOFF		58
#define pcAutoIgnitionON	59
#define pcAutoIgnitionOFF	60
#define pcFlightDirectorOFF	61
#define pcFlightDirectorON	62
#define pcFlightDirectorAUTO	63
#define pcFuelSelectOFF		64
#define pcFuelSelectLEFT	65
#define pcFuelSelectRIGHT	66
#define pcFuelSelectBOTH	67
#define pcAudioPanelCOM1	68		// val = 0=off 1=on
#define pcAudioPanelCOM2	69		// val = 0=off 1=on
#define pcAudioPanelNAV1	70		// val = 0=off 1=on
#define pcAudioPanelNAV2	71		// val = 0=off 1=on
#define pcAudioPanelADF1	72		// val = 0=off 1=on
#define pcAudioPanelADF2	73		// val = 0=off 1=on
#define pcAudioPanelDME		74		// val = 0=off 1=on
#define pcAudioPanelMarker	75		// val = 0=off 1=on
#define pcBaroUP			76		// Baro up 0.05in
#define pcBaroDOWN			77		// Baro down 0.05in
#define pcNav1OBS			78		// VAL = -1 or +1
#define pcNav2OBS			79		// VAL = -1 or +1
#define pcSetNavComAdfMODE	80		// VAL = required mode. 0=Nav1, 1=Nav2, 2=Com1, 3=Com2, 4=ADF1, 5=ADF2
#define pcSwapCOM1			81		// Swap com1 - com1_standby freq's
#define	pcSwapCOM2			82		// Swap com2...
#define pcSwapNAV1			83		// Swap nav1...
#define pcSwapNAV2			84		// Swap nav2...
#define pcSwapADF1			85		// Swap adf1...
#define	pcSwapADF2			86		// Swap adf2...
#define pcCom1stMajUP		87
#define pcCom1stMajDOWN		88
#define pcCom1stMinUP		89
#define pcCom1stMinDOWN		90
#define pcNav1stMajUP		91
#define pcNav1stMajDOWN		92
#define pcNav1stMinUP		93
#define pcNav1stMinDOWN		94
#define pcCom2stMajUP		95
#define pcCom2stMajDOWN		96
#define pcCom2stMinUP		97
#define pcCom2stMinDOWN		98
#define pcNav2stMajUP		99
#define pcNav2stMajDOWN		100
#define pcNav2stMinUP		101
#define pcNav2stMinDOWN		102
#define pcADF2_1DOWN		103
#define pcADF2_1UP			104
#define pcADF2_2DOWN		105
#define pcADF2_2UP			106
#define pcADF2_3DOWN		107
#define pcADF2_3UP			108
#define pcADF1s_1DOWN		109
#define pcADF1s_1UP			110
#define pcADF1s_2DOWN		111
#define pcADF1s_2UP			112
#define pcADF1s_3DOWN		113
#define pcADF1s_3UP			114
#define pcADF2s_1DOWN		115
#define pcADF2s_1UP			116
#define pcADF2s_2DOWN		117
#define pcADF2s_2UP			118
#define pcADF2s_3DOWN		119
#define pcADF2s_3UP			120
#define pcEFISwxr			121
#define pcEFIStcas			122
#define pcEFISairport		123
#define pcEFISfix			124
#define pcEFISvor			125
#define pcEFISndb			126
#define pcRMIleftmode		127
#define pcRMIrightmode		128
#define pcGearHandleDown	129
#define pcGearHandleUp		130
#define pcTrimUpRight		131		// Data byte gives surface type.
#define pcTrimDownLeft		132		// 1=Elevator, 2=Aileron, 3=Rudder.
#define	pcDMEMajDown		133
#define	pcDMEMinDown		134
#define pcDMEMinUp			135
#define	pcDMEMajUp			136
#define	pcDMESetMode		137
#define pcFADECON			138
#define	pcFADECOFF			139
#define pcBUOXYON			140
#define pcBUOXYOFF			141
#define pcEngStrOn			142
#define pcEngStrOff			143
#define pcEngOn				144
#define pcEngOff			145
#define	pcSeatUp			146
#define	pcSeatDown			147
#define pcCamDefault		148
#define pcAntiSkidOn		149
#define pcAntiSkidOff		150
#define pcNVGToggle			151
#define pcAPUOn				152
#define pcAPUOff			153
/*

NOTES:

Trim values from X-Plane 9.70 are wrong. Instead of -1 to +1, they are:
Aileron: +/- 0.25
Elevator: +/- 0.5
Rudder: +/- 0.25


1Kg = 2.20462262 lb

AP_status
0=AP off
1=ATHR on
2=HDG on
4=Wing Leveller on
8=Air Speed Hold (hold via pitch rather than throttle)
16=V/S on
32=HOLD on
64=FLCH on
128=PITCH on
256=LNAV on but inactive (red)
512=LNAV on and active (green)
1024=VNAV on but inactive (red)
2048-VNAV on and active (green)

AP_mode
0=off
1=Flight Director
2=On

EFISmode
0=none
1=TCAS
2=Airports
4=Waypoints
8=VORS
16=NDBs


Va	Maneuvering Speed is the maximum speed at which application of full available
	aerodynamic control will not overstress the airplane.

Vfe	Maximum Flap Extended Speed is the highest speed permissible with wing flaps
	in a prescribed extended position

Vle	Maximum Landing Gear Extended Speed is the maximum speed at which an airplane
	can be safely flown with the landing gear extended

Vlo	Maximum Landing Gear Operating Speed is the maximum speed at which the landing
	gear can be safely extended or retracted.

Vne	Never Exceed Speed is the speed limit that may not be exceeded at any time.

Vno	Maximum Structural Cruising Speed is the speed that should not be exceeded
	except in smooth air and then only with caution.

Vs	Stalling Speed or the minimum steady flight speed at which the airplane is
	controllable.

Vso	Stalling Speed or the minimum steady flight speed at which the airplane is
	controllable in the landing configuration.

Vx	Best Angle-of-Climb Speed is the airspeed which delivers the greatest gain
	of altitude in the shortest possible horizontal distance.

Vy	Best Rate-of-Climb Speed is the airspeed which delivers the greatest gain
	in altitude in the shortest possible time.

Vy	Best Rate-of-Climb Speed is the airspeed which delivers the greatest gain
	in altitude in the shortest possible time.



(*)  MAP RANGE
0 = 10Nm
1 = 20
2 = 40
3 = 80
4 = 160
5 = 320
6 = 640

*/


#endif
