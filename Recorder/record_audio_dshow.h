#pragma once

#include <string>

#include "record_audio.h"

namespace am {

	class record_audio_dshow :public record_audio
	{
	public:
		record_audio_dshow();
		~record_audio_dshow();

		virtual int init(const std::string &device_name,
			const std::string &device_id,
			bool is_input);

		virtual int start();

		virtual int pause();

		virtual int resume();

		virtual int stop();

		virtual const AVRational get_time_base();
		virtual int64_t get_start_time();

	private:
		int decode(AVFrame *frame, AVPacket *packet);
		void record_loop();
		void cleanup();
	private:
		AVFormatContext *_fmt_ctx;
		AVInputFormat *_input_fmt;
		AVCodecContext *_codec_ctx;
		AVCodec *_codec;

		int _stream_index;
	};

}