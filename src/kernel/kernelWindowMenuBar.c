//
//  Visopsys
//  Copyright (C) 1998-2015 J. Andrew McLaughlin
//
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation; either version 2 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
//  for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//  kernelWindowMenuBar.c
//

// This code is for managing kernelWindowMenuBar objects.
// These are just images that appear inside windows and buttons, etc

#include "kernelWindow.h"	// Our prototypes are here
#include "kernelDebug.h"
#include "kernelError.h"
#include "kernelFont.h"
#include "kernelMalloc.h"

static void (*saveMenuFocus)(kernelWindow *, int) = NULL;
static int (*saveMenuMouseEvent)(kernelWindow *, kernelWindowComponent *,
	windowEvent *) = NULL;
static int (*saveMenuKeyEvent)(kernelWindow *, kernelWindowComponent *,
	windowEvent *) = NULL;

extern kernelWindowVariables *windowVariables;


static inline int menuTitleWidth(kernelWindowComponent *component, int num)
{
	kernelWindowMenuBar *menuBar = component->data;
	asciiFont *font = (asciiFont *) component->params.font;
	kernelWindow *menu = menuBar->menu[num];

	int width = (windowVariables->border.thickness * 2);

	if (font)
		width += kernelFontGetPrintedWidth(font, (char *) menu->title);

	return (width);
}


static inline int menuTitleHeight(kernelWindowComponent *component)
{
	asciiFont *font = (asciiFont *) component->params.font;

	int height = (windowVariables->border.thickness * 2);

	if (font)
		height += font->glyphHeight;

	return (height);
}


static void changedVisible(kernelWindowComponent *component)
{
	kernelDebug(debug_gui, "WindowMenuBar changed visible title");

	if (component->draw)
		component->draw(component);

	component->window
		->update(component->window, component->xCoord, component->yCoord,
			component->width, component->height);
}


static void menuFocus(kernelWindow *menu, int focus)
{
	// We just want to know when the menu has lost focus, so we can
	// un-highlight the appropriate menu bar header

	kernelWindowComponent *menuBarComponent = menu->parentWindow->menuBar;

	kernelDebug(debug_gui, "WindowMenuBar menu %s focus",
		(focus? "got" : "lost"));

	if (saveMenuFocus)
		// Pass the event on.
		saveMenuFocus(menu, focus);

	if (!focus)
		// No longer visible
		changedVisible(menuBarComponent);
}


static int menuMouseEvent(kernelWindow *menu,
	kernelWindowComponent *itemComponent, windowEvent *event)
{
	int status = 0;
	kernelWindowComponent *menuBarComponent = menu->parentWindow->menuBar;
	kernelWindowMenuBar *menuBar = menuBarComponent->data;

	kernelDebug(debug_gui, "WindowMenuBar menu mouse event");

	if (saveMenuMouseEvent)
		// Pass the event on.
		status = saveMenuMouseEvent(menu, itemComponent, event);

	// Now determine whether the menu went away
	if (!(menu->flags & WINFLAG_HASFOCUS))
		menuBar->raisedMenu = NULL;

	return (status);
}


static int menuKeyEvent(kernelWindow *menu,
	kernelWindowComponent *itemComponent, windowEvent *event)
{
	int status = 0;
	kernelWindowComponent *menuBarComponent = menu->parentWindow->menuBar;
	kernelWindowMenuBar *menuBar = menuBarComponent->data;
	int menuNumber = -1;
	kernelWindow *newMenu = NULL;
	kernelWindowContainer *menuContainer = NULL;
	int count;

	kernelDebug(debug_gui, "WindowMenuBar menu key event");

	if (saveMenuKeyEvent)
		// Pass the event on.
		status = saveMenuKeyEvent(menu, itemComponent, event);

	// Now determine whether the menu went away
	if (!(menu->flags & WINFLAG_HASFOCUS))
	{
		menuBar->raisedMenu = NULL;
		return (status);
	}

	if (event->type != EVENT_KEY_DOWN)
		// Not interested
		return (status);

	// If the user has pressed the left or right cursor keys, that means they
	// want to switch menus
	if ((event->key == keyLeftArrow) || (event->key == keyRightArrow))
	{
		// Find out where the menu is in our list
		for (count = 0; count < menuBar->numMenus; count ++)
		{
			if (menuBar->menu[count] == menu)
			{
				menuNumber = count;
				break;
			}
		}

		if (event->key == keyLeftArrow)
		{
			// Cursor left
			if (menuNumber > 0)
				menuNumber -= 1;
		}
		else
		{
			// Cursor right
			if (menuNumber < (menuBar->numMenus - 1))
				menuNumber += 1;
		}

		newMenu = menuBar->menu[menuNumber];

		if (newMenu != menu)
		{
			menuContainer = newMenu->mainContainer->data;

			if (menuContainer->numComponents)
			{
				kernelDebug(debug_gui, "WindowMenuBar show new menu %s",
					newMenu->title);

				// Old one is no longer visible
				kernelWindowSetVisible(menu, 0);

				newMenu->xCoord = (menu->parentWindow->xCoord +
					menuBarComponent->xCoord +
					menuBar->menuXCoord[menuNumber]);
				newMenu->yCoord = (menu->parentWindow->yCoord +
					menuBarComponent->yCoord +
					menuTitleHeight(menuBarComponent));

				// Set the new one visible
				kernelWindowSetVisible(newMenu, 1);
				menuBar->raisedMenu = menu;

				changedVisible(menuBarComponent);
			}
		}
	}

	return (status);
}


