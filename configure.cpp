#include "definitions.h"
#include "Plugin.h"

//V241 for VS2008

//Called when the user hits the configure button in MPQDraft or FireGraft
BOOL WINAPI Plugin::Configure(HWND hParentWnd)
{
	/*
	 *  You can't really change the version from
	 *  here with the current format.  However, you
	 *  can check the version and probably set any
	 *  globals.
	 */

	MessageBox(
		hParentWnd,
		PLUGIN_NAME " (ID: " STR(PLUGIN_ID) ")"
		"\nCompiled by Pr0nogo"
		"\nPronogo@hotmail.com"
		"\nBuilt on " __DATE__ " " __TIME__
		"\nGLHF"
		,
		PLUGIN_NAME,
		MB_TASKMODAL
	);

	return TRUE;
}

