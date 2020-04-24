#pragma once

#include "InputEvents.hpp"
#include "DrawUtils.hpp"

#include <functional>
#include <vector>

#define DEEP_HIGHLIGHT 200
#define THICK_HIGHLIGHT 150
#define HIGHLIGHT 100
#define NO_HIGHLIGHT 0

#if defined(USE_RAMFS)
#define RAMFS "resin:/"
#else
#define RAMFS "resin/"
#endif

#if defined(_3DS) || defined(_3DS_MOCK)
#define SCREEN_WIDTH	400
#define SCREEN_HEIGHT	480
#define PANE_WIDTH 300
#define SCALER 2
#else
#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720
#define PANE_WIDTH 720
#define SCALER 1
#endif

class Element
{
public:
	virtual ~Element();

	// process any input that is received for this element
	virtual bool process(InputEvents* event);

	// display the current state of the display
	virtual void render(Element* parent);

	// invoked on touchdown/up events
	bool onTouchDown(InputEvents* event);
	bool onTouchDrag(InputEvents* event);
	bool onTouchUp(InputEvents* event);

	// hide the element
	void hide() { this->hidden = true; }
	// unhide the element
	void unhide() { this->hidden = false; }

	// the action to call (from binded callback) on touch or button selection
	// https://stackoverflow.com/questions/14189440/c-class-member-callback-simple-examples
	std::function<void()> action;

	// visible GUI child elements of this element
	std::vector<Element*> elements;

	void append(Element *element);
	void remove(Element *element);
	void removeAll(void);

	// position the element (x and y are percents of the screen)
	void position(int x, int y);

	// recalculate xAbs and yAbs based on the given parent
	void recalcPosition(Element* parent);

	// update and propogate a new window and renderer to all children
	void setCST(CST_Renderer *renderer, CST_Window *window);

	// the scale of the element (and its subelements!)
	float scale = 1.0f;

	// Chesto main graphics variables
	CST_Window* window = NULL;
	CST_Renderer* renderer = NULL;

	// whether or not this element can be touched (highlights bounds)
	bool touchable = false;

	// whether or not this element is currently being dragged
	bool dragging = false;

	// whether or not this element needs the screen redrawn next time it's processed
	bool needsRedraw = false;
	//
	//    // can be used for highlighting the selected element
	//    int animationCounter = 0;

	// the last Y, X coordinate of the mouse (from a drag probably)
	int lastMouseY = 0, lastMouseX = 0;

	// whether this element should skip rendering or not
	bool hidden = false;

	// how much time is left in an elastic-type flick/scroll
	// set by the last distance traveled in a scroll, and counts down every frame
	int elasticCounter = 0;

	// width and height of this element (must be manually set, isn't usually calculated (but is in some cases, like text or images))
	int width = 0, height = 0;

	typedef Element super;

	// position relative to parent (if given) or screen (NULL parent)
	int x = 0, y = 0;

	// actual onscreen position (calculated at render time)
	int xAbs = 0, yAbs = 0;

	//rotation angle in degrees
	double angle=0;
};
