#define PROGNAME "X-Panel for Hawk"
#define VERSION	 "0.0.38"
#define AUTHOR	 "Roy Coates/Jacob Phillips, 2018-20"

/*

		XXXXXXXXXXXXXXXXXXXXXXX
		X   IMPORTANT NOTE:   X
		XXXXXXXXXXXXXXXXXXXXXXX

		This program MUST be compiled for 32-bit to retain compatibility with the Phidget21 library!

		EDIT strcpy(REMOTEHOST, "172.16.2.1");	// IP of X-Plane Master TO THE IP OF THE CLIENT

*/


/*
	0.0.03		12-08-2017		Draw outline around each instrument area. VSI, AI and Compass now to correct scale.
	0.0.04		13-08-2017		Added most of the EMP panel.
	0.0.05		14-08-2017		Adding standby Altimeter.
	0.0.06		15-08-2017		Much work on finishing standby instrument functionality.
	0.0.07		25-08-2017		Enabled SDL Joystick handing. Detects connected bodnar board.
	0.0.08		25-08-2017		Started new MFD mode handling code.
	0.0.09		26-08-2017		Much work on startup routine plus added pString96 font and Joystick recognition.
	0.0.10		18-09-2017		Added network capability and plugin. 
	0.0.12		07-12-2017		Change of IP with move to 192.168.2 subnet.
	. . .
	0.0.15		06-07-2018		Updated to work with 2 Bodnar's and 3 Phidgets.
	0.0.16		10-07-2018
	0.0.17		11-07-2018		More tweaks.
	0.0.18		11-07-2018		Added Bodnar test code.
	0.0.19		12-07-2018
	0.0.20		16-11-2018		Adding code to handle MFD buttons.
	0.0.21		09-11-2018		Getting switches working.
	0.0.22		10-11-2018		Added airspeed pointer.  Removed ENTER on startup.
	0.0.23		10-11-2018		Added more info to ADI display.
								Map Range now has limits (0-5)
	0.0.24		11-11-2018		CWP YAW Warning was on wrong Phidget.  Oops, my bad.
								Added Windspeed/Dir to ADI
	0.0.25		07-Jul-2019		Catch-up.
	0.0.26		11-Jul-2019		Added GO, NWS (Green & Yellow) & CPR Phidget Lights
	0.0.27		12-Jul-2019		Added LP OFF & PITOT Phidget Lights
	0.0.28		17-Jul-2019		Added LP OFF & FADEC Switches
	0.0.29		18-Jul-2019		Added MFC & BUOXY Phidget Light as well as BUOXY & Engine Start Switch
	0.0.30		22-Jul-2019		Added Seat Raise and Lower Switch, also test for warning panel and nvg switch
	0.0.31		24-Sep-2019		Modified for new multi-player IP address range  172.16.2.x
	0.0.32		07-Oct-2019		Fixed annoying CAS: output.
	. . .
	0.0.33		05-02-2020		JB - Fixed numbering issue on MFD displays
	0.0.34b1	07-02-2020		JB - Added basic HSI compass rose to replace dummy texture
	0.0.34b2	09-02-2020		JB - Added EFRCS/checklist display (mfdEFRCS 7)
									 Changed label[9] on most screens to display CHK, and change screens when pressed
	0.0.34b3	11-02-2020		JB - Added DUMMY screen (mfdDUMMY 8)
									 Added txHSICOMPASS, bound to HSI screen w/ lubber line + HDG M display + triangle
	0.0.34b4	13-02-2020		JB - Fixed annoying labelling issue on dummy tx - DrawLabels(LEFT) -> DrawLabels(n) oopsie
									 AVIONICS BOX - fixed SDLK_0 error on phidget exe - button 1, 11, 21, 31 now ,0 ;0 /0 NULLNULL respectively
	0.0.34b5	14-02-2020		JB - hap valentine
								   - too much to note, see https://pastebin.com/XMPZW4zP
	0.0.34		17-02-2020		JB - The battery off update: https://pastebin.com/fS8nzTWP
	0.0.35		18-02-2020		JB - EMP additions and general optimisations, bug fixing https://pastebin.com/MpZCfsbb
	0.0.36		25-02-2020		JB - Added initial screen to force switchgear to OFF state
								   - Added simultaneous EFRCS warping
								   - Other small improvements https://pastebin.com/hZaVdB9C
	0.0.37		27-02-2020		JB - Changed colour of APU, IGN, and SRG from AMBER and DARKAMBER to GREEN and DARKGREEN
								   - Added fire indicator
	0.0.38		02-03-2020		JB - Added a peak neg G display
								   - Completed FIRE and APU FIRE indicators https://pastebin.com/01rzMPHn

*/

#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"Glu32.lib")
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>	
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_syswm.h"
#include "SDL_joystick.h"
#include "texture.h"
#include "netrec.h"
#include "utils.h"
#include "globals.h"
//#include "Xgui.h"
#include "inbuf.h"
#include "lodepng.h"
#include <phidget21.h>
#include "hershey.h"


int OPENGL_MAJOR_VERSION = 2;
int OPENGL_MINOR_VERSION = 1;
int testDone;

float	DEBUGAIRSPEED = 0.0f;

SDL_Window*		screen;				// Was SDL_Surface for SDL 1.2
SDL_Event		event;
GLint			sWIDTH = 1920;		//1366	Our initial screen size.
GLint			sHEIGHT = 1080;		//768;
GLint			tick = 0;			// Elapsed time in seconds.
GLint			oldtick = 0;
GLint			halftick = 0;
GLint			oldhalftick = 0;

GLfloat	R = 1.0f;
GLfloat G = 1.0f;
GLfloat B = 1.0f;

// Our background Colour.
GLfloat	BackR = 0.0f;
GLfloat	BackG = 0.0f;
GLfloat	BackB = 0.0f;
GLfloat	BackAlpha = 1.0f;

// These are used by the logging system.
FILE		*logfile;				// Handle for our logfile.
SYSTEMTIME	sysclock;
GLchar		Monthname[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
GLchar		Dayname[7][10] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

// Array of textures.
Texture		tex[16];			// Textures.
GLuint		base;				// Base Display List For The general purpose Font

// Define names/numbers for our textures.
#define txPARTS		0	// Random support parts.
#define txFONT		1	// Our general purpose (but complete) font.
#define txVSI		2	// Standby VSI
#define txADI		3	// Standby ADI
#define txHEADING	4	// Standby Heading Indicator
#define txALTIMETER	5	// Standby Altimeter face.
#define txAIRSPEED	6	// Standby Airspeed face.
#define txATTITUDE	7	// Large (1024x1024) attitude background.
#define txADIMASK	8	// mask for above
#define LINEAR		9	// Temp texture bank for map from X-Panel
#define SUPPORT1	10	// Temp ....  as above.
#define DUMMY		11	// Dummy graphic
#define	txADICOMPASS	12	// ADI compass rose
#define txHSICOMPASS	13  // HSI compass rose
#define txBATTFAIL		14  // Battery fail screen

// SOCKET STUFF
// Data from X-Plane plugin.
NR			netbuf;					// This holds our incoming data.
GLint		insocket;				// RX Socket Handle from X-Plane OR from a Master X-Panel.
GLint		INPORT = 59500;			// Socket Port we're listening on.  (59500)
char		REMOTEHOST[16];			// IP of remote host RUNNING X-PLANE.
#define		WSANOTINITIALIZED 10093
WSADATA		wsaData;				// Winsock Init Data Record.
DWORD		nonBlocking = 1;		// Settings for the UDP sockets.
typedef		GLint socklen_t;
GLint		rxbytes;
GLint		lasterror;
unsigned	int	maximum_packet_size = sizeof(netbuf);
sockaddr_in from;
socklen_t	fromLength = sizeof(from);
bool		TRIGGER = TRUE;			// True if we got net-data from X-Plane.
int			cmdsocket;				// Socket for sending TO x-plane.
sockaddr_in	sockbuf;
int			CMDPORT = 60001;		// Port we talk TO x-plane with.
NRIN		outbuf;					// Outgoing data buffer (for commands to X-Plane).


// Text Input
SDL_Keycode		Key;				// Key last pressed.
char		keypress;				// Current keypress to handle.
SDL_Keycode	MFDKey = NULL;

// BODNAR
GLint	numjoys = 0;		// How many are there?
SDL_Joystick	*Joy[5];	// Allow for 5 Bodnar/HID boards.
GLint	bodnar[10];			// Array for indexing bodnar boards.
GLint	BODNAR1 = FALSE;	// These become true if the board is found.
GLint	BODNAR2 = FALSE;
GLint	BODNAR3 = FALSE;
int Button[10][32];			// Button[bodnar 1-3][Button 0-31]
int OldButton[10][32];

// PHIDGET 
GLint	PhidgetDelay = 500;					// No of mS to wait on a phidget to attach.
//Declare an InterfaceKit handle
CPhidgetInterfaceKitHandle ifKit1 = 0;
CPhidgetInterfaceKitHandle ifKit2 = 0;
CPhidgetInterfaceKitHandle ifKit3 = 0;
CPhidgetInterfaceKitHandle ifKit4 = 0;
GLint	Kit1ID = 346890;			// 
GLint	Kit2ID = 476259;			// 
GLint	Kit3ID = 501742;			// 
GLint	Kit4ID = 81250;				// 
bool	Kit1 = false;
bool	Kit2 = false;
bool	Kit3 = false;
bool	Kit4 = false;


bool quit = false;
GLchar	sbuf[255];		// General purpose string buffer.
GLchar	sbuf2[255];		// General purpose string buffer.


// MFD positions.
int		mfd_size = 450;
int	mfdx[3] = { 0, 736, 1477 };
int mfdy[3] = { 220, 5, 220 };
// Names for the MFD's
#define	LEFT	0
#define	CENTRE	1
#define	RIGHT	2
// Modes of operation for the 3 MFD's.
#define mfdOFF	0		// MFD is off.  Does nothing.
#define	mfdINIT	1		// This is for the master MFD - to do initialisation of devices etc.
#define mfdHSI	2		// Show the HSI.
#define mfdADI	3		// Show the PFD style ADI.
#define	mfdMAP	4		// Show the moving map.
#define mfdDEBUG 5		// Show debug info.
#define mfdELEC	6		// Show Electrical Systems
// Mode Control for the 3 MFD's.
int	mode[3] = { mfdINIT, 0, 0 };			// main mode (startup, map, etc)
int submode[3] = { 0, 0, 0 };		// sub-mode... 
int mfdtimer[3] = { 0, 0, 0 };		// General timers for each MFD.
#define mfdEFRCS 7		// Show checklist
#define mfdDUMMY 8		// Show dummy texture
#define mfdEFRCS2 9		// Show checklist page 2
#define mfdEFRCS3 10	// Show checklist page 3
#define mfdINISTAT 11 // initial status of switchgear
bool	done = false;

// Labels for MFD's
char	label[11][11];
int		clabel[11];

// Switch Input Control (KE108)
int		Input = -1;			// Determines which MFD the next key is for. LEFT, CENTRE or RIGHT.   -1 = None.

// Misc Flight Data
float	PeakG = 0.0f;		// Keeps track of peak G
float	PeakNegG = 0.0f;	// Keeps track of peak negative G

// DEBUG VARIABLES
float VSIangle = 0.0f;		// Rotation Angle.
float RPM = 0.0f;
float RPMdir = 0.25f;
float TTemp = 400.0f;		// turbine temp
float TTempdir = 1.0f;
int ODO = 0;
float alt = 0.0;
float altdir = 2.5;
float pitch = 0.0;
float pitchdir = 0.05f;
float roll = 0.0;
float rolldir = 0.05f;


// Nav Data stuff from X-Panel.
struct FIXREC {
	double	lat;
	double	lon;
	char	name[6];
};
FIXREC	Fix[130000];
int		fixcnt = 0;
struct NAVREC{
	int	type;
	double	lat;
	double	lon;
	int		freq;
	float	slave;
	float	bearing;		// ILS bearing in TRUE degrees.
	char	runway[4];		// ILS
	char	apticao[5];		// ILS Airport ICAO
	char	ident[5];
	char	name[50];
};
NAVREC	navaid[30000];			// Array of Navaids.
int		navcnt = 0;				// No of Navaids counter.
struct APTREC{
	char ICAO[5];		// ICAO code for airport.
	char name[50];		// Common name.
	int  elev;			// Elevation in feet AMSL.
	double	lat;		// Position of Airport - for mapping only!
	double	lon;
	int		runways;	// How many runways do we have ?
	int		category;	// Use this to store runway surface type.
	int		tag;		// Use this to keep track in cached list.
};
APTREC	Airport[35000];
int		aptcnt = 0;
struct RWYREC{
	int		airport;	// Array index of the airport this runway belongs to.
	float	width;		// Width of runway in metres.
	int		surface;	// Surface. 1=Asphalt 2=Concrete 3=Grass 4=Dirt 5=Gravel 12=lakebed 13=water 14=ice
	char	name1[5];	// eg: 16L or 28R or 90C
	double	lat1;		// Latitude of threshold in decimal degrees.
	double	lon1;		// Longitude ... same.
	char	name2[5];	// eg: 16L or 28R or 90C
	double	lat2;		// Latitude of threshold in decimal degrees.
	double	lon2;		// Longitude ... same.
	double  length;		// Length of Runway in Metres.
	double	bearing;	// Bearing of Runway FOR INTERNAL USE ONLY!!!!!!!!
};
RWYREC	Runway[35000];
int		rwycnt = 0;

bool	MAP_NORTH = FALSE;		// Which way is up on ALL maps?
int		NAVTEST = 0;



// David Allerton's Font Data.
static unsigned int gfont8Offsets[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4, 14, 18, 36,
	42, 60, 82, 90, 108, 130, 138, 172, 194, 194,
	194, 194, 194, 194, 194, 194, 208, 230, 250, 264,
	278, 290, 310, 322, 334, 348, 362, 368, 378, 386,
	404, 418, 436, 456, 480, 488, 500, 506, 516, 526,
	536, 544, 544, 544, 544, 544, 544, 544, 544, 544,
	544, 544, 544, 544, 544, 544, 544, 544, 544, 544,
	544, 544, 544, 544, 544, 544, 544, 544, 544, 544,
	544, 544, 544, 544, 544, 544, 544, 544 };

static unsigned int gfont8Size[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 5, 2, 9, 3,
	9, 11, 4, 9, 11, 4, 17, 11, 0, 0,
	0, 0, 0, 0, 0, 7, 11, 10, 7, 7,
	6, 10, 6, 6, 7, 7, 3, 5, 4, 9,
	7, 9, 10, 12, 4, 6, 3, 5, 5, 5,
	4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0 };

static unsigned char gfont8Tab[544] = {
	0, 4, 4, 4, 4, 1, 5, 1, 5, 0, 4, 0, 4, 1, 0, 0, 4, 8, 0, 1,
	0, 7, 1, 8, 3, 8, 4, 7, 4, 1, 3, 0, 1, 0, 0, 1, 0, 6, 2, 8,
	2, 0, 0, 5, 0, 7, 1, 8, 3, 8, 4, 7, 4, 5, 0, 2, 0, 0, 4, 0,
	0, 7, 1, 8, 3, 8, 4, 7, 4, 5, 1, 4, 4, 3, 4, 1, 3, 0, 1, 0,
	0, 1, 3, 0, 3, 8, 0, 3, 4, 3, 4, 8, 0, 8, 0, 5, 3, 5, 4, 4,
	4, 1, 3, 0, 1, 0, 0, 1, 4, 7, 3, 8, 1, 8, 0, 7, 0, 1, 1, 0,
	3, 0, 4, 1, 4, 3, 3, 4, 0, 4, 0, 8, 4, 8, 4, 5, 0, 0, 3, 4,
	1, 4, 0, 3, 0, 1, 1, 0, 3, 0, 4, 1, 4, 3, 3, 4, 1, 4, 0, 5,
	0, 7, 1, 8, 3, 8, 4, 7, 4, 5, 3, 4, 0, 1, 1, 0, 3, 0, 4, 1,
	4, 7, 3, 8, 1, 8, 0, 7, 0, 5, 1, 4, 4, 4, 0, 0, 0, 4, 2, 8,
	4, 4, 4, 0, 4, 4, 0, 4, 2, 4, 4, 3, 4, 1, 3, 0, 0, 0, 0, 8,
	3, 8, 4, 7, 4, 5, 2, 4, 0, 4, 4, 5, 4, 7, 3, 8, 1, 8, 0, 7,
	0, 1, 1, 0, 3, 0, 4, 1, 4, 3, 0, 8, 0, 0, 3, 0, 4, 1, 4, 7,
	3, 8, 0, 8, 4, 0, 0, 0, 0, 8, 4, 8, 0, 8, 0, 4, 4, 4, 0, 0,
	0, 8, 4, 8, 0, 8, 0, 4, 4, 4, 4, 7, 3, 8, 1, 8, 0, 7, 0, 1,
	1, 0, 3, 0, 4, 1, 4, 4, 2, 4, 0, 0, 0, 8, 0, 4, 4, 4, 4, 8,
	4, 0, 0, 0, 4, 0, 2, 0, 2, 8, 4, 8, 0, 8, 0, 8, 4, 8, 2, 8,
	2, 0, 1, 0, 0, 1, 0, 3, 0, 0, 0, 8, 0, 4, 2, 4, 4, 8, 2, 4,
	4, 0, 0, 8, 0, 0, 4, 0, 0, 0, 0, 8, 2, 4, 4, 8, 4, 0, 0, 0,
	0, 8, 4, 0, 4, 8, 0, 1, 0, 7, 1, 8, 3, 8, 4, 7, 4, 1, 3, 0,
	1, 0, 0, 1, 0, 0, 0, 8, 3, 8, 4, 7, 4, 5, 3, 4, 0, 4, 4, 0,
	1, 0, 0, 1, 0, 7, 1, 8, 3, 8, 4, 7, 4, 0, 1, 3, 0, 0, 0, 8,
	3, 8, 4, 7, 4, 5, 3, 4, 0, 4, 3, 4, 4, 3, 4, 0, 4, 7, 3, 8,
	1, 8, 0, 7, 0, 5, 1, 4, 3, 4, 4, 3, 4, 1, 3, 0, 1, 0, 0, 1,
	0, 8, 4, 8, 2, 8, 2, 0, 0, 8, 0, 1, 1, 0, 3, 0, 4, 1, 4, 8,
	0, 8, 2, 0, 4, 8, 0, 8, 0, 0, 2, 4, 4, 0, 4, 8, 0, 8, 4, 0,
	2, 4, 4, 8, 0, 0, 0, 8, 2, 4, 4, 8, 2, 4, 2, 0, 0, 8, 4, 8,
	0, 0, 4, 0 };

static unsigned int gfont12Offsets[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 10, 10, 28,
	34, 50, 76, 84, 102, 126, 132, 164, 186, 186,
	186, 186, 186, 186, 186, 186, 200, 220, 236, 250,
	264, 276, 296, 308, 320, 334, 346, 352, 362, 370,
	388, 402, 424, 442, 466, 474, 480, 486, 496, 506,
	516, 524, 524, 524, 524, 524, 524, 524, 524, 524,
	524, 524, 524, 524, 524, 524, 524, 524, 524, 524,
	524, 524, 524, 524, 524, 524, 524, 524, 524, 524,
	524, 524, 524, 524, 524, 524, 524, 524 };

static unsigned int gfont12Size[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 5, 0, 9, 3,
	8, 13, 4, 9, 12, 3, 16, 11, 0, 0,
	0, 0, 0, 0, 0, 7, 10, 8, 7, 7,
	6, 10, 6, 6, 7, 6, 3, 5, 4, 9,
	7, 11, 9, 12, 4, 3, 3, 5, 5, 5,
	4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0 };

static unsigned char gfont12Tab[524] = {
	3, 0, 5, 0, 5, 2, 3, 2, 3, 0, 0, 2, 0, 10, 2, 12, 4, 12, 6, 10,
	6, 2, 4, 0, 2, 0, 0, 2, 0, 8, 3, 12, 3, 0, 0, 10, 2, 12, 4, 12,
	6, 10, 6, 7, 0, 2, 0, 0, 6, 0, 0, 10, 2, 12, 4, 12, 6, 10, 6, 8,
	4, 6, 3, 6, 4, 6, 6, 4, 6, 2, 4, 0, 2, 0, 0, 2, 4, 0, 4, 12,
	0, 5, 6, 5, 0, 2, 2, 0, 4, 0, 6, 2, 6, 6, 4, 8, 0, 8, 0, 12,
	6, 12, 6, 10, 4, 12, 2, 12, 0, 10, 0, 6, 0, 2, 2, 0, 4, 0, 6, 2,
	6, 4, 4, 6, 0, 6, 0, 12, 6, 12, 0, 0, 1, 6, 0, 8, 0, 10, 2, 12,
	4, 12, 6, 10, 6, 8, 4, 6, 1, 6, 0, 4, 0, 2, 2, 0, 4, 0, 6, 2,
	6, 4, 4, 6, 0, 2, 2, 0, 4, 0, 6, 2, 6, 10, 4, 12, 2, 12, 0, 10,
	0, 8, 2, 6, 6, 6, 0, 0, 0, 8, 3, 12, 6, 8, 6, 0, 6, 5, 0, 5,
	0, 0, 0, 11, 4, 11, 6, 9, 6, 7, 4, 5, 6, 4, 6, 2, 4, 0, 0, 0,
	6, 10, 4, 12, 2, 12, 0, 10, 0, 2, 2, 0, 4, 0, 6, 2, 0, 0, 4, 0,
	6, 2, 6, 10, 4, 12, 0, 12, 0, 0, 6, 11, 0, 11, 0, 6, 6, 6, 0, 6,
	0, 0, 6, 0, 6, 11, 0, 11, 0, 6, 6, 6, 0, 6, 0, 0, 6, 10, 4, 12,
	2, 12, 0, 10, 0, 2, 2, 0, 4, 0, 6, 2, 6, 6, 2, 6, 0, 0, 0, 11,
	0, 6, 6, 6, 6, 11, 6, 0, 2, 11, 6, 11, 4, 11, 4, 0, 6, 0, 2, 0,
	0, 2, 1, 0, 3, 0, 4, 2, 4, 11, 2, 11, 6, 11, 0, 11, 0, 0, 0, 5,
	6, 9, 0, 5, 6, 0, 0, 11, 0, 0, 6, 0, 0, 0, 0, 12, 3, 7, 6, 12,
	6, 0, 0, 0, 0, 11, 6, 0, 6, 11, 0, 2, 0, 10, 2, 12, 4, 12, 6, 10,
	6, 2, 4, 0, 2, 0, 0, 2, 0, 0, 0, 11, 4, 11, 6, 9, 6, 6, 4, 5,
	0, 5, 5, 1, 4, 0, 2, 0, 0, 2, 0, 10, 2, 12, 4, 12, 6, 10, 6, 2,
	5, 1, 3, 3, 0, 0, 0, 11, 4, 11, 6, 9, 6, 6, 4, 5, 0, 5, 4, 5,
	6, 0, 6, 9, 4, 11, 2, 11, 0, 9, 0, 8, 2, 6, 4, 6, 6, 4, 6, 2,
	4, 0, 2, 0, 0, 2, 0, 11, 6, 11, 3, 11, 3, 0, 0, 11, 3, 0, 6, 11,
	0, 11, 3, 0, 6, 11, 0, 11, 0, 0, 3, 7, 6, 0, 6, 11, 0, 11, 6, 0,
	3, 5, 6, 11, 0, 0, 0, 11, 3, 7, 6, 11, 3, 7, 3, 0, 0, 11, 6, 11,
	0, 0, 6, 0 };


// Make a log entry.
void log(GLchar *entry)
{
	fprintf(logfile, "%s", entry);
	fflush(logfile);
}


// **************************************************************************************
// Create the network socket to RX data from X-Plane.
// **************************************************************************************
int CreateInputSocket(void)
{
	fprintf(logfile, "Initialising Input Network Interface - ");  fflush(logfile);
	GLint ierr;
	ierr = WSAStartup(MAKEWORD(2, 2), &wsaData);				// Start WinSock.
	if (ierr != 0) return 1;									// No Winsock 2.2
	insocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (insocket <= 0) { return 2; }						// Failed to create socket.
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons((unsigned short)INPORT);										// Set the input PORT.
	if (bind(insocket, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) { return 3; }	// Failed to BIND
	if (ioctlsocket(insocket, FIONBIO, &nonBlocking) != 0) { return 4; }					// Failed to set non-blocking.
	fprintf(logfile, "OK.\n");	fflush(logfile);
	return 0;
}



// **************************************************************************************
// Create the network socket to TX data to X-Plane.
// **************************************************************************************
int	CreateCommandSocket(void)
{
	fprintf(logfile, "Initialising Return Network Interface - ");  fflush(logfile);
	int ierr = WSAStartup(MAKEWORD(2, 2), &wsaData);			// Start WinSock.
	if (ierr != 0) return 1;		// No Winsock 2.2
	cmdsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// if SocketOut <= 0 then failed.
	sockbuf.sin_family = AF_INET;
	sockbuf.sin_port = htons(CMDPORT);
//	strcpy(REMOTEHOST, "192.168.2.1");
	strcpy(REMOTEHOST, "172.16.2.1");			// Master machine on multiplayer network.
	//	strcpy(REMOTEHOST,"127.0.0.1");
	sockbuf.sin_addr.s_addr = inet_addr(REMOTEHOST);
	if (ierr == 0) { fprintf(logfile, "OK.\n");	fflush(logfile); } else { fprintf(logfile, "FAILED.\n");  fflush(logfile); }
	return ierr;
}


// Send a packet TO X-PLANE.
void SendBuffer(void)
{
	sendto(cmdsocket, (const char*)&outbuf, sizeof(outbuf), 0, (struct sockaddr *)&sockbuf, sizeof(sockbuf));
}


void SendCommand(GLint i, GLint n)
{
	outbuf.ID = 1;
	outbuf.cmnd = i;
	outbuf.value = n;
	SendBuffer();
}



// **************************************************************************************
// Print a string using our general purpose font.
// **************************************************************************************
GLvoid glPrint(GLint x, GLint y, char *string, int set)	// Where The Printing Happens
{
	if (set>1) { set = 1; }
	glBindTexture(GL_TEXTURE_2D, tex[txFONT].texID);			// Select Our Font Texture
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glTranslated(x, y, 0);								// Position The Text (0,0 - Bottom Left)
	glListBase(base - 32 + (128 * set));						// Choose The Font Set (0 or 1)
	glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);// Write The Text To The Screen
	glPopMatrix();										// Restore The Old Projection Matrix
}

GLvoid glPrintf(GLfloat x, GLfloat y, char *string, int set)	// Where The Printing Happens
{
	if (set>1) { set = 1; }
	glBindTexture(GL_TEXTURE_2D, tex[txFONT].texID);			// Select Our Font Texture
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glTranslatef(x, y, 0);								// Position The Text (0,0 - Bottom Left)
	glListBase(base - 32 + (128 * set));						// Choose The Font Set (0 or 1)
	glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);// Write The Text To The Screen
	glPopMatrix();										// Restore The Old Projection Matrix
}



