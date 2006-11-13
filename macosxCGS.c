/* -- macosxCGS.c -- */

/*
 * We need to keep this separate from nearly everything else, e.g. rfb.h
 * and the other stuff, otherwise it does not work properly, mouse drags
 * will not work!!
 */

#if (defined(__MACH__) && defined(__APPLE__))

#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

extern CGDirectDisplayID displayID;

void macosxCGS_get_all_windows(void);

typedef CGError       CGSError;
typedef long          CGSWindowCount;
typedef void *        CGSConnectionID;
typedef int           CGSWindowID;
typedef CGSWindowID*  CGSWindowIDList;
typedef CGWindowLevel CGSWindowLevel;
typedef NSRect        CGSRect;

extern CGSConnectionID _CGSDefaultConnection ();

extern CGSError CGSGetOnScreenWindowList (CGSConnectionID cid,
    CGSConnectionID owner, CGSWindowCount listCapacity,
    CGSWindowIDList list, CGSWindowCount *listCount);

extern CGSError CGSGetScreenRectForWindow (CGSConnectionID cid,
    CGSWindowID wid, CGSRect *rect);

extern CGWindowLevel CGSGetWindowLevel (CGSConnectionID cid,
    CGSWindowID wid, CGSWindowLevel *level);

typedef enum _CGSWindowOrderingMode {
    kCGSOrderAbove                =  1, // Window is ordered above target.
    kCGSOrderBelow                = -1, // Window is ordered below target.
    kCGSOrderOut                  =  0  // Window is removed from the on-screen window list.
} CGSWindowOrderingMode;

extern OSStatus CGSOrderWindow(const CGSConnectionID cid,
    const CGSWindowID wid, CGSWindowOrderingMode place, CGSWindowID relativeToWindowID);

static CGSConnectionID cid = NULL;

int macwinmax = 0; 
typedef struct windat {
	int win;
	int x, y;
	int width, height;
	int level;
} windat_t;

#define MAXWINDAT 2048
windat_t macwins[MAXWINDAT]; 
static CGSWindowID _wins[MAXWINDAT]; 

extern double dnow(void);

extern int dpy_x, dpy_y;


int CGS_levelmax;
int CGS_levels[16];

void macosxCGS_get_all_windows(void) {
	static double last = 0.0;
	static int first = 1;
	double dt = 0.0, now = dnow();
	int i, db = 0;
	CGSWindowCount cap = (CGSWindowCount) MAXWINDAT;
	CGSWindowCount cnt = 0;
	CGSError err; 

	if (first) {
		first = 0;
		CGS_levelmax = 0;
		CGS_levels[CGS_levelmax++] = (int) kCGDraggingWindowLevel;	/* 500 */
		if (0) CGS_levels[CGS_levelmax++] = (int) kCGHelpWindowLevel;		/* 102 */
		if (0) CGS_levels[CGS_levelmax++] = (int) kCGPopUpMenuWindowLevel;	/* 101 */
		CGS_levels[CGS_levelmax++] = (int) kCGMainMenuWindowLevelKey;	/*  24 */
		CGS_levels[CGS_levelmax++] = (int) kCGFloatingWindowLevel;	/*   3 */
		CGS_levels[CGS_levelmax++] = (int) kCGNormalWindowLevel;	/*   0 */
	}

	if (cid == NULL) {
		cid = _CGSDefaultConnection();
		if (cid == NULL) {
			return;
		}
	}

	if (dt > 0.0 && now < last + dt) {
		return;
	}

	err = CGSGetOnScreenWindowList(cid, NULL, cap, _wins, &cnt);

if (db) fprintf(stderr, "cnt: %d err: %d\n", cnt, err);

	if (err != 0) {
		return;
	}
	
	last = now;

	macwinmax = 0; 

	for (i=0; i < (int) cnt; i++) {
		CGSRect rect;
		CGSWindowLevel level;
		int j, keepit = 0;
		err = CGSGetScreenRectForWindow(cid, _wins[i], &rect);
		if (err != 0) {
			continue;
		}
		if (rect.origin.x == 0 && rect.origin.y == 0) {
			if (rect.size.width == dpy_x) {
				if (rect.size.height == dpy_y) {
					continue;
				}
			}
		}
		err = CGSGetWindowLevel(cid, _wins[i], &level);
		if (err != 0) {
			continue;
		}
		for (j=0; j<CGS_levelmax; j++) {
			if ((int) level == CGS_levels[j]) {
				keepit = 1;
				break;
			}
		}
		if (! keepit) {
			continue;
		}

		macwins[macwinmax].level  = (int) level;
		macwins[macwinmax].win    = (int) _wins[i];
		macwins[macwinmax].x      = (int) rect.origin.x;
		macwins[macwinmax].y      = (int) rect.origin.y;
		macwins[macwinmax].width  = (int) rect.size.width;
		macwins[macwinmax].height = (int) rect.size.height;
if (db) fprintf(stderr, "i=%03d ID: %06d  x: %03d  y: %03d  w: %03d h: %03d level: %d\n", i, _wins[i],
    (int) rect.origin.x, (int) rect.origin.y,(int) rect.size.width, (int) rect.size.height, (int) level);

		macwinmax++;
	}
}

#endif	/* __APPLE__ */

