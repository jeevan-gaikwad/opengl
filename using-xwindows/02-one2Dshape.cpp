#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

using namespace std;

//global variable declarations
bool bFullscreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

void CreateWindow(void);
void ToggleFullscreen(void);
void initialize();
void uninitialize();
void resize(int,int);
void display(void);
GLXContext gGLXContext;

int main(int argc,char* argv[])
{
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	bool bDone=false;
	CreateWindow();
	initialize();
	//Message loop
	XEvent event;
	KeySym keysym;

    while(bDone==false){

	while(XPending(gpDisplay)){

        XNextEvent(gpDisplay,&event);
        switch(event.type){
		case MapNotify:
                	break;
		case KeyPress:
                	keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
                    	switch(keysym){
                        	case XK_Escape:
                                	uninitialize();
                                	exit(EXIT_SUCCESS);
                        	case XK_F:
                        	case XK_f:
                                	if(bFullscreen==false){
                                    		ToggleFullscreen();
                                    		bFullscreen=true;
                                	}else{
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
				uninitialize();
				exit(EXIT_SUCCESS);
			default:
					break;
    			}
		}
		display();
	}
	uninitialize();
	exit(EXIT_SUCCESS);
}
void CreateWindow(void){
	//function prototypes
	void uninitialize();
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;
	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE,1,
		GLX_GREEN_SIZE,1,
		GLX_BLUE_SIZE,1,
		GLX_ALPHA_SIZE,1,
		None
	};
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL){
		fprintf(stderr,"Error: Unable to open X Display.\n Exiting now..\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}
	defaultScreen=XDefaultScreen(gpDisplay);
	//defaultDepth=DefaultDepth(gpDisplay,defaultScreen);
	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);
	
	//XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo);
	

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),gpXVisualInfo->visual,AllocNone);
	gColormap=winAttribs.colormap;
	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);
	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;
	gWindow=XCreateWindow(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),0,0,giWindowWidth,giWindowHeight,0,gpXVisualInfo->depth,InputOutput,
	gpXVisualInfo->visual,styleMask,&winAttribs);
	if(!gWindow){
		fprintf(stderr,"Error: Failed to create main window\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}
	XStoreName(gpDisplay,gWindow,"First XWindow");

	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void){
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
	xev.xclient.data.l[0]=bFullscreen?0:1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",false);
	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),False, StructureNotifyMask,&xev);

}

void initialize(){
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	glClearColor(1.0f,0.0f,0.0f,0.0f);
	resize(giWindowHeight,giWindowHeight);
}
void display(){
	//to clear all pixels

	glClear(GL_COLOR_BUFFER_BIT);
	//###### TRIANGLE ######
	glBegin(GL_TRIANGLES);
	//white : Colour of triangle
	glColor3f(1.0f,1.0f,1.0f);
	//co-ordinates of triangle
	glVertex3f(0.0f,50.0f,0.0f); //apex of triangle
	glVertex3f(-50.0f,-50.0f,0.0f); //left-bottom tip of triangle
	glVertex3f(50.0f,-50.0f,0.0f); //right-bottom tip of triangle
	glEnd();
	glFlush();
}
void resize(int width,int height){
	if(height==0)
		height=1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	if(width<=height)
		glOrtho(-100.0f,100.0f,(-100.0f * (height/width)),(100.0f * (height/width)),-100.0f,100.0f); 
	else
		glOrtho(-100.0f,100.0f,(-100.0f * (width/height)),(100.0f * (width/height)),-100.0f,100.0f); 

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
}
void uninitialize(void){
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();
	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext){
		glXMakeCurrent(gpDisplay,0,0);
	}
	if(gGLXContext){
		glXDestroyContext(gpDisplay,gGLXContext);
	}
	if(gWindow){
		XDestroyWindow(gpDisplay,gWindow);
	}

	if(gColormap){
		XFreeColormap(gpDisplay,gColormap);
	}
	if(gpXVisualInfo){
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}

	if(gpDisplay){
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}