// **************************************************************************************
// Check (and get) Net Data.
// **************************************************************************************
void GetNetData(void)
{
	if (insocket != INVALID_SOCKET)		// ***** CHECK FOR NETWORK DATA *****
	{
		//memset((char *)&netbuf,0,sizeof(netbuf));
		rxbytes = recvfrom(insocket, (char*)&netbuf, maximum_packet_size, 0, (sockaddr*)&from, &fromLength);
		lasterror = WSAGetLastError();
		if (lasterror != WSANOTINITIALIZED && lasterror != WSAEWOULDBLOCK) { TRIGGER = TRUE; }
	}
	sprintf(sbuf, "(%d)", rxbytes);
	glPrint(10, 10, sbuf, 0);
}



void Circle2(int x, int y, int r)
{
	int y1 = y + r;
	int x1 = x;
	glBegin(GL_LINE_STRIP);
	for (float angle = 0.0f; angle <= (2.01f*pi); angle += 0.02f)
	{
		int x2 = int(x + r * hisin(angle));
		int y2 = int(y + r * hicos(angle));
		glVertex2d(x1, y1);
		y1 = y2;
		x1 = x2;
	}
	glEnd();
}



// From X-Panel.
// Read the earth_fix.dat file.
void LoadFixes(void)
{
	FILE	*fix;
	char	inbuf[1024];
	int		i;
	char	*tokenptr;
	int		column = 0;
	char	*S[128];
	fix = fopen("navdata/earth_fix.dat", "r");
	fgets(inbuf, sizeof(inbuf), fix);					// Skip A or I line (1)
	fgets(inbuf, sizeof(inbuf), fix);					// Skip (c) line. (2)
	fixcnt = 0;
	while (fgets(inbuf, sizeof(inbuf), fix) != NULL)
	{
		i = strlen(inbuf);
		if (inbuf[i - 1] == '\n') { inbuf[i - 1] = 0; }	// Zap any newlines.
		if (strlen(inbuf) != 0)					// Ignore blank lines.
		{
			column = 0;
			tokenptr = strtok(inbuf, " ");			// Tokenise the input buffer.
			while (tokenptr != NULL) {			// Get array of column data.
				S[column] = tokenptr;
				tokenptr = strtok(NULL, " ");
				column++;
			}
			Fix[fixcnt].lat = atof(S[0]);
			Fix[fixcnt].lon = atof(S[1]);
			strcpy(Fix[fixcnt].name, S[2]);
			fixcnt++;							// Line Counter.
		}
	}
	fclose(fix);
	fprintf(logfile,"Loaded %d Fixes.\n", fixcnt); fflush(logfile);
}

// Read the earth_nav.dat file.
void LoadNav(void)
{
	FILE *nav;
	char inbuf[1024];
	int count = 0;			// Line counter.
	char *tokenptr;
	int column = 0;
	char *S[128];
	char ch;
	int i;
	int type;
	int LIF = 0;
	//	if (!LOAD_SCENERY) { return; }
	//	nav=fopen("navdata/earth_nav.dat","r");
	//	while (fgets(inbuf,sizeof(inbuf),nav)!=NULL) { LIF++; }		// Sneaky pre-load to count lines.
	//	fclose(nav);
	nav = fopen("navdata/earth_nav.dat", "r");
	fgets(inbuf, sizeof(inbuf), nav);					// Skip A or I line (1)
	fgets(inbuf, sizeof(inbuf), nav);					// Skip (c) line. (2)
	while (fgets(inbuf, sizeof(inbuf), nav) != NULL)
	{
		i = strlen(inbuf);
		if (inbuf[i - 1] == '\n') { inbuf[i - 1] = 0; }	// Zap any newlines.
		if (strlen(inbuf) != 0)					// Ignore blank lines.
		{
			count++;							// Line Counter.
			//if (count % 500 == 0)					// Update the progress bar ?
			//{
			//	Progbar(725,580,250,8,count,LIF);
			//	SDL_GL_SwapBuffers();
			//	//SwapBuffers(hDC);					// Swap Buffers.
			//}
			column = 0;
			tokenptr = strtok(inbuf, " ");			// Tokenise the input buffer.
			while (tokenptr != NULL) {			// Get array of column data.
				S[column] = tokenptr;
				tokenptr = strtok(NULL, " ");
				column++;
			}
			type = atoi(S[0]);	// Get NavAid Type.
			navaid[navcnt].type = type;					// Navaid Type
			navaid[navcnt].lat = atof(S[1]);		// Latitude
			navaid[navcnt].lon = atof(S[2]);		// Longitude
			navaid[navcnt].freq = atoi(S[4]);			// Freq (integer)
			if (type == 2)		// NDB.
			{
				strcpy(navaid[navcnt].ident, S[7]);			// ICAO Identifier.
				for (i = 8; i<column; i++)	{
					strcat(navaid[navcnt].name, S[i]);
					strcat(navaid[navcnt].name, " ");
				}
				navcnt++;
			}
			else if (type == 3)			// VOR
			{
				navaid[navcnt].slave = float(atof(S[6]));	// Slaved variation for VOR
				strcpy(navaid[navcnt].ident, S[7]);			// ICAO Identifier.
				for (i = 8; i<column; i++)	{
					strcat(navaid[navcnt].name, S[i]);
					strcat(navaid[navcnt].name, " ");
				}
				navcnt++;
			}
			else if (type == 4 || type == 5)			// Localiser (inc VOR-DME's & VORTAC's)
			{
				navaid[navcnt].bearing = float(atof(S[6]));	// ILS bearing in TRUE degrees.
				strcat(navaid[navcnt].apticao, S[8]);		// Associated airport ICAO.
				strcat(navaid[navcnt].runway, S[9]);			// Associated Runway (eg: "16L")
				strcpy(navaid[navcnt].ident, S[7]);			// ICAO Identifier.
				for (i = 8; i<column; i++)	{
					strcat(navaid[navcnt].name, S[i]);
					strcat(navaid[navcnt].name, " ");
				}
				navcnt++;
			}
			else if (type == 6)								// Glideslope associated with an ILS.
			{
				navaid[navcnt].bearing = float(atof(S[6]));	// ILS bearing in TRUE degrees.
				strcat(navaid[navcnt].apticao, S[8]);		// Associated airport ICAO.
				strcat(navaid[navcnt].runway, S[9]);			// Associated Runway (eg: "16L")
				strcpy(navaid[navcnt].ident, S[7]);			// ICAO Identifier.
				for (i = 10; i<column; i++)	{
					strcat(navaid[navcnt].name, S[i]);
					strcat(navaid[navcnt].name, " ");
				}
				navcnt++;
			}
			else if (type == 7 || type == 8 || type == 9)			// Marker Beacons.
			{
				navaid[navcnt].bearing = float(atof(S[6]));	// ILS bearing in TRUE degrees.
				strcat(navaid[navcnt].apticao, S[8]);		// Associated airport ICAO.
				strcat(navaid[navcnt].runway, S[9]);			// Associated Runway (eg: "16L")
				for (i = 10; i<column; i++)	{
					strcat(navaid[navcnt].name, S[i]);
					strcat(navaid[navcnt].name, " ");
				}
				navcnt++;
			}
			else if (type == 12 || type == 13)			// DME
			{
				navaid[navcnt].bearing = float(atof(S[6]));	// DME BIAS IN NAUTICAL MILES.
				strcat(navaid[navcnt].ident, S[7]);			// ICAO.
				strcat(navaid[navcnt].apticao, S[8]);		// Associated Airport ICAO if applicable.
				strcat(navaid[navcnt].runway, S[9]);			// Associated Runway (eg: "16L") if applicable.
				for (i = 10; i<column; i++)	{
					strcat(navaid[navcnt].name, S[i]);
					strcat(navaid[navcnt].name, " ");
				}
				navcnt++;
			}
		}
	}
	fclose(nav);
	//sprintf(sbuf, "Processed %d lines.\n", count);		log(sbuf);
	sprintf(sbuf, "Loaded %d Navaids.\n", navcnt);		log(sbuf);
	//ch=getchar();
	//return;
}


// NOTE! This isn't a real font, it's the 9x15 drawn smaller.
void Print8x12(GLfloat X, GLfloat Y, char *s)
{
	GLint i;
	GLint c;
	GLfloat xo, yo;
	GLfloat px = 1.0 / 512.0;
	glBindTexture(GL_TEXTURE_2D, tex[SUPPORT1].texID);			// Select Our Font Texture
	glEnable(GL_TEXTURE_2D);
	for (i = 0; i<strlen(s); i++)
	{
		c = s[i];			// Character ASCII value.
		if (c != 32)
		{
			if (c>88)	{ yo = 185; c -= 89; }
			else		{ yo = 200; c -= 33; }
			xo = (c * 9) * px;		// Horizontal Offset.
			DrawTex(X, Y, 8, 12, xo, yo*px, xo + (9 * px), (yo + 15)*px);
		}
		X += 8;
	}
}



void LoadAirports(void)
{
	FILE *apt;
	char inbuf[1024];
	int column = 0;
	char *S[128];
	int i;
	char *tokenptr;
	int	rectype;
	int count = 0;
	int LIF = 0;
	//if (!LOAD_SCENERY) { return; }
	//	apt=fopen("navdata/apt.dat","r");
	//	while (fgets(inbuf,sizeof(inbuf),apt)!=NULL) { LIF++; }
	//	fclose(apt);
	apt = fopen("navdata/apt.dat", "r");
	fgets(inbuf, 1024, apt);		// A or I line.
	fgets(inbuf, 1024, apt);		// Copyright Info.
	while (fgets(inbuf, sizeof(inbuf), apt) != NULL)
	{
		i = strlen(inbuf);
		if (inbuf[i - 1] == '\n') { inbuf[i - 1] = 0; }
		if (strlen(inbuf) != 0)
		{
			count++;
			column = 0;
			tokenptr = strtok(inbuf, " ");
			while (tokenptr != NULL) {
				S[column] = tokenptr;
				tokenptr = strtok(NULL, " ");
				column++;
			}
			rectype = atoi(S[0]);							// What type of record do we have?
			if (rectype == 1)								// LAND AIRPORT.
			{
				//if (count % 100 == 0)					// Update the progress bar ?
				//{
				//	Progbar(725,560,250,8,count,LIF);
				//	SDL_GL_SwapBuffers();
				//	//SwapBuffers(hDC);					// Swap Buffers.
				//}
				Airport[aptcnt].elev = atoi(S[1]);		// Elevation AMSL in feet.
				strcpy(Airport[aptcnt].ICAO, S[4]);		// ICAO
				for (i = 5; i<column; i++) {
					strcat(Airport[aptcnt].name, S[i]);
					strcat(Airport[aptcnt].name, " ");
				}
				Airport[aptcnt].name[strlen(Airport[aptcnt].name) - 1] = 0;		// Zap the trailing space.
				Airport[aptcnt].runways = 0;
				Airport[aptcnt].category = 2;			// Set runway type default to grass or gravel strip.
				Airport[aptcnt].tag = aptcnt;				// Need this for cached list.
				//printf("%s (%s) Elev=%d(ft)\n",Airport[aptcnt].ICAO,Airport[aptcnt].name,Airport[aptcnt].elev);
				aptcnt++;
			}
			else if (rectype == 16)						// SEAPLANE BASE HEADER
			{
			}
			else if (rectype == 17)						// HELIPORT
			{
			}
			else if (rectype == 100)						// RUNWAY
			{
				Runway[rwycnt].airport = aptcnt - 1;		// Array index of host airport.
				Airport[aptcnt - 1].runways++;			// Add a runway to the Airport counter.
				Runway[rwycnt].width = float(atof(S[1]));	// Width of runway in metres.
				Runway[rwycnt].surface = atoi(S[2]);		// Surface type.

				if (Runway[rwycnt].surface < 3) { Airport[aptcnt - 1].category = 1; }		//  Upgrade to Concrete or Asphalt Runway.
				if (Runway[rwycnt].surface > 5) { Airport[aptcnt - 1].category = 3; }		//  Downgrade to lakebed water or ice.

				strcpy(Runway[rwycnt].name1, S[8]);		// 1st Name of runway. eg: 16R
				Runway[rwycnt].lat1 = atof(S[9]);			// Lat of 1st threshold.
				Runway[rwycnt].lon1 = atof(S[10]);		// Lat of 1st threshold.
				Airport[aptcnt - 1].lat = Runway[rwycnt].lat1;	// Use these values for general airport position (mapping)
				Airport[aptcnt - 1].lon = Runway[rwycnt].lon1;
				//			sprintf(sbuf,"%f , %f",Airport[aptcnt-1].lat,Airport[aptcnt-1].lon);  Print16x16(800,200,sbuf); 	SwapBuffers(hDC);
				strcpy(Runway[rwycnt].name2, S[17]);		// 2nd Name of runway. eg: 16R
				Runway[rwycnt].lat2 = atof(S[18]);		// Lat of 2nd threshold.
				Runway[rwycnt].lon2 = atof(S[19]);		// Lat of 2nd threshold.
				// Calc runway length.
				double dLat = Runway[rwycnt].lat1 - Runway[rwycnt].lat2;
				double dLon = Runway[rwycnt].lon1 - Runway[rwycnt].lon2;
				dLat = DegToRad(dLat);
				dLon = DegToRad(dLon);
				float a = float(losin(dLat / 2.0f) * losin(dLat / 2.0f) + locos(DegToRad(Runway[rwycnt].lat1)) *
					locos(DegToRad(Runway[rwycnt].lat2)) * losin(dLon / 2.0f) * losin(dLon / 2.0f));
				float c = 2 * fastatan2(sqrt(a), sqrt(1.0f - a));
				float d = 6371.0f * c;
				Runway[rwycnt].length = d;
				// Calc runway bearing.
				float y = float(losin(dLon) * locos(Runway[rwycnt].lat2));
				float x = float(locos(DegToRad(Runway[rwycnt].lat1))*losin(DegToRad(Runway[rwycnt].lat2)) -
					losin(DegToRad(Runway[rwycnt].lat1))*locos(DegToRad(Runway[rwycnt].lat2))*locos(dLon));
				float brng = fastatan2(y, x);
				brng = float(brng*180.0f / pi);		// Convert back to Degrees from Radians.
				brng = float(int(brng + 360) % 360);		// Normalise the bearing to fit 0'->360' range.
				//if (strstr(Airport[aptcnt-1].ICAO,"EGGP")!=NULL)		// Test Case.
				//{
				//	printf("%s - %f Degrees.\n",Airport[aptcnt-1].ICAO,brng);
				//	char ch=getchar();
				//}
				Runway[rwycnt].bearing = brng;
				rwycnt++;
			}
		}
	}
	fclose(apt);
	printf("Found %d Records.\n", count);
	printf("Found %d Airports.\n", aptcnt);
	printf("Found %d Runways.\n", rwycnt);
}



