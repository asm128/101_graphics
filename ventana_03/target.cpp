#include <Windows.h>
#include "framework.h"

int							drawBuffer							(::HDC hdc, const SColor* target, SCoord sizeTarget)				{
	struct SOffscreenDetail { // raii destruction of resources (calls free() and DeleteObject() when the drawBuffer() ends and offscreenDetail goes out of scope)
		uint32_t					BitmapInfoSize					= 0;
		::BITMAPINFO				* BitmapInfo					= 0;
		::HDC						IntermediateDeviceContext		= 0;    // <- note, we're creating, so it needs to be destroyed (not released)
		::HBITMAP					IntermediateBitmap				= 0;

									~SOffscreenDetail				()				{
			if(BitmapInfo					) ::free			(BitmapInfo					);
			if(IntermediateBitmap			) ::DeleteObject	(IntermediateBitmap			);
			if(IntermediateDeviceContext	) ::DeleteDC		(IntermediateDeviceContext	);
		}
	}																	offscreenDetail								= {};
	// We need to allocate some memory in order to rearrange our lines because our coordinate system is inverted
	const uint32_t														bytesToCopy									= sizeof(::RGBQUAD) * sizeTarget.x * sizeTarget.y;
	offscreenDetail.BitmapInfoSize									= sizeof(::BITMAPINFO) + bytesToCopy;
	offscreenDetail.BitmapInfo										= (::BITMAPINFO*)::malloc(offscreenDetail.BitmapInfoSize);

	//memcpy(&offscreenDetail.BitmapInfo->bmiColors[1], colorArray[0].begin(), metricsSource.x * metricsSource.y * sizeof(::gpk::SColorBGRA));
	for(uint32_t y = 0, yMax = sizeTarget.y; y < yMax; ++y)	// This inverts the Y axis of the source by copying to temporary memory which is later sent to the temporary bitmap trough SetDIBits().
		memcpy(&offscreenDetail.BitmapInfo->bmiColors[1 + y * sizeTarget.x], &target[(sizeTarget.y - 1 - y) * sizeTarget.x], sizeTarget.x * sizeof(SColor));

	// Fill this structure required for Windows to understand the input format of our bitmap
	offscreenDetail.BitmapInfo->bmiHeader							= {sizeof(::BITMAPINFO)};
	offscreenDetail.BitmapInfo->bmiHeader.biWidth					= sizeTarget.x;
	offscreenDetail.BitmapInfo->bmiHeader.biHeight					= sizeTarget.y;
	offscreenDetail.BitmapInfo->bmiHeader.biPlanes					= 1;
	offscreenDetail.BitmapInfo->bmiHeader.biBitCount				= 32;
	offscreenDetail.BitmapInfo->bmiHeader.biCompression				= BI_RGB;
	offscreenDetail.BitmapInfo->bmiHeader.biSizeImage				= bytesToCopy;
	offscreenDetail.BitmapInfo->bmiHeader.biXPelsPerMeter			= 0x0ec4; // Paint and PSP use these values.
	offscreenDetail.BitmapInfo->bmiHeader.biYPelsPerMeter			= 0x0ec4; // Paint and PSP use these values.

	offscreenDetail.IntermediateDeviceContext						= ::CreateCompatibleDC(hdc);    // <- note, we're creating, so it needs to be destroyed
	if(offscreenDetail.IntermediateDeviceContext) {
		char																* ppvBits									= 0;
		offscreenDetail.IntermediateBitmap								= ::CreateDIBSection(offscreenDetail.IntermediateDeviceContext, offscreenDetail.BitmapInfo, DIB_RGB_COLORS, (void**) &ppvBits, NULL, 0);	// Create intermediate bitmap
		if(offscreenDetail.IntermediateBitmap) { // We need to avoid this if CreateDIBSection() failed.
			::SetDIBits(NULL, offscreenDetail.IntermediateBitmap, 0, sizeTarget.y, &offscreenDetail.BitmapInfo->bmiColors[1], offscreenDetail.BitmapInfo, DIB_RGB_COLORS);
			::HBITMAP															hBmpOld										= (::HBITMAP)::SelectObject(offscreenDetail.IntermediateDeviceContext, offscreenDetail.IntermediateBitmap);    // <- altering state
			::BitBlt(hdc, 0, 0, sizeTarget.x, sizeTarget.y, offscreenDetail.IntermediateDeviceContext, 0, 0, SRCCOPY);
			::SelectObject(hdc, hBmpOld);	// put the old bitmap back in the DC (restore state).
		}
	}
	return 0;
}
