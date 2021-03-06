// This file is part of PinballY
// Copyright 2018 Michael J Roberts | GPL v3 or later | NO WARRANTY
//
// Base frame window.  This is the base class for our top-level
// windows.

#pragma once

#include <unordered_set>
#include "BaseWin.h"
#include "BaseView.h"

class ViewWin;

class FrameWin : public BaseWin
{
public:
	FrameWin(const TCHAR *configVarPrefix, int iconId, int grayIconId);

	// is the window activated?
	bool IsActivated() const { return isActivated; }

	// are we in full-screen mode?
	bool IsFullScreen() const { return fullScreenMode; }

	// toggle between regular and full-screen mode
	void ToggleFullScreen();

	// Show/hide the frame window.  This updates the window's UI
	// visibility and saves the config change.
	void ShowHideFrameWindow(bool show);

	// Restore visibility from the saved configuration settings
	void RestoreVisibility();

	// Create our window, loading the settings from the configuration
	bool CreateWin(HWND parent, int nCmdShow, const TCHAR *title);

	// get my view window
	BaseWin *GetView() const { return view; }

	// update my menu
	virtual void UpdateMenu(HMENU hMenu, BaseWin *fromWin) override;

protected:
	virtual ~FrameWin();

	// Borderless mode: if set, we hide the title bar and sizing
	// borders.
	virtual bool IsBorderless() const { return false; }

	// Is this a hideable window?  If true, we'll hide the window on
	// a Minimize or Close command, instead of actually minimizing or
	// closing it.  Most of our secondary windows (backglass, DMD,
	// topper) use this behavior.
	virtual bool IsHideable() const { return false; }

	// Create my view window child.  Subclasses must override this
	// to create the appropriate view window type.
	virtual BaseView *CreateViewWin() = 0;

	// Customize the system menu.  This does nothing by default.
	// Subclasses can override this to add custom commands to the
	// window's system menu.
	virtual void CustomizeSystemMenu(HMENU);

	// Copy the given context menu to the system menu, excluding
	// the given commands.
	void CopyContextMenuToSystemMenu(HMENU contextMenu, HMENU systemMenu, 
		const std::unordered_set<UINT> &excludeCommandIDs);

	// add an item to the system menu
	void AddSystemMenu(HMENU m, int cmd, int idx);

	// get the initial window position
	virtual RECT GetCreateWindowPos(int &nCmdShow) override;

	// initialize
	virtual bool InitWin() override;

	// register my window class
	virtual const TCHAR *RegisterClass() override;

	// window proc
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam) override;

	// window creation
	virtual bool OnCreate(CREATESTRUCT *cs) override;

	// close the window
	virtual bool OnClose() override;

	// window activation
	virtual bool OnNCActivate(bool active, HRGN updateRegion) override;
	virtual bool OnActivate(int waCode, int minimized, HWND hWndOther) override;

	// erase the background
	virtual bool OnEraseBkgnd(HDC) override { return 0; }

	// paint the window contents
	virtual void OnPaint(HDC hdc) override;

	// size/move
	virtual void OnResize(int width, int height) override;
	virtual void OnMove(POINT pos) override;

	// get min/max window size
	virtual bool OnGetMinMaxInfo(MINMAXINFO *mmi) override;

	// paint the custom frame caption
	void PaintCaption(HDC hdc);

	// Non-client mouse clicks
	virtual bool OnNCMouseButtonUp(int button, UINT hit, POINT pt) override;

	// handle a command
	virtual bool OnCommand(int cmd, int source, HWND hwndControl) override;
	virtual bool OnSysCommand(WPARAM wParam, LPARAM lParam) override;

	// command command handler
	bool DoCommand(int cmd);

	// We use the DWM "extend frame into client" mechanism to take
	// over the entire frame as client area and do custom drawing.
	// This means we can simply return from WM_NCCALCSIZE with the
	// parameter rectangles unchanged.
	virtual bool OnNCCalcSize(bool validateClientRects, NCCALCSIZE_PARAMS *, UINT&) override;

	// Non-client hit testing.  This handles a WM_NCHITTEST in the
	// frame.  If the caller is using custom frame drawing, it needs
	// to test for hits to the caption bar, system menu icon, and
	// sizing borders.
	virtual bool OnNCHitTest(POINT ptMouse, UINT &hit) override;

	// initialize a menu popup
	virtual bool OnInitMenuPopup(HMENU hMenu, int itemPos, bool isWinMenu) override;

	// save the current window position in the config
	void WindowPosToConfig();

	// update the view layout
	virtual void UpdateLayout();

	// private window class messages (WM_USER to WM_APP-1)
	virtual bool OnUserMessage(WPARAM wParam, LPARAM lParam) { return false; }

	// Figure the frame parameters.  This is called when the window is
	// created, activated, or resized, so that we can update the custom
	// frame size element calculations.
	virtual void FigureFrameParams();

	// Normal window placement and style.  When we switch to full-screen mode, 
	// we store the current window style and placement here so that we can 
	// restore them when switching back to windowed mode.
	WINDOWPLACEMENT normalWindowPlacement;
	DWORD normalWindowStyle;

	// show the system menu
	void ShowSystemMenu(int x, int y);

	// Get the bounding rectangle of the nth monitor (n >= 1), in desktop
	// window coordinates.  Returns true if the given monitor was found,
	// false if no such monitor exists.
	bool GetDisplayMonitorCoords(int n, RECT &rc);

	// main view window
	RefPtr<BaseView> view;

	// window icons, for the active and inactive window states
	HICON icon;
	HICON grayIcon;

	// Custom frame parameters
	//
	// 'dwmExtended' tells us if we succeeded in extended the window frame 
	// into the client area, which we do at window activation time.  If this
	// is true, we take over the whole window rect as the client area and 
	// draw our custom caption, otherwise we let Windows handle the caption
	// via the normal non-caption size calculation and painting.
	//
	// 'frameBorders' gives the size of the border area we draw within the
	// client area.  In principle, we could use this to draw all of the 
	// non-client controls (caption and sizing border) in the client space,
	// but for compatibility with Windows 10, we can only safely use this
	// to draw the caption.  So the left, bottom, and right elements are
	// always zero in our implementation; only the top is actually used.
	// We nonetheless keep the whole rectangle, for greater flexibility if
	// we should later want to change this to do custom sizing frame drawing
	// for the older themes that have normal non-client-area frames.
	//
	// 'captionOfs' is the offset of the caption area from the top left
	// of the window rect, taking into account the sizing borders on the
	// top and left edges, if any.  (These are removed in maximized and
	// full-screen modes, for example.)
	//
	bool dwmExtended;			// frame extended into client with DWM
	RECT frameBorders;          // frame border widths for DWM
	POINT captionOfs;           // caption offset from top left

	// window icon size
	SIZE szIcon;

	// current mode - windowed or full-screen
	bool fullScreenMode;

	// is the window currently activated?
	bool isActivated;

	// window closed
	bool closed;

	// configuration variables
	TSTRINGEx configVarPos;
	TSTRINGEx configVarMaximized;
	TSTRINGEx configVarMinimized;
	TSTRINGEx configVarFullScreen;
	TSTRINGEx configVarVisible;

	// window subclass registration
	static const TCHAR *frameWinClassName;
	static bool frameWinClassRegistered;
	static ATOM frameWinClassAtom;
};