// Draw a block from the currently bound texture.
void DrawChunk(float x, float y, int w, int h, float x1, float y1, float x2, float y2)
{
	glBegin(GL_QUADS);
	glTexCoord2f(x2, y1);	glVertex2f(x + w, y);		// Bottom Right
	glTexCoord2f(x1, y1);	glVertex2f(x, y);		// Bottom Left
	glTexCoord2f(x1, y2);	glVertex2f(x, y + h);		// Top Left
	glTexCoord2f(x2, y2);	glVertex2f(x + w, y + h);	// Top Right
	glEnd();
}
// Draw a block from the currently bound texture.
void DrawChunki(int x, int y, int w, int h, float x1, float y1, float x2, float y2)
{
	glBegin(GL_QUADS);
	glTexCoord2f(x2, y1);	glVertex2d(x + w, y);		// Bottom Right
	glTexCoord2f(x1, y1);	glVertex2d(x, y);		// Bottom Left
	glTexCoord2f(x1, y2);	glVertex2d(x, y + h);		// Top Left
	glTexCoord2f(x2, y2);	glVertex2d(x + w, y + h);	// Top Right
	glEnd();
}



static void gfont8Char(char Ch, int x, int y)
{
	unsigned int offset;
	unsigned int size;
	unsigned int i;

	offset = (unsigned int)gfont8Offsets[Ch];
	size = (unsigned int)gfont8Size[Ch];
	glBegin(GL_LINE_STRIP);
	{
		for (i = 1; i <= size; i += 1) {
			glVertex2i(x + (int)gfont8Tab[offset], y + (int)gfont8Tab[offset + 1]);
			offset = offset + 2;
		}
	}
	glEnd();
}


static void gfont12Char(char Ch, int x, int y)
{
	unsigned int offset;
	unsigned int size;
	unsigned int i;

	offset = gfont12Offsets[Ch];
	size = gfont12Size[Ch];
	glBegin(GL_LINE_STRIP);
	{
		for (i = 1; i <= size; i += 1) {
			glVertex2i(x + (int)gfont12Tab[offset], y + (int)gfont12Tab[offset + 1]);
			offset = offset + 2;
		}
	}
	glEnd();
}




int LoadPNGTexture(GLchar *filename, GLint tid)
{
	unsigned char* image;
	unsigned w, h;
	unsigned error;
	GLuint texName;
	error = lodepng_decode32_file(&image, &w, &h, filename);
	if (error)
	{
		fprintf(logfile, "Error %d reading texture %s.\n", error, filename); fflush(logfile);
		return(0);
	}
	else
	{
		// Flip the Image.
		unsigned char *imagePtr = &image[0];
		int halfTheHeightInPixels = h / 2;
		int heightInPixels = h;
		// Assuming RGBA for 4 components per pixel.
		int numColorComponents = 4;
		// Assuming each color component is an unsigned char.
		int widthInChars = w * numColorComponents;
		unsigned char *top = NULL;
		unsigned char *bottom = NULL;
		unsigned char temp = 0;
		for (int i = 0; i < halfTheHeightInPixels; ++i)
		{
			top = imagePtr + i * widthInChars;
			bottom = imagePtr + (heightInPixels - i - 1) * widthInChars;
			for (int w = 0; w < widthInChars; ++w)
			{
				// Swap the chars around.
				temp = *top;
				*top = *bottom;
				*bottom = temp;
				++top;
				++bottom;
			}
		}
		// Flip Complete - now store it.
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &tex[tid].texID);
		tex[tid].width = w;
		tex[tid].height = h;
		glBindTexture(GL_TEXTURE_2D, tex[tid].texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, tex[tid].width, tex[tid].height, GL_RGBA, GL_UNSIGNED_BYTE, &tex[tid].imageData);		// THIS ADDED IN NPOT QUEST!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}
	free(image);
	return 1;
}



// **************************************************************************************
// Create the list for our general purpose 16x16 font used by glPrint() Font is 16x16.
// **************************************************************************************
GLvoid BuildFont(GLvoid)									// Build Our Font Display List
{
	GLfloat	cx;												// Holds Our X Character Coord
	GLfloat	cy;												// Holds Our Y Character Coord
	GLuint	loop;				// Generic Loop Variable
	base = glGenLists(256);									// Creating 256 Display Lists
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex[txFONT].texID);		// Select Our Font Texture
	for (loop = 0; loop<256; loop++)						// Loop Through All 256 Lists
	{
		cx = float(loop % 16) / 16.0f;						// X Position Of Current Character
		cy = float(loop / 16) / 16.0f;						// Y Position Of Current Character
		glNewList(base + loop, GL_COMPILE);					// Start Building A List
		glBegin(GL_QUADS);									// Use A Quad For Each Character
		glTexCoord2f(cx, 1 - cy - 0.0625f);					// Texture Coord (Bottom Left)
		glVertex2i(0, 0);									// Vertex Coord (Bottom Left)
		glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);		// Texture Coord (Bottom Right)
		glVertex2i(16, 0);									// Vertex Coord (Bottom Right)
		glTexCoord2f(cx + 0.0625f, 1 - cy);					// Texture Coord (Top Right)
		glVertex2i(16, 16);									// Vertex Coord (Top Right)
		glTexCoord2f(cx, 1 - cy);							// Texture Coord (Top Left)
		glVertex2i(0, 16);									// Vertex Coord (Top Left)
		glEnd();											// Done Building Our Quad (Character)
		glTranslated(10, 0, 0);								// Move To The Right Of The Character
		glEndList();										// Done Building The Display List
	}														// Loop Until All 256 Are Built
}




// Draw a 7-Segment style string.
void SevenString(GLint x, GLint y, char *string, GLint spacing)
{
	// Coords for 1st char, a period, are:  0.0f , 0.8369140625f, 0.01171875f , 0.8583984375f
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);		// Select Our Utility Texture
	double px = 1.0f / 1024.0f;						// How big are our pixels?
	double w = 12.0f * px;							// How wide is one character?

	for (int i = 0; i < strlen(string); i++)
	{
		int asc = (netbuf.switches.battery[0] == 0) ? 2 : string[i] - 45;		// If battery is off, draw the darkgreen outline, otherwise our first char is a dash '-'
		double xo = asc * w;								// This gets us to our character.
		DrawChunki(x, y, 12, 22, xo, 0.8369140625f, xo + w, 0.8583984375f);		// Draw the char.
		x += 12 + spacing;
	}
}


// Draw 6x9 px text. FULLER fONT. Caps Only. String should be in sbuf.
void pString96(int x, int y, int spc)
{
	unsigned int i, b;
	float px = 1.0f / 1024.0f;
	float xo;
	float yo = 827.0f * px;
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
	glEnable(GL_TEXTURE_2D);
	for (i = 0; i<strlen(sbuf); i++)
	{
		sbuf[i] = toupper(sbuf[i]);
		b = sbuf[i] - 37;						// '%' = ASCII 37.
		if (b != -11)							// This would be a space.
		{
			xo = 154.0f *px;					// First char (period).
			xo += float((b*6.0f)*px);			// Calc X offset to char.
			DrawChunki(x, y, 6, 9, xo, yo, xo + (6.0f*px), yo + (9.0f*px));
		}
		x += (6 + spc);
	}
}


// Draw 6x9 px text but LARGER (7x10). FULLER fONT. Caps Only. String should be in sbuf.
void PrintMed(int x, int y, int spc)
{
	unsigned int i, b;
	float px = 1.0f / 1024.0f;
	float xo;
	float yo = 827.0f * px;
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
	glEnable(GL_TEXTURE_2D);
	for (i = 0; i<strlen(sbuf); i++)
	{
		sbuf[i] = toupper(sbuf[i]);
		b = sbuf[i] - 37;						// '%' = ASCII 37.
		if (b != -11)							// This would be a space.
		{
			xo = 154.0f *px;					// First char (period).
			xo += float((b*6.0f)*px);			// Calc X offset to char.
			DrawChunki(x, y, 7, 10, xo, yo, xo + (6.0f*px), yo + (9.0f*px));
		}
		x += (7 + spc);
	}
}


// Draw 6x9 px text but SMALLER (6x8). FULLER fONT. Caps Only. String should be in sbuf.
void PrintSmall(int x, int y, int spc)
{
	unsigned int i, b;
	float px = 1.0f / 1024.0f;
	float xo;
	float yo = 827.0f * px;
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
	glEnable(GL_TEXTURE_2D);
	for (i = 0; i<strlen(sbuf); i++)
	{
		sbuf[i] = toupper(sbuf[i]);
		b = sbuf[i] - 37;						// '%' = ASCII 37.
		if (b != -11)							// This would be a space.
		{
			xo = 154.0f *px;					// First char (period).
			xo += float((b*6.0f)*px);			// Calc X offset to char.
			DrawChunki(x, y, 6, 8, xo, yo, xo + (6.0f*px), yo + (9.0f*px));
		}
		x += (6 + spc);
	}
}


void Heading_Init()
{
	LoadPNGTexture("bitmaps/heading.png", txHEADING);
}
void Altimeter_Init()
{
	LoadPNGTexture("bitmaps/altimeter.png", txALTIMETER);
}
void Airspeed_Init()
{
	LoadPNGTexture("bitmaps/airspeed.png", txAIRSPEED);
}



void Compass_Draw()
{
	int HEADINGx = 491;		// Position of VSI, lower-left corner.
	int HEADINGy = 824;
	float HEADINGscale = 0.76f;		// Scale of drawing.
	glColor4fv(gWHITE);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslated(HEADINGx, HEADINGy, 0);
		glScalef(HEADINGscale, HEADINGscale, 1.0);								// Set the scaling factor.
		glBindTexture(GL_TEXTURE_2D, tex[txHEADING].texID);					// Instrument Face
		glPushMatrix();
			glTranslated(127, 127, 0);
			glRotatef(netbuf.flight.magheading, 0.0, 0.0, 1.0);
			glTranslated(-127, -127, 0);
			DrawTexi(0, 0, 255, 255, 0.0f, 0.0f, 1.0f, 1.0f);
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
		DrawTex(56.5f, 58.0f, 141, 141, 0.251953125f, 0.8623046875f, 0.3896484375f, 1.0f);					// Centre disc with Lubber Line.
		DrawTex(127.0-6.5, 240.0, 13, 13, 0.0f, 0.865234375f, 0.0126953125f, 0.8779296875f);			// Top static pointer.
		DrawTex(127.0-6.5, 0.0, 13, 13, 0.013671875f, 0.865234375f, 0.0263671875f, 0.8779296875f);		// Bottom static pointer
		glTranslated(-HEADINGx, -HEADINGy, 0);
	glPopMatrix();
}








/*
	Altimeter scale is 36 degrees between 100 foot ticks.
	Coords for needle:   0.0f , 0.798828125f, 0.1416015625f , 0.8193359375f
*/
void Altimeter_Draw()
{
	// Draw the Altimeter 'ODO' Digits first.
	double yo = 0.0;
	float px = 1.0f / 1024.0f;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);			// Select Our Utility Texture
	glColor4fv(gWHITE);
	alt = netbuf.flight.altitude;
	int tenthou = int(alt / 10000.0);
	int thou = int((alt - (tenthou * 10000)) / 1000.0);
	int hun = int(alt - (tenthou * 10000) - (thou * 1000));
	// Draw the ten-thou box first. Use the checker box if it's zero.
	yo = tenthou * (34.0 * px);
	// if thousands are >= 9 then need a partial shift.
	int rem = 0;
	if (thou >= 9)
	{
		rem = int(alt - (tenthou * 10000) - (thou * 1000));
		yo += rem * 0.034 * px;
	}
	DrawTexi(1201, 684, 16, 24, 0.0f, 0.009765625f + yo, 0.015625f, 0.033203125f + yo);

	// Draw the thousands value.
	yo = thou * 34.0 * px;
	// Now take into account the value of the hundreds, to give us that "mid shift" look.
	// Only shift IF we're at or above 900 and use value of tens to give the shift.
	float frem = 0.0;
	frem = alt - (tenthou * 10000) - (thou * 1000) - (hun * 100);
	if (hun >= 900)
	{
		yo += frem * 0.034 * px;
	}
	DrawTexi(1220, 679, 16, 34, 0.021484375f, 0.0048828125f + yo, 0.037109375f, 0.0380859375f + yo);

	
//	sprintf(sbuf, "Alt: %f tenthou: %d  thou: %d  hun: %d  frem: %f", alt, tenthou, thou, hun, frem);
//	glPrint(100, 40, sbuf, 0);


	// *************************************************************************
	glColor4fv(gGREY);					// Draw baro setting
	Block(1218, 614, 43, 16);
	glColor4fv(gDARKGREY);
	Block(1218, 615, 42, 16);
	glColor4fv(gWHITE);
	sprintf(sbuf, "1013");
	glPrint(1215, 615, sbuf, 0);
	// Now draw the instrument itself.
	int Altx = 1143;
	int Alty = 575;
	float ALTIMETERscale = 0.76f;		// Scale of drawing.
	glColor4fv(gWHITE);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslated(Altx, Alty, 0);
		glScalef(ALTIMETERscale, ALTIMETERscale, 1.0);
		glBindTexture(GL_TEXTURE_2D, tex[txALTIMETER].texID);					// Instrument Face
		DrawTexi(0, 0, 256, 256, 0.0f, 0.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
		glPushMatrix();
			glTranslated(127, 127, 0);
			float pangle = -90.0 - (hun * 0.36f);
			glRotatef(pangle, 0.0, 0.0, 1.0);
			glTranslated(-127, -127, 0);
			DrawTex(127.0f-105.0f, 127.0f-9.5f, 145, 21, 0.0f, 0.798828125f, 0.1416015625f, 0.8193359375f);	// Pointer
		glPopMatrix();
		glTranslated(-Altx, -Alty, 0);
	glPopMatrix();
}

void VSI_Draw()
{
	float VSIscale = 0.76f;		// Scale of drawing.
	int VSIx = 1263;		    // Position of VSI, lower-left corner.
	int VSIy = 824;
	float v = netbuf.flight.vs;

	if (abs(netbuf.flight.vs) <= 1000.0f) { v = abs(netbuf.flight.vs) * 0.06f; }
	else if (abs(netbuf.flight.vs) <= 2000.0f) { v = (abs(netbuf.flight.vs) - 1000.0) * 0.03f + 60.0f; }
	else { v = (abs(netbuf.flight.vs) - 2000) * 0.02f + 90.0f; }
	if (v > 170.0f) { v = 170.0f; }
	if (netbuf.flight.vs > 0.0f) { v = -v; }

	VSIangle = netbuf.flight.vs;
	VSIangle = v;

	glColor4fv(gWHITE);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslated(VSIx, VSIy, 0);
		glScalef(VSIscale, VSIscale, 1.0);								// Set the scaling factor.
		glBindTexture(GL_TEXTURE_2D, tex[txVSI].texID);					// Instrument Face
		DrawTexi(0, 0, 256, 256, 0.0f, 0.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
		glPushMatrix();
			glTranslated(127, 127, 0);
			glRotatef(VSIangle, 0.0, 0.0, 1.0);
			glTranslated(-127, -127, 0);
			DrawTex(8.0f, 110.5f, 137, 35, 0.0f, 0.9658203125f, 0.1337890625f, 1.0f);	// Pointer
		glPopMatrix();
		glTranslated(-VSIx, -VSIy, 0);
	glPopMatrix();
}


void Airspeed_Draw()
{
	int Airx = 587;
	int Airy = 575;
	float AIRSPEEDscale = 0.76f;		// Scale of drawing.

	float v = netbuf.flight.Vind;

	if (v <= 50.0f) { v = -90-v * 0.16; }	// 0 - 50 kts
	else
	if (v <= 100.0f) { v = -98-(v - 50.0f)*0.64f; }		// 50 - 100 kts
	else
	if (v <= 150.0f) { v = -130-(v - 100.0f)*0.9f; }	// 100 - 150 kts
	else
	if (v <= 200.0f) { v = -175-(v - 150.0f)*0.9f; }		// 150 - 200 kts
	else
		if (v <= 250.0f) { v = -220-(v - 200.0f)*0.84f; }	// 200-250kts
	else
		if (v <= 300.0f) { v = -262-(v - 250.0f)*0.62f; }	// 250 - 300 kts
	else
		if (v <= 350.0f) { v = -293-(v - 300.0f)*0.6f; }		// 300-350 kts
	else
		if (v <= 400.0f)	{ v = -323-(v - 350.0f)*0.5f; }	// 350-400 kts
	else
		if (v <= 450.0f) { v = -348-(v - 400.0f)*0.38f; }	// 400 - 450 kts
	else
		if (v <= 500.0f) { v = -367-(v - 450.0f)*0.4f; }		// 450 - 500 kts
	else
		if (v <= 550.0f) { v = -387-(v - 500.0f)*0.4f; }		// 500 - 550 kts
	else
		if (v <= 600.0f) { v = -407-(v - 550.0f)*0.36f; }	// 550 - 600 kts
	else
		if (v > 600.0f) { v = -425-(v - 600.0f) * 0.36f; }		// Greater than 600.
	if (v < -440) { v = -440.0f; }							// Needle stops here!
	// greater than 600 here !

	//v = 0.0f;
	sprintf(sbuf, "KTS X 100");

	glColor4fv(gWHITE);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslated(Airx, Airy, 0);
		glScalef(AIRSPEEDscale, AIRSPEEDscale, 1.0);
		glBindTexture(GL_TEXTURE_2D, tex[txAIRSPEED].texID);					// Instrument Face
		DrawTexi(0, 0, 256, 256, 0.0f, 0.0f, 1.0f, 1.0f);
		PrintSmall(100, 70, 0);
		glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
		glPushMatrix();
			glTranslated(127, 127, 0);
			glRotatef(v, 0.0, 0.0, 1.0);
			glTranslated(-127, -127, 0);
			DrawTex(8.0f, 110.5f, 137, 35, 0.0f, 0.9658203125f, 0.1337890625f, 1.0f);	// Pointer
		glPopMatrix();
		glTranslated(-Airx, -Airy, 0);
	glPopMatrix();
}



// Prepare the VSI.
void VSI_Init()
{
	LoadPNGTexture("bitmaps/parts.png", txPARTS);
	LoadPNGTexture("bitmaps/vsi.png", txVSI);
	LoadPNGTexture("bitmaps/tape.png", txATTITUDE);
	LoadPNGTexture("bitmaps/adimask.png", txADIMASK);
	LoadPNGTexture("bitmaps/linear.png", LINEAR);
	LoadPNGTexture("bitmaps/support1.png", SUPPORT1);
	LoadPNGTexture("bitmaps/dummy.png", DUMMY);
	LoadPNGTexture("bitmaps/adicompass.png", txADICOMPASS);
	LoadPNGTexture("bitmaps/hsicompass.png", txHSICOMPASS);
	LoadPNGTexture("bitmaps/battery_fail.png", txBATTFAIL);
}




void ADI_Init()
{
	LoadPNGTexture("bitmaps/adi.png", txADI);
}


void OdoNumber(GLint x, GLint y, char *string)
{
	float px = 1.0 / 1024.0;
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
	glEnable(GL_TEXTURE_2D);
	glColor4fv(gWHITE);
	for (int i = 0; i < strlen(string); i++)
	{
		int c = string[i] - 48;
		float yo = c * 34.0 * px;
		DrawTexi(x, y, 11, 14, 0.021484375f, 0.009765625f + yo, 0.037109375f, 0.033203125f + yo);
		x += 11;
	}
}

// Draw the standby ADI.
void ADI_Draw()
{

	// ADI
	int ADIx = 853;
	int ADIy = 592;
	float ADIscale = 0.82f;
	if (netbuf.switches.battery[0] == 0) return;
	glBindTexture(GL_TEXTURE_2D, tex[txADI].texID);
	glEnable(GL_TEXTURE_2D);
	glColor4fv(gWHITE);
//	netbuf.flight.pitch = pitch;
	GLfloat offset = netbuf.flight.pitch;															// Offset for pitch.
	if (offset > +80.0f) { offset = +80.0f; }														// CHECK FOR PITCH LIMITS.
	if (offset < -80.0f) { offset = -80.0f; }
	offset = offset * 0.005859375f;																	// Allow for 3px per degree of pitch.
	GLfloat a = 0.25f + offset;
	GLfloat b = a + 0.5f;
	glPushMatrix();
		glTranslated(ADIx, ADIy, 0);
		glScalef(ADIscale, ADIscale, 1.0);
		glScissor(ADIx,ADIy,int(256*ADIscale),int(256*ADIscale));
		glEnable(GL_SCISSOR_TEST);

		// Rotate here to show Roll.
		glPushMatrix();
			glTranslated(128, 128, 0);
			glRotatef(netbuf.flight.roll, 0.0, 0.0, 1.0);
			glTranslated(-128, -128.0, 0);
			DrawTexi(-128, -128, 512, 512, 0.0f, a-0.25, 1.0f, b+0.25);		// Horizon Tape.
//			DrawTexi(0, 0, 256, 256, 0.25f, a, 0.75f, b);		// Horizon Tape.
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);
		DrawTex(0 + 76.0f, 0 + 121.0f, 103, 10, 0.13671875f, 0.990234375f, 0.2373046875f, 1.0f);		// Aircraft Stable Centre.
		DrawTex(0 + 35.0f, 0 + 193.0f, 186, 59, 0.0f, 0.9033203125f, 0.181640625f, 0.9609375f);						// Roll Ring at Top.
		DrawTex(0 + 128.0f - 56.0f, 0 + 5.0f, 113, 17, 0.13671875f, 0.97265625f, 0.2470703125f, 0.9892578125f);		// Sideslip box at bottom.
		DrawTex(122.5, 8.0, 11, 11, 0.02734375f, 0.865234375f, 0.0380859375f, 0.8759765625f);			// Slip Ball.
		glDisable(GL_SCISSOR_TEST);
		glColor4fv(gBLACK);
		Block(0, 232, 40, 18);				// Block at top left for IAS
		Block(194, 5, 51, 17);				// Block bottom right for baro setting (mbar)
//		glColor4fv(gWHITE);
//		sprintf(sbuf, "520");
//		glPrint(3, 42, sbuf, 0);
		sprintf(sbuf, "%3.0f", netbuf.flight.Vind);
		OdoNumber(3, 234, sbuf);
		sprintf(sbuf, "%4d", int(netbuf.flight.baro / 0.0295301f));
		OdoNumber(196, 7, sbuf); 
		glTranslated(-ADIx, -ADIy, 0);
	glPopMatrix();
}




