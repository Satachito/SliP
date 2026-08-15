//	SliP on Windows — the same screen the other apps have.
//	SwiftUI-CPP/ContentView.swift is the original, and this follows it: a mode
//	picker, a keep-session switch and Run across the top, the editor and the
//	symbol keypad above, the results below.
//
//	There is no bridge here.  The Mac, browser and Android hosts sit on the far
//	side of a language boundary and read the JSON that C++/Embed.hpp writes for
//	them; this host is C++, so it calls SessionRun and reads the list directly.
//
//	The boundary that does exist is the encoding one.  Win32 is UTF-16 and the
//	interpreter is UTF-8, so every string is converted at the edge — and the
//	conversion has to be the real one, because SliP's alphabet reaches past the
//	BMP: 𝑒 is U+1D452, a surrogate pair in UTF-16 and four bytes in UTF-8.

//	The Windows headers come first, and they have to.  JP.h opens with
//	`using namespace std;`, and rpcndr.h — reached from windows.h — declares
//	`typedef unsigned char byte`.  With std already in scope that name is
//	ambiguous against std::byte, and oaidl.h stops compiling on its own
//	declarations.  Parsed in this order there is no std to be ambiguous with.
#define	WIN32_LEAN_AND_MEAN
#define	NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include "Embed.hpp"

extern void Build();

//	────────────────────────────────────────────────  encoding

static std::wstring
Wide( std::string const& _ ) {
	if( _.empty() ) return {};
	auto n = MultiByteToWideChar( CP_UTF8, 0, _.data(), (int)_.size(), nullptr, 0 );
	std::wstring $( n, L'\0' );
	MultiByteToWideChar( CP_UTF8, 0, _.data(), (int)_.size(), $.data(), n );
	return $;
}

static std::string
Utf8( std::wstring const& _ ) {
	if( _.empty() ) return {};
	auto n = WideCharToMultiByte( CP_UTF8, 0, _.data(), (int)_.size(), nullptr, 0, nullptr, nullptr );
	std::string $( n, '\0' );
	WideCharToMultiByte( CP_UTF8, 0, _.data(), (int)_.size(), $.data(), n, nullptr, nullptr );
	return $;
}

//	────────────────────────────────────────────────  the window

//	The operators are most of the language and none of them are on a keyboard.
static wchar_t const* const SYMBOLS[] = {
	L"'", L"@", L"£", L"¶", L"¤", L"∅",
	L"×", L"÷", L"±", L"·", L"∈", L"∋",
	L"¿", L"¬", L"¡", L"¦", L"§", L"∥",
	L"⟨", L"⟩", L"«", L"»", L"\U0001D452", L"π",
};
static int const COLUMNS = 6;
static int const SYMBOL_COUNT = (int)( sizeof( SYMBOLS ) / sizeof( *SYMBOLS ) );

enum : int {
	ID_CALCULATOR = 100, ID_PROGRAMMING, ID_KEEP, ID_RUN,
	ID_EDITOR, ID_RESULTS, ID_SYMBOL = 200,
};

static HWND			gEditor, gResults, gCalculator, gProgramming, gKeep, gRun;
static HWND			gSymbol[ SYMBOL_COUNT ];
static HFONT		gMono, gUI;
static EmbedSession*	gSession;
static int			gDPI = 96;

static int	Scale( int _ ) { return MulDiv( _, gDPI, 96 ); }

static void
MakeFonts() {
	if( gMono ) DeleteObject( gMono );
	if( gUI )   DeleteObject( gUI );
	auto mono = [ & ]( int pt ) {
		return CreateFontW(
			-MulDiv( pt, gDPI, 72 ), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
			FIXED_PITCH | FF_MODERN, L"Consolas" );
	};
	gMono = mono( 11 );
	NONCLIENTMETRICSW m{ sizeof( m ) };
	SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof( m ), &m, 0 );
	m.lfMessageFont.lfHeight = -MulDiv( 9, gDPI, 72 );
	gUI = CreateFontIndirectW( &m.lfMessageFont );
}

static void
SetFonts() {
	for( auto h: { gCalculator, gProgramming, gKeep, gRun } )
		SendMessageW( h, WM_SETFONT, (WPARAM)gUI, TRUE );
	SendMessageW( gEditor,  WM_SETFONT, (WPARAM)gMono, TRUE );
	SendMessageW( gResults, WM_SETFONT, (WPARAM)gMono, TRUE );
	for( int i = 0; i < SYMBOL_COUNT; i++ )
		SendMessageW( gSymbol[ i ], WM_SETFONT, (WPARAM)gMono, TRUE );
}

