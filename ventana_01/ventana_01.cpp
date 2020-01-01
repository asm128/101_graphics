#include <Windows.h>
#include <cstdint>

// Callback for window events (required to handle window events such as click or closing the window)
LRESULT WINAPI						WndProc					(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_DESTROY:	// Catch the window's DESTROY message which is posted to the queue when the window is closed.
		PostQuitMessage(0);	// Signal the WM_QUIT message.
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Use this structure to simplify the use of 2d values.
struct SCoord {
	int							x;
	int							y;

	// This operator allows us to scale the vector components in a single step
	SCoord						operator /				(double scalar)		const	{ return SCoord{(int)(x / scalar), (int)(y / scalar)}; }
};

// Use this structure to simplify the use of color values.
struct SColor {
	uint8_t						b;
	uint8_t						g;
	uint8_t						r;
	uint8_t						a;

	// These operators allow us to scale the colors in a single step
	SColor						operator /				(double scalar)		const	{ return SColor{(uint8_t)(b / scalar), (uint8_t)(g / scalar), (uint8_t)(r / scalar)}; }
	SColor						operator *				(double scalar)		const	{ return SColor{(uint8_t)(b * scalar), (uint8_t)(g * scalar), (uint8_t)(r * scalar)}; }

};

// Use this to convert from our color format to MS Windows' format.
//   a b g r
// 0x00FF0000;
//   a r g b
// 0x00FF0000;
COLORREF					toColorRef				(SColor color)				{ return ((int)color.r) | (((int)color.g) << 8) | (((int)color.b) << 16); }

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
	HWND							newWindow				= CreateWindowExA(0, className, "Ventana 0", WS_OVERLAPPEDWINDOW, 100, 100, windowSize.x, windowSize.y, 0, 0, hInstance, 0);
	ShowWindow(newWindow, nShowCmd);	// The window is hidden by default. Show the window with the SW_SHOW parameter.

	// ----- Set some variables to enable changing the shape of things during execution
	SCoord							position				= windowSize / 2;
	SColor							color					= {0, 0, 0xFF};
	double							factor					= 1;

	bool							running					= true;	// Use this variable to signal when the window is closed
	while(running) {	// Loop until the window is closed
		MSG								msg						= {};
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {	// Retrieve the first message from the event queue
			TranslateMessage(&msg);
			DispatchMessage(&msg);	// This function causes our WndProc() function to be called with the message as parameter
			if(msg.message == WM_QUIT)	// This message is sent by PostQuitMessage(0) used in the window event processor
				running						= false;
		}

		// ----- Change some parameters on each iteration (also called "frame") to animate the resulting shapes.
		++position.x;
		if(position.x >= windowSize.x)
			position.x					= 0;
		factor						-= .01;
		if(factor < 0)
			factor						= 1;

		// ----- Draw some pixels on the window's client area.
		HDC								dc						= GetDC(newWindow);
		for(int y = 0; y < 5; ++y) // Draw a vertical line with a modified color value.
			SetPixel(dc, position.x, position.y + y, toColorRef(color * factor));
		ReleaseDC(newWindow, dc);
		Sleep(10);	// Add some delay before moving on to the next iteration.
	}
	UnregisterClass(className, hInstance);
	return EXIT_SUCCESS;
}
