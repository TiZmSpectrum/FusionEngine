/*************************************************************************/
/*  context_gl_win.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#if defined(OPENGL_ENABLED) || defined(LEGACYGL_ENABLED) || defined(GLES2_ENABLED)

//
// C++ Implementation: context_gl_x11
//
// Description: 
//
//
// Author: Juan Linietsky <reduzio@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#define WINVER 0x0500
#include "context_gl_win.h"

//#include "drivers/opengl/glwrapper.h"
//#include "ctxgl_procaddr.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB  0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB  0x2092
#define WGL_CONTEXT_FLAGS_ARB          0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002

typedef HGLRC (APIENTRY* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);


void ContextGL_Win::release_current() {


	wglMakeCurrent(hDC,NULL);

}


void ContextGL_Win::make_current() {

	wglMakeCurrent(hDC,hRC);
}

int ContextGL_Win::get_window_width() {

	return OS::get_singleton()->get_video_mode().width;
}

int ContextGL_Win::get_window_height() {

	return OS::get_singleton()->get_video_mode().height;
}

void ContextGL_Win::swap_buffers() {

	SwapBuffers(hDC);
}

/*
static GLWrapperFuncPtr wrapper_get_proc_address(const char* p_function) {

	print_line(String()+"getting proc of: "+p_function);
	GLWrapperFuncPtr func=(GLWrapperFuncPtr)get_gl_proc_address(p_function);
	if (!func) {
		print_line("Couldn't find function: "+String(p_function));
		print_line("error: "+itos(GetLastError()));
	}
	return func;

}
*/


Error ContextGL_Win::initialize() {

	static	PIXELFORMATDESCRIPTOR pfd= {
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,	
		24,
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,// No Alpha Buffer
		0,// Shift Bit Ignored
		0,// No Accumulation Buffer
		0, 0, 0, 0,// Accumulation Bits Ignored
		24,// 24Bit Z-Buffer (Depth Buffer)  
		0,// No Stencil Buffer
		0,// No Auxiliary Buffer
		PFD_MAIN_PLANE, // Main Drawing Layer
		0,// Reserved
		0, 0, 0	// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd))) {
		MessageBoxA(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE;								// Return FALSE
	}

	if (!(pixel_format=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		MessageBoxA(NULL,"Can't Find A Suitable pixel_format.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,pixel_format,&pfd))		// Are We Able To Set The Pixel Format?
	{
		MessageBoxA(NULL,"Can't Set The pixel_format.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		MessageBoxA(NULL,"Can't Create A Temporary GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE;								// Return FALSE
	}

	wglMakeCurrent(hDC,hRC);

	if (opengl_3_context) {

		int attribs[] = {
			 WGL_CONTEXT_MAJOR_VERSION_ARB, 3,//we want a 3.1 context
			 WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			 //and it shall be forward compatible so that we can only use up to date functionality
			 WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0}; //zero indicates the end of the array

		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL; //pointer to the method
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

		if(wglCreateContextAttribsARB == NULL) //OpenGL 3.0 is not supported
		{
			MessageBoxA(NULL,"Cannot get Proc Adress for CreateContextAttribs", "ERROR",MB_OK|MB_ICONEXCLAMATION);
			wglDeleteContext(hRC);
			return ERR_CANT_CREATE;
		}

		HGLRC new_hRC;
		if (!(new_hRC=wglCreateContextAttribsARB(hDC,0, attribs)))
		{
			wglDeleteContext(hRC);
			MessageBoxA(NULL,"Can't Create An OpenGL 3.1 Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			return ERR_CANT_CREATE;								// Return false
		}
		wglMakeCurrent(hDC,NULL);
		wglDeleteContext(hRC);
		hRC=new_hRC;

		if (!wglMakeCurrent(hDC,hRC)) 				// Try To Activate The Rendering Context
		{
			MessageBoxA(NULL,"Can't Activate The GL 3.1 Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			return ERR_CANT_CREATE;							// Return FALSE
		}

		printf("Activated GL 3.1 context");
	}


//	glWrapperInit(wrapper_get_proc_address);

	return OK;
}

ContextGL_Win::ContextGL_Win(HWND hwnd,bool p_opengl_3_context) {

	opengl_3_context=p_opengl_3_context;
	hWnd=hwnd;
}

ContextGL_Win::~ContextGL_Win() {


}


#endif
