#include <Windows.h>
#include "framework.h"

// Callback for window events (required to handle window events such as click or closing the window)
LRESULT WINAPI						WndProc					(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_DESTROY:	// Catch the window's DESTROY message which is posted to the queue when the window is closed.
		PostQuitMessage(0);	// Signal the WM_QUIT message.
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Use this to convert from our color format to MS Windows' format.
COLORREF					toColorRef				(SColor color)																		{ return ((int)color.r) | (((int)color.g) << 8) | (((int)color.b) << 16); }

// Assigns a color value to the target pixel array
void						setPixel				(SColor * target, SCoord sizeTarget, SCoord position, SColor color)					{
	if( position.x >= 0 && position.x < sizeTarget.x
	 && position.y >= 0 && position.y < sizeTarget.y
	)
		target[position.x + sizeTarget.x * position.y]	= color;
}

int							drawRectangle			(SColor * target, SCoord sizeTarget, SCoord position, SCoord size, SColor color)	{
	for(int y = 0; y < size.y; ++y)	// bajo un y
	for(int x = 0; x < size.x; ++x)	{ // dibujo todos los x
		SCoord							finalPosition			= {position.x + x, position.y + y};
		setPixel(target, sizeTarget, finalPosition, color);
	}
	return 0;
}

int							drawCircle				(SColor * target, SCoord sizeTarget, SCoord position, double radius, SColor color)	{
	for(int y = -(int)radius; y < radius; ++y)	// bajo un y
	for(int x = -(int)radius; x < radius; ++x)	{ // dibujo todos los x
		SCoord							testPosition			= {x, y};
		if(testPosition.Length() < radius) {
			SCoord							finalPosition			= {position.x + x, position.y + y};
			setPixel(target, sizeTarget, finalPosition, color);
		}
	}
	return 0;
}

// Copy pixels to window's client area.
int							drawTarget				(HDC dc, SColor * pixels, SCoord size)												{
	for(int y = 0; y < size.y; ++y)	// bajo un y
	for(int x = 0; x < size.x; ++x)	// dibujo todos los x
		SetPixel(dc, x, y, toColorRef(pixels[y * size.x + x]));	// Call the WinAPI SetPixel() function (which is very slow).
	return 0;
}

// Generate a 64-bit pseudorandom value.
uint64_t					noise					(uint64_t x, uint64_t noiseSeed = 16381)								noexcept	{
	x							= (x << 13) ^ x;
	x							= (x * (x * x * noiseSeed + 715827883ULL)  + 10657331232548839ULL);
	return x ^ (x >> 16);
}

int		WINAPI				WinMain
	( _In_		HINSTANCE		hInstance
	, _In_opt_	HINSTANCE		//hPrevInstance
	, _In_		LPSTR			//lpCmdLine
	, _In_		int				nShowCmd
	) {
	// Specify window class parameters.
	WNDCLASSEX						wndClass				= {sizeof(WNDCLASSEX)};
	const char						className	[]			= "ventana_main";
	wndClass.lpfnWndProc		= WndProc;
	wndClass.lpszClassName		= className;
	wndClass.hInstance			= hInstance;
	RegisterClassEx(&wndClass);	// Registers the class

	// Create the window
	SCoord							windowSize				= {640, 480};
	SCoord							windowPosition			= {100, 100};
	const char						windowTitle	[]			= "Ventana 0";

	HWND							newWindow				= CreateWindowExA(0, className, windowTitle, WS_OVERLAPPEDWINDOW
		, windowPosition.x
		, windowPosition.y
		, windowSize.x
		, windowSize.y
		, 0, 0, hInstance, 0
	);
	ShowWindow(newWindow, nShowCmd);	// The window is hidden by default. Show the window with the SW_SHOW parameter.

	bool							running					= true;

	// ----- Set some variables to enable changing the shape of things during execution
	SColor							color					= {0, 0, 0xFF};
	double							factor					= 1;
	SCoord							rectanglePosition		= {100, 100};	//
	SCoord							circlePosition			= {400, 400};	//
	int								circleRadius			= 64;

	uint64_t						iStep					= 0;	// The iteration step of the main loop
	SColor							* targetPixels			= 0;	// Store here the memory address of our local pixel area
	while(running) {	// Loop until the window is closed
		MSG								msg						= {};
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {	// Retrieve the first message from the event queue
			TranslateMessage(&msg);
			DispatchMessage(&msg);	// This function causes our WndProc() function to be called with the message as parameter
			if(msg.message == WM_QUIT)	// This message is sent by PostQuitMessage(0) used in the window event processor
				running						= false;	// Stop execution by ending the main loop
		}

		// ----- Adjust target pixels memory size to match the window size in case it's changed.
		RECT							rc						= {};
		GetClientRect(newWindow, &rc);
		if(windowSize.x != rc.right || windowSize.y != rc.bottom) {
			windowSize					= {rc.right, rc.bottom};
			free(targetPixels);	// Free the previous memory used for drawing (if any).
			targetPixels				= (SColor*)malloc(windowSize.x * (uint64_t)windowSize.y * sizeof(SColor));	// Get memory for drawing.
		}
		if(0 == targetPixels)
			return EXIT_FAILURE;

		// ----- Change some parameters on each iteration (also called "frame") to animate the resulting shapes.
		factor						+= .05;
		if(factor > 1)
			factor						= 0;

		circlePosition.x			+= int(10 * factor);
		if(circlePosition.x >= windowSize.x)
			circlePosition.x			= -circleRadius * 2;

		// ----- Draw random noise background.
		const int						pixelCount				= windowSize.x * windowSize.y;
		for(uint64_t iPixel = 0; iPixel < pixelCount; ++iPixel)
			targetPixels[iPixel]		=
				{ (uint8_t)(noise(iStep * pixelCount * 3 + iPixel * 3 + 0)) // Don't care about this formula right now
				, (uint8_t)(noise(iStep * pixelCount * 3 + iPixel * 3 + 1)) // But essentially what it does is to call noise() making sure it gets a different parameter value on each iteration,
				, (uint8_t)(noise(iStep * pixelCount * 3 + iPixel * 3 + 2)) // for each frame, during the entire execution of the program.
				};

		// ----- Draw some shapes to our in-memory image.
		drawRectangle	(targetPixels, windowSize, rectanglePosition	, {(int)(256 * factor), 8}, color * factor);
		drawCircle		(targetPixels, windowSize, circlePosition		, circleRadius * factor, color * factor);

		// ----- Copy the resulting in-memory image to the client area memory of the window.
		HDC								dc						= GetDC(newWindow);	// Get the window's Device Context, which provides access to the drawing device and to the client area of the window.
		drawBuffer		(dc, targetPixels, windowSize);	// Copy the target to the window's client area (fast version).
		//drawTarget	(dc, targetPixels, windowSize);	// Copy the target to the window's client area (slow version).
		ReleaseDC(newWindow, dc);	// The device context needs to be unlocked after use.

		++iStep;	// Increase our frame counter.
	}

	free(targetPixels);	// Free the memory used for drawing

	UnregisterClass(className, hInstance);
	return EXIT_SUCCESS;
}