// Draw Navaids. W = Half the width of the map.
void Draw_Navaids(int X, int Y, int W)
{
	int i, k;			// General Counters.
	float	maxdiff;	// These are used to determine items in range.
	int		range;
	float	scale;
	float	dx, dy, l, a, b, x, y;
	float	inter;

	switch (netbuf.switches.maprange) {
	case 0: range = 10; break;
	case 1: range = 20; break;
	case 2: range = 40; break;
	case 3: range = 80; break;
	case 4: range = 160; break;
	case 5: range = 320; break;
	case 6: range = 640; break;
	}

	//range = 20;

	scale = float(W / range);				// 438 is half the width of the map.
	inter = sqrt(float(range*range) + float(range*range))*1.25f;
	maxdiff = abs(inter / 60.0f);

	NAVTEST = 0;
	glColor4fv(gWHITE);

	for (i = 1; i<navcnt; i++)
	{
		if (abs(netbuf.nav.latitude - navaid[i].lat)<maxdiff && abs(netbuf.nav.longitude - navaid[i].lon)<maxdiff)
		{
			dx = float(abs(netbuf.nav.longitude - navaid[i].lon));
			dy = float(abs(netbuf.nav.latitude - navaid[i].lat));
			dx = float(abs(dx*(60.0f*locos(DegToRad(netbuf.nav.latitude)))));
			dy = abs(dy*60.0f);
			dx = dx*scale;
			dy = dy*scale;
			if (navaid[i].lon < netbuf.nav.longitude) { dx = -dx; }
			if (navaid[i].lat < netbuf.nav.latitude) { dy = -dy; }
			l = sqrt((dx*dx) + (dy*dy));
			if (!MAP_NORTH) { a = 360.0f - netbuf.flight.magheading; }
			b = fastatan2(dy, dx);					// Angle to Navaid in Radians.
			b = float((b*180.0f) / pi);				// Angle to Navaid in Degrees.
			b = b - 90.0f;
			x = float(l * losin(DegToRad(a - b)));
			y = float(l * locos(DegToRad(a - b)));
			x += X;
			y += Y;
			if (navaid[i].type == 2 && netbuf.switches.EFISndb)				 // NDB
			{
				glBindTexture(GL_TEXTURE_2D, tex[LINEAR].texID);
				DrawTex(x - 10.0, y - 10.0, 20, 20, 0.017578125f, 0.21875f, 0.037109375f, 0.23828125f);
				//Print10x14(Round(x+14),Round(y-7),CACHE_navaid[i].ident);
				//glPrintf(x+14.0, y-7.0, CACHE_navaid[i].ident,0);
				Print8x12(x + 14.0, y - 7.0, navaid[i].ident);
				NAVTEST++;
			}
			else if (navaid[i].type == 3 && netbuf.switches.EFISvor)		 // VOR
			{
				glBindTexture(GL_TEXTURE_2D, tex[SUPPORT1].texID);
				//DrawTex(x-10.0,y-10.0,20, 20, 0.017578125f , 0.251953125f , 0.037109375f , 0.271484375f);
				DrawTex(x - 10.0, y - 10.0, 20, 20, 0.306640625f, 0.75390625f, 0.35546875f, 0.802734375f); // VOR
				//Print10x14(Round(x+14),Round(y-7),CACHE_navaid[i].ident);
				//glPrintf(x+14.0, y-7.0, CACHE_navaid[i].ident,0);
				Print8x12(x + 14.0, y - 7.0, navaid[i].ident);
				NAVTEST++;
			}
		}
	}
}



void Draw_Airports(int X, int Y, int W)
{
	GLint i, k;
	float	maxdiff;	// These are used to determine items in range.
	int		range;
	float	scale;
	float	dx, dy, l, a, b, x, y;
	float	inter;

	if (!netbuf.switches.EFISairport) { return; }

	switch (netbuf.switches.maprange) {
	case 0: range = 10; break;
	case 1: range = 20; break;
	case 2: range = 40; break;
	case 3: range = 80; break;
	case 4: range = 160; break;
	case 5: range = 320; break;
	case 6: range = 640; break;
	}

	//range = 20;

	scale = float(W / range);										// W is half the width of the map.
	inter = sqrt(float(range*range) + float(range*range))*1.25f;	// Added 25% to cover all.
	maxdiff = abs(inter / 60.0f);									// Max difference in lat or lon to include/exclude on map.

	for (i = 0; i<aptcnt; i++)
	{
		if (abs(netbuf.nav.latitude - Airport[i].lat)<maxdiff && abs(netbuf.nav.longitude - Airport[i].lon)<maxdiff)
		{
			dx = float(abs(netbuf.nav.longitude - Airport[i].lon));
			dy = float(abs(netbuf.nav.latitude - Airport[i].lat));
			dx = float(abs(dx*(60.0f*locos(DegToRad(netbuf.nav.latitude)))));
			dy = abs(dy*60.0f);
			dx = dx*scale;
			dy = dy*scale;
			if (Airport[i].lon < netbuf.nav.longitude) { dx = -dx; }
			if (Airport[i].lat < netbuf.nav.latitude) { dy = -dy; }
			l = sqrt((dx*dx) + (dy*dy));
			if (!MAP_NORTH) { a = 360.0f - netbuf.flight.magheading; }
			//				a=360.0f - netbuf.nav.Magheading;
			b = fastatan2(dy, dx);						// Angle to Navaid in Radians.
			b = float((b*180.0f) / pi);				// Angle to Navaid in Degrees.
			b = b - 90.0f;
			x = float(l * losin(DegToRad(a - b)));
			y = float(l * locos(DegToRad(a - b)));
			y = Y + y;		// Add offset to map display centre-point.
			x = X + x;
			glBindTexture(GL_TEXTURE_2D, tex[SUPPORT1].texID);
			//DrawTex(x-10, y-10, 20, 20, 0.111328125f , 0.421875f , 0.130859375f , 0.44140625f);		// ROUND AIRFIELD ICON
			DrawTex(x - 10, y - 10, 20, 20, 0.306640625f, 0.8046875f, 0.35546875f, 0.853515625f);	// Airfield Icon
			//glPrintf(x+13.0, y-8.0, CACHE_Airport[i].ICAO, 0);
			Print8x12(x + 13.0, y - 8.0, Airport[i].ICAO);
			//switch (CACHE_Airport[i].category)
			//{
			//	case 1 : DrawTex(x-10,y-10,20,20,0.111328125f , 0.421875f , 0.130859375f , 0.44140625f);		// ROUND AIRFIELD ICON
			//		break;
			//	case 2 : DrawTex(Round(x-12),Round(y-12),25,25,0.8271484375f , 0.5849609375f, 0.8515625f , 0.609375f);		// MAGENTA AIRFIELD ICON
			//		break;
			//	case 3 : DrawTex(Round(x-12),Round(y-12),25,25,0.8271484375f , 0.560546875f, 0.8515625f , 0.5849609375f);		// MAGENTA CIRCLE AIRFIELD ICON
			//		break;
			//	default : break ;
			//}
			// DRAW RUNWAY LINE ON AIRPORT ICON
			glEnable(GL_LINE_SMOOTH);
			int r = 0;
			for (k = 0; k<rwycnt; k++)
			{
				if (Runway[k].airport == Airport[i].tag)				// Is this runway at our airport?
				{
					double rb;			// Runway bearing.
					if (MAP_NORTH) { rb = Runway[k].bearing; }
					else { rb = netbuf.flight.magheading - Runway[k].bearing; }
					rb = int(rb + 360) % 360;
					glPushMatrix();
					glTranslatef(x, y, 1.0f);
					glRotatef(float(rb), 0.0f, 0.0f, 1.0f);
					glTranslatef(-x, -y, -1.0f);
					glDisable(GL_TEXTURE_2D);
					glColor3f(1.0f, 1.0f, 1.0f);
					glBegin(GL_LINES);
					glVertex2f(x, y - 6.5f);
					glVertex2f(x, y + 6.5f);
					glEnd();
					glEnable(GL_TEXTURE_2D);
					glPopMatrix();
					r++;
					if (r == Airport[i].runways) { k = rwycnt; }	// No need to look any further.
				}
			}
		}
	}
}



void Draw_TCAS(int X, int Y, int W)
{
	double x2, y2;			// Pixel Coords for Heading Line.
	GLint i, j, k;
	float	maxdiff;	// These are used to determine items in range.
	int		range;
	float	scale;
	float	dx, dy, l, a, b, x, y;
	float	inter;
	char	sbuf[128];

	if (!netbuf.switches.EFIStcas) { return; }

	switch (netbuf.switches.maprange) {
	case 0: range = 10; break;
	case 1: range = 20; break;
	case 2: range = 40; break;
	case 3: range = 80; break;
	case 4: range = 160; break;
	case 5: range = 320; break;
	case 6: range = 640; break;
	}

	//range = 20;

	scale = float(W / range);										// W is half the width of the map.
	inter = sqrt(float(range*range) + float(range*range))*1.25f;	// Added 25% to cover all.
	maxdiff = abs(inter / 60.0f);									// Max difference in lat or lon to include/exclude on map.
	for (i = 1; i<20; i++)		// Loop through all other aircraft.
	{
		if (abs(netbuf.nav.Xlat[i] - netbuf.nav.latitude) < maxdiff && abs(netbuf.nav.Xlon[i] - netbuf.nav.longitude) < maxdiff
			&& netbuf.nav.Xalt[i] > 0.0f)
		{
			dx = float(abs(netbuf.nav.longitude - netbuf.nav.Xlon[i]));
			dy = float(abs(netbuf.nav.latitude - netbuf.nav.Xlat[i]));
			dx = float(abs(dx*(60.0f*locos(DegToRad(netbuf.nav.latitude)))));
			dy = abs(dy*60.0f);
			dx = dx*scale;
			dy = dy*scale;
			if (netbuf.nav.Xlon[i] < netbuf.nav.longitude) { dx = -dx; }
			if (netbuf.nav.Xlat[i] < netbuf.nav.latitude) { dy = -dy; }
			l = sqrt((dx*dx) + (dy*dy));			// Distance to Aircraft in pixels.
			if (!MAP_NORTH) { a = 360.0f - netbuf.flight.magheading; }
			else { a = 0.0f; }
			b = fastatan2(dy, dx);						// Angle to Traffic in Radians.
			b = float((b*180.0f) / pi);				// Angle to Traffic in Degrees.
			b = b - 90.0f;
			x = float(l * losin(DegToRad(a - b)));
			y = float(l * locos(DegToRad(a - b)));
			y = Y + y;		// Add offset to map display centre-point.
			x = X + x;
			l = l / scale;			// Distance to aircraft in Nm.
			k = int(abs((netbuf.nav.Xalt[i] * 3.2808399) - netbuf.flight.altitude) / 100);		// Altitude Difference in hundreds of feet.
			if (netbuf.nav.Xalt[i] < netbuf.flight.altitude) { k = -k; }
			/*
			// THIS CODE DRAWS THE HEADING VECTOR - IT WORKS, BUT BOEING DOESN'T SEEM TO HAVE IT.
			a=netbuf.flight.magheading;
			a=a-float((netbuf.nav.Xhdg[i]));
			a=a + 90.0f;
			a=float(int(a+360.0f)%360);
			x2=x + (80.0f*locos(DegToRad(a)));		// Calc & Draw heading indicator line for traffic.
			y2=y + (80.0f*losin(DegToRad(a)));
			glEnable(GL_LINE_SMOOTH);
			glDisable(GL_TEXTURE_2D);
			glColor3f(1.0f,1.0f,1.0f);
			if (l <= 3.0f) { glColor3f(1.0f,1.0f,0.0f); }		// Yellow if closer than 3NM.
			glLineWidth(2.0f);
			glBegin(GL_LINES);
			glVertex2d(x,y);
			glVertex2d(x2,y2);
			glEnd();
			glEnable(GL_TEXTURE_2D);
			*/
			sprintf(sbuf, "%+d", k);			// Draw Altitude difference (ft).
			j = strlen(sbuf);
			j = int((j * 10) / 2);				// Calc drawing X offset.
			float y3 = y + 12.0f;				// Position to draw altitiude diff if above us.
			if (netbuf.nav.Xalt[i] < netbuf.flight.altitude) { y3 = y - 26.0f; }	// Position to draw altitiude diff if above us.
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			if (l >= 5.0f) glColor4fv(gWHITE);
			else if (l >= 3.0f) glColor4fv(gYELLOW);
			else glColor4fv(gRED);

			glEnable(GL_TEXTURE_2D);
			DrawTex(x - 12, y - 12, 25, 25, 0.359375f, 0.8046875f, 0.408203125f, 0.853515625f);			// Diamond a/c Marker
			if (netbuf.nav.Xvsi[i] <= -2.54)
			{
				DrawTex(x - 12 + 25, y - 8, 11, 15, 0.36328125f, 0.765625f, 0.384765625f, 0.794921875f);
			}	// Down Arrow.
			else if (netbuf.nav.Xvsi[i] >= 2.54)
			{
				DrawTex(x - 12 + 25, y - 6, 11, 15, 0.388671875f, 0.765625f, 0.41015625f, 0.794921875f);
			}	// Up Arrow.
			glDisable(GL_BLEND); Print8x12(float(x - j), float(y3), sbuf); glEnable(GL_BLEND);					// Print Altitude difference.
		}
	}
}



// **************************************************************************************
// Open a log file and time-stamp it.
// **************************************************************************************
void OpenLog(void)
{
	// Lets keep a log.
	logfile = fopen("log.txt", "w");
	GetLocalTime(&sysclock);
	fprintf(logfile, "%s Version %s Starting up on ", PROGNAME, VERSION);
	fflush(logfile);
	fprintf(logfile, "%s, %u-%s-%d at %02u:%02u.\n\n", Dayname[sysclock.wDayOfWeek - 1], sysclock.wDay,
		Monthname[sysclock.wMonth - 1], sysclock.wYear, sysclock.wHour, sysclock.wMinute);
	fflush(logfile);
}


void TimerCheck(void)
{
	int newtick = SDL_GetTicks();
	if (newtick - oldhalftick > 500)
	{
		oldhalftick = newtick;
		halftick++;
	}
	if (newtick - oldtick > 1000)
	{
		oldtick = newtick;
		tick++;
		mfdtimer[0]++;
		mfdtimer[1]++;
		mfdtimer[2]++;
		ODO++;
		if (ODO > 9) ODO = 0;
	}
	//sprintf(sbuf, "Tick: %d  HalfTick: %d", tick, halftick);
	//glPrint(10, 10, sbuf, 0);
	//sprintf(sbuf, "%d   %d   %d", submode[0], submode[1], submode[2]);
	//glPrint(275, 10, sbuf, 0);
}

// **************************************************************************************
// Preparatory Stuff for the SDL library.
// **************************************************************************************
int PrepSDL(void)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	char sbuf[128];
	sprintf(sbuf, "%s v%s", PROGNAME, VERSION);
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// "There was an error initing SDL2: " << SDL_GetError() << std::endl;
		return 1;
	}
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, OPENGL_PROFILE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);

	screen = SDL_CreateWindow(sbuf, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sWIDTH, sHEIGHT, SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);

	if (screen == nullptr) {
		//std::cerr << "There was an error creating the window: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(screen);

	if (context == nullptr) {
		//std::cerr << "There was an error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	const unsigned char *version = glGetString(GL_VERSION);
	if (version == nullptr) {
		//std::cerr << "There was an error with OpenGL configuration:" << std::endl;
		return 1;
	}

	SDL_GL_MakeCurrent(screen, context);
	// Start the timer callback.
	//SDL_TimerID my_timer_id = SDL_AddTimer(1000, &timer_callback, 0);
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	return 0;
}



// **************************************************************************************
// Set up our openGL viewport.
// **************************************************************************************
void InitGL(void)
{
	glClearColor(BackR, BackG, BackB, BackAlpha);
	glViewport(0, 0, sWIDTH, sHEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);										// Enable Blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Type Of Blending To Use
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing    ****
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do  ****
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, sWIDTH, 0.0f, sHEIGHT, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
}







void mousetrack(void)
{
	int mx, my;
	mx = event.motion.x;
	my = event.motion.y;
	SDL_GetMouseState(&mx, &my);
	char s[255];
	sprintf(s, "Mouse: %d , %d", mx, my);
	glColor3f(0.0f, 0.0f, 0.0f);
	Block(0, 0, 350, 250);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPrint(50, 70, s, 0);
}


// Draw the 'ticks' around an mfd where x and y denote the bottom left corner of the mfd.
void TickDraw(int n)
{
	if (netbuf.switches.battery[0] == 0) return;
	glColor4fv(gGREEN);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);
	int yoff = 24;
	int ysize = 73;
	// Left Edge
	for (int i = 0; i < 6; i++)
	{
		glBegin(GL_LINES);					// Left side vertical
		glVertex2d(mfdx[n], mfdy[n] +yoff + (i * ysize));
		glVertex2d(mfdx[n]+30, mfdy[n] +yoff + (i * ysize));
		glEnd();
	}
	// Right Edge
	for (int i = 0; i < 6; i++)
	{
		glBegin(GL_LINES);					// 
		glVertex2d(mfdx[n] + mfd_size, mfdy[n] + yoff + (i * ysize));
		glVertex2d(mfdx[n] + mfd_size - 30, mfdy[n] + yoff + (i * ysize));
		glEnd();
	}
}



void test(float xo, float yo)
{
	int i;
	int oldx = -1;
	int oldy = -1;
	int offset = 0;
	int ascii = 66 - 32;
	glColor4fv(gWHITE);
	glLineWidth(1.0f);
	glPushMatrix();
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glScalef(0.5f, 0.5f, 0.5f);
	for (ascii = 0; ascii < 95; ascii++)
	{
		int num = simplex[ascii][0];		// No of vertices.
		int width = simplex[ascii][1];		// Width of character. 
		oldx = -1;
		oldy = -1;
		int n = 2;
		for (i = 0; i < num; i++)
		{
			int xx = simplex[ascii][n];
			int yy = simplex[ascii][n + 1];
			n = n + 2;
			if ((xx!=-1) && (yy!=-1) && (oldx!=-1) && (oldy!=-1)) doline(xo + offset + oldx, yo + oldy, xo + offset + xx, yo + yy);
			oldx = xx;
			oldy = yy;
		}
		offset += width;
	}
	glScalef(1.0f, 1.0f, 1.0f);
	glPopMatrix();
}


void TextOut(float xo, float yo)
{
	int j = 0;
	int i = 0;
	int oldx = -1;
	int oldy = -1;
	int offset = 0;
	int ascii = 66 - 32;
	glColor4fv(gWHITE);
	glLineWidth(1.5f);
	glPushMatrix();
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//	glScalef(0.5f, 0.5f, 0.5f);
	for (j = 0; j < strlen(sbuf); j++)
	{
		ascii = sbuf[j] - 32;
		int num = simplex[ascii][0];		// No of vertices.
		int width = simplex[ascii][1];		// Width of character. 
		oldx = -1;
		oldy = -1;
		int n = 2;
		for (i = 0; i < num; i++)
		{
			int xx = simplex[ascii][n];
			int yy = simplex[ascii][n + 1];
			n = n + 2;
			if ((xx != -1) && (yy != -1) && (oldx != -1) && (oldy != -1)) doline(xo + offset + oldx, yo + oldy, xo + offset + xx, yo + yy);
			oldx = xx;
			oldy = yy;
		}
		offset += width;
	}
//	glScalef(1.0f, 1.0f, 1.0f);
	glPopMatrix();
}


