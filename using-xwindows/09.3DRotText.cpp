#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <memory.h> 
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h> 
#include <X11/XKBlib.h>
#include <X11/keysym.h> 

#include <GL/gl.h>
#include <GL/glx.h>
#include "SOIL.h"

#define TRUE 1
#define FALSE 0
#include <GL/glu.h>
//namespaces
using namespace std;

//global variable declarations
bool bFullscreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

GLfloat angleTri=0.0f;
GLfloat angleSquare=0.0f;

GLXContext gGLXContext;

void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);
void spin(void);
GLuint	Texture_Kundali; //texture object for Kundali texture
GLuint	Texture_Stone; //texture object for Stone texture
//entry-point function
int main(void)
{
	//function prototypes
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int,int);
	void uninitialize();
	
	//variable declarations
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	
	bool bDone=false;
	
	//code
	CreateWindow();

	//initialize
	initialize();
	
	//Message Loop
	XEvent event;
	KeySym keysym;
	
	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);
			switch(event.type)
			{
				case MapNotify:
					break;
				case KeyPress:
					keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym)
					{
						case XK_Escape:
							uninitialize();
							exit(0);
						case XK_F:
						case XK_f:
							if(bFullscreen==false)
							{
								ToggleFullscreen();
								bFullscreen=true;
							}
							else
							{
								ToggleFullscreen();
								bFullscreen=false;
							}
							break;
						default:
							break;
					}
					break;
				case ButtonPress: 
					switch(event.xbutton.button)
					{
						case 1: 
						    break;
						case 2: 
						    break;
						case 3: 
						    break;
						default:
						    break;
			}	
					break;
				case MotionNotify: 
					break;
				case ConfigureNotify: 
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
				case Expose: 
					break;
				case DestroyNotify:
					break;
				case 33:
					bDone=true;
					break;
				default:
					break;
			}
		
		}

		display();
		spin();
	}
	
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	//function prorttypes
	void uninitialize(void);

	//variable declarations
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_ALPHA_SIZE, 1,
		None 
	}; 
	
	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR : Unable To Open X Display.\nExitting Now...\n");
		uninitialize();
		exit(1);
	}
	
	defaultScreen=XDefaultScreen(gpDisplay);
	
	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);
		
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay, 
					    RootWindow(gpDisplay, gpXVisualInfo->screen), 
					    gpXVisualInfo->visual,
 					    AllocNone);
	gColormap=winAttribs.colormap;

	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask= ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask |
			       StructureNotifyMask;
	
	styleMask=CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
	
	gWindow=XCreateWindow(gpDisplay,
			      RootWindow(gpDisplay,gpXVisualInfo->screen),
			      0,
			      0,
			      giWindowWidth,
			      giWindowHeight,
			      0,
			      gpXVisualInfo->depth,
			      InputOutput,
			      gpXVisualInfo->visual,
			      styleMask,
			      &winAttribs);
	if(!gWindow)
	{
		printf("ERROR : Failed To Create Main Window.\nExitting Now...\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"One 2D Shape Perspective");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
	//variable declarations
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
	//code
	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));
	
	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;
	
	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1]=fullscreen;
	
	XSendEvent(gpDisplay,
	           RootWindow(gpDisplay,gpXVisualInfo->screen),
	           False,
	           StructureNotifyMask,
	           &xev);
}

