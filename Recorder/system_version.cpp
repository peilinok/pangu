#include "system_version.h"

#include <Windows.h>

#include "utils_string.h"
#include "log_helper.h"

#define WINVER_REG_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"

typedef DWORD(WINAPI *get_file_version_info_size_w_t)(LPCWSTR module,
	LPDWORD unused);
typedef BOOL(WINAPI *get_file_version_info_w_t)(LPCWSTR module, DWORD unused,
	DWORD len, LPVOID data);
typedef BOOL(WINAPI *ver_query_value_w_t)(LPVOID data, LPCWSTR subblock,
	LPVOID *buf, PUINT sizeout);

static get_file_version_info_size_w_t get_file_version_info_size = NULL;
static get_file_version_info_w_t get_file_version_info = NULL;
static ver_query_value_w_t ver_query_value = NULL;
static bool ver_initialized = false;
static bool ver_initialize_success = false;

static bool initialize_version_functions(void)
{
	HMODULE ver = GetModuleHandleW(L"version");

	ver_initialized = true;

	if (!ver) {
		ver = LoadLibraryW(L"version");
		if (!ver) {
			al_error("failed to load windows version library");
			return false;
		}
	}

	get_file_version_info_size =
		(get_file_version_info_size_w_t)GetProcAddress(
			ver, "GetFileVersionInfoSizeW");
	get_file_version_info = (get_file_version_info_w_t)GetProcAddress(
		ver, "GetFileVersionInfoW");
	ver_query_value =
		(ver_query_value_w_t)GetProcAddress(ver, "VerQueryValueW");

	if (!get_file_version_info_size || !get_file_version_info ||
		!ver_query_value) {
		al_error("failed to load windows version functions");
		return false;
	}

	ver_initialize_success = true;
	return true;
}

namespace am {

	bool system_version::get_dll(const std::string tar, winversion_info * info)
	{
		VS_FIXEDFILEINFO *file_info = NULL;
		UINT len = 0;
		BOOL success;
		LPVOID data;
		DWORD size;
		std::wstring wtar = utils_string::ascii_unicode(tar);

		if (!ver_initialized && !initialize_version_functions())
			return false;
		if (!ver_initialize_success)
			return false;

		size = get_file_version_info_size(wtar.c_str(), NULL);
		if (!size) {
			al_error("failed to get %s version info size",tar.c_str());
			return false;
		}

		data = malloc(size);
		if (!get_file_version_info(wtar.c_str(), 0, size, data)) {
			al_error("failed to get %s version info", tar.c_str());
			free(data);
			return false;
		}

		success = ver_query_value(data, L"\\", (LPVOID *)&file_info, &len);
		if (!success || !file_info || !len) {
			al_error("failed to get %s version info value",tar.c_str());
			free(data);
			return false;
		}

		info->major = (int)HIWORD(file_info->dwFileVersionMS);
		info->minor = (int)LOWORD(file_info->dwFileVersionMS);
		info->build = (int)HIWORD(file_info->dwFileVersionLS);
		info->revis = (int)LOWORD(file_info->dwFileVersionLS);

		free(data);
		return true;
	}

	void system_version::get_win(winversion_info * info)
	{
		static winversion_info ver = { 0 };
		static bool got_version = false;

		if (!info)
			return;

		if (!got_version) {
			get_dll("kernel32", &ver);
			got_version = true;

			if (ver.major == 10) {
				HKEY key;
				DWORD size, win10_revision;
				LSTATUS status;

				status = RegOpenKeyW(HKEY_LOCAL_MACHINE, WINVER_REG_KEY,
					&key);
				if (status != ERROR_SUCCESS)
					return;

				size = sizeof(win10_revision);

				status = RegQueryValueExW(key, L"UBR", NULL, NULL,
					(LPBYTE)&win10_revision,
					&size);
				if (status == ERROR_SUCCESS)
					ver.revis = (int)win10_revision > ver.revis
					? (int)win10_revision
					: ver.revis;

				RegCloseKey(key);
			}
		}

		*info = ver;
	}

	bool system_version::is_win8_or_above()
	{
		winversion_info info;

		get_win(&info);


		return info.major > 6 || (info.major == 6 && info.minor >= 2);
	}

	bool system_version::is_win10_or_above(int build_number) {
    winversion_info info;
    get_win(&info);
    return info.major > 6 || (info.major == 6 && info.minor >= 2);
  }

	bool system_version::is_32()
	{
#if defined(_WIN64)
		return false;
#elif defined(_WIN32)
		BOOL b64 = false;
		return !(IsWow64Process(GetCurrentProcess(), &b64) && b64);
#endif
	}

}