static void
Layout( int W, int H ) {
	auto pad	= Scale( 8 );
	auto bar	= Scale( 30 );
	auto keyH	= Scale( 30 );
	auto keypad	= keyH * ( ( SYMBOL_COUNT + COLUMNS - 1 ) / COLUMNS ) + pad;

	auto y = pad;
	auto x = pad;
	auto seg = Scale( 96 );
	MoveWindow( gCalculator,  x, y, seg, bar, TRUE );	x += seg;
	MoveWindow( gProgramming, x, y, seg, bar, TRUE );	x += seg + pad * 2;
	MoveWindow( gKeep, x, y + Scale( 6 ), Scale( 108 ), bar, TRUE );
	MoveWindow( gRun, W - pad - Scale( 84 ), y, Scale( 84 ), bar, TRUE );

	y += bar + pad;
	auto rest	= H - y - pad - keypad;
	auto editH	= rest * 40 / 100;

	MoveWindow( gEditor, pad, y, W - pad * 2, editH, TRUE );
	y += editH + pad;

	auto keyW = ( W - pad * 2 ) / COLUMNS;
	for( int i = 0; i < SYMBOL_COUNT; i++ )
		MoveWindow(
			gSymbol[ i ],
			pad + ( i % COLUMNS ) * keyW, y + ( i / COLUMNS ) * keyH,
			keyW, keyH, TRUE
		);
	y += keypad;

	MoveWindow( gResults, pad, y, W - pad * 2, H - y - pad, TRUE );
}

//	────────────────────────────────────────────────  running

static void
Append( std::wstring const& _, COLORREF color, int points ) {
	CHARFORMAT2W cf{};
	cf.cbSize		= sizeof( cf );
	cf.dwMask		= CFM_COLOR | CFM_SIZE | CFM_FACE;
	cf.crTextColor	= color;
	cf.yHeight		= points * 20;			//	twips
	wcscpy( cf.szFaceName, L"Consolas" );
	SendMessageW( gResults, EM_SETSEL, (WPARAM)-1, (LPARAM)-1 );
	SendMessageW( gResults, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
	SendMessageW( gResults, EM_REPLACESEL, FALSE, (LPARAM)_.c_str() );
}

static void
Run() {
	if( !SendMessageW( gKeep, BM_GETCHECK, 0, 0 ) ) ResetEmbedSession( gSession );

	auto n = GetWindowTextLengthW( gEditor );
	std::wstring w( n, L'\0' );
	GetWindowTextW( gEditor, w.data(), n + 1 );

	auto calculator = SendMessageW( gCalculator, BM_GETCHECK, 0, 0 ) == BST_CHECKED;

	SendMessageW( gResults, WM_SETREDRAW, FALSE, 0 );
	SendMessageW( gResults, EM_SETSEL, 0, (LPARAM)-1 );
	SendMessageW( gResults, EM_REPLACESEL, FALSE, (LPARAM)L"" );

	for( auto const& entry: SessionRun( gSession, Utf8( w ), calculator ) ) {
		if( entry.source.size() )
			Append( Wide( entry.source ) + L"\r\n", RGB( 128, 128, 128 ), 8 );
		Append(
			Wide( entry.failed ? entry.error : entry.value ) + L"\r\n"
		,	entry.failed ? RGB( 192, 32, 32 ) : RGB( 0, 0, 0 )
		,	11
		);
	}

	SendMessageW( gResults, WM_SETREDRAW, TRUE, 0 );
	InvalidateRect( gResults, nullptr, TRUE );
}

//	Insert at the caret, the way the keypad does everywhere else.
static void
Insert( wchar_t const* _ ) {
	SendMessageW( gEditor, EM_REPLACESEL, TRUE, (LPARAM)_ );
	SetFocus( gEditor );
}

//	────────────────────────────────────────────────

static LRESULT CALLBACK
Proc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) {
	switch( msg ) {
	case WM_CREATE: {
		auto make = [ & ]( wchar_t const* cls, wchar_t const* text, DWORD style, int id ) {
			return CreateWindowExW(
				0, cls, text, WS_CHILD | WS_VISIBLE | style,
				0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)id, nullptr, nullptr );
		};
		gCalculator  = make( L"BUTTON", L"Calculator",
		                     BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP, ID_CALCULATOR );
		gProgramming = make( L"BUTTON", L"Programming",
		                     BS_AUTORADIOBUTTON | BS_PUSHLIKE, ID_PROGRAMMING );
		gKeep        = make( L"BUTTON", L"Keep session", BS_AUTOCHECKBOX, ID_KEEP );
		gRun         = make( L"BUTTON", L"Run", BS_DEFPUSHBUTTON, ID_RUN );
		SendMessageW( gCalculator, BM_SETCHECK, BST_CHECKED, 0 );

		gEditor = CreateWindowExW(
			WS_EX_CLIENTEDGE, L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
			0, 0, 0, 0, hwnd, (HMENU)ID_EDITOR, nullptr, nullptr );
		SetWindowTextW( gEditor,
			L"// Select Calculator, then press Ctrl+Enter.\r\n"
			L"1 + 2 × 3\r\ncosπ\r\n[1 2 3] + 10" );

		gResults = CreateWindowExW(
			WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, 0, 0, hwnd, (HMENU)ID_RESULTS, nullptr, nullptr );
		SendMessageW( gResults, EM_SETBKGNDCOLOR, 0, (LPARAM)GetSysColor( COLOR_WINDOW ) );

		for( int i = 0; i < SYMBOL_COUNT; i++ )
			gSymbol[ i ] = make( L"BUTTON", SYMBOLS[ i ], BS_PUSHBUTTON, ID_SYMBOL + i );

		MakeFonts();
		SetFonts();
		return 0;
	}
	case WM_SIZE:
		Layout( LOWORD( lp ), HIWORD( lp ) );
		return 0;

	case WM_DPICHANGED: {
		gDPI = HIWORD( wp );
		MakeFonts();
		SetFonts();
		auto r = (RECT*)lp;
		SetWindowPos( hwnd, nullptr, r->left, r->top,
		              r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE );
		return 0;
	}
	case WM_COMMAND: {
		auto id = LOWORD( wp );
		if( id == ID_RUN ) { Run(); return 0; }
		if( id >= ID_SYMBOL && id < ID_SYMBOL + SYMBOL_COUNT ) {
			Insert( SYMBOLS[ id - ID_SYMBOL ] );
			return 0;
		}
		return 0;
	}
	case WM_DESTROY:
		DeleteEmbedSession( gSession );
		PostQuitMessage( 0 );
		return 0;
	}
	return DefWindowProcW( hwnd, msg, wp, lp );
}