// Draw a blank MFD  (1-3)
void BlankMFD(int n)
{
	glColor4fv(gBLACK);
	Block(mfdx[n], mfdy[n], mfd_size, mfd_size);
	// Might not need ticks yet.
	//	glColor4fv(gWHITE);
	//	TickDraw(mx, my);
	// Draw outline of MFD area - Pretty much for debug purposes only.
	//glColor4fv(gDARKGREY);
	//glDisable(GL_TEXTURE_2D);
	//dolinei(mx, my, mx, my + 450);	//Left
	//dolinei(mx, my + 450, mx + 450, my + 450);	// Top
	//dolinei(mx + 450, my + 450, mx + 450, my);	// Right
	//dolinei(mx + 450, my, mx, my);				// Bottom
	//glEnable(GL_TEXTURE_2D);
}

/*void BATToff(int n)
{
	glColor4fv(gWHITE);
	glBindTexture(GL_TEXTURE_2D, tex[txBATTFAIL].texID);
	DrawChunki(mfdx[n], mfdy[n], 450, 450, 0.0f, 0.0f, 1.0f, 1.0f);
}*/


// This is the unused space at top left.
void ShowVoidSpace(void)
{
	glColor4fv(gWHITE);
	glLineWidth(2.0f);
	glDisable(GL_TEXTURE_2D);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	doline(0, 1080 - 250, 425, 1080);		// top Slope
	doline(425, 1080, 425, 1080 - 250);	// Right Edge
	doline(425, 1080 - 250, 0, 1080 - 250);	// Bottom
	glEnable(GL_TEXTURE_2D);
}

// Draw the FIRE indicator at top left
void FireIndicator(void)
{
	glColor4fv(gVERYDARKGREY);
	glLineWidth(1.5f);
	glDisable(GL_TEXTURE_2D);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	/*doline(207, 964, 275, 1004);  // bottom
	doline(207, 964, 191, 991);   // left
	doline(275, 1004, 259, 1031); // right
	doline(191, 991, 259, 1031);  // top*/
	doline(180, 948, 292, 1014);  // bottom   these calcs just work trust me
	doline(180, 948, 154, 993);   // left     outline 2/3rds that of sby instruments
	doline(266, 1059, 292, 1014); // right
	doline(266, 1059, 154, 993);  // top
	glEnable(GL_TEXTURE_2D);
	netbuf.ann.enginefire[0] == 0 ? glColor4fv(gBURGUNDY) : glColor4fv(gRED);
	glPushMatrix();
	glTranslatef(204, 981, 0);
	glScalef(2, 2, 0);
	glRotatef(30.6f, 0, 0, 1.0);
	sprintf(sbuf, "FIRE");	PrintMed(0, 0, 0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

// Draw the APU indicator at top right
void APUIndicator(void)
{
	/*glColor4fv(gVERYDARKGREY);
	glLineWidth(2.0f);
	glDisable(GL_TEXTURE_2D);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	doline(1920 - 180, 948,  1920 - 292, 1014);  // bottom
	doline(1920 - 180, 948,  1920 - 154, 993 );	  // left
	doline(1920 - 266, 1059, 1920 - 292, 1014); // right
	doline(1920 - 266, 1059, 1920 - 154, 993 );  // top
	glEnable(GL_TEXTURE_2D);
	//netbuf.ann.enginefire[0] == 0 ? glColor4fv(gBURGUNDY) : glColor4fv(gRED); //need to add APU fire annunciator to netrec
	glColor4fv(gBURGUNDY);
	glPushMatrix();
	glTranslatef(1674, 1004, 0);
	glScalef(2, 2, 0);
	glRotatef(-30.6f, 0, 0, 1.0);
	sprintf(sbuf, "APU");	PrintMed(0, 0, 0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();*/
}


// Update the Engine Management Panel.
void UpdateEMP(void)
{
	// Outline of the working area...
	glColor4fv(gDARKGREY);
	glLineWidth(1.0f);
	glDisable(GL_TEXTURE_2D);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	doline(1515, 1080 - 250, 1515, 1080 - 0);	// Left edge
	doline(1515, 1080 - 0, 1920, 1080 - 250);	// Top slope
	doline(1920, 1080 - 250, 1515, 1080 - 250);	// Bottom
	// End of outline code.
	// Outline of fuel section
	glColor4fv(gDARKGREEN);
	glLineWidth(1.0f);
	doline(1535, 999 - 59,  1535, 999 - 103);	// Left inner edge
	doline(1535, 999 - 59,  1520, 999 - 59 );	// Left horizontal
	doline(1520, 999 - 59,  1520, 999 - 0  );	// Left outer edge
	doline(1520, 999 - 0,	1636, 999 - 0  );	// Top
	doline(1636, 999 - 0,   1670, 999 - 21 );	// Right slope
	doline(1670, 999 - 21,  1670, 999 - 59 );	// Right outer edge
	doline(1670, 999 - 59,  1649, 999 - 59 );	// Right horizontal
	doline(1649, 999 - 59,  1649, 999 - 103);	// Right inner edge
	doline(1520, 999 - 103, 1568, 999 - 103);	// Bottom Left
	doline(1610, 999 - 103, 1800, 999 - 103);	// Bottom Right
	glColor4fv(gGREEN);
	//	doline(1575, 850, 1575, 870);
	glEnable(GL_TEXTURE_2D);
	sprintf(sbuf, "%3.1f", netbuf.engine.N1[0]);
	SevenString(1570, 835, sbuf, 4);		// RPM
	sprintf(sbuf, "%4d", int(netbuf.engine.ittdegc[0]));
	SevenString(1720, 835, sbuf, 4);		// T 'C
	sprintf(sbuf, "%03d", int(netbuf.engine.ff[0] * 60.0));
	SevenString(1665, 900, sbuf, 4);		// Kg/Min
	sprintf(sbuf, "%4d", int(netbuf.fuel.tank[0] / 2));		// FIDDLED FOR CURRENT A/C MODEL
	SevenString(1525, 950, sbuf, 4);		// PORT
	sprintf(sbuf, "%4d", int(netbuf.fuel.tank[0] / 2));
	SevenString(1605, 950, sbuf, 4);		// STBD
	sprintf(sbuf, "%4d", int(netbuf.fuel.tank[0]));
	SevenString(1560, 905, sbuf, 4);		// TOTAL
	glColor4fv(gDARKGREEN);
	Block(1545, 870, 135, 12);		// RPM area
	Block(1690, 870, 135, 12);		// T6 temp area
	if (netbuf.switches.battery[0] == 0);
	else {
		glColor4fv(gGREEN);
		int x = (netbuf.engine.N1[0] < 100) ? int(netbuf.engine.N1[0] * 1.227f) : 122.7f; // If the N1 % is above 100, cap the bar so it doesn't overflow
		Block(1545, 870, x, 12);		// RPM area
		x = int(netbuf.engine.ittdegc[0] - 200.0f);				// Lowest temp shown.
		x = (x > 0) ? x : 0;
		x = int(x * 0.20769230769230769230769230769231f);		// Scale it.
		Block(1690, 870, x, 12);		// T6 Temp area
	}
	glColor4fv(gDARKGREEN);
	sprintf(sbuf, "APU");		glPrint(1515, 998, sbuf, 0);
	if (netbuf.switches.battery[0] == 0) glColor4fv(gDARKGREEN); // If battery is off draw text in dark green
	else if (netbuf.elec.igniter[0]) { glColor4fv(gGREEN); } else { glColor4fv(gDARKGREEN); } // If switch is on, draw in bright green, else dark green
	sprintf(sbuf, "IGN");
	glPrint(1552, 998, sbuf, 0);
	glColor4fv(gDARKGREEN);
	sprintf(sbuf, "SRG");
	glPrint(1589, 998, sbuf, 0);
	// IGN SRG");
	(netbuf.switches.battery[0] == 0) ? glColor4fv(gDARKGREEN) : glColor4fv(gGREEN); // If battery is off, draw in dark green, if on, draw in bright green
	sprintf(sbuf, "FUEL kg");	pString96(1571, 985, 1);
	sprintf(sbuf, "PORT");		pString96(1535, 976, 1);
	sprintf(sbuf, "STBD");		pString96(1627, 976, 1);
	sprintf(sbuf, "TOTAL");		pString96(1573, 892, 1);
	sprintf(sbuf, "F/F");		pString96(1685, 930, 1);
	sprintf(sbuf, "kg/Min");	pString96(1715, 907, 1);
	sprintf(sbuf, "RPM PCNT");	pString96(1637, 842, 1);
	sprintf(sbuf, "T6 TEMP");	pString96(1786, 842, 1);
}



// Update the general info mode on an mfd.
void GenMode(int n)
{
	glColor4fv(gWHITE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);		// Select Our Utility Texture
	DrawChunki(mfdx[n]+60, mfdy[n]+330, 341, 63, 0.404296875f, 0.935546875f, 0.7373046875f, 0.9970703125f);		// Hawk Logo
	glDisable(GL_TEXTURE_2D);
	if (submode[n] == 0)		// Cold start - Init.
	{
		submode[n] = 1;			// Mark init as underway.
		mfdtimer[n] = 0;		// Reset the timer.
		return;
	}
	if (submode[n] == 1)			// init is underway.
	{
		sprintf(sbuf, "GENMODE INIT");			glPrint(mfdx[n] + 150, mfdy[n] + 200, sbuf, 0);
		sprintf(sbuf, "(c)2017 Roy Coates");	glPrint(mfdx[n] + 150, mfdy[n] + 180, sbuf, 0);
		if (mfdtimer[n] == 5)
		{
			submode[n] = 2;			// init complete.
			return;
		}
	}
	if (submode[n] == 2)
	{
		sprintf(sbuf, "GENMODE COMPLETE");
		glPrint(mfdx[n] + 150, mfdy[n] + 200, sbuf, 0);

		int jays = SDL_NumJoysticks();
		sprintf(sbuf, "Found %d Interface Boards.", jays);
		glPrint(mfdx[n] + 150, mfdy[n] + 180, sbuf, 0);
		
		//int butts = SDL_JoystickNumButtons( handle );		// Need to OPEN the joystick first to get a handle to it
	}
}



// Draw the MFD button labels.
void DrawLabels(int which)
{
	int x1, x2, y;
	glColor4fv(gCYAN);
	x1 = 5;
	x2 = 400;		// glPrint
	x2 = 420;		// pString96
	y  = 348;
	for (int i = 1; i < 6; i++)
	{
		sprintf(sbuf, label[i]);
		//glPrint(mfdx[which] + x1, mfdy[which] + y, sbuf, 0);
		glColor4fv(gGREY);	pString96(mfdx[which] + x1 + 1, mfdy[which] + y - 1, 1);
		glColor4fv(gCYAN);	pString96(mfdx[which] + x1, mfdy[which] + y, 1);
		sprintf(sbuf, label[i + 5]);
		//glPrint(mfdx[which] + x2, mfdy[which] + y, sbuf, 0);
		glColor4fv(gGREY);	pString96(mfdx[which] + x2 + 1, mfdy[which] + y - 1, 1);
		glColor4fv(gCYAN);	pString96(mfdx[which] + x2, mfdy[which] + y, 1);
		y -= 73;
	}
}


void SetColor(int n)
{
	switch (n) 
	{
		case cWHITE: glColor4fv(gWHITE); break;
		case cBLACK: glColor4fv(gBLACK); break;
		case cAMBER: glColor4fv(gAMBER); break;
	}

}


void MFDhsi(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	BlankMFD(n);
	int mx, my;
	mx = int(mfdx[n] + (mfd_size / 2));
	my = int(mfdy[n] + (mfd_size / 2));
	strcpy(label[1], "SYST");	strcpy(label[2], "ENG");	strcpy(label[3], "DMMY");	strcpy(label[4], "ELEC");	strcpy(label[5], "STRS");
	strcpy(label[6], "MAP");	strcpy(label[7], "I/O");	strcpy(label[8], "ADI");	strcpy(label[9], "CHK");	strcpy(label[10], "ACPT");
	DrawLabels(n);
	if ((Input == n) && (MFDKey != NULL))
	{
		Input = -1;
		if (MFDKey == SDLK_2) { mode[n] = mfdDUMMY; }			// Switch to dummy texture
		if (MFDKey == SDLK_3) { mode[n] = mfdELEC; }			// Switch to ELEC view
		if (MFDKey == SDLK_5) { mode[n] = mfdMAP; }				// Switch to MAP view
		if (MFDKey == SDLK_6) { mode[n] = mfdDEBUG; }			// Switch to I/O view
		if (MFDKey == SDLK_7) { mode[n] = mfdADI; }				// Switch to ADI view
		if (MFDKey == SDLK_8) {
			if (mode[LEFT] == mfdEFRCS2 || mode[CENTRE] == mfdEFRCS2 || mode[RIGHT] == mfdEFRCS2) { mode[n] = mfdEFRCS2; }		// Warp to the currently displaying EFRCS screen
			else {
				if (mode[LEFT] == mfdEFRCS3 || mode[CENTRE] == mfdEFRCS3 || mode[RIGHT] == mfdEFRCS3) { mode[n] = mfdEFRCS3; }
				else {
					mode[n] = mfdEFRCS;		// If no EFRCS display up, display EFRCS1
				}
			}
		}
		MFDKey = NULL;
	}
	//glColor4fv(gAMBER);
	//	sprintf(sbuf, "I AM AN HSI");
	//glPrint(mfdx[n] + 140, mfdy[n] + 50, sbuf, 0);
	//glColor4fv(gGREEN);
	//sprintf(sbuf, "Pitch: %f", netbuf.flight.pitch);
	//glPrint(mfdx[n] + 140, mfdy[n] + 300, sbuf, 0);
	//sprintf(sbuf, "Roll: %f", netbuf.flight.roll);
	//glPrint(mfdx[n] + 140, mfdy[n] + 285, sbuf, 0);
	//sprintf(sbuf, "V/S: %f", netbuf.flight.vs);
	//glPrint(mfdx[n] + 140, mfdy[n] + 270, sbuf, 0);
	//sprintf(sbuf, "Airspeed: %f", netbuf.flight.Vind);
	//glPrint(mfdx[n] + 140, mfdy[n] + 255, sbuf, 0);
	//sprintf(sbuf, "Altitude: %f", netbuf.flight.altitude);
	//glPrint(mfdx[n] + 140, mfdy[n] + 240, sbuf, 0);
	//sprintf(sbuf, "Alpha: %f", netbuf.flight.AoA);
	//glPrint(mfdx[n] + 140, mfdy[n] + 225, sbuf, 0);
	//sprintf(sbuf, "Baro: %4d", int(netbuf.flight.baro / 0.0295301f));
	//glPrint(mfdx[n] + 140, mfdy[n] + 210, sbuf, 0);
	glColor4fv(gAMBER);
	if (netbuf.engine.N1[0] < 10) { sprintf(sbuf, "FOR ENGINE START, PRESS 'CHK'"); PrintMed(mfdx[n] + 110, mfdy[n] + 400, 1); }
	// Compass Rose.
	glScissor(mfdx[n], mfdy[n], mfd_size, mfd_size);
	glEnable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, tex[txHSICOMPASS].texID);
	glPushMatrix();
	glScalef(0.68f, 0.68f, 0);
	switch (n)
	{
	case 0: glTranslated(mfdx[n] + 330, mfdy[n] + 390, 0); break;
	case 1: glTranslated(mfdx[n] + 676, mfdy[n] + 290, 0); break;
	case 2: glTranslated(mfdx[n] + 1025, mfdy[n] + 390, 0); break;
	}
	glRotatef(netbuf.flight.magheading, 0.0, 0.0, 1.0);
	glTranslated(-mfdx[n] - 225, -mfdy[n] - 225, 0);
	DrawTexi(mfdx[n], mfdy[n], 450, 450, 0.0f, 0.0f, 1.0f, 1.0f);
	glPopMatrix();
	glDisable(GL_SCISSOR_TEST);
	glColor4fv(gWHITE);
	Block(mfdx[n] + 224, mfdy[n] + 353, 3, 15);                        // Lubber line above compass.
	sprintf(sbuf, "%03d", Round(netbuf.flight.magheading));
	PrintMed(mfdx[n] + 214, mfdy[n] + 373, 1);
	glColor4fv(gGREEN);    sprintf(sbuf, "M"); PrintMed(mfdx[n] + 239, mfdy[n] + 373, 1);
	glBindTexture(GL_TEXTURE_2D, tex[LINEAR].texID);                    // Aircraft Triangle
	glColor4fv(gWHITE);
	DrawChunki(mx - 5, my - 41, 10, 22, 0.91796875f, 0.255859375f, 0.9375f, 0.27734375f);
}

void MFDdummy(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	BlankMFD(n);
	strcpy(label[1], "SYST");	strcpy(label[2], "ENG");	strcpy(label[3], "HSI");	strcpy(label[4], "ELEC");	strcpy(label[5], "STRS");
	strcpy(label[6], "MAP");	strcpy(label[7], "I/O");	strcpy(label[8], "");	strcpy(label[9], "CHK");	strcpy(label[10], "ACPT");
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_2) { mode[n] = mfdHSI; }			// Switch to HSI view
		MFDKey = NULL;
		Input = -1;
	}

	glBindTexture(GL_TEXTURE_2D, tex[DUMMY].texID);		// Select Our Dummy Texture
	glColor4fv(gWHITE);
	DrawChunki(mfdx[n], mfdy[n], 450, 450, 0.0f, 0.0f, 1.0f, 1.0f);
	TickDraw(n);
	DrawLabels(n);
}

