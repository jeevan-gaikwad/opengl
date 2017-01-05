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
//#include <GL/glu.h>
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
bool bLight = false; //for whether lighting is ON/OFF, by default 'off'
GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };
void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);
void spin(void);
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
						case XK_l:
						case XK_L:
							if (bLight == false)
							{
								bLight = true;
								glEnable(GL_LIGHTING);
							}
							else
							{
								bLight = false;
								glDisable(GL_LIGHTING);
							}
							break;
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
	
	XStoreName(gpDisplay,gWindow,"Lighting The Pyramid And Cube");
	
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
	//function prototype
	void resize(int, int);
	
	//code
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	glClearColor(1.0f,0.0f,0.0f,0.0f);
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient); //setup ambient light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse); //setup diffuse light
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition); //position the light
	glEnable(GL_TEXTURE_2D); //enable texture mapping
	glEnable(GL_LIGHT1); //enable above configured LIGHT1 
	resize(giWindowWidth,giWindowHeight);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT);
    
    glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -6.0f);
	glRotatef(angleTri,0.0f,1.0f,0.0f);
	glBegin(GL_TRIANGLES);
	//****FRONT FACE****

	glColor3f(1.0f, 0.0f, 0.0f); //red : Colour of apex triangle

	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glColor3f(0.0f, 1.0f, 0.0f); //green : Colour of left-bottom tip of triangle

	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom tip of triangle

	glColor3f(0.0f, 0.0f, 1.0f); //blue : Colour of right-bottom tip of triangle

	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom tip of triangle

	//****RIGHT FACE****

	glColor3f(1.0f, 0.0f, 0.0f); //red : Colour of apex of triangle

	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glColor3f(0.0f, 0.0f, 1.0f); //blue : Colour of left-bottom tip of triangle

	glVertex3f(1.0f, -1.0f, 1.0f); //left-bottom tip of triangle

	glColor3f(0.0f, 1.0f, 0.0f); //green : Colour of right-bottom tip of triangle

	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom tip of triangle

	//****BACK FACE****

	glColor3f(1.0f, 0.0f, 0.0f); //red : Colour of apex triangle

	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle

	glColor3f(0.0f, 1.0f, 0.0f); //green : Colour of left-bottom tip of triangle

	glVertex3f(1.0f, -1.0f, -1.0f); //left-bottom tip of triangle



	glColor3f(0.0f, 0.0f, 1.0f); //blue : Colour of right-bottom tip of triangle

	glVertex3f(-1.0f, -1.0f, -1.0f); //right-bottom tip of triangle



	//****LEFT FACE****

	glColor3f(1.0f, 0.0f, 0.0f); //red : Colour of apex of triangle

	glVertex3f(0.0f, 1.0f, 0.0f); //apex of triangle



	glColor3f(0.0f, 0.0f, 1.0f); //blue : Colour of left-bottom tip of triangle

	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom tip of triangle


	glColor3f(0.0f, 1.0f, 0.0f); //green : Colour of rigt-bottom tip of triangle

	glVertex3f(-1.0f, -1.0f, 1.0f); //right-bottom tip of triangle

	glEnd();

	//###### SQUARE ######

	glLoadIdentity();

	glTranslatef(1.5f, 0.0f, -6.0f);
	glRotatef(angleSquare,1.0f,0.0f,0.0f);
	glBegin(GL_QUADS);
	//NOTE : EACH FACE OF A CUBE IS A SQUARE

	//****TOP FACE****
	glColor3f(1.0f, 0.0f, 0.0f); //red : Colour of top face
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of top face
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of top face
	glVertex3f(-1.0f, 1.0f, 1.0f); //left-bottom of top face
	glVertex3f(1.0f, 1.0f, 1.0f); //right-bottom of top face

	//****BOTTOM FACE****
	glColor3f(0.0f, 1.0f, 0.0f); //green : Colour of bottom face
	glVertex3f(1.0f, -1.0f, -1.0f); //right-top of bottom face
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-top of bottom face
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom of bottom face
	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom of bottom face

	//****FRONT FACE****
	glColor3f(0.0f, 0.0f, 1.0f); //blue : Colour of front face
	glVertex3f(1.0f, 1.0f, 1.0f); //right-top of front face
	glVertex3f(-1.0f, 1.0f, 1.0f); //left-top of front face
	glVertex3f(-1.0f, -1.0f, 1.0f); //left-bottom of front face
	glVertex3f(1.0f, -1.0f, 1.0f); //right-bottom of front face

	//****BACK FACE****
	glColor3f(1.0f, 1.0f, 0.0f); //yellow : Colour of back face
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of back face
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of back face
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom of back face
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom of back face

	//****LEFT FACE****
	glColor3f(0.0f, 1.0f, 1.0f); //cyan : Colour of left face
	glVertex3f(-1.0f, 1.0f, 1.0f); //right-top of back face
	glVertex3f(-1.0f, 1.0f, -1.0f); //left-top of back face
	glVertex3f(-1.0f, -1.0f, -1.0f); //left-bottom of back face
	glVertex3f(-1.0f, -1.0f, 1.0f); //right-bottom of back face

	//****RIGHT FACE****
	glColor3f(1.0f, 0.0f, 1.0f); //magenta : Colour of right face
	glVertex3f(1.0f, 1.0f, -1.0f); //right-top of back face
	glVertex3f(1.0f, 1.0f, 1.0f); //left-top of back face
	glVertex3f(1.0f, -1.0f, 1.0f); //left-bottom of back face
	glVertex3f(1.0f, -1.0f, -1.0f); //right-bottom of back face


	glEnd();

	
	glFlush();
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
