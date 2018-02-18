///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file theApp.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 应用程序框架
///
///////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
// Standard wxWidgets headers
//-----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#	pragma hdrstop
#endif

// For all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "wx/image.h"               // wxImage

//-----------------------------------------------------------------------------
// Header of this .cpp file
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "app.h"
#include "MainFrame.h"
#include "PythonScript.h"

#include "MessageHandler.h"


using namespace xgc::net;
using namespace xgc::message;

IMPLEMENT_APP(theApp)

theApp::theApp()
	: mMessageQueue( xgc_nullptr )
	, mServerHandle( INVALID_NETWORK_HANDLE )
{
}

theApp::~theApp()
{
}

bool theApp::Initialize( int & argc, wxChar ** argv )
{
	if( false == __super::Initialize( argc, argv ) )
		return false;

	if( !CreateNetwork( eNetLibrary::asio ) )
		return false;

	if( !InitNetwork( 4 ) )
		return false;

	if( !init_filewatcher( 0 ) )
		return false;

	if( !getScript().Init() )
		return false;

	InitProcessor();

	mServerHandle = StartServer( "0.0.0.0", 20375, ProtocalDefault, 0, 0, 10, mMessageQueue );

	wxASSERT_MSG( mServerHandle, "server start error" );

	return true;
}

bool theApp::OnInit()
{
	if ( !wxApp::OnInit() )
		return false;

	// If there is any of a certain format of image in the xrcs, then first
	// load a handler for that image type. This example uses XPMs & a gif, but
	// if you want PNGs, then add a PNG handler, etc. See wxImage::AddHandler()
	// documentation for the types of image handlers available.
	wxImage::AddHandler(XGC_NEW wxXPMHandler);
	wxImage::AddHandler(XGC_NEW wxGIFHandler);

	// Make an instance of your derived frame. Passing NULL (the default value
	// of MyFrame's constructor is NULL) as the frame doesn't have a parent
	// since it is the main application window.
	CMainFrame *frame = XGC_NEW CMainFrame();

	// Show the frame as it's created initially hidden.
	frame->Show(true);

	return true;
}

int theApp::OnExit()
{
	return __super::OnExit();
}

void theApp::CleanUp()
{
	CloseServer( mServerHandle );

	getScript().Fini();

	fini_filewatcher();

	FiniNetwork();
	DestroyNetwork();

	__super::CleanUp();
}

bool theApp::ProcessIdle()
{
	DeliverMessage( mMessageQueue, 100 );
	do_filewatcher_notify();
	return __super::ProcessIdle();
}