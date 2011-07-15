// $Id: streamAnim.c,v 1.7 2009/12/21 18:00:59 hayes Exp $
// $Author: hayes $
// $Date: 2009/12/21 18:00:59 $
// $Revision: 1.7 $
//
// streamAnim is designed to be a lower level image streaming utility
// to XWindows platforms including Linux with a minimal windowing and
// library need.  streamAnim never completely loads the stream of
// images into memory, only loading one image at a time.  This is
// exceptionaly useful with very large images and very large streams
// of images which can rapidly bring a systems memory grinding to a
// halt in mid animation.....
//
// This version relies on imlib for greater flexibility in image
// format loading....
//
// http://freshmeat.net/projects/imlib/
// http://ftp.gnome.org/pub/GNOME/sources/imlib/
// http://ftp.gnome.org/pub/GNOME/sources/imlib/
// http://ftp.gnome.org/pub/GNOME/sources/imlib/1.9/imlib-1.9.15.tar.bz2
// http://ftp.gnome.org/pub/GNOME/sources/imlib/1.9/imlib-1.9.15.tar.gz
//
// Copyright (C) 1999 Paul Robert Hayes
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// When reporting or displaying any results or animations created
// using this code or modification of this code, make the appropriate
// citation referencing streamAnim by name and including the version
// number.  You should know better. 8)
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// http://www.cemtach.com/
//

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <Imlib.h>
#include <string.h>
#include <linux/limits.h>
#include <errno.h>

#include "getopt.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct _listItem
  {
  void *filename;
  //
  struct _listItem *next;
  };


struct _sList
  {
  struct _listItem *head;
  struct _listItem *tail;
  };



void
usage()
  {
  fprintf(stderr, "\t\n");
  fprintf(stderr, "Copyright (C) 1999-\n");
  fprintf(stderr, "\tstreamAnim is designed for minimal memory footprint while delivering frames to the screen as fast as possible.\n");
  fprintf(stderr, "\t\n");
  fprintf(stderr, "Usage:\n streamAnim [options] imageFiles\n");
  fprintf(stderr, "\t\n");
  fprintf(stderr, "\t--resize   resize at each frame, in case images vary in dimensions\n");
  fprintf(stderr, "\t--loop     loop through all images and repeat\n");
  fprintf(stderr, "\t--scale {factor}   scale by {factor} amount\n");
  fprintf(stderr, "\t--sleep {seconds}   sleep for X seconds after each frame\n");
  fprintf(stderr, "\t--usleep {microseconds}   sleep for X microseconds after each frame\n");
  fprintf(stderr, "\t--listFile {myFile}   parse myFile acquiring image file names from it (1 per line)\n");
  fprintf(stderr, "\t\n");
  }



void
addFile(struct _sList *thisList,
	char *filename)
  {
  struct _listItem *newItem = NULL;

  newItem = malloc(sizeof(struct _listItem));
  if (newItem == NULL)
    {
    fprintf(stderr, "Unable to allocate memory for file list storage: %s", strerror(errno));
    exit(1);
    }
  newItem->next = NULL;
  //
  newItem->filename = strdup(filename);
  if (newItem->filename == NULL)
    {
    fprintf(stderr, "Unable to allocate memory for file name storage: %s", strerror(errno));
    exit(1);
    }
  //
  if (thisList->head == NULL)
    {
    thisList->head = newItem;
    thisList->tail = thisList->head;
    //
    return;
    }
  //
  // Otherwise add into the tail...
  thisList->tail->next = newItem;
  thisList->tail = newItem;
  
  }


