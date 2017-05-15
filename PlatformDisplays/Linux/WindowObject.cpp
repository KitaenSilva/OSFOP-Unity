#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>

extern "C" {
	int ShowWindow(int winWidth, int winHeight);
	void CloseWindow();
	void Blit(char* image, int imageWidth, int imageHeight);
	void EnableClickthrough(int winWidth, int winHeight);
}

void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        //abort();
}

int x, y;

int width, height;

Display* display;
Window win;
GC gc;
XVisualInfo vinfo;
char* image;
XImage *ximage;
XImage *ximage1;
XImage *ximage2;
bool ximageinitialized;
bool clickthroughenabled;

int keep_running;

Region CreateRegion(int x, int y, int w, int h) {
    Region region = XCreateRegion();
    XRectangle rectangle;
    rectangle.x = x;
    rectangle.y = y;
    rectangle.width = w;
    rectangle.height = h;
    XUnionRectWithRegion(&rectangle, region, region);

    return region;
}

void swapBuffers()
{
	ximage = ximage2;
	ximage2 = ximage1;
}

extern "C" void InitializeBuffers(char* buffer1, char* buffer2)
{
	
}

extern "C" void EnableClickthrough(int winWidth, int winHeight)
{
	Region region = CreateRegion(0, 0, winWidth, winHeight);
    XShapeCombineRegion(display, win, ShapeBounding, 0, 0, region, ShapeSet);
    XDestroyRegion(region);
    
	region = CreateRegion(0, 0, winWidth, winHeight);
    XShapeCombineRegion(display, win, ShapeInput, 0, 0, region, ShapeSet);
    XDestroyRegion(region);
    clickthroughenabled = true;
}

extern "C" void DisableClickthrough(int winWidth, int winHeight)
{
	Region region = CreateRegion(0, 0, winWidth, winHeight);
    XShapeCombineRegion(display, win, ShapeBounding, 0, 0, region, ShapeSet);
    XDestroyRegion(region);
    
	region = CreateRegion(0, 0, 0, 0);
    XShapeCombineRegion(display, win, ShapeInput, 0, 0, region, ShapeSet);
    XDestroyRegion(region);
    clickthroughenabled = false;
}
extern "C" void CloseWindow()
{
	keep_running = 0;
	XEvent event;
	memset(&event, 0, sizeof(event));
	event.type = ButtonPress;
	
	XSendEvent(display, win, true, NoEventMask, &event);
	XFlush(display);
}

extern "C" void Blit(char* sourceImage, int imageWidth, int imageHeight)
{
	if (width != imageWidth || height != imageHeight || !ximageinitialized) {
		width = imageWidth;
		height = imageHeight;
		if (ximageinitialized) {
			XDestroyImage(ximage);
			XFlush(display);
		}
		image = (char*)malloc(width * height * 4);
		ximage = XCreateImage(display, vinfo.visual, 32, ZPixmap, 0, image, width, height, 32, 0);
		ximageinitialized = true;
		if (clickthroughenabled) {
			EnableClickthrough(imageWidth, imageHeight);
		} else {
			DisableClickthrough(imageWidth, imageHeight);
		}
	}
	memcpy(image, sourceImage, width * height * 4);
	XPutImage(display, win, gc, ximage, 0, 0, 0, 0, width, height);
	XFlush(display);
}

extern "C" int ShowWindow(int winWidth, int winHeight)
{
	width = 512;
	height = 512;
	keep_running = -1;
	ximageinitialized = false;
    display = XOpenDisplay(NULL);

    XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &vinfo);

    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
    attr.override_redirect = True;
    attr.background_pixmap = None;
    attr.border_pixmap = None;
    attr.border_pixel = 0;
    attr.background_pixel = 0;

    win = XCreateWindow(display, DefaultRootWindow(display), 0, 0, winWidth, winHeight, 0,
			vinfo.depth, InputOutput, vinfo.visual,
			CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attr);
    XSelectInput(display, win, StructureNotifyMask);
    gc = XCreateGC(display, win, 0, 0);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, win, &wm_delete_window, 1);
    
    DisableClickthrough(winWidth, winHeight);

    XMapWindow(display, win);
    
    ximageinitialized = false;

    if (keep_running == -1)
		keep_running = 1;
	else
		keep_running = 0;
	
    XEvent event;

    while (keep_running) {
        XNextEvent(display, &event);
        
        //XPutImage(display, win, gc, ximage, 0, 0, 0, 0, width, height);

        switch(event.type) {
            case ClientMessage:
                if (event.xclient.message_type == XInternAtom(display, "WM_PROTOCOLS", 1) && (Atom)event.xclient.data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", 1))
                    keep_running = 0;

                break;

            default:
                break;
        }
        usleep(50000);
    }

	XDestroyImage(ximage);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    return 0;
}
