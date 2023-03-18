#pragma once

#include <functional>
#include <atomic>
#include <string>

namespace am {

	class record_audio;
	class record_desktop;

	struct MUX_STREAM_T;
	struct MUX_SETTING_T;

	typedef std::function<void(const uint8_t *data, int size, int width, int height,int type)> cb_yuv_data;

	class muxer_file
	{
	public:
		muxer_file();
		virtual ~muxer_file();

		virtual int init(
			const char *output_file,
			record_desktop *source_desktop,
			record_audio ** source_audios,
			const int source_audios_nb,
			const MUX_SETTING_T &setting
		) = 0;

		virtual int start() = 0;
		virtual int stop() = 0;

		virtual int pause() = 0;
		virtual int resume() = 0;

		inline void registe_yuv_data(cb_yuv_data on_yuv_data) {
			_on_yuv_data = on_yuv_data;
		}

		inline void set_preview_enabled(bool enable) {
			_preview_enabled = enable;
		}

	protected:
		cb_yuv_data _on_yuv_data;

		std::atomic_bool _inited;
		std::atomic_bool _running;
		std::atomic_bool _paused;
		std::atomic_bool _preview_enabled;

		bool _have_v, _have_a;

		std::string _output_file;
	};


}