int
main(int argc, char **argv)
  {
  Display *disp;
  ImlibData *id;
  XSetWindowAttributes attr;
  Window win;
  ImlibImage *im;
  Pixmap p,m;
  int w,h;
  int c, i, j, k, numberFiles;
//  char **fileNames;
  char *listFilename = NULL;
  struct _sList fileList;
  struct _listItem *listItem = NULL;
  char parseLine[PATH_MAX];
  FILE *parseStream = NULL;
  char parseStreamRoot[PATH_MAX];
  char testFilename[PATH_MAX];
  char cwd[PATH_MAX];
  char *pathSplit = NULL;
  char *imageFilename = NULL;
  XWindowChanges windowChanges;

  unsigned int sleepValue = 0;
  unsigned int usleepValue = 0;
  float scaleValue = 1.0;

  XTextProperty windowTitle;

  int loop = FALSE;
  int resize = FALSE;

  fileList.head = NULL;
  fileList.tail = NULL;


  if (argc<=1)
    {
    usage();
    exit(1);
    }

  getcwd(cwd, PATH_MAX);

  //
  // Options parsing to explore variations without recompiling....
  while (1)
    {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] =
      {
      {"loop", 0, 0, 0},
      {"resize", 0, 0, 0},
      {"listFile", 1, 0, 0},
      {"sleep", 1, 0, 0},
      {"scale", 1, 0, 0},
      {"usleep", 1, 0, 0},
      {0, 0, 0, 0}
      };
    
    c = getopt_long (argc, argv, "",
		     long_options, &option_index);
    
    if (c == -1)
      break;

    if (c == EOF)
      break;
    
    switch (c)
      {
      case 0:
	if (strcmp(long_options[option_index].name, "loop") == 0)
	  {
	  loop = TRUE;
	  }
	if (strcmp(long_options[option_index].name, "resize") == 0)
	  {
	  resize = TRUE;
	  }
	if (strcmp(long_options[option_index].name, "usleep") == 0)
	  {
	  usleepValue = strtoul(optarg, NULL, 10);
	  }
	if (strcmp(long_options[option_index].name, "sleep") == 0)
	  {
	  sleepValue = strtoul(optarg, NULL, 10);
	  }
	if (strcmp(long_options[option_index].name, "scale") == 0)
	  {
	  scaleValue = strtod(optarg, NULL);
	  }
	if (strcmp(long_options[option_index].name, "listFile") == 0)
	  {
	  listFilename = strdup(optarg);
	  }
	break;

	
      default:
	usage();
	exit(1);
      }
    }
  //
  // access list file if it's available...
  if (listFilename != NULL)
    {
    if (access(listFilename, R_OK) != 0)
      {
      fprintf(stderr, "%s", strerror(errno));
      }
    else
      {
      parseStream = fopen(listFilename, "r");
      strncpy(parseStreamRoot, listFilename, PATH_MAX);
      //
      pathSplit = strrchr(parseStreamRoot, '/');
      if (pathSplit != NULL)
	{
	pathSplit[0] = '\0';
	}
      //
      while (fgets(parseLine, PATH_MAX, parseStream) != NULL)
	{
	while ( (pathSplit = strrchr(parseLine, '\n')) != NULL)
	  {
	  pathSplit[0] = '\0';
	  }
	//
	// Try to access the file listed on this line
	if (access(parseLine, R_OK) == 0)
	  {
	  addFile(&fileList, parseLine);
	  continue;
	  }
	//
	snprintf(testFilename, PATH_MAX, "%s/%s", cwd, parseLine);
	if (access(testFilename, R_OK) == 0)
	  {
	  addFile(&fileList, testFilename);
	  continue;
	  }
	//
	snprintf(testFilename, PATH_MAX, "%s/%s", parseStreamRoot, parseLine);
	if (access(testFilename, R_OK) == 0)
	  {
	  addFile(&fileList, testFilename);
	  continue;
	  }
	//
	// We could not find the file....
	fprintf(stderr, "Unable to access: [%s] %s\n", parseLine, strerror(errno));
	} // while (fgets(parseLine, PATH_MAX, parseStream) != NULL)
      }
    }

  //
  // Acquire the remaining commandline values as filenames....
  j=optind;
  while(optind < argc)
    {
    //
    fprintf(stderr, "%s\n", argv[optind]);
    //
    if (access(argv[optind], F_OK) != 0)
      {
      fprintf(stderr, "%s", strerror(errno));
      }
    else
      {
      addFile(&fileList, argv[optind]);
      }
    //
    optind++;
    }
  //
  //
  listItem = fileList.head;
  while(listItem != NULL)
    {
    fprintf(stderr, "%s\n", listItem->filename);
    //
    listItem = listItem->next;
    }



  fprintf(stderr, "\n");
  //
  // Connect to the default Xserver
  //
  disp=XOpenDisplay(NULL);
  //
  // Immediately afterwards Intitialise Imlib
  //
  id=Imlib_init(disp);
  //
  // Load the image specified as the first argument
  //
  imageFilename = (fileList.head)->filename;
  im=Imlib_load_image(id,imageFilename);
  //
  // Suck the image's original width and height out of the Image structure
  //
  w=im->rgb_width;
  h=im->rgb_height;
  //
  // Create a Window to display in
  //
  win=XCreateWindow(disp,DefaultRootWindow(disp),0,0,w,h,0,id->x.depth,
		    InputOutput,id->x.visual,0,&attr);
  XSelectInput(disp,win,StructureNotifyMask);
  //
  // Actually display the window
  //
  XMapWindow(disp,win);
  //
  // Event loop to watch for resizes and file changes.....
  //
  if (loop)
    (fileList.tail)->next = fileList.head;


  listItem = fileList.head;
  while(listItem != NULL)
    {
    imageFilename = listItem->filename;
    //
    //
    if (sleepValue > 0)
      sleep(sleepValue);
    if (usleepValue > 0)
      usleep(usleepValue);
    //
    // Set new title for new file...
    XStringListToTextProperty(&imageFilename, 1, &windowTitle);
    XSetWMName(disp, win, &windowTitle);
    XSetWMIconName(disp, win, &windowTitle);
    //
    //
    Imlib_free_pixmap(id,p);
    //
    // Load the image specified as the first argument
    Imlib_kill_image(id,im);
    im=Imlib_load_image(id,imageFilename);
    //
    // Suck the image's original width and height out of the Image
    // structure
    w=im->rgb_width*scaleValue;
    h=im->rgb_height*scaleValue;
    //
    if (resize)
      {
      windowChanges.width = w;
      windowChanges.height = h;
      //
      XConfigureWindow(disp, win, CWWidth | CWHeight, &windowChanges);
      }
    //
    // Re-render the Image to the new size
    Imlib_render(id,im,w,h);
    //
    // Extract the Image and mask pixmaps from the Image
    p=Imlib_move_image(id,im);
    //
    // The mask will be 0 if the image has no transparency
    m=Imlib_move_mask(id,im);
    //
    // Put the Image in the window, at the window's size and apply a
    // shape mask if applicable, or remove one if not
    Imlib_apply_image(id,im,win);
    //
    // Put the Image pixmap in the background of the window
    XSetWindowBackgroundPixmap(disp,win,p);
    //
    //  	/* If there was a mask to the image, set the Image's
    //  	mask to it */ if (m)
    //  	XShapeCombineMask(disp,win,ShapeBounding,0,0,m,ShapeSet);
    //  	/* Actually display the window */
    //  	XMapWindow(disp,win);
    XClearWindow(disp,win);
    //
    // Synchronize with the Xserver without discarding any events to
    // refresh with the new image
    XSync(disp,False);
    //
    //
    listItem = listItem->next;
    }
  }




// typedef struct {
// 	int x, y;
// 	int width, height;
// 	int border_width;
// 	Window sibling;
// 	int stack_mode;
// } XWindowChanges;


// /* Configure window value mask bits */

// #define CWX		(1<<0)
// #define CWY		(1<<1)
// #define CWWidth		(1<<2)
// #define CWHeight	(1<<3)
// #define CWBorderWidth	(1<<4)
// #define CWSibling	(1<<5)
// #define CWStackMode	(1<<6)

