/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
@end

// To implement right click menus on OSX correctly, we need to keep the right
// click event around (with all its gory platform specific details) until we
// can pass it back to NSMenu.
NSEvent* GetLastRightClickEvent();
