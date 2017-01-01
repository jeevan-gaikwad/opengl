#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

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
void uninitialize();
int main(int argc,char* argv[])
{
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;

	CreateWindow();

	//Message loop
	XEvent event;
    KeySym keysym;

    while(1){
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
			case MotionNotify:
					break;
			case ConfigureNotify:
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					break;
			case Expose:
					break;
			case DestroyNotify:
					break;
			case 33:
					uninitialize();
					exit(EXIT_SUCCESS);
			default:
					break;
    	}
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

	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL){
		fprintf(stderr,"Error: Unable to open X Display.\n Exiting now..\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}
	defaultScreen=XDefaultScreen(gpDisplay);
	defaultDepth=DefaultDepth(gpDisplay,defaultScreen);
	gpXVisualInfo=(XVisualInfo*)malloc(sizeof(XVisualInfo));
	if(gpXVisualInfo==NULL){
		fprintf(stderr,"Error: Unable to allocate memory for VisualInfo.\nExiting now...\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}
	XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo);
	if(gpXVisualInfo==NULL){
		fprintf(stderr,"Error: Unable to get a Visual.\nExiting now...\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}

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
void uninitialize(void){
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