void MFDadi(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	BlankMFD(n);
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_2) { mode[n] = mfdHSI; }			// Switch to HSI view
		if (MFDKey == SDLK_5) { mode[n] = mfdMAP; }			// Switch to MAP view.
		if (MFDKey == SDLK_7) { mode[n] = mfdADI; }			// Switch to ADI view
		if (MFDKey == SDLK_8) {
			if (mode[LEFT] == mfdEFRCS2 || mode[CENTRE] == mfdEFRCS2 || mode[RIGHT] == mfdEFRCS2) { mode[n] = mfdEFRCS2; }
			else {
				if (mode[LEFT] == mfdEFRCS3 || mode[CENTRE] == mfdEFRCS3 || mode[RIGHT] == mfdEFRCS3) { mode[n] = mfdEFRCS3; }
				else {
					mode[n] = mfdEFRCS;
				}
			}
		}
		MFDKey = NULL;
		Input = -1;
	}
	glColor4fv(gWHITE);
	glScissor(mfdx[n], mfdy[n], mfd_size, mfd_size);
	glEnable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, tex[txATTITUDE].texID);		// Select Our large ADI Texture

	GLfloat offset = netbuf.flight.pitch;															// Offset for pitch.
	if (offset > +80.0f) { offset = +80.0f; }														// CHECK FOR PITCH LIMITS.
	if (offset < -80.0f) { offset = -80.0f; }
	offset = offset * 0.0048828125f;																// 5px per degree of pitch.
	GLfloat a = offset;
	GLfloat b = a + 1.0f;

	glPushMatrix();
	glTranslated(mfdx[n], mfdy[n], 0);
	// Rotate here to show Roll.
	glPushMatrix();
	glTranslated(230, 190, 0);
	glRotatef(netbuf.flight.roll, 0.0, 0.0, 1.0);
	glTranslated(-230, -190, 0);
	//DrawChunki(-281, -321, 1024, 1024, 0.0f, 0.0f, 1.0f, 1.0f);		// Draw attitude ladder tape  1024x1024.
	DrawChunki(-281 + 256, -321 + 256, 512, 512, 0.25f, 0.25f + a, 0.75f, 0.75f + a);		// Draw attitude ladder tape.
	//	DrawTexi(-128, -128, 512, 512, 0.0f, a - 0.25, 1.0f, b + 0.25);		// Horizon Tape.
	glPopMatrix();
	glTranslated(-mfdx[n], -mfdy[n], 0);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex[txADIMASK].texID);			// 
	DrawChunki(mfdx[n] + 0, mfdy[n] + 0, 450, 450, 0.0f, 0.0f, 1.0f, 1.0f);		// Mask over the attitude tape.

	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);			// 
	DrawChunki(mfdx[n] + 112, mfdy[n] + 178, 237, 18, 0.0439453125f, 0.7763671875f, 0.275390625f, 0.7939453125f);		// Aircraft Symbol

	DrawChunki(mfdx[n] + 221, mfdy[n] + 56, 20, 15, 0.4296875f, 0.8876953125f, 0.44921875f, 0.90234375f);							// Roll pointer at bottom.

	// Compass Rose.
	glScissor(mfdx[n] + 20, mfdy[n] + 300, mfd_size - 50, 60);		// Rough calcs - seems to do the job!
	glEnable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, tex[txADICOMPASS].texID);			// 
	glPushMatrix();
	glTranslated(mfdx[n] + 231, mfdy[n] + 189, 0);
	glRotatef(netbuf.flight.magheading, 0.0, 0.0, 1.0);
	glTranslated(-mfdx[n] - 231, -mfdy[n] - 189, 0);
	DrawTexi(mfdx[n], mfdy[n], 450, 450, 0.0f, 0.0f, 1.0f, 1.0f);
	glPopMatrix();
	glDisable(GL_SCISSOR_TEST);
	glColor4fv(gWHITE);
	Block(mfdx[n] + 230, mfdy[n] + 350, 3, 15);						// Lubber line above compass.
	sprintf(sbuf, "%03d", Round(netbuf.flight.magheading));
	PrintMed(mfdx[n] + 220, mfdy[n] + 375, 1);
	glColor4fv(gGREEN);	sprintf(sbuf, "T"); PrintMed(mfdx[n] + 245, mfdy[n] + 375, 1);

	//TickDraw(n);
	glColor4fv(gGREEN);		sprintf(sbuf, "CAS");								PrintMed(mfdx[n] + 55, mfdy[n] + 370, 1);
	glColor4fv(gWHITE);		sprintf(sbuf, "%03.0f", netbuf.flight.Vind);		PrintMed(mfdx[n] + 85, mfdy[n] + 370, 1);			// vIndicated for CAS:  (Knots)
	glColor4fv(gWHITE);		sprintf(sbuf, "%3.2f", netbuf.flight.Mach);			PrintMed(mfdx[n] + 78, mfdy[n] + 350, 1);			// Mach No
	glColor4fv(gGREEN);		sprintf(sbuf, "PKG");								PrintMed(mfdx[n] + 55, mfdy[n] + 330, 1);			// Peak G
	glColor4fv(gWHITE);		sprintf(sbuf, "%+3.1f", PeakG);						PrintMed(mfdx[n] + 85, mfdy[n] + 330, 1);			// Peak G
	glColor4fv(gWHITE);		sprintf(sbuf, "%-3.1f", PeakNegG);					PrintMed(mfdx[n] + 85, mfdy[n] + 310, 1);			// Peak Negative G


	glColor4fv(gWHITE);		sprintf(sbuf, "%5d", int(netbuf.flight.altitude));	PrintMed(mfdx[n] + 335, mfdy[n] + 370, 1);
	glColor4fv(gGREEN);		sprintf(sbuf, "FT");								PrintMed(mfdx[n] + 380, mfdy[n] + 370, 1);
	glColor4fv(gWHITE);		sprintf(sbuf, "%4d", int(netbuf.flight.baro));		PrintMed(mfdx[n] + 343, mfdy[n] + 350, 1);
	glColor4fv(gGREEN);		sprintf(sbuf, "MB");								PrintMed(mfdx[n] + 380, mfdy[n] + 350, 1);
	glColor4fv(gWHITE);		sprintf(sbuf, "%+3d", int(netbuf.env.oat));			PrintMed(mfdx[n] + 351, mfdy[n] + 330, 1);
	glColor4fv(gGREEN);		sprintf(sbuf, "'C");								PrintMed(mfdx[n] + 380, mfdy[n] + 330, 1);


	glColor4fv(gGREEN);		sprintf(sbuf, "HDG");								PrintMed(mfdx[n] + 55, mfdy[n] + 60, 1);	// HDG
	glColor4fv(gWHITE);		sprintf(sbuf, "%3.0f", netbuf.flight.magheading);	PrintMed(mfdx[n] + 95, mfdy[n] + 60, 1);
	glColor4fv(gGREEN);		sprintf(sbuf, "CRS");								PrintMed(mfdx[n] + 55, mfdy[n] + 40, 1);	// CRS
	glColor4fv(gWHITE);		sprintf(sbuf, "%3.0f", netbuf.flight.heading);		PrintMed(mfdx[n] + 95, mfdy[n] + 40, 1); // glPrint(mfdx[n] + 80, mfdy[n] + 40, sbuf, 0);

	//	sprintf(sbuf, "%3.0f", netbuf.flight.Vind);		glColor4fv(gWHITE);		glPrint(mfdx[n] + 50, mfdy[n] + 360, sbuf, 0);
	//	sprintf(sbuf, "KTS");		glColor4fv(gGREEN);		glPrint(mfdx[n] + 90, mfdy[n] + 360, sbuf, 0);

	glColor4fv(gGREEN);
	Box(mfdx[n] + 145, mfdy[n] + 350, 50, 40);
	sprintf(sbuf, "T");															PrintMed(mfdx[n] + 151, mfdy[n] + 373, 1);
	glColor4fv(gWHITE);		sprintf(sbuf, "%3d", int(netbuf.flight.Vtrue));		PrintMed(mfdx[n] + 168, mfdy[n] + 373, 1);
	glColor4fv(gGREEN);		sprintf(sbuf, "G");									PrintMed(mfdx[n] + 151, mfdy[n] + 356, 1);
	glColor4fv(gWHITE);		sprintf(sbuf, "%3d", int(netbuf.flight.Vgrnd));		PrintMed(mfdx[n] + 168, mfdy[n] + 356, 1);


	glColor4fv(gGREEN);		sprintf(sbuf, "FLP");								PrintMed(mfdx[n] + 55, mfdy[n] + 285, 1);
	glColor4fv(gWHITE);		sprintf(sbuf, "%2d", int(netbuf.airframe.flap[0]));	PrintMed(mfdx[n] + 85, mfdy[n] + 285, 1);
	if (netbuf.airframe.gear[0] == 1){
		glColor4fv(gGREEN);		sprintf(sbuf, "GEAR");							PrintMed(mfdx[n] + 55, mfdy[n] + 270, 1);	// Displays when gear is down and locked
	}
	else if (netbuf.airframe.gear[0] > 0.0f){
		glColor4fv(gAMBER);		sprintf(sbuf, "GEAR");							PrintMed(mfdx[n] + 55, mfdy[n] + 270, 1);	// Displays when gear is transitioning
	}
	if (netbuf.airframe.speedbrakeratio == 1){
		glColor4fv(gGREEN);		sprintf(sbuf, "SPDBK");							PrintMed(mfdx[n] + 55, mfdy[n] + 255, 1);	// Displays when speedbrake is deployed
	}


	// Wind Speed and Direction.
	glColor4fv(gGREEN);
	glDisable(GL_TEXTURE_2D);
	Circle(mfdx[n] + 370, mfdy[n] + 50, 30);

	//	dolinei(mfdx[n] + 370, mfdy[n] + 50, mfdx[n] + 380, mfdy[n] + 50);
	//	dolinei(mfdx[n] + 370, mfdy[n] + 50, mfdx[n] + 370, mfdy[n] + 60);
	glPushMatrix();
	glColor4fv(gWHITE);
	glTranslated(mfdx[n] + 370, mfdy[n] + 50, 0);
	glRotatef(-netbuf.env.winddir, 0.0f, 0.0f, 1.0f);
	glTranslated(-mfdx[n] - 370, -mfdy[n] - 50, 0);
	dolinei(mfdx[n] + 370, mfdy[n] + 20, mfdx[n] + 370, mfdy[n] + 78);			// Line, bottom to top.
	dolinei(mfdx[n] + 370, mfdy[n] + 78, mfdx[n] + 365, mfdy[n] + 70);			// Arrowhead
	dolinei(mfdx[n] + 370, mfdy[n] + 78, mfdx[n] + 375, mfdy[n] + 70);			// Arrowhead
	glPopMatrix();
	glColor4fv(gBLACK);
	Block(mfdx[n] + 357, mfdy[n] + 42, 26, 13);
	glColor4fv(gWHITE);
	sprintf(sbuf, "%03d", netbuf.env.windspeed);	PrintMed(mfdx[n] + 358, mfdy[n] + 43, 1);
	strcpy(label[1], "SYST");	strcpy(label[2], "ENG");	strcpy(label[3], "HSI");	strcpy(label[4], "ELEC");	strcpy(label[5], "STRS");
	strcpy(label[6], "MAP");	strcpy(label[7], "I/O");	strcpy(label[8], "");	strcpy(label[9], "CHK");	strcpy(label[10], "ACPT");
	DrawLabels(n);
	glDisable(GL_SCISSOR_TEST);
}


void MFDmap(int n)
{

	/*  FROM SimDriver
	char		buf[128];				// Buffer used to send cmd's to X-Plane

	void SendBuf(void)
	{
	sendto(cmdsocket, (const char*)&buf, sizeof(buf), 0, (struct sockaddr *)&sockbuf, sizeof(sockbuf));
	unsigned char a = buf[0];
	sprintf(sbuf, "Sent -> Int: %d, ASCII %c", a, buf[0]);
	MemoAdd(sbuf);
	}

	int outch = 143;
	buf[0] = outch;
	SendBuf();

	*/
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	BlankMFD(n);
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_2) { mode[n] = mfdHSI; }			// Switch to HSI view.
		if (MFDKey == SDLK_4) { mode[n] = mfdADI; }			// Switch to ADI view
		if ((MFDKey == SDLK_5) && (netbuf.switches.maprange < 5)) { SendCommand(pcMapRangeUP, 0); log("Map Range UP\n"); }			// Map Range +
		if ((MFDKey == SDLK_6) && (netbuf.switches.maprange > 0)) { SendCommand(pcMapRangeDOWN, 0); log("Map Range DOWN\n"); }		// Map Range -
		if (MFDKey == SDLK_8) {
			if (mode[LEFT] == mfdEFRCS2 || mode[CENTRE] == mfdEFRCS2 || mode[RIGHT] == mfdEFRCS2) { mode[n] = mfdEFRCS2; }
			else {
				if (mode[LEFT] == mfdEFRCS3 || mode[CENTRE] == mfdEFRCS3 || mode[RIGHT] == mfdEFRCS3) { mode[n] = mfdEFRCS3; }
				else {
					mode[n] = mfdEFRCS;
				}
			}
		}
		Input = -1;
		MFDKey = NULL;
	}
	int mx, my;
	mx = int(mfdx[n] + (mfd_size / 2));
	my = int(mfdy[n] + (mfd_size / 2));
	strcpy(label[1], "SYST");	strcpy(label[2], "ENG");	strcpy(label[3], "HSI");	strcpy(label[4], "ELEC");	strcpy(label[5], "ADI");
	strcpy(label[6], "RNG+");	strcpy(label[7], "RNG-");	strcpy(label[8], "TRCK");	strcpy(label[9], "CHK"); /*NRTH*/	strcpy(label[10], "MAIN");
	DrawLabels(n);
	//	glColor4fv(gYELLOW);
	//	sprintf(sbuf, "I AM A MAP");
	//	glPrint(mfdx[n] + 140, mfdy[n] + 200, sbuf, 0);
	glScissor(mfdx[n], mfdy[n], mfd_size, mfd_size);
	glEnable(GL_SCISSOR_TEST);
	glColor4fv(gDARKGREEN);
	glDisable(GL_TEXTURE_2D);
	Circle(mx, my, 100);
	Circle(mx, my, 200);
	glEnable(GL_TEXTURE_2D);
	glColor4fv(gWHITE);
	Draw_Navaids(mx, my, int(mfd_size / 2));
	Draw_Airports(mx, my, int(mfd_size / 2));
	Draw_TCAS(mx, my, int(mfd_size / 2));
	glBindTexture(GL_TEXTURE_2D, tex[LINEAR].texID);		// 
	glColor4fv(gWHITE);
	DrawChunki(mx - 9, int(my - 19), 18, 19, 0.74609375f, 0.263671875f, 0.78125f, 0.30078125f);	// Aircraft Outline.
	glDisable(GL_SCISSOR_TEST);
	glColor4fv(gMAGENTA);
	sprintf(sbuf, "%03.0f", netbuf.flight.magheading);
	glPrint(mfdx[n] + 200, mfdy[n] + 433, sbuf, 0);
}

void Poll(void)
{
	SDL_PollEvent(&event);			// Check for any events.
	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym)
		{
		case SDLK_ESCAPE:	quit = true; break;
		case SDLK_RETURN:	done = true; break;
		}
		break;
	}
}


// This is THE Master Initialise routine. Check files and hardware here.
void MFDInitialise(int n)
{
	int Y = 390;
	BlankMFD(n);
	log("Initialising...\n");
	glColor4fv(gWHITE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex[txPARTS].texID);		// Select Our Utility Texture
	DrawChunki(mfdx[n] + 155, mfdy[n] + 415, 341/2, 63/2, 0.404296875f, 0.935546875f, 0.7373046875f, 0.9970703125f);		// Hawk Logo
	glDisable(GL_TEXTURE_2D);
	glColor4fv(gWHITE);
	sprintf(sbuf, "%s v", PROGNAME);
	strcat(sbuf, VERSION);
	pString96(mfdx[n] + 155, mfdy[n] + Y, 1);	Y -= 15;
	sprintf(sbuf, "%s", AUTHOR);
	pString96(mfdx[n] + 185, mfdy[n] + Y, 1);	Y -= 15;

	log("Looking for Interfaces.\n");
	glColor4fv(gGREEN);
	sprintf(sbuf, "HARDWARE INITIALISATION:");			glColor4fv(gYELLOW);	pString96(mfdx[n] + 40, mfdy[n] + Y, 1);	Y -= 15;
	// START WITH BODNAR INTERFACES
	bodnar[1] = 99;						// 99 indicates board not found.
	bodnar[2] = 99;
	bodnar[3] = 99;
	numjoys = SDL_NumJoysticks();
	glColor4fv(gGREEN);
	sprintf(sbuf, "Detected %d HID Interface(s)\n", numjoys);						pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	log(sbuf);
	for (int i = 0; i < numjoys; i++)
	{
		Joy[i] = SDL_JoystickOpen(i);
		if (Joy[i] == NULL) { log("HID Error!\n"); sprintf(sbuf, "Bad HID Connect!"); glPrint(mfdx[n] + 70, mfdy[n] + Y, sbuf, 0); Y -= 15;  }
//		sprintf(sbuf, "Axes: %d", SDL_JoystickNumAxes(Joy[i]));					pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
//		SDL_JoystickGUID g = SDL_JoystickGetGUID(Joy[i]);
//		SDL_JoystickGetGUIDString(g, sbuf2, sizeof(sbuf));
//		strcpy(sbuf, "GUID: ");
//		strcat(sbuf, sbuf2);													pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
//		log(sbuf);
//		sprintf(sbuf, "Buttons: %d", SDL_JoystickNumButtons(Joy[i]));			pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
//		sprintf(sbuf, "Hats: %d", SDL_JoystickNumHats(Joy[i]));					pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
		sprintf(sbuf, "Name: /%s/\n", SDL_JoystickName(Joy[i]));				log(sbuf);
		sprintf(sbuf, "IndexName: %s\n", SDL_JoystickNameForIndex(i));				log(sbuf);
	}

	// Which order did Windows detect them?
	for (int i = 0; i < numjoys; i++)
	{
		if (strcmp(SDL_JoystickName(Joy[i]), "BU0836X Interface") == 0) { BODNAR1 = TRUE; bodnar[1] = i; }
		if (strcmp(SDL_JoystickName(Joy[i]), "BU0836X Interface 2") == 0) { BODNAR2 = TRUE; bodnar[2] = i; }
		if (strcmp(SDL_JoystickName(Joy[i]), "BU0836X Interface 3") == 0) { BODNAR3 = TRUE; bodnar[3] = i; }
	}
	// Report Bodnar status.
	if (BODNAR1)
	{
		glColor4fv(gGREEN);
		sprintf(sbuf, "Bodnar01 - OK");	pString96(mfdx[n] + 70, mfdy[n] + Y, 1); Y -= 15;
	}
	else
	{
		glColor4fv(gRED);
		sprintf(sbuf, "Bodnar01 - MISSING");	pString96(mfdx[n] + 70, mfdy[n] + Y, 1); Y -= 15;
		sprintf(sbuf, "Bodnar 1 missing\n");	log(sbuf);
	}
	if (BODNAR2)
	{
		glColor4fv(gGREEN);
		sprintf(sbuf, "Bodnar02 - OK");	pString96(mfdx[n] + 70, mfdy[n] + Y, 1); Y -= 15;
	}
	else
	{
		glColor4fv(gRED);
		sprintf(sbuf, "Bodnar02 - MISSING");	pString96(mfdx[n] + 70, mfdy[n] + Y, 1); Y -= 15;
		sprintf(sbuf, "Bodnar 2 missing\n");	log(sbuf);
	}
	if (BODNAR3)
	{
		glColor4fv(gGREEN);
		sprintf(sbuf, "Bodnar03 - OK");	pString96(mfdx[n] + 70, mfdy[n] + Y, 1); Y -= 15;
	}
	else
	{
		glColor4fv(gRED);
		sprintf(sbuf, "Bodnar03 - MISSING");	pString96(mfdx[n] + 70, mfdy[n] + Y, 1); Y -= 15;
		sprintf(sbuf, "Bodnar 3 missing\n");	log(sbuf);
	}


	// REPORT PHIDGET STATUS
	if (Kit1)
	{
		glColor4fv(gGREEN);	sprintf(sbuf, "Phidget-A: OK");	 pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	}
	else
	{
		glColor4fv(gRED);	sprintf(sbuf, "Phidget-A: MISSING");	 pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	}
	if (Kit2)
	{
		glColor4fv(gGREEN);	sprintf(sbuf, "Phidget-B: OK");	 pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	}
	else
	{
		glColor4fv(gRED);	sprintf(sbuf, "Phidget-B: MISSING");	 pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	}
	if (Kit3)
	{
		glColor4fv(gGREEN);	sprintf(sbuf, "Phidget-C: OK");	 pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	}
	else
	{
		glColor4fv(gRED);	sprintf(sbuf, "Phidget-C: MISSING");	 pString96(mfdx[n] + 70, mfdy[n] + Y, 1);	Y -= 15;
	}

	// Load nav data (routines from X-Panel)
	glColor4fv(gAMBER);
	sprintf(sbuf, "Loading Fixes...");
	pString96(mfdx[n] + 40, mfdy[n] + Y, 1); 
	LoadFixes();
	sprintf(sbuf, "(%d)", fixcnt);
	pString96(mfdx[n] + 210, mfdy[n] + Y, 1); Y -= 15;
	sprintf(sbuf, "Loading Navaids...");
	pString96(mfdx[n] + 40, mfdy[n] + Y, 1);	
	LoadNav();
	sprintf(sbuf, "(%d)", navcnt);
	pString96(mfdx[n] + 210, mfdy[n] + Y, 1);	Y -= 15;
	sprintf(sbuf, "Loading Airports...");
	pString96(mfdx[n] + 40, mfdy[n] + Y, 1);	
	LoadAirports();
	sprintf(sbuf, "(%d)", aptcnt);
	pString96(mfdx[n] + 210, mfdy[n] + Y, 1);	Y -= 15;

	// Cycle Phidgets
	int b = 15;
	if (Kit1)
	{
		for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, i, 1); Sleep(b); }
	}
	if (Kit2)
	{
		for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit2, i, 1); Sleep(b); }
	}
	if (Kit3)
	{
		for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit3, i, 1); Sleep(b); }
	}
	Sleep(500);
	if (Kit1)
	{
		for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, i, 0); Sleep(b); }
	}
	if (Kit2)
	{
		for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit2, i, 0); Sleep(b); }
	}
	if (Kit3)
	{
		for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit3, i, 0); Sleep(b); }
	}

	// All done.
	glColor4fv(gAMBER);
	sprintf(sbuf, "PRESS ENTER TO CONTINUE!");  pString96(mfdx[n] + 100, mfdy[n] + 5, 1);
	SDL_GL_SwapWindow(screen);
	//do { Poll(); } while (!done);							//  Keep polling until ENTER is pressed.
	mode[LEFT] = mfdINISTAT;									// This is what we'll be doing *after* initialisation.
	mode[CENTRE] = mfdINISTAT;									// These are the initial modes for each screen.
	mode[RIGHT] = mfdINISTAT;
}


void MFDelec(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	if ((Input == n) && (MFDKey != NULL))					// Key 0 returns us to HSI
	{
		if (MFDKey == SDLK_0) { mode[n] = mfdHSI; }
		Input = -1;
		MFDKey = NULL;
	}
	BlankMFD(n);
	glColor4fv(gAMBER);
	int y = 415;				// Highest point we want to display text.
	int x = 50;
	sprintf(sbuf, "ELEC SYSTEMS INFO:   Version %s", VERSION);				pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 30;
	glColor4fv(gGREEN);
	sprintf(sbuf, "Battery 1: %d", netbuf.switches.battery[0]);				pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Battery 2: %d", netbuf.switches.battery[1]);				pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Yaw Damper: %d", netbuf.switches.yawdamper);				pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Strobe Light: %d", netbuf.switches.strobe);				pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Ignition: %d", netbuf.elec.igniter[0]);					pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Fuel Pump: %d", netbuf.switches.fuelpump[0]);			pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Generator: %d", netbuf.switches.generator[0]);			pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "Pitot Heat: %d", netbuf.switches.pitot);					pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "LPFuelCock: %d", netbuf.fuel.selected);					pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "FADEC: %d", netbuf.fuel.fadec[0]);						pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "BUOXY: %d", netbuf.switches.BUOXYON);					pString96(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	strcpy(label[1], "RTN");	strcpy(label[2], "");	strcpy(label[3], "");	strcpy(label[4], "");	strcpy(label[5], "");
	strcpy(label[6], "");	strcpy(label[7], "");	strcpy(label[8], "");	strcpy(label[9], "");	strcpy(label[10], "");
	DrawLabels(n);
}