int WINAPI
wWinMain( HINSTANCE instance, HINSTANCE, PWSTR, int show ) {

	Build();
	gSession = NewEmbedSession();

	LoadLibraryW( L"Msftedit.dll" );			//	registers RICHEDIT50W
	INITCOMMONCONTROLSEX icc{ sizeof( icc ), ICC_STANDARD_CLASSES };
	InitCommonControlsEx( &icc );

	WNDCLASSEXW wc{};
	wc.cbSize			= sizeof( wc );
	wc.lpfnWndProc		= Proc;
	wc.hInstance		= instance;
	wc.hCursor			= LoadCursorW( nullptr, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)( COLOR_BTNFACE + 1 );
	wc.lpszClassName	= L"SliPWindow";
	wc.hIcon			= LoadIconW( instance, MAKEINTRESOURCEW( 1 ) );
	wc.hIconSm			= wc.hIcon;
	RegisterClassExW( &wc );

	auto hwnd = CreateWindowExW(
		0, wc.lpszClassName, L"SliP",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 760, 720,
		nullptr, nullptr, instance, nullptr );
	gDPI = GetDpiForWindow( hwnd );
	MakeFonts();
	SetFonts();
	{	RECT r; GetClientRect( hwnd, &r ); Layout( r.right, r.bottom ); }

	ShowWindow( hwnd, show );
	UpdateWindow( hwnd );

	//	Ctrl+Enter runs, as ⌘↵ does on the Mac.
	ACCEL accel[] = { { FVIRTKEY | FCONTROL, VK_RETURN, ID_RUN } };
	auto  table   = CreateAcceleratorTableW( accel, 1 );

	MSG msg;
	while( GetMessageW( &msg, nullptr, 0, 0 ) > 0 ) {
		if( TranslateAcceleratorW( hwnd, table, &msg ) ) continue;
		if( IsDialogMessageW( hwnd, &msg ) ) continue;
		TranslateMessage( &msg );
		DispatchMessageW( &msg );
	}
	return 0;
}