static int add(kernelWindowComponent *menuBarComponent, objectKey menuObj)
{
	// Add the supplied menu to the menu bar.

	kernelWindow *menu = menuObj;
	kernelWindowMenuBar *menuBar = menuBarComponent->data;

	// If we don't have the menu's focus(), mouseEvent(), and keyEvent()
	// function pointers saved, save them now
	if (!saveMenuFocus)
		saveMenuFocus = menu->focus;
	if (!saveMenuMouseEvent)
		saveMenuMouseEvent = menu->mouseEvent;
	if (!saveMenuKeyEvent)
		saveMenuKeyEvent = menu->keyEvent;

	menu->focus = &menuFocus;
	menu->mouseEvent = &menuMouseEvent;
	menu->keyEvent = &menuKeyEvent;

	menuBar->menu[menuBar->numMenus++] = menu;

	return (0);
}


static int layout(kernelWindowComponent *component)
{
	// Do layout for the menu bar.

	int status = 0;
	kernelWindowMenuBar *menuBar = component->data;
	int width = 0;
	int count;

	kernelDebug(debug_gui, "WindowMenuBar layout");

	for (count = 0; count < menuBar->numMenus; count ++)
	{
		if (count)
			menuBar->menuXCoord[count] = (menuBar->menuXCoord[count - 1] +
				menuBar->menuTitleWidth[count - 1]);

		menuBar->menuTitleWidth[count] = menuTitleWidth(component, count);

		width += menuBar->menuTitleWidth[count];
	}

	component->width = component->window->buffer.width;
	component->minWidth = width;

	component->doneLayout = 1;

	return (status = 0);
}


static int draw(kernelWindowComponent *component)
{
	// Draw the menu bar component

	int status = 0;
	kernelWindowMenuBar *menuBar = component->data;
	kernelWindow *menu = NULL;
	int xCoord = 0, titleWidth = 0, titleHeight = 0;
	int count;

	kernelDebug(debug_gui, "WindowMenuBar draw '%s' menu bar",
		component->window->title);

	layout(component);

	// Draw the background of the menu bar
	kernelGraphicDrawRect(component->buffer,
		(color *) &(component->params.background), draw_normal,
		component->xCoord, component->yCoord, component->width,
		component->height, 1, 1);

	// Loop through all the menus and draw their names on the menu bar
	for (count = 0; count < menuBar->numMenus; count ++)
	{
		menu = menuBar->menu[count];
		xCoord = menuBar->menuXCoord[count];
		titleWidth = menuBar->menuTitleWidth[count];
		titleHeight = menuTitleHeight(component);

		if (menu->flags & WINFLAG_VISIBLE)
		{
			kernelDebug(debug_gui, "WindowMenuBar title %d '%s' is visible",
				count, menu->title);
			kernelGraphicDrawGradientBorder(component->buffer,
				(component->xCoord + xCoord), component->yCoord, titleWidth,
				titleHeight, windowVariables->border.thickness,
				(color *) &(component->params.background),
				windowVariables->border.shadingIncrement, draw_normal,
				border_all);
		}

		if (component->params.font)
		{
			kernelGraphicDrawText(component->buffer,
				(color *) &(component->params.foreground),
				(color *) &(component->params.background),
				(asciiFont *) component->params.font,
				(char *) menu->title, draw_normal,
				(component->xCoord + xCoord +
					windowVariables->border.thickness),
				(component->yCoord + windowVariables->border.thickness));
		}
	}

	return (status);
}


