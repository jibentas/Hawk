
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL_opengl.h"

#define pi 3.14159265


char *Trim(char* string)
{
	size_t size;
	char *end;
	size = strlen(string);
	if (!size) return string;
	end = string + size -1;
	while (end >= string && isspace(*end)) end--;
	*(end + 1) = '\0';
	while(*string && isspace(*string)) string++;
	return string;
}

void uppercase( char *string )
{
	unsigned int i;
	for (i=0;i<=strlen(string);i++)
	{
		string[i]=toupper(string[i]);
	}
}

// **************************************************************************************
// Draw a block from the currently bound texture.
// **************************************************************************************
void DrawTex(GLfloat x, GLfloat y, GLint w, GLfloat h, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	glBegin(GL_QUADS);
		glTexCoord2f(x2,y1);	glVertex2f(x+w,y);		// Bottom Right
		glTexCoord2f(x1,y1);	glVertex2f(x,y);		// Bottom Left
		glTexCoord2f(x1,y2);	glVertex2f(x,y+h);		// Top Left
		glTexCoord2f(x2,y2);	glVertex2f(x+w, y+h);	// Top Right
	glEnd();
	//glBegin(GL_QUADS);
	//	glTexCoord2f(x2,y1);	glVertex2f(x,y+h);		// Top Left
	//	glTexCoord2f(x1,y1);	glVertex2f(x+w,y+h);	// Top Right
	//	glTexCoord2f(x1,y2);	glVertex2f(x+w,y);		// Bottom Right
	//	glTexCoord2f(x2,y2);	glVertex2f(x, y);	// Bottom Left
	//glEnd();
}


void DrawTexi(GLint x, GLint y, GLint w, GLint h, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
//	x+=float(XO);		// Add the window origin offset.
//	y+=float(YO);
	glBegin(GL_QUADS);
		glTexCoord2d(x2,y1);	glVertex2d(x+w,y);		// Bottom Right
		glTexCoord2d(x1,y1);	glVertex2d(x,y);		// Bottom Left
		glTexCoord2d(x1,y2);	glVertex2d(x,y+h);		// Top Left
		glTexCoord2d(x2,y2);	glVertex2d(x+w, y+h);	// Top Right
	glEnd();
}


GLint Round(double x) 
{
	return (GLint) floor(x+0.5f);
}


double RadToDeg(double f)
{
	f=f * (180.0f / pi);
	return f;
}

double DegToRad(double f)
{
	f=(pi/180.0f) * f;
	return f;
}

double RSCtoRPM(double r)
{
	r=RadToDeg(r);
	r=r*60.0f;
	r=r/360.0f;
	return r;
}

bool FileExists(const char * filename)
{
	if (FILE * file = fopen(filename,"r"))
	{
		fclose(file);
		return TRUE;
	}
	return FALSE;
}


// Low Precision Sine Function.
double losin(double x)
{
double sin;
	if (x < -3.14159265)
	    x += 6.28318531;
	else
	if (x >  3.14159265)
	    x -= 6.28318531;
	//compute sine
	if (x < 0)
	    sin = 1.27323954 * x + .405284735 * x * x;
	else
	    sin = 1.27323954 * x - 0.405284735 * x * x;
	return sin;
}


double hisin(double x)
{
	double sin;
	if (x < -3.14159265)
	    x += 6.28318531;
	else
	if (x >  3.14159265)
	    x -= 6.28318531;
	//compute sine
	if (x < 0)
	{
	    sin = 1.27323954 * x + .405284735 * x * x;
	    if (sin < 0)
	        sin = .225 * (sin *-sin - sin) + sin;
	    else
	        sin = .225 * (sin * sin - sin) + sin;
	}
	else
	{
	    sin = 1.27323954 * x - 0.405284735 * x * x;
	    if (sin < 0)
	        sin = .225 * (sin *-sin - sin) + sin;
	    else
		    sin = .225 * (sin * sin - sin) + sin;
	}
	return sin;
}


