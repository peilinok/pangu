#ifndef UTILS_WINVERSION
#define UTILS_WINVERSION

#include <stdint.h>
#include <string>

namespace am {
	typedef struct _winversion_info {
		int major;
		int minor;
		int build;
		int revis;
	}winversion_info;

	class system_version
	{
	private:
		system_version();

	public:
		static bool get_dll(const std::string tar, winversion_info *info);

		static void get_win(winversion_info *info);

		static bool is_win8_or_above();

		static bool is_win10_or_above(int build_number = -1);

		static bool is_32();

	private:
		std::string _target_file;
	};

}

#endif