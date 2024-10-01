/*****************************************************************************
 *
 * Copyright 1989-1993 ParcPlace Systems
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of ParcPlace not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  ParcPlace makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * ParcPlace DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ParcPlace BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA  OR  PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * $Id: bitmap.C,v 9.6 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Bitmap file routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: bitmap.C,v 9.6 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "list.H"
#include "util.H"
#include "parse.H"
#include <sys/file.h>
#include "debug.H"
#include "bitmap.H"

static char *defaultPath1 = SWM_DEFAULTS;
static char *defaultPath2 = "/usr/include/X11/bitmaps/";

/***********************************************************************
 *
 *  Procedure:
 *      wmInitBitmapPath
 *
 *  Function:
 *	Initialize the bitmap path to search when looking for bitmap
 *	files.
 *
 ***********************************************************************
 */

void
wmInitBitmapPath()
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmInitBitmapPath:\n");
#endif

    char *ptr = RM->get_resource("bitmapPath", "BitmapPath");
    if (!ptr)
	ptr = RM->get_resource("bitmapFilePath", "BitmapFilePath");

    if (ptr != NULL)
    {
	wmParse("wmStrings", ptr);
	if (wmParseError)
	{
	    fprintf(stderr,
		"swm: wmInitBitmapPath: error parsing bitmap path\n");
	}
	else
	{
	    while ((ptr = (char *)wmStringList.get()) != NULL)
	    {
		ptr = wmExpandFilename(ptr);
		wmScr->bitmapPathList.append((ent)ptr);
	    }
	}
    }
    wmScr->bitmapPathList.append((ent)defaultPath1);
    wmScr->bitmapPathList.append((ent)defaultPath2);

#ifdef DEBUG
    if (wmDebug)
    {
	for (char *np = (char *)wmScr->bitmapPathList.first(); np != NULL;
	    np = (char *)wmScr->bitmapPathList.next())
	{
	    fprintf(dfp, "  %s\n", np);
	}
    }
#endif

}

/***********************************************************************
 *
 *  Procedure:
 *      wmFindBitmap
 *
 *  Function:
 *	Find a bitmap file and return a pointer to the wmBitmap structure
 *	describing the bitmap.  Note that this routine will cache the
 *	bitmap information to help cut down on trips to the server.
 *
 ***********************************************************************
 */

wmBitmap *
wmFindBitmap(
    char *name		    // the file name to find
    )
{
    char *np;
    char *newfile = NULL;
    wmBitmap *wbm = NULL;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmFindBitmap: \"%s\"\n", name);
#endif

    if (name[0] == '@')
	name = &name[1];

    // check the context to see if we have already found this one
    if ((XFindContext(DPY, (Window)name, wmScr->bitmaps, (caddr_t *)&wbm)) == XCNOENT)
    {
	// try the file name all by itself
	if (access(name, R_OK) == 0)
	{
	    // found it 
	    newfile = name;
	}
	else
	{
	    // add each entry in the bitmapPath list to try and find it
	    for (np = (char *)wmScr->bitmapPathList.first(); np != NULL;
		np = (char *)wmScr->bitmapPathList.next())
	    {
		newfile = (char *)malloc(strlen(np) + strlen(name) + 5);
		strcpy(newfile, np);
		strcat(newfile, "/");
		strcat(newfile, name);
#ifdef DEBUG
		if (wmDebug)
		    fprintf(dfp, "    trying \"%s\"\n", newfile);
#endif
		if (access(newfile, R_OK) == 0)
		    break;
		free(newfile);
		newfile = NULL;
	    }
	}
	if (newfile == NULL)
	{
	    fprintf(stderr, "swm: wmFindBitmap: bitmap file \"%s\" not found\n",
		name);
	}
	else
	{
	    wbm = new wmBitmap(name, newfile);
	}
    }
    return (wbm);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmBitmap::wmBitmap
 *
 *  Function:
 *	Constructor for the wmBitmap class
 *
 ***********************************************************************
 */

wmBitmap::wmBitmap(
    char *name,			// the name returned from the resource database
    char *fullname		// the final full pathname
    )
{
    int xh, yh;

    path = fullname;
    XSaveContext(DPY, (Window)name, wmScr->bitmaps, (caddr_t)this);
    if (XReadBitmapFile(DPY, wmScr->root, fullname, &width, &height, &pixmap, &xh, &yh) != BitmapSuccess)
	pixmap = NULL;
}