static int focus(kernelWindowComponent *component, int got)
{
	// We just want to know if we lost the focus, because in that case,
	// any raised menu will have gone away.

	kernelWindowMenuBar *menuBar = component->data;

	kernelDebug(debug_gui, "WindowMenuBar %s focus", (got? "got" : "lost"));

	if (!got)
	{
		menuBar->raisedMenu = NULL;
		changedVisible(component);
	}

	return (0);
}


static int mouseEvent(kernelWindowComponent *component, windowEvent *event)
{
	kernelWindowMenuBar *menuBar = component->data;
	kernelWindow *menu = NULL;
	kernelWindowContainer *menuContainer = NULL;
	int xCoord = 0, width = 0;
	int count;

	// If there are no menu components, quit here
	if (!menuBar->numMenus)
		return (0);

	// Events other than left mouse down are not interesting
	if (event->type != EVENT_MOUSE_LEFTDOWN)
		return (0);

	kernelDebug(debug_gui, "WindowMenuBar mouse event");

	// Determine whether to set a menu visible now by figuring out whether a
	// menu title was clicked.
	for (count = 0; count < menuBar->numMenus; count ++)
	{
		menu = menuBar->menu[count];
		menuContainer = menu->mainContainer->data;

		xCoord = (component->window->xCoord + component->xCoord +
			menuBar->menuXCoord[count]);
		width = menuBar->menuTitleWidth[count];

		if ((event->xPosition >= xCoord) &&
			(event->xPosition < (xCoord + width)))
		{
			if (menu != menuBar->raisedMenu)
			{
				// The menu was not previously raised, so we will show it.
				kernelDebug(debug_gui, "WindowMenuBar show menu %d '%s'",
					count, menu->title);

				menu->xCoord = xCoord;
				menu->yCoord = (component->window->yCoord + component->yCoord +
					menuTitleHeight(component));

				if (menuContainer->numComponents)
					kernelWindowSetVisible(menu, 1);

				menuBar->raisedMenu = menu;
			}
			else
			{
				// The menu was previously visible, so we won't re-show it.
				kernelDebug(debug_gui, "WindowMenuBar menu %d '%s' re-clicked",
					count, menu->title);

				menuBar->raisedMenu = NULL;
			}

			changedVisible(component);
			break;
		}
	}

	return (0);
}


static int destroy(kernelWindowComponent *component)
{
	kernelWindowMenuBar *menuBar = component->data;

	kernelDebug(debug_gui, "WindowMenuBar destroy");

	if (component->window && (component->window->menuBar == component))
		component->window->menuBar = NULL;

	// Release all our memory
	if (menuBar)
	{
		kernelFree(component->data);
		component->data = NULL;
	}

	return (0);
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Below here, the functions are exported for external use
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

kernelWindowComponent *kernelWindowNewMenuBar(kernelWindow *window,
	componentParameters *params)
{
	// Formats a kernelWindowComponent as a kernelWindowMenuBar

	kernelWindowComponent *component = NULL;
	kernelWindowMenuBar *menuBar = NULL;

	// Check params
	if (!window || !params)
	{
		kernelError(kernel_error, "NULL parameter");
		return (component = NULL);
	}

	if (window->type != windowType)
	{
		kernelError(kernel_error, "Menu bars can only be added to windows");
		return (component = NULL);
	}

	// Get the basic component structure
	component = kernelWindowComponentNew(window->sysContainer, params);
	if (!component)
		return (component);

	component->type = menuBarComponentType;
	component->flags |= WINFLAG_CANFOCUS;
	// Only want this to be resizable horizontally
	component->flags &= ~WINFLAG_RESIZABLEY;

	// Set the functions
	component->add = &add;
	component->layout = &layout;
	component->draw = &draw;
	component->focus = &focus;
	component->mouseEvent = &mouseEvent;
	component->destroy = &destroy;

	// If font is NULL, use the default
	if (!component->params.font)
		component->params.font = windowVariables->font.varWidth.small.font;

	// Get memory for this menu bar component
	menuBar = kernelMalloc(sizeof(kernelWindowMenuBar));
	if (!menuBar)
	{
		kernelWindowComponentDestroy(component);
		return (component = NULL);
	}

	component->data = (void *) menuBar;

	component->width = window->buffer.width;
	component->height = (windowVariables->border.thickness * 2);
	if (component->params.font)
		component->height += ((asciiFont *)
			component->params.font)->glyphHeight;
	component->minWidth = component->width;
	component->minHeight = component->height;

	window->menuBar = component;

	return (component);
}

