//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/system/wm_tools.hpp>

#ifdef _WIN32
#pragma warning(suppress: 4091)
#include <Shlobj.h>
#include <atlbase.h>
#endif

namespace wui
{

#ifdef _WIN32
void hide_taskbar_icon(system_context &ctx)
{
	CComPtr<ITaskbarList> pBuilder;
	HRESULT hr = pBuilder.CoCreateInstance(CLSID_TaskbarList);
	if (SUCCEEDED(hr))
	{
		pBuilder->HrInit();
		pBuilder->DeleteTab(ctx.hwnd);
	}
}

void show_taskbar_icon(system_context &ctx)
{
	CComPtr<ITaskbarList> pBuilder;
	HRESULT hr = pBuilder.CoCreateInstance(CLSID_TaskbarList);
	if (SUCCEEDED(hr))
	{
		pBuilder->HrInit();
		pBuilder->AddTab(ctx.hwnd);
	}
}

rect get_screen_size(system_context &context)
{
	MONITORINFO mi = { sizeof(mi) };
	if (GetMonitorInfo(MonitorFromWindow(context.hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
	{
		auto width = mi.rcMonitor.right - mi.rcMonitor.left;
		auto height = mi.rcMonitor.bottom - mi.rcMonitor.top;

		return { 0, 0, width, height };
	}

	return { 0 };
}

#else

void hide_taskbar_icon(system_context &ctx)
{
}

void show_taskbar_icon(system_context &ctx)
{
}

rect get_screen_size(system_context &context)
{
	if (context.screen)
	{
		return { 0, 0, context.screen->width_in_pixels, context.screen->height_in_pixels };
	}
	return { 0 };
}

#endif

}
