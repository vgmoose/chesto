#include "Element.hpp"
#include <algorithm>

Element::~Element()
{
	removeAll();
}

bool Element::process(InputEvents* event)
{
	// whether or not we need to update the screen
	bool ret = false;

	// if we're hidden, don't process input
	if (hidden) return ret;

	// do any touch down, drag, or up events
	if (touchable)
	{
		ret |= onTouchDown(event);
		ret |= onTouchDrag(event);
		ret |= onTouchUp(event);
	}

	// call process on subelements
	for (int x = 0; x < this->elements.size(); x++)
		if (this->elements.size() > x && this->elements[x])
			ret |= this->elements[x]->process(event);

	ret |= this->needsRedraw;
	this->needsRedraw = false;

	return ret;
}

void Element::render(Element* parent)
{
	//if we're hidden, don't render
	if (hidden) return;

	// this needs to happen before subelement rendering
	this->recalcPosition(parent);

	// go through every subelement and run render
	for (Element* subelement : elements)
	{
		subelement->render(this);
	}

	// if we're touchable, and we have some animation counter left, draw a rectangle+overlay
	if (this->touchable && this->elasticCounter > THICK_HIGHLIGHT)
	{
		CST_Rect d = { this->xAbs - 5, this->yAbs - 5, this->width + 10, this->height + 10 };
		CST_SetDrawBlend(this->renderer, true);
		CST_SetDrawColorRGBA(this->renderer, 0xad, 0xd8, 0xe6, 0x90);
		CST_FillRect(this->renderer, &d);
	}

	if (this->touchable && this->elasticCounter > NO_HIGHLIGHT)
	{
		CST_Rect d = { this->xAbs - 5, this->yAbs - 5, this->width + 10, this->height + 10 };
		rectangleRGBA(this->renderer, d.x, d.y, d.x + d.w, d.y + d.h, 0x66, 0x7c, 0x89, 0xFF);

		if (this->elasticCounter == THICK_HIGHLIGHT)
		{
			// make it a little thicker by drawing more rectangles TODO: better way to do this?
			for (int x = 0; x < 5; x++)
			{
				rectangleRGBA(this->renderer, d.x + x, d.y + x, d.x + d.w - x, d.y + d.h - x, 0x66 - x * 10, 0x7c + x * 20, 0x89 + x * 10, 0xFF);
			}
		}
	}
}

void Element::recalcPosition(Element* parent) {
	// calculate absolute x/y positions
	if (parent)
	{
		this->xAbs = parent->xAbs + this->x;
		this->yAbs = parent->yAbs + this->y;
	} else {
		this->xAbs = this->x;
		this->yAbs = this->y;
	}
}

void Element::position(int x, int y)
{
	this->x = x;
	this->y = y;
}

bool Element::onTouchDown(InputEvents* event)
{
	if (!event->isTouchDown())
		return false;

	if (!event->touchIn(this->xAbs, this->yAbs, this->width, this->height))
		return false;

	// mouse pushed down, set variable
	this->dragging = true;
	this->lastMouseY = event->yPos;
	this->lastMouseX = event->xPos;

	// turn on deep highlighting during a touch down
	if (this->touchable)
		this->elasticCounter = DEEP_HIGHLIGHT;

	return true;
}

bool Element::onTouchDrag(InputEvents* event)
{
	bool ret = false;

	if (!event->isTouchDrag())
		return false;

	// minimum amount of wiggle allowed by finger before calling off a touch event
	int TRESHOLD = 40 / SCALER / SCALER;

	// we've dragged out of the icon, invalidate the click by invoking onTouchUp early
	// check if we haven't drifted too far from the starting variable (treshold: 40)
	if (this->dragging && (abs(event->yPos - this->lastMouseY) >= TRESHOLD || abs(event->xPos - this->lastMouseX) >= TRESHOLD))
	{
		ret |= (this->elasticCounter > 0);
		this->elasticCounter = NO_HIGHLIGHT;
	}

	// ontouchdrag never decides whether to update the view or not
	return ret;
}

bool Element::onTouchUp(InputEvents* event)
{
	if (!event->isTouchUp())
		return false;

	bool ret;

	// ensure we were dragging first (originally checked the treshold above here, but now that actively invalidates it)
	if (this->dragging)
	{
		// check that this click is in the right coordinates for this square
		// and that a subscreen isn't already being shown
		// TODO: allow buttons to activae this too?
		if (event->touchIn(this->xAbs, this->yAbs, this->width, this->height))
		{
			// elasticCounter must be nonzero to allow a click through (highlight must be shown)
			if (this->elasticCounter > 0 && action != NULL)
			{
				// invoke this element's action
				this->action();
				ret |= true;
			}
		}
	}

	// release mouse
	this->dragging = false;

	// update if we were previously highlighted, cause we're about to remove it
	ret |= (this->elasticCounter > 0);

	this->elasticCounter = 0;

	return ret;
}

void Element::setCST(CST_Renderer *renderer, CST_Window *window) {
	this->renderer = renderer;
	this->window = window;
	for (Element* child : this->elements) {
		child->setCST(renderer, window);
	}
}

void Element::append(Element *element)
{
	auto position = std::find(elements.begin(), elements.end(), element);
	if (position == elements.end()) {
		elements.push_back(element);
		element->setCST(this->renderer, this->window);
	}
}

void Element::remove(Element *element)
{
	auto position = std::find(elements.begin(), elements.end(), element);
	if (position != elements.end())
		elements.erase(position);
}

void Element::removeAll(void)
{
	elements.clear();
}
