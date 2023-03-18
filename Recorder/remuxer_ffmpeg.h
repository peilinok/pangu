#ifndef REMUXER_FFMPEG
#define REMUXER_FFMPEG

#include <map>
#include <atomic>
#include <functional>
#include <thread>
#include <string>

namespace am {
	typedef void(*cb_remux_progress)(const char *, int, int);
	typedef void(*cb_remux_state)(const char *, int, int);

	typedef struct _REMUXER_PARAM {
		char src[260];
		char dst[260];
		int64_t src_size;
		std::atomic_bool running;
		cb_remux_progress cb_progress;
		cb_remux_state cb_state;
	}REMUXER_PARAM;

	typedef std::function<void(REMUXER_PARAM*)> thread_remuxing;

	typedef struct _REMUXER_HANDLE {
		REMUXER_PARAM param;
		std::thread fn;
	}REMUXER_HANDLE;
	

	class remuxer_ffmpeg
	{
	private:
		remuxer_ffmpeg(){}
		
		~remuxer_ffmpeg() { destroy_remux(); }

	public:
		static remuxer_ffmpeg *instance();
		static void release();

		int create_remux(const REMUXER_PARAM & param);

		void remove_remux(std::string src);

		void destroy_remux();

	private:
		std::map<std::string, REMUXER_HANDLE*> _handlers;
	};

}



#endif // !REMUXER_FFMPEG