double locos(double x)
{
	double cos;
	//compute cosine: sin(x + PI/2) = cos(x)
	x += 1.57079632;
	if (x >  3.14159265)
	    x -= 6.28318531;
	if (x < 0)
	    cos = 1.27323954 * x + 0.405284735 * x * x;
	else
	    cos = 1.27323954 * x - 0.405284735 * x * x;
	return cos;
}


double hicos(double x)
{
	double cos;
	x += 1.57079632;
	if (x >  3.14159265)
		x -= 6.28318531;
	if (x < 0)
	{
	    cos = 1.27323954 * x + 0.405284735 * x * x;
	    if (cos < 0)
			cos = .225 * (cos *-cos - cos) + cos;
	    else
			cos = .225 * (cos * cos - cos) + cos;
	}
	else
	{
	    cos = 1.27323954 * x - 0.405284735 * x * x;
	    if (cos < 0)
			cos = .225 * (cos *-cos - cos) + cos;
	    else
			cos = .225 * (cos * cos - cos) + cos;
	}
	return cos;
}


// max |error| > 0.01
float fastatan2( float y, float x )
{
	const float ONEQTR_PI = float(pi / 4.0);
	const float THRQTR_PI = float(3.0 * pi / 4.0);
	float r, angle;
	float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if ( x < 0.0f )
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if ( y < 0.0f )
		return( -angle );     // negate if in quad III or IV
	else
		return( angle );
}



void Circle(int x, int y, int r)
{
	int y1=y+r;
	int x1=x;
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glBegin(GL_LINE_STRIP);
	for (float angle=0.0f; angle<=(2.01f*pi); angle+=0.02f)
	{
		int x2 = int(x + r * hisin(angle));
		int y2 = int(y + r * hicos(angle));
		glVertex2d(x1,y1);
		y1=y2;
		x1=x2;
	}
	glEnd();
}


void HalfCircle(int x, int y, int r)
{
	int y1=y+r;
	int x1=x;
	x1=0; y1=0;
	glBegin(GL_LINE_STRIP);
	for (float angle=-0.5f*float(pi); angle<=(0.5f*float(pi)); angle+=0.02f)
	{
		int x2 = int(x + r * hisin(angle));
		int y2 = int(y + r * hicos(angle));
		if (x1!=0 && y1!=0) glVertex2d(x1,y1);
		y1=y2;
		x1=x2;
	}
	glEnd();
}



// Checks format and numeric values are correct for an IP address. Returns True/False.
int is_valid_ip(const char *ip_str)
{
	unsigned int n1,n2,n3,n4;

	if(sscanf(ip_str,"%u.%u.%u.%u", &n1, &n2, &n3, &n4) != 4) return 0;

	if((n1 != 0) && (n1 <= 255) && (n2 <= 255) && (n3 <= 255) && (n4 <= 255)) {
		char buf[64];
		sprintf(buf,"%u.%u.%u.%u",n1,n2,n3,n4);
		if(strcmp(buf,ip_str)) return 0;
		return 1;
	}
	return 0;
}


// Draw a basic quad using the current color.
void Block(GLint x, GLint y, GLint w, GLint h)
{
	glDisable(GL_TEXTURE_2D);			// Draw the main Box Body
	glBegin(GL_QUADS); glVertex2d(x, y); glVertex2d(x + w, y); glVertex2d(x + w, y + h);	glVertex2d(x, y + h); glEnd();	// Body
	glEnable(GL_TEXTURE_2D);
}

// Draw a basic box using the current color.
void Box(GLint x, GLint y, GLint w, GLint h)
{
	glDisable(GL_TEXTURE_2D);			// Draw the main Box Body
	glBegin(GL_LINE_STRIP); glVertex2d(x, y); glVertex2d(x + w, y); glVertex2d(x + w, y + h);	glVertex2d(x, y + h); glVertex2d(x, y); glEnd();	// Body
	glEnable(GL_TEXTURE_2D);
}

void doline(float x1, float y1, float x2, float y2)
{
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}


void dolinei(int x1, int y1, int x2, int y2)
{
	glBegin(GL_LINES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glEnd();
}