int accept_press = 0;  // global variable to count ACPT button presses in efrcs1-3

void efrcs_pointer(int n)
{
	glColor4fv(gWHITE);
	glLineWidth(1.0f);
	glDisable(GL_TEXTURE_2D);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	doline(mfdx[n] + 35, mfdy[n] + 363 + accept_press, mfdx[n] + 45, mfdy[n] + 360 + accept_press);	// Top edge
	doline(mfdx[n] + 45, mfdy[n] + 360 + accept_press, mfdx[n] + 35, mfdy[n] + 357 + accept_press);	// Bottom edge
	doline(mfdx[n] + 35, mfdy[n] + 357 + accept_press, mfdx[n] + 35, mfdy[n] + 363 + accept_press);	// Left side
	glEnable(GL_TEXTURE_2D);
}

void MFDefrcs(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_0) { 
			switch (n) {
				case LEFT:
					mode[n] = mfdHSI; break;
				case CENTRE:
					mode[n] = mfdADI; break;
				case RIGHT:
					mode[n] = mfdMAP; break;
			}
		}
		if (MFDKey == SDLK_1) { mode[n] = mfdINISTAT; }
		if (MFDKey == SDLK_5) { mode[n] = mfdHSI; }
		if (MFDKey == SDLK_6) { mode[n] = mfdADI; }
		if (MFDKey == SDLK_7) { mode[n] = mfdMAP; }
		if (MFDKey == SDLK_8) { if (accept_press > -210) { accept_press = accept_press - 15; } }	// Move the pointer down if ACPT is pressed
		if (MFDKey == SDLK_9) { // Switch to START Chekclist
			accept_press = 0; 
			if (n == LEFT) {
				if (mode[CENTRE] == mfdEFRCS) { mode[CENTRE] = mfdEFRCS2; }
				if (mode[RIGHT] == mfdEFRCS) { mode[RIGHT] = mfdEFRCS2; }
			}
			if (n == CENTRE) {
				if (mode[LEFT] == mfdEFRCS) { mode[LEFT] = mfdEFRCS2; }
				if (mode[RIGHT] == mfdEFRCS) { mode[RIGHT] = mfdEFRCS2; }
			}
			if (n == RIGHT) {
				if (mode[LEFT] == mfdEFRCS) { mode[LEFT] = mfdEFRCS2; }
				if (mode[CENTRE] == mfdEFRCS) { mode[CENTRE] = mfdEFRCS2; }
			}
			mode[n] = mfdEFRCS2;
		}		
		Input = -1;
		MFDKey = NULL;
	}
	BlankMFD(n);
	strcpy(label[1], "RTN");	strcpy(label[2], "");	strcpy(label[3], "");	strcpy(label[4], "");	strcpy(label[5], "");
	strcpy(label[6], "HSI");	strcpy(label[7], "ADI");	strcpy(label[8], "MAP");	strcpy(label[9], "ACPT");	strcpy(label[10], "NEXT");
	DrawLabels(n);

	glColor4fv(gWHITE);
	int y = 385;				// Highest point we want to display text.
	int x = 50;
	sprintf(sbuf, "BEFORE START CHECKS");					PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 30;
	(netbuf.switches.battery[0] == 1) && (netbuf.switches.battery[1] == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);			// these clauses show GREEN text when the condition is met, AMBER when not
	sprintf(sbuf, "BOTH BATTERIES  -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.engine.throttle[0] < 0.1f) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "THROTTLE        -             IDLE");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.fuel.fadec[0] == 1) ? glColor4fv(gGREEN) : (netbuf.fuel.selected == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);   // additional clause to stay green if user has reached FADEC step
	sprintf(sbuf, "LP FUEL COCK    -           CLOSED");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.fuelpump[0] == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "FUEL PUMP       -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.engine.mixture[0] == 0.0f) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "ENG START SW    -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.generator[0] == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "MAIN GEN        -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	//if (netbuf.switches.APUgen == 0) { glColor4fv(gGREEN); }
	//else { glColor4fv(gAMBER); }
	glColor4fv(gGREEN);
	sprintf(sbuf, "APU GEN         -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15; // Need to implement
	(netbuf.switches.pitot == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "PROBE HEATERS   -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.yawdamper == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "YAW DAMPER      -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.flight.antiSkid == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "ANTI SKID SW    -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.BUOXYON == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "BACK UP OXYGEN  -             NORM");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.fuel.fadec[0] == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "FADEC           -              EEC");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15; //EEC - Electronic Engine Controller | MFC - Manual Fuel Control
	(netbuf.elec.igniter[0] == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "IGNITION        -          ISOLATE");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.fuel.selected == 4) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "LP FUEL COCK    - OPEN AND GUARDED");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	//if (netbuf.switches.APUgen == 1) { glColor4fv(gGREEN); }
	//else { glColor4fv(gAMBER); }
	glColor4fv(gGREEN);
	sprintf(sbuf, "APU GEN         -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15; // Need to implement
	efrcs_pointer(n);
}

void MFDefrcs2(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_0) { 
			accept_press = 0; 
			if (n == LEFT) {
				if (mode[CENTRE] == mfdEFRCS2) { mode[CENTRE] = mfdEFRCS; }
				if (mode[RIGHT] == mfdEFRCS2) { mode[RIGHT] = mfdEFRCS; }
			}
			if (n == CENTRE) {
				if (mode[LEFT] == mfdEFRCS2) { mode[LEFT] = mfdEFRCS; }
				if (mode[RIGHT] == mfdEFRCS2) { mode[RIGHT] = mfdEFRCS; }
			}
			if (n == RIGHT) {
				if (mode[LEFT] == mfdEFRCS2) { mode[LEFT] = mfdEFRCS; }
				if (mode[CENTRE] == mfdEFRCS2) { mode[CENTRE] = mfdEFRCS; }
			}
			mode[n] = mfdEFRCS;
		}			
		if (MFDKey == SDLK_5) { mode[n] = mfdHSI; }
		if (MFDKey == SDLK_6) { mode[n] = mfdADI; }
		if (MFDKey == SDLK_7) { mode[n] = mfdMAP; }
		if (MFDKey == SDLK_8) { if (accept_press > -195) { accept_press = accept_press - 15; } }	// Move the pointer down if ACPT is pressed
		if (MFDKey == SDLK_9) { // Key 9 brings us to Page 3
			accept_press = 0;
			if (n == LEFT) {
				if (mode[CENTRE] == mfdEFRCS2) { mode[CENTRE] = mfdEFRCS3; }
				if (mode[RIGHT] == mfdEFRCS2) { mode[RIGHT] = mfdEFRCS3; }
			}
			if (n == CENTRE) {
				if (mode[LEFT] == mfdEFRCS2) { mode[LEFT] = mfdEFRCS3; }
				if (mode[RIGHT] == mfdEFRCS2) { mode[RIGHT] = mfdEFRCS3; }
			}
			if (n == RIGHT) {
				if (mode[LEFT] == mfdEFRCS2) { mode[LEFT] = mfdEFRCS3; }
				if (mode[CENTRE] == mfdEFRCS2) { mode[CENTRE] = mfdEFRCS3; }
			}
			mode[n] = mfdEFRCS3;
		}
		Input = -1;
		MFDKey = NULL;
	}
	BlankMFD(n);
	strcpy(label[1], "RTN");	strcpy(label[2], "");	strcpy(label[3], "");	strcpy(label[4], "");	strcpy(label[5], "");
	strcpy(label[6], "HSI");	strcpy(label[7], "ADI");	strcpy(label[8], "MAP");	strcpy(label[9], "ACPT");	strcpy(label[10], "NEXT");
	DrawLabels(n);

	glColor4fv(gWHITE);
	int y = 385;				// Highest point we want to display text.
	int x = 50;
	sprintf(sbuf, "ENGINE START");								PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 30;
	glColor4fv(gGREEN);
	netbuf.engine.mixture[0] == 1.0f ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "ENG START SW    -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.fuelpump[0] == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "FUEL PUMP       -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.generator[0] == 1) ? glColor4fv(gGREEN) : (netbuf.elec.igniter[0] == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);  // additional clause to stay green if user has started the engine
	sprintf(sbuf, "IGNITION        -           NORMAL");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.engine.N1[0] > 5) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);  
	sprintf(sbuf, "ENG START SW    -     START FOR 3S");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	Button[1][10] == 0 ? glColor4fv(gGREEN) : glColor4fv(gAMBER);	// Used button press here instead of netbuf
	sprintf(sbuf, "ENG START SW    -         RESET ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.engine.N1[0] < 27) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "N1              -    STABLE AT <27");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.engine.ittdegc[0] < 600) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "EGT TEMP        -     STABLE <600C");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	((netbuf.engine.ff[0] * 60.0) < 10) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "FUEL FLOW       -  STABLE <10 kg/m");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.generator[0] == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "MAIN GEN        -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	//if (netbuf.switches.APUgen == 0) { glColor4fv(gGREEN); }
	//else { glColor4fv(gAMBER); }
	glColor4fv(gGREEN);
	sprintf(sbuf, "APU GEN         -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15; // Need to implement
	(netbuf.elec.igniter[0] == 0) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "IGNITION        -          ISOLATE");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.pitot == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "PROBE HEATERS   -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.switches.yawdamper == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "YAW DAMPER      -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.flight.antiSkid == 1) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);
	sprintf(sbuf, "ANTI SKID SW    -               ON");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 30;
	glColor4fv(gCYAN);
	sprintf(sbuf, "NO CAUTIONS OR WARNINGS");					PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	efrcs_pointer(n);
}

void MFDefrcs3(int n)
{
	BlankMFD(n);
	if (netbuf.switches.battery[0] == 0) return;
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_0) {
			accept_press = 0;
			if (n == LEFT) {
				if (mode[CENTRE] == mfdEFRCS3) { mode[CENTRE] = mfdEFRCS2; }
				if (mode[RIGHT] == mfdEFRCS3) { mode[RIGHT] = mfdEFRCS2; }
			}
			if (n == CENTRE) {
				if (mode[LEFT] == mfdEFRCS3) { mode[LEFT] = mfdEFRCS2; }
				if (mode[RIGHT] == mfdEFRCS3) { mode[RIGHT] = mfdEFRCS2; }
			}
			if (n == RIGHT) {
				if (mode[LEFT] == mfdEFRCS3) { mode[LEFT] = mfdEFRCS2; }
				if (mode[CENTRE] == mfdEFRCS3) { mode[CENTRE] = mfdEFRCS2; }
			}
			mode[n] = mfdEFRCS2;
		}
		if (MFDKey == SDLK_5) { mode[n] = mfdHSI; }
		if (MFDKey == SDLK_6) { mode[n] = mfdADI; }
		if (MFDKey == SDLK_7) { mode[n] = mfdMAP; }
		if (MFDKey == SDLK_8) { if (accept_press > -270) { accept_press = accept_press - 15; } }	// Move the pointer down if ACPT is pressed
		Input = -1;
		MFDKey = NULL;
	}
	BlankMFD(n);
	strcpy(label[1], "RTN");	strcpy(label[2], "");	strcpy(label[3], "");	strcpy(label[4], "");	strcpy(label[5], "");
	strcpy(label[6], "HSI");	strcpy(label[7], "ADI");	strcpy(label[8], "MAP");	strcpy(label[9], "ACPT");	strcpy(label[10], "");
	DrawLabels(n);

	glColor4fv(gWHITE);
	int y = 385;				// Highest point we want to display text.
	int x = 50;
	sprintf(sbuf, "BEFORE TAKE-OFF CHECKS");					PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 30;
	glColor4fv(gGREEN);
	sprintf(sbuf, "TRIMS");										PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	(netbuf.airframe.flap_ratio == 0.5) ? glColor4fv(gGREEN) : glColor4fv(gAMBER);		// If flaps aren't 20, display AMBER
	sprintf(sbuf, "FLAPS           -           MIDDLE");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	glColor4fv(gGREEN);
	sprintf(sbuf, "FUEL");										PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "INSTRUMENTS     -   HORIZONS ERECT");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "                -    PRESSURES SET");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "                -   AP AS REQUIRED");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "VISOR/MASK");								PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "CANOPY");									PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "PINS");										PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "COMMAND EJECT");								PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "EMERG BRIEF");								PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 45;

	glColor4fv(gWHITE);
	sprintf(sbuf, "RUNWAY CHECKS");								PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 30;
	glColor4fv(gGREEN);
	sprintf(sbuf, "MASS LIVE");									PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "LIGHTS AS REQUIRED");						PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "NWS ADVISORY ON");							PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	sprintf(sbuf, "NO CAUTIONS OR WARNINGS");					PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
	efrcs_pointer(n);
}

int engine_start_check = 1;

void MFDinistat(int n)
{
	BlankMFD(n);
	engine_start_check = 1;
	if (netbuf.switches.battery[0] == 0) return;
	int count = 10;
	glColor4fv(gAMBER);
	int y = 385;				// Highest point we want to display text.
	int x = 50;
	if (count != 0)
	{
		sprintf(sbuf, "SWITCHES INCORRECTLY CONFIGURED"); PrintMed(mfdx[n] + 110, mfdy[n] + 400, 1);
		if (netbuf.switches.battery[1] == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);			// these clauses show nothing when the condition is met, AMBER when not
		sprintf(sbuf, "BATTERY 2       -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.fuel.selected == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "LP FUEL COCK    -           CLOSED");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.switches.fuelpump[0] == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "FUEL PUMP       -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.engine.mixture[0] == 0.0f) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "ENG START SW    -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.switches.generator[0] == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "MAIN GEN        -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		//if (netbuf.switches.APUgen == 0) { glColor4fv(gGREEN); }
		//else { glColor4fv(gAMBER); }
		glColor4fv(gBLACK);
		sprintf(sbuf, "APU GEN         -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15; // Need to implement
		if (netbuf.switches.pitot == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "PROBE HEATERS   -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.switches.yawdamper == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "YAW DAMPER      -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.flight.antiSkid == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "ANTI SKID SW    -              OFF");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		if (netbuf.switches.BUOXYON == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "BACK UP OXYGEN  -             NORM");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		//if (netbuf.fuel.fadec[0] == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		//sprintf(sbuf, "FADEC           -              MFC");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15; //EEC - Electronic Engine Controller | MFC - Manual Fuel Control
		if (netbuf.elec.igniter[0] == 0) { glColor4fv(gBLACK); count--; } else glColor4fv(gAMBER);
		sprintf(sbuf, "IGNITION        -          ISOLATE");			PrintMed(mfdx[n] + x, mfdy[n] + y, 1);			y -= 15;
		glColor4fv(gWHITE);
		sprintf(sbuf, "%d", count);			PrintMed(mfdx[n] + 10, mfdy[n] + 10, 1);
	}
	if (count == 0)
	{
		engine_start_check = 0;
		mode[LEFT] = mfdHSI;		// Modes for each screen after turning switchgear off
		mode[CENTRE] = mfdADI;
		mode[RIGHT] = mfdMAP;
	}
}

void MFDdebug(int n)
{
	BlankMFD(n);
	//if (netbuf.switches.battery[0] == 0) return;
	if ((Input == n) && (MFDKey != NULL))
	{
		if (MFDKey == SDLK_0) { mode[n] = mfdHSI; }			// Key 0 returns us to HSI
		Input = -1;
		MFDKey = NULL;
	}
	BlankMFD(n);
	strcpy(label[1], "RTN");	strcpy(label[2], "");	strcpy(label[3], "");	strcpy(label[4], "");	strcpy(label[5], "");
	strcpy(label[6], " ");	strcpy(label[7], "");	strcpy(label[8], "");	strcpy(label[9], "");	strcpy(label[10], "");
	DrawLabels(n);
	glColor4fv(gAMBER);
	int y = 415;				// Highest point we want to display text.
	int x = 50;
	sprintf(sbuf, "DEBUG I/O INFO:   Version %s", VERSION);										pString96(mfdx[n]+x, mfdy[n]+y, 1);			y -= 30;

	glColor4fv(gGREEN);
	sprintf(sbuf, "Bodnar1       Bodnar2       Bodnar3");		pString96(mfdx[n] + x, mfdy[n] + y, 1);		y -= 15;

	//if (BODNAR1)
	//{
	//	glColor4fv(gGREEN);
	//	for (int i = 0; i < 32; i++)
	//	{
	//		int b = SDL_JoystickGetButton(Joy[bodnar[1]], i);
	//		sprintf(sbuf, "BTN%d %d", i, b);
	//		pString96(mfdx[n] + x, mfdy[n] + y, 1);
	//		y -= 15;
	//	}
	//}

	glColor4fv(gWHITE);
	for (int i = 0; i < 32; i++)
	{
			sprintf(sbuf, "Btn%02d %d", i, Button[1][i]);		PrintSmall(mfdx[n] + x, mfdy[n] + y, 1);		
			sprintf(sbuf, "Btn%02d %d", i, Button[2][i]);		PrintSmall(mfdx[n] + x + 100, mfdy[n] + y, 1);
			sprintf(sbuf, "Btn%02d %d", i, Button[3][i]);		PrintSmall(mfdx[n] + x + 200, mfdy[n] + y, 1);
			y -= 11;
	}


	if (Key != SDL_SCANCODE_UNKNOWN)
	{
		sprintf(sbuf, "%c", Key); glColor4fv(gWHITE);  pString96(mfdx[n] + 10, mfdy[n] + 10, 1);
	}
}


// Update MFD[n] depending on what mode it's in.
void UpdateMFD(int n)
{
	if (mode[n] == mfdOFF)	{ BlankMFD(n);  return; }		// MFD is off, do nothing and return (should blank the MFD screen perhaps?)
	if (mode[n] == mfdINIT)	{ MFDInitialise(n); }			// Oooh.. do the Master Initialisation routine.
	if (mode[n] == mfdHSI)	{ MFDhsi(n); }					// Dislay the HSI
	if (mode[n] == mfdADI)	{ MFDadi(n); }					// Dislay the ADI
	if (mode[n] == mfdMAP)	{ MFDmap(n); }					// Dislay the MAP
	if (mode[n] == mfdDEBUG) { MFDdebug(n); }				// Show debug info.
	if (mode[n] == mfdELEC) { MFDelec(n); }					// Show electrical systems info.
	if (mode[n] == mfdDUMMY) { MFDdummy(n); }				// Show dummy texture
	if (mode[n] == mfdEFRCS) { MFDefrcs(n); }				// Show checklist
	if (mode[n] == mfdEFRCS2) { MFDefrcs2(n); }				// Show checklist page 2
	if (mode[n] == mfdEFRCS3) { MFDefrcs3(n); }				// Show checklist page 3
	if (mode[n] == mfdINISTAT) { MFDinistat(n); }			// initial status of switchgear
}





// Callback for device attached.
int CCONV AttachHandler(CPhidgetHandle IFK, void *userptr)
{
	int serialNo;
	const char *name;
	CPhidget_getDeviceName(IFK, &name);
	CPhidget_getSerialNumber(IFK, &serialNo);
	if (serialNo == Kit1ID) { Kit1 = TRUE; }
	if (serialNo == Kit2ID) { Kit2 = TRUE; }
	//	if (serialNo == Kit2ID) { Kit2 = TRUE; CPhidgetInterfaceKit_set_OnInputChange_Handler(ifKit2, InputChangeHandler, NULL); }
	if (serialNo == Kit3ID) { Kit3 = TRUE; }
	if (serialNo == Kit4ID) { Kit4 = TRUE; }
	sprintf(sbuf, "%s %6d attached.\n", name, serialNo);
	log(sbuf);
	return 0;
}
// Callback for device detached.
int CCONV DetachHandler(CPhidgetHandle IFK, void *userptr)
{
	int serialNo;
	const char *name;
	CPhidget_getDeviceName(IFK, &name);
	CPhidget_getSerialNumber(IFK, &serialNo);
	if (serialNo == Kit1ID) { Kit1 = FALSE; }
	if (serialNo == Kit2ID) { Kit2 = FALSE; }
	if (serialNo == Kit3ID) { Kit3 = FALSE; }
	if (serialNo == Kit4ID) { Kit4 = FALSE; }
	sprintf(sbuf, "%s %6d detached.\n", name, serialNo);
	log(sbuf);
	return 0;
}

int CCONV ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown)
{
	int serialNo;
	CPhidget_getSerialNumber(IFK, &serialNo);
	if (serialNo == Kit1ID) { Kit1 = FALSE; }
	if (serialNo == Kit2ID) { Kit2 = FALSE; }
	if (serialNo == Kit3ID) { Kit3 = FALSE; }
	if (serialNo == Kit4ID) { Kit4 = FALSE; }
	sprintf(sbuf, "Phidget Error handled. %d - %s\n", ErrorCode, unknown);
	log(sbuf);
	return 0;
}



