/*********************************************************************
	wildWeasel.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wildWeasel.h"

#pragma region timer
//-----------------------------------------------------------------------------
// Name: timer::start()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::timer::setFunc(masterMind* ww, void timerFunc(void* pUser), void* pUser)
{
	this->timerFunc	= timerFunc;
	this->pUser		= pUser;
	this->ww		= ww;
}

//-----------------------------------------------------------------------------
// Name: timer::start()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::timer::start(unsigned int milliseconds)
{
	if (!ww) return;
	if (timerId && ww) {
		KillTimer(ww->getHwnd(), timerId);
		timerId = 0;
	}
	this->milliseconds	= milliseconds;
	this->timerId 		= SetTimer(ww->getHwnd(), (UINT_PTR) this, milliseconds, (TIMERPROC) timer::TimerProc);
}

//-----------------------------------------------------------------------------
// Name: timer::start()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::timer::start(masterMind* ww, void timerFunc(void* pUser), void* pUser, unsigned int milliseconds)
{
	if (ww == nullptr || timerFunc == nullptr) return;
	if (timerId && this->ww) {
		KillTimer(this->ww->getHwnd(), timerId);
		timerId = 0;
	}
	this->timerFunc		= timerFunc;
	this->pUser			= pUser;
	this->ww			= ww;
	this->milliseconds	= milliseconds;
	timerId				= SetTimer(ww->getHwnd(), (UINT_PTR) this, milliseconds, (TIMERPROC) timer::TimerProc);
}

//-----------------------------------------------------------------------------
// Name: timer::pause()
// Desc:
//-----------------------------------------------------------------------------
void wildWeasel::timer::pause()
{
	if (timerId && ww) {
		KillTimer(ww->getHwnd(), timerId);
		timerId = 0;
	}
}

//-----------------------------------------------------------------------------
// Name: timer::resume()
// Desc:
//-----------------------------------------------------------------------------
void wildWeasel::timer::resume()
{
	if (!timerId && ww) {
		timerId = SetTimer(ww->getHwnd(), (UINT_PTR) this, this->milliseconds, (TIMERPROC) timer::TimerProc);
	}
}

//-----------------------------------------------------------------------------
// Name: timer::timer()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::timer::timer()
{
}

//-----------------------------------------------------------------------------
// Name: timer::isActive()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::timer::isActive()
{
	return (timerId != 0);
}

//-----------------------------------------------------------------------------
// Name: timer::terminate()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::timer::terminate()
{
	if (timerId && ww) {
		KillTimer(ww->getHwnd(), timerId);
		timerId = 0;
	}
}

//-----------------------------------------------------------------------------
// Name: timer::~timer()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::timer::~timer()
{
	terminate();
}

//-----------------------------------------------------------------------------
// Name: TimerProc()
// Desc: 
//-----------------------------------------------------------------------------
VOID CALLBACK wildWeasel::timer::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	wildWeasel::timer* me = (wildWeasel::timer*) idEvent;
	me->timerFunc(me->pUser);
}

// //-----------------------------------------------------------------------------
// // Name: startTimer()
// // Desc: 
// //-----------------------------------------------------------------------------
// void wildWeasel::startTimer(wildWeasel::timer& theTimer, void timerFunc(void* pUser), void* pUser, unsigned int milliseconds)
// {
// 	timer* aNewTimer = new timer(this, timerFunc, pUser, milliseconds);
// 	userTimers.push_back(aNewTimer);
// }
// 
// //-----------------------------------------------------------------------------
// // Name: terminateTimer()
// // Desc: 
// //-----------------------------------------------------------------------------
// void wildWeasel::terminateTimer(timer* theTimer)
// {
// 	userTimers.remove(theTimer);
// 	delete theTimer;
// }
#pragma endregion

#pragma region eventFollower

list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_keyDown;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_mouseMoved;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_verticalWheelMoved;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_horizontalWheelMoved;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_leftMouseButtonPressed;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_leftMouseButtonReleased;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_rightMouseButtonPressed;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_rightMouseButtonReleased;
list<wildWeasel::eventFollower*>	wildWeasel::eventFollower::list_windowSizeChanged;

//-----------------------------------------------------------------------------
// Name: ~eventFollower()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::eventFollower::~eventFollower()
{
	list_keyDown					.remove(this);
	list_mouseMoved					.remove(this);
	list_verticalWheelMoved			.remove(this);
	list_horizontalWheelMoved		.remove(this);
	list_leftMouseButtonPressed		.remove(this);
	list_leftMouseButtonReleased	.remove(this);
	list_rightMouseButtonPressed	.remove(this);
	list_rightMouseButtonReleased	.remove(this);
	list_windowSizeChanged			.remove(this);
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::eventFollower::followEvent()
// Desc: 
//-----------------------------------------------------------------------------
// void wildWeasel::addEventFollower(eventFollower* newFollower, eventType type)
void wildWeasel::eventFollower::followEvent(eventFollower* newFollower, eventType type)
{
	switch (type)
	{
	case eventType::KEYDOWN:					/*followingObjects.*/list_keyDown					.push_back(newFollower); break;
	case eventType::LEFT_MOUSEBUTTON_PRESSED:	/*followingObjects.*/list_leftMouseButtonPressed	.push_back(newFollower); break;
	case eventType::LEFT_MOUSEBUTTON_RELEASED:	/*followingObjects.*/list_leftMouseButtonReleased	.push_back(newFollower); break;
	case eventType::MOUSEMOVED:					/*followingObjects.*/list_mouseMoved				.push_back(newFollower); break;
	case eventType::VERTICAL_WHEEL_MOVED:		/*followingObjects.*/list_verticalWheelMoved		.push_back(newFollower); break;
	case eventType::HORIZONTAL_WHEEL_MOVED:		/*followingObjects.*/list_horizontalWheelMoved		.push_back(newFollower); break;
	case eventType::RIGHT_MOUSEBUTTON_PRESSED:	/*followingObjects.*/list_rightMouseButtonPressed	.push_back(newFollower); break;
	case eventType::RIGHT_MOUSEBUTTON_RELEASED: /*followingObjects.*/list_rightMouseButtonReleased	.push_back(newFollower); break;
	case eventType::WINDOWSIZE_CHANGED:			/*followingObjects.*/list_windowSizeChanged			.push_back(newFollower); break;
	}
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::eventFollower::forgetEvent()
// Desc: 
//-----------------------------------------------------------------------------
// void wildWeasel::removeEventFollower(eventFollower* newFollower, eventType type)
void wildWeasel::eventFollower::forgetEvent(eventFollower* newFollower, eventType type)
{
	switch (type)
	{
	case eventType::KEYDOWN:					/*followingObjects.*/list_keyDown					.remove(newFollower);	break;
	case eventType::LEFT_MOUSEBUTTON_PRESSED:	/*followingObjects.*/list_leftMouseButtonPressed	.remove(newFollower);	break;
	case eventType::LEFT_MOUSEBUTTON_RELEASED:	/*followingObjects.*/list_leftMouseButtonReleased	.remove(newFollower);	break;
	case eventType::MOUSEMOVED:					/*followingObjects.*/list_mouseMoved				.remove(newFollower);	break;
	case eventType::VERTICAL_WHEEL_MOVED:		/*followingObjects.*/list_verticalWheelMoved		.remove(newFollower);	break;
	case eventType::HORIZONTAL_WHEEL_MOVED:		/*followingObjects.*/list_horizontalWheelMoved		.remove(newFollower);	break;
	case eventType::RIGHT_MOUSEBUTTON_PRESSED:	/*followingObjects.*/list_rightMouseButtonPressed	.remove(newFollower);	break;
	case eventType::RIGHT_MOUSEBUTTON_RELEASED: /*followingObjects.*/list_rightMouseButtonReleased	.remove(newFollower);	break;
	case eventType::WINDOWSIZE_CHANGED:			/*followingObjects.*/list_windowSizeChanged			.remove(newFollower);	break;
	}
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::eventFollower::anySubItemFocused()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::eventFollower::anySubItemFocused()
{
	// anything selected at all?
	if (guiElemEvFol::um_focusedButton  == nullptr) return false;
	if (guiElemEvFol::um_focusedButton  == this)	return true;
	
	// check list of all mother elements of the focused item
	// ... return (find(...) != guiElemEvFol::um_focusedButton->motherFollowers.end());
	for (auto& curElement : guiElemEvFol::um_focusedButton->motherFollowers) {
		if (curElement == this) return true;
	}

	// no subitem focused
	return false;
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::eventFollower::anySubItemSelected()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::eventFollower::anySubItemSelected()
{
	// anything selected at all?
	if (guiElemEvFol::um_selectedButton == nullptr) return false;
	if (guiElemEvFol::um_selectedButton == this)	return true;
	
	// check list of all mother elements of the focused item
	// ... return (find(...) != guiElemEvFol::um_focusedButton->motherFollowers.end());
	for (auto& curElement : guiElemEvFol::um_selectedButton->motherFollowers) {
		if (curElement == this) return true;
	}

	// no subitem focused
	return false;
}
#pragma endregion

#pragma region fRECT

//-----------------------------------------------------------------------------
// Name: wildWeasel::fRECT::getRECT()
// Desc: 
//-----------------------------------------------------------------------------
const RECT wildWeasel::fRECT::getRECT()
{
	RECT rc = { (LONG) left, (LONG) top, (LONG) right, (LONG) bottom };

	return rc;
}
#pragma endregion

#pragma region color

wildWeasel::color wildWeasel::color::lightGreen	() { return color{ 0.5f, 1.0f, 0.5f, 1.0f }; }
wildWeasel::color wildWeasel::color::green		() { return color{ 0.0f, 1.0f, 0.0f, 1.0f }; };
wildWeasel::color wildWeasel::color::red		() { return color{ 1.0f, 0.0f, 0.0f, 1.0f }; };
wildWeasel::color wildWeasel::color::lightBlue	() { return color{ 0.5f, 0.5f, 1.0f, 1.0f }; };
wildWeasel::color wildWeasel::color::blue		() { return color{ 0.0f, 0.0f, 1.0f, 1.0f }; };
wildWeasel::color wildWeasel::color::white		() { return color{ 1.0f, 1.0f, 1.0f, 1.0f }; };
wildWeasel::color wildWeasel::color::black		() { return color{ 0.0f, 0.0f, 0.0f, 1.0f }; };
wildWeasel::color wildWeasel::color::gray		() { return color{ 0.4f, 0.4f, 0.4f, 1.0f }; };
wildWeasel::color wildWeasel::color::darkGray	() { return color{ 0.1f, 0.1f, 0.1f, 1.0f }; };

//-----------------------------------------------------------------------------
// Name: wildWeasel::color::limitBetweenZeroAndOne()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::color::limitBetweenZeroAndOne()
{
	if (r > 1) r = 1;	if (r < 0) r = 0;
	if (g > 1) g = 1;	if (g < 0) g = 0;
	if (b > 1) b = 1;	if (b < 0) b = 0;
	if (a > 1) a = 1;	if (a < 0) a = 0;
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::color::isSameAs()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::color::isSameAs(const color& c, float totalAllowedDelta)
{
	float totalDelta	= abs(r - c.r) 
						+ abs(g - c.g) 
						+ abs(b - c.b) 
						+ abs(a - c.a);
	
	return (totalDelta <= totalAllowedDelta);
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::color::getCOLORREF()
// Desc: 
//-----------------------------------------------------------------------------
const COLORREF wildWeasel::color::getCOLORREF()
{
	color c(r, g, b, a);

	c.limitBetweenZeroAndOne();

	return ((((LONG) (c.r * 255)) & 0x000000ff) <<  0)
		+  ((((LONG) (c.g * 255)) & 0x000000ff) <<  8)
		+  ((((LONG) (c.b * 255)) & 0x000000ff) << 16)
		+  ((((LONG) (c.a * 255)) & 0x000000ff) << 24);
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::color::rainbow()
// Desc: 
//-----------------------------------------------------------------------------
const wildWeasel::color wildWeasel::color::rainbow(float f)
{
	// locals
	color c;
	float		AbschnittLaenge = 1.0f / 6;

	// ... should be rewritten !!!
	#define	WERT_TO_X_UP(p)		((f - ((p)-1) * AbschnittLaenge) / AbschnittLaenge)
	#define	WERT_TO_X_DOWN(p)	(1 - ((f - ((p)-1) * AbschnittLaenge) / AbschnittLaenge))

	// Rot -> Lila
	if (f < 1 * AbschnittLaenge)		c = color(1, 0, WERT_TO_X_UP(1), 1); 
	
	// Lila -> Blau
	else if (f < 2 * AbschnittLaenge)	c = color(WERT_TO_X_DOWN(2), 0, 1, 1); 

	// Blau -> T�rkis
	else if (f < 3 * AbschnittLaenge)	c = color(0, WERT_TO_X_UP(3), 1, 1); 

	// T�rkis -> Gr�n
	else if (f < 4 * AbschnittLaenge)	c = color(0, 1, WERT_TO_X_DOWN(4), 1); 

	// Gr�n -> Gelb
	else if (f < 5 * AbschnittLaenge)	c = color(WERT_TO_X_UP(5), 1, 0, 1); 

	// Gelb -> Rot
	else if (f < 6 * AbschnittLaenge)	c = color(1, WERT_TO_X_DOWN(6), 0, 1); 

	return c;
}

#pragma endregion

/*** To Do's ********************************************************************************
- none
*********************************************************************************************/