void initialize(void)
{
	static  char stone_path[]="/home/jeevan/opengl_progs/xwindows_opengl_by_me/opengl/opengl/using-xwindows/Stone.bmp";
	static char kundali_path[]="/home/jeevan/opengl_progs/xwindows_opengl_by_me/opengl/opengl/using-xwindows/Vijay_Kundali.bmp";
	//function prototype
	void resize(int, int);
	int LoadGLTextures(GLuint *, char *textureImage,int height,int width);
	
	//code
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	glClearColor(1.0f,0.0f,0.0f,0.0f);
	LoadGLTextures(&Texture_Kundali, stone_path,256,256);
	LoadGLTextures(&Texture_Stone,kundali_path ,256,256);
	resize(giWindowWidth,giWindowHeight);

}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT);
    
    // ###### PYRAMID ######

	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -6.0f);
	glRotatef(angleTri, 0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, Texture_Stone);
	glBegin(GL_TRIANGLES);
	//NOTE : EACH FACE OF A PYRAMID (EXCEPT THE BASE/BOTTOM) IS A TRIANGLE

	//NOTE : EACH FACE OF A PYRAMID (EXCEPT THE BASE/BOTTOM) IS A TRIANGLE

	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom tip of triangle

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom tip of triangle

	//****RIGHT FACE****
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f); //left-bottom tip of triangle

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom tip of triangle

	//****BACK FACE****
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); //left-bottom tip of triangle

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f); //right-bottom tip of triangle

	//****LEFT FACE****
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom tip of triangle

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f); //right-bottom tip of triangle
	glEnd();

	// ###### CUBE ######

	glLoadIdentity();
	glTranslatef(1.5f, 0.0f, -6.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(angleSquare, 1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, Texture_Kundali);
	glBegin(GL_QUADS);
	//NOTE : EACH FACE OF A CUBE IS A SQUARE

	//****TOP FACE****
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of top face

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of top face

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f); //left-bottom of top face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f); //right-bottom of top face

	//****BOTTOM FACE****
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f); //right-top of bottom face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-top of bottom face

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom of bottom face

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom of bottom face

	//****FRONT FACE****
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f); //right-top of front face

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f); //left-top of front face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom of front face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom of front face

	//****BACK FACE****
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); //right-top of back face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-top of back face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-bottom of back face

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f); //right-bottom of back face

	//****LEFT FACE****
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f); //right-top of left face

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of left face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom of left face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f); //right-bottom of left face

	//****RIGHT FACE****
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of right face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f); //left-top of right face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f); //left-bottom of right face

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom of right face

	glEnd();
}

void resize(int width,int height)
{
	//code
	if(height==0)
		height=1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	perspectiveGL(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
int LoadGLTextures(GLuint *texture, char* textureImage,int height,int width)
{
	//variable declarations
	//HBITMAP hBitmap;
	//BITMAP bmp;
	int iStatus = FALSE;
	unsigned char* image;
	//code
	glGenTextures(1, texture); //1 image
	*texture = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture 
	(
		textureImage,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	
	if (*texture) //if bitmap exists 
	{
		iStatus = TRUE;	
		glActiveTexture(GL_TEXTURE0);
    		glBindTexture(GL_TEXTURE_2D, *texture);
        	image = SOIL_load_image(textureImage, &width, &height, 0, SOIL_LOAD_RGB);
        	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        	SOIL_free_image_data(image);	
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 4); //pixel storage mode (word alignment/4 bytes)
		//glBindTexture(GL_TEXTURE_2D, *texture); //bind texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//generate mipmapped texture (3 bytes, width, height & data from bmp)
		gluBuild2DMipmaps(GL_TEXTURE_2D);

		//DeleteObject(hBitmap); //delete unwanted bitmap handle
	}
	return(iStatus);
}
void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
     const GLdouble pi = 3.1415926535897932384626433832795;
     GLdouble fW, fH;
 
     //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
 
     fH = tan(fovY / 360 * pi) * zNear;
 
     fW = fH * aspect;
 
 
 
     glFrustum(-fW, fW, -fH, fH, zNear, zFar);
 
}

void uninitialize(void)
{
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}
	
	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
	}
	
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}
	
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;	
	}

	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}

void spin(void)
{
	//code
	angleTri = angleTri + 1.5f ;
			
	if (angleTri >= 360.0f)
		angleTri = angleTri - 360.0f;

	angleSquare = angleSquare + 1.5f;
	if (angleSquare >= 360.0f)
		angleSquare = angleSquare - 360.0f;

}