void Phidget_Init()
{
	CPhidgetInterfaceKit_create(&ifKit1);											// Create an instance of a phidget.
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)ifKit1, AttachHandler, NULL);		// Create an attach handler.
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)ifKit1, DetachHandler, NULL);		// Create a detach handler.
	CPhidget_set_OnError_Handler((CPhidgetHandle)ifKit1, ErrorHandler, NULL);		// Create an error handler.
	CPhidget_open((CPhidgetHandle)ifKit1, Kit1ID);									// This *requests* a connection, need to wait for it to attach though.
	CPhidget_waitForAttachment((CPhidgetHandle)ifKit1, PhidgetDelay);				// Allow time for the device to attach.
	CPhidgetInterfaceKit_create(&ifKit2);											// Create an instance of a phidget.
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)ifKit2, AttachHandler, NULL);		// Create an attach handler.
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)ifKit2, DetachHandler, NULL);		// Create a detach handler.
	CPhidget_set_OnError_Handler((CPhidgetHandle)ifKit2, ErrorHandler, NULL);		// Create an error handler.
	CPhidget_open((CPhidgetHandle)ifKit2, Kit2ID);									// This *requests* a connection, need to wait for it to attach though.
	CPhidget_waitForAttachment((CPhidgetHandle)ifKit2, PhidgetDelay);				// Allow time for the device to attach.
	CPhidgetInterfaceKit_create(&ifKit3);											// Create an instance of a phidget.
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)ifKit3, AttachHandler, NULL);		// Create an attach handler.
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)ifKit3, DetachHandler, NULL);		// Create a detach handler.
	CPhidget_set_OnError_Handler((CPhidgetHandle)ifKit3, ErrorHandler, NULL);		// Create an error handler.
	CPhidget_open((CPhidgetHandle)ifKit3, Kit3ID);									// This *requests* a connection, need to wait for it to attach though.
	CPhidget_waitForAttachment((CPhidgetHandle)ifKit3, PhidgetDelay);				// Allow time for the device to attach.
}


// Update Phidgets
void UpdatePhidgets(void)
{
	// Warning Panel Test Button
		if ((Button[2][30] == 1) && ((netbuf.switches.battery[0]) || (netbuf.switches.battery[1])))
	{
		testDone = 1;
		if (Kit1)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, i, 1); }
		}
		if (Kit2)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit2, i, 1); }
		}
		if (Kit3)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit3, i, 1); }
		}
	}
	else if ((Button[2][30] == 0) && (testDone == 1))
	{
		testDone = 0;
		if (Kit1)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, i, 0); }
		}
		if (Kit2)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit2, i, 0); }
		}
		if (Kit3)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit3, i, 0); }
		}
	}
	// Phidget Main
	else if ((netbuf.switches.battery[0]) || (netbuf.switches.battery[1]))
	{
		if (Kit1)
		{
			if (netbuf.ann.enginefire[0])		{ CPhidgetInterfaceKit_setOutputState(ifKit1, 0, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 0, 0); }		// 0. FIRE
			if (netbuf.elec.APUrunning)			{ CPhidgetInterfaceKit_setOutputState(ifKit1, 1, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 1, 0); }		// 1. APU
			if (netbuf.ann.hydraulic_pressure)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, 2, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 2, 0); }		// 2. HYD
			if ((netbuf.switches.gearhandle == 0) && (netbuf.flight.altitude < 5000.0f) && (netbuf.flight.Vind < 160.0f))
			{
				CPhidgetInterfaceKit_setOutputState(ifKit1, 7, 1);																										// 7. GEAR
			}
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 7, 0); }
			if (netbuf.ann.generator[0])		{ CPhidgetInterfaceKit_setOutputState(ifKit1, 8, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 8, 0); }		// 8. GEN
			if ((netbuf.flight.depres_slow) || (netbuf.flight.depres_fast) || (netbuf.flight.cabin_alt_act > (netbuf.flight.cabin_alt_set + 50.0f)))
			{
				{ CPhidgetInterfaceKit_setOutputState(ifKit1, 9, 1); }																									// 9. CPR
			}
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 9, 0); }
			if (netbuf.fuel.fuelcap)			{ CPhidgetInterfaceKit_setOutputState(ifKit1, 10, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 10, 0); }	// 10. LP OFF
			if (netbuf.ann.hydraulic_pressure)  { CPhidgetInterfaceKit_setOutputState(ifKit1, 12, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 12, 0); }	// 12. HYD1
			if (netbuf.ann.hydraulic_pressure)  { CPhidgetInterfaceKit_setOutputState(ifKit1, 13, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 13, 0); }	// 13. HYD2
			if (netbuf.airframe.nws == 0)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, 14, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit1, 14, 0); }		// 14. NWS-Yellow
		}
		if (Kit2)
		{
			if (netbuf.ann.generator[0])     { CPhidgetInterfaceKit_setOutputState(ifKit2, 0, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 0, 0); }			// 0. A-GEN
			if (netbuf.ann.oil_pressure[0])  { CPhidgetInterfaceKit_setOutputState(ifKit2, 2, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 2, 0); }			// 2. OIL pressure
			if (netbuf.fuel.fadec[0] == 0)	 { CPhidgetInterfaceKit_setOutputState(ifKit2, 4, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 4, 0); }			// 4. MFC
			if (netbuf.ann.fuel_pressure[0]) { CPhidgetInterfaceKit_setOutputState(ifKit2, 5, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 5, 0); }			// 5. FPR pressure
			if (netbuf.ann.fuel_quantity)	 { CPhidgetInterfaceKit_setOutputState(ifKit2, 6, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 6, 0); }			// 6. FUEL qty
			if (netbuf.switches.pitot == 0)	 { CPhidgetInterfaceKit_setOutputState(ifKit2, 8, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 8, 0); }			// 8. PITOT
			if (netbuf.switches.yawdamper == 0)	{ CPhidgetInterfaceKit_setOutputState(ifKit2, 9, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 9, 0); }		// 9. YAW
			if (netbuf.flight.antiSkid == 0) { CPhidgetInterfaceKit_setOutputState(ifKit2, 10, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 10, 0); }	// 10. Antiskid
			if (netbuf.ann.ap_disconnect)	 { CPhidgetInterfaceKit_setOutputState(ifKit2, 11, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 11, 0); }		// 11. AP
			if (netbuf.elec.stall)			 { CPhidgetInterfaceKit_setOutputState(ifKit2, 12, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit2, 12, 0); }		// 12. STALL
		}
		if (Kit3)
		{
			if ((netbuf.flight.cabin_alt_act > 26000.0f) || (netbuf.switches.BUOXYON == 1))
			{
				CPhidgetInterfaceKit_setOutputState(ifKit3, 0, 1);																											// 0. BUOXY
			}
			else { CPhidgetInterfaceKit_setOutputState(ifKit3, 0, 0); }
			if (netbuf.engine.N1[0] > 80.0f)			{ CPhidgetInterfaceKit_setOutputState(ifKit3, 1, 1); }
			else { CPhidgetInterfaceKit_setOutputState(ifKit3, 1, 0); }	// 1. RAM
			if ((netbuf.airframe.nws) && (netbuf.flight.onground_all))
			{
				CPhidgetInterfaceKit_setOutputState(ifKit3, 2, 1);																											// 2. NWS-Green
			}
			else { CPhidgetInterfaceKit_setOutputState(ifKit3, 2, 0); }
			if ((netbuf.flight.onground_all) && (netbuf.engine.N1[0] > 95.0f))
			{
				CPhidgetInterfaceKit_setOutputState(ifKit3, 3, 1);																											// 3. GO
			}
			else { CPhidgetInterfaceKit_setOutputState(ifKit3, 3, 0); }
		}
	}
	else
	{
		if (Kit1)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit1, i, 0); }
		}
		if (Kit2)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit2, i, 0); }
		}
		if (Kit3)
		{
			for (int i = 0; i < 16; i++)	{ CPhidgetInterfaceKit_setOutputState(ifKit3, i, 0); }
		}
	}
}

void KeyDownHandler(void)
{
	Key = NULL;
}


void UpdateBodnars(void)
{
	int i, b;
	for (i = 0; i < numjoys; i++)			// For each board.
	{
		if (bodnar[i] != 99)				// If the board is connected.
		{
			for (b = 0; b < 32; b++)		// For each button
			{
				Button[i][b] = SDL_JoystickGetButton(Joy[bodnar[i]], b);
			}
		}
	}
	// Test buttons here.
	if (Button[1][0] != OldButton[1][0])
	{
		if (Button[1][0] == 1) { SendCommand(pcFuelSelectBOTH, 0); log("LPFuelCock Open\n"); }		// LPFuelCock Open
		if (Button[1][0] == 0) { SendCommand(pcFuelSelectOFF, 0); log("LPFuelCock Closed\n"); }		// LPFuelCock Closed
	}
	if (Button[1][1] != OldButton[1][1])
	{
		if (Button[1][1] == 1) { SendCommand(pcIgniterON, 0); log("Ignition ON\n"); }				// Ignition ON
		if (Button[1][1] == 0) { SendCommand(pcIgniterOFF, 0); log("Ignition OFF\n"); }				// Ignition OFF
	}
	if (Button[1][2] != OldButton[1][2])
	{
		if (Button[1][2] == 1) { SendCommand(pcFADECON, 0); log("FADEC ON\n"); }					// FADEC ON
		if (Button[1][2] == 0) { SendCommand(pcFADECOFF, 0); log("FADEC OFF\n"); }					// FADEC OFF
	}
	if (Button[1][4] != OldButton[1][4])
	{
		if (Button[1][4] == 1) { SendCommand(pcBUOXYON, 0); log("BackUpOxy ON\n"); }				// BUOXY ON
		if (Button[1][4] == 0) { SendCommand(pcBUOXYOFF, 0); log("BackUpOxy OFF\n"); }				// BUOXY OFF
	}
	if (Button[1][6] != OldButton[1][6])
	{
		if (Button[1][6] == 1) { SendCommand(pcYawDamperON, 0); log("Yaw Damper ON\n"); }			// Yaw Damper ON
		if (Button[1][6] == 0) { SendCommand(pcYawDamperOFF, 0); log("Yaw Damper OFF\n"); }			// Yaw Damper OFF
	}
	if (Button[1][7] != OldButton[1][7])
	{
		if (Button[1][7] == 1) { SendCommand(pcPitotHeatON, 0); log("Pitot Heat ON\n"); }			// Pitot (Probe) Heat ON
		if (Button[1][7] == 0) { SendCommand(pcPitotHeatOFF, 0); log("Pitot Heat OFF\n"); }			// Pitot (Probe) Heat OFF
	}
	if (Button[1][8] != OldButton[1][8])
	{
		if (Button[1][8] == 1) { SendCommand(pcTrimDownLeft, 3); log("Rudder Trim LEFT\n"); }		// Rudder Trim Left
	}
	if (Button[1][9] != OldButton[1][9])
	{
		if (Button[1][9] == 1) { SendCommand(pcTrimUpRight, 3); log("Rudder Trim RIGHT\n"); }		// Rudder Trim Right
	}
	if (Button[1][10] != OldButton[1][10])
	{
		if (engine_start_check == 0) {
			if (Button[1][10] == 1) { SendCommand(pcEngStrOn, 0); log("Engine Start On\n"); }			// Engine Start On
		}
		if (Button[1][10] == 0) { SendCommand(pcEngStrOff, 0); log("Engine Start Off\n"); }			// Engine Start Off
	}
	if (Button[1][11] != OldButton[1][11])
	{
		if (Button[1][11] == 0) { SendCommand(pcEngOn, 0); log("Engine On\n"); }					// Engine On  **Note Reversed**
		if (Button[1][11] == 1) { SendCommand(pcEngOff, 0); log("Engine Off\n"); }					// Engine Off **Note Reversed**
	}
	if (Button[1][12] != OldButton[1][12])
	{
		if (Button[1][12] == 0) { SendCommand(pcAPUOn, 0); log("APU On\n"); }					// Engine On  **Note Reversed**
		if (Button[1][12] == 1) { SendCommand(pcAPUOff, 0); log("APU Off\n"); }
	}
	if (Button[1][13] != OldButton[1][13])
	{
		if (Button[1][13] == 1) { SendCommand(pcGeneratorON, 0); log("Generator ON\n"); }			// Generator ON
		if (Button[1][13] == 0) { SendCommand(pcGeneratorOFF, 0); log("Generator OFF\n"); }			// Generator OFF
	}
	if (Button[1][14] != OldButton[1][14])
	{
		if (Button[1][14] == 1) { SendCommand(pcBatteryON, 0); log("Battery 1 ON\n"); }				// Battery 1 ON
		if (Button[1][14] == 0) { SendCommand(pcBatteryOFF, 0); log("Battery 1 OFF\n"); }			// Battery 1 OFF
	}
	if (Button[1][15] != OldButton[1][15])
	{
		if (Button[1][15] == 1) { SendCommand(pcBatteryON, 1); log("Battery 2 ON\n"); }				// Battery 2 ON
		if (Button[1][15] == 0) { SendCommand(pcBatteryOFF, 1); log("Battery 2 OFF\n"); }			// Battery 2 OFF
	}
	if (Button[1][16] != OldButton[1][16])
	{
		if (Button[1][16] == 1) { SendCommand(pcFuelPumpON, 0); log("Fuel Pump ON\n"); }			// Fuel Pump ON
		if (Button[1][16] == 0) { SendCommand(pcFuelPumpOFF, 0); log("Fuel Pump OFF\n"); }			// fuel Pump OFF
	}
	if (Button[1][17] != OldButton[1][17])
	{
		if (Button[1][17] == 1) { SendCommand(pcAntiSkidOn, 0); log("Antiskid ON\n"); }				// Antiskid ON
		if (Button[1][17] == 0) { SendCommand(pcAntiSkidOff, 0); log("Antiskid OFF\n"); }			// Antiskid OFF
	}
	if (Button[1][21] != OldButton[1][21])
	{
		if (Button[1][21] == 1) { SendCommand(pcNVGToggle, 0); log("Night Vision Toggled\n"); }		// Night Vision Goggles Toggle
	}
	if (Button[2][5] != OldButton[2][5])
	{
		if (Button[2][5] == 1) { SendCommand(pcBeaconON, 0); log("Beacon Lights ON\n"); }
	}
	if (Button[2][9] != OldButton[2][9])
	{
		if (Button[2][9] == 1) { SendCommand(pcBeaconOFF, 0); log("Beacon Lights OFF\n"); }
	}

	if (Button[2][10] != OldButton[2][10])
	{
		if (Button[2][10] == 1) { SendCommand(pcNavLightOFF, 0); log("Navigation Lights OFF\n"); }
	}
	if (Button[2][6] != OldButton[2][6])
	{
		if (Button[2][6] == 1) { SendCommand(pcNavLightON, 0); log("Navigation Lights ON\n"); }
	}
	if (Button[2][7] != OldButton[2][7])
	{
		if (Button[2][7] == 1) { SendCommand(pcStrobeON, 0); log("Strobe Lights ON\n"); }			// Strobe Lights ON
		if (Button[2][7] == 0) { SendCommand(pcStrobeOFF, 0); log("Strobe Lights OFF\n"); }			// Strobe Lights OFF
	}
	if (Button[2][13] != OldButton[2][13])
	{
		if (Button[2][13] == 1) { SendCommand(pcLandingLightON, 0); log("Landing Lights ON\n"); }	// Landing Lights ON
		if (Button[2][13] == 0) { SendCommand(pcLandingLightOFF, 0); log("Landing Lights OFF\n"); }	// Landing Lights OFF
	}
	if (Button[2][18] != OldButton[2][18])
	{
		if (Button[2][18] == 1) { SendCommand(pcSeatUp, 0);  SendCommand(pcCamDefault, 0); log("Seat Raised\n"); }		// Seat Raised Up
	}
	if (Button[2][19] != OldButton[2][19])
	{
		if (Button[2][19] == 1) { SendCommand(pcSeatDown, 0); SendCommand(pcCamDefault, 0); log("Seat Lowered\n"); }	// Seat Lowered
	}
	
	
	// Finally, update the saved button state.
	for (i = 0; i < numjoys; i++)			// For each board.
	{
		for (b = 0; b < 32; b++)
		{
			OldButton[i][b] = Button[i][b];							// Save the state for later comparison.
		}
	}
}


// Handle state of Bodnar1 switches.
//void UpdateBodnar1(void)
//{
//	if (bodnar[1] == 99) { return; }			// No Bodnar to read!
//	int i;
//	// Read switch states into an array.
//	for (i = 0; i < 32; i++)			
//	{
//		Button[i] = SDL_JoystickGetButton(Joy[bodnar[1]], i);
//	}
//	// If they differ from the old state, perform necessary action.
//	if (Button[0] != Bodnar1OLD[0]) { }
//	// finally, update the OLD state array.
//	for (i = 0; i < 32; i++)
//	{
//		Bodnar1OLD[i] = Button[i];
//	}
//}



void MFDLeftKey(void)
{
	int Key = event.key.keysym.sym;
}



//
// Here we go, it all happens here...
//
int WINAPI WinMain(HINSTANCE	hInstance,						// Instance
	HINSTANCE	hPrevInstance,					// Previous Instance
	LPSTR		lpCmdLine,						// Command Line Parameters
	int			nCmdShow)						// Window Show State
{
	for (int i = 0; i < 32; i++)				// Set buttons to unkown state.
	{
		OldButton[1][i] = 0;
		OldButton[2][i] = 0;
		OldButton[3][i] = 0;
	}

	OpenLog();
	PrepSDL();
	InitGL();
	SDL_GL_SetSwapInterval(0);					// This disables vsync.  (1, enables it).
	fprintf(logfile, "OpenGL Initialised.\n");
	LoadPNGTexture("bitmaps/font3.png", txFONT);
	BuildFont();

	Phidget_Init();

	VSI_Init();
	ADI_Init();
	Heading_Init();			// Load bitmap(s) for the Standby Heading indicator.
	Altimeter_Init();		// Load bitmap(s) for the Standby Altimeter.
	Airspeed_Init();		// Load bitmap(s) for the Standby Airspeed Indicator.

	// Sort out the Networking.
	strcpy(REMOTEHOST, "127.0.0.1");
	GLint err = CreateInputSocket();	// This is where we get our data from.
	sprintf(sbuf, "Create Input Socket Returned: %d\n", err);			log(sbuf);
	if (err != 0) { exit(1); }
	strcpy(REMOTEHOST, "127.0.0.1");	// IP of X-Plane Master //172.16.2.1
	err = CreateCommandSocket();		// Open a socket TO x-plane.
	sprintf(sbuf, "Create Command Socket Returned: %d\n", err);			log(sbuf);
	if (err != 0) { exit(1); }

	float i = 0;
	SendCommand(pcCamDefault, 0);

	while (!quit)						// MAIN PROGRAM LOOP STARTS HERE
	{
		GetNetData();						// Update the network buffer.
		if (netbuf.flight.G > PeakG) { PeakG = netbuf.flight.G; }
		if (netbuf.flight.G < PeakNegG) { PeakNegG = netbuf.flight.G; }
		if (Button[2][14] == 1) {
			PeakG = 0;
			PeakNegG = 0;
		}
		/* Set the background black then Clear The Screen And The Depth Buffer */
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VSIangle = float(i);
		VSI_Draw();
		i+=0.5f;
		if (i > 359) { i = 0; }
		ADI_Draw();
		Compass_Draw();
		Altimeter_Draw();
		Airspeed_Draw();

		UpdatePhidgets();					// Update Phidgets.
		UpdateBodnars();					// Update Bodnar boards.

		glEnable(GL_LINE_SMOOTH);
		ShowVoidSpace();		// these just draw placeholders for each instrument.
		FireIndicator();
		APUIndicator();
		UpdateEMP();
		UpdateMFD(LEFT);		TickDraw(LEFT);
		UpdateMFD(CENTRE);		TickDraw(CENTRE);
		UpdateMFD(RIGHT);		TickDraw(RIGHT);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glDisable(GL_TEXTURE_2D);

		TimerCheck();					// Check on timed events.
		SDL_GL_SwapWindow(screen);		// Update the screen.
		SDL_Delay(10);					// Hang on a mo ....

		Key = SDL_SCANCODE_UNKNOWN;
		SDL_PollEvent(&event);			// Check for any events.
		switch (event.type)
		{
		case SDL_MOUSEMOTION:
				mousetrack();
				break;
		case SDL_TEXTINPUT:
			Key = event.key.keysym.scancode;
			KeyDownHandler();
			break;
		case SDL_KEYDOWN:							// Master key polling event.
			if (Input != -1) { MFDKey = event.key.keysym.sym; event.key.keysym.sym = NULL;  break; }
			switch (event.key.keysym.sym)
			{
			case SDLK_COMMA:		Input = LEFT; break;	// Next char is for mfd[LEFT]
			case SDLK_SEMICOLON:	Input = CENTRE; break;	// Next char is for mfd[CENTRE]
			case SDLK_SLASH:		Input = RIGHT; break;	// Next char is for mfd[RIGHT]
			case SDLK_ESCAPE:		quit = true; break;
			case SDLK_RETURN:		done = true; break;
			}
			break;
		}
	}								// MAIN PROGRAM LOOP ENDS HERE

}
