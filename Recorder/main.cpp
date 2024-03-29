#include "device_audios.h"

#include "record_audio_factory.h"
#include "record_desktop_factory.h"
#include "headers_mmdevice.h"

#include "encoder_aac.h"
#include "resample_pcm.h"
#include "filter_aresample.h"

#include "muxer_define.h"
#include "muxer_ffmpeg.h"

#include "utils_string.h"
#include "system_version.h"
#include "error_define.h"
#include "log_helper.h"
#include "hardware_acceleration.h"

#include "remuxer_ffmpeg.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#define USE_WASAPI 1

#define V_FRAME_RATE 30
#define V_BIT_RATE 1280*1000
#define V_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define V_HEIGHT  GetSystemMetrics(SM_CYSCREEN);
#define V_QUALITY 100

#define A_SAMPLE_CHANNEL 2
#define A_SAMPLE_RATE 44100
#define A_BIT_RATE 128000


//for test muxer
static am::record_audio *_recorder_speaker = nullptr;
static am::record_audio *_recorder_microphone = nullptr;
static am::record_desktop *_recorder_desktop = nullptr;
static am::muxer_file *_muxer;
static am::record_audio *audios[2];

//for test audio record
static am::record_audio *_recorder_audio = nullptr;
static am::encoder_aac *_encoder_aac = nullptr;
static am::resample_pcm *_resample_pcm = nullptr;
static am::filter_aresample *_filter_aresample = nullptr;

static int _sample_in = 0;
static int _sample_size = 0;
static int _resample_size = 0;
static uint8_t *_sample_buffer = nullptr;
static uint8_t *_resample_buffer = nullptr;

int start_muxer() {
	std::string input_id, input_name, out_id, out_name;

	am::device_audios::get_default_input_device(input_id, input_name);

	al_info("use default input aduio device:%s", input_name.c_str());

	am::device_audios::get_default_ouput_device(out_id, out_name);

	al_info("use default output aduio device:%s", out_name.c_str());

	//first audio resrouce must be speaker,otherwise the audio pts may be not correct,may need to change the filter amix descriptions with duration & sync option
#if !USE_WASAPI

	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_DSHOW, &_recorder_speaker);
	_recorder_speaker->init(
		am::utils_string::ascii_utf8("audio=virtual-audio-capturer"),
		am::utils_string::ascii_utf8("audio=virtual-audio-capturer"), 
		false
	);

	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_DSHOW, &_recorder_microphone);
	_recorder_microphone->init(am::utils_string::ascii_utf8("audio=") + input_name, input_id, true);
#else
	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_speaker);
	_recorder_speaker->init(out_name, out_id, false);

	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_microphone);
	_recorder_microphone->init(input_name, input_id, true);
#endif // !USE_WASAPI


	record_desktop_new(RECORD_DESKTOP_TYPES::DT_DESKTOP_WIN_DUPLICATION, &_recorder_desktop);

	RECORD_DESKTOP_RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = V_WIDTH;
	rect.bottom = V_HEIGHT;

	_recorder_desktop->init(rect, V_FRAME_RATE);

	audios[0] = _recorder_microphone;
	audios[1] = _recorder_speaker;

	_muxer = new am::muxer_ffmpeg();


	am::MUX_SETTING setting;
	setting.v_frame_rate = V_FRAME_RATE;
	setting.v_bit_rate = V_BIT_RATE;
	setting.v_width = V_WIDTH;
	setting.v_height = V_HEIGHT;
	setting.v_qb = V_QUALITY;
	setting.v_encoder_id = am::EID_VIDEO_X264;
	setting.v_out_width = 1920;
	setting.v_out_height = 1080;

	setting.a_nb_channel = A_SAMPLE_CHANNEL;
	setting.a_sample_fmt = AV_SAMPLE_FMT_FLTP;
	setting.a_sample_rate = A_SAMPLE_RATE;
	setting.a_bit_rate = A_BIT_RATE;

	int error = _muxer->init(am::utils_string::ascii_utf8("..\\..\\save.mp4").c_str(), _recorder_desktop, audios, 2, setting);
	if (error != AE_NO) {
		return error;
	}

	_muxer->start();

	return error;
}

void stop_muxer()
{
	_muxer->stop();

	delete _recorder_desktop;

	if(_recorder_speaker)
		delete _recorder_speaker;

	if (_recorder_microphone)
		delete _recorder_microphone;

	delete _muxer;
}

void test_recorder()
{
	//av_log_set_level(AV_LOG_DEBUG);

	start_muxer();

	getchar();

	//stop have bug that sometime will stuck
	stop_muxer();

	al_info("record stoped...");
}

void show_devices()
{
	std::list<am::DEVICE_AUDIOS> devices;

	am::device_audios::get_input_devices(devices);

	for each (auto device in devices)
	{
		al_info("audio input name:%s id:%s", device.name.c_str(), device.id.c_str());
	}

	am::device_audios::get_output_devices(devices);

	for each (auto device in devices)
	{
		al_info("audio output name:%s id:%s", device.name.c_str(), device.id.c_str());
	}
}

void on_aac_data(AVPacket *packet) {
	al_debug("on aac data :%d", packet->size);
}

void on_aac_error(int) {

}

void on_pcm_error(int, int) {

}

void on_pcm_data1(AVFrame *frame, int index) {

	int copied_len = 0;
	int sample_len = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, (AVSampleFormat)frame->format, 1);
	int remain_len = sample_len;

	int is_planner = av_sample_fmt_is_planar((AVSampleFormat)frame->format);

	while (remain_len > 0) {//should add is_planner codes
							//cache pcm
		copied_len = min(_sample_size - _sample_in, remain_len);
		if (copied_len) {
			memcpy(_sample_buffer + _sample_in, frame->data[0] + sample_len - remain_len, copied_len);
			_sample_in += copied_len;
			remain_len = remain_len - copied_len;
		}

		//got enough pcm to encoder,resample and mix
		if (_sample_in == _sample_size) {
			int ret = _resample_pcm->convert(_sample_buffer, _sample_size, _resample_buffer,_resample_size);
			if (ret > 0) {
				_encoder_aac->put(_resample_buffer, _resample_size, frame);
			}
			else {
				al_debug("resample audio %d failed,%d", index, ret);
			}

			_sample_in = 0;
		}
	}
}

void on_pcm_data(AVFrame *frame, int index) {
	_filter_aresample->add_frame(frame);
}

void on_aresample_data(AVFrame * frame,int index) {
	int copied_len = 0;
	int sample_len = av_samples_get_buffer_size(frame->linesize, frame->channels, frame->nb_samples, (AVSampleFormat)frame->format, 1);
	sample_len = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, (AVSampleFormat)frame->format, 1);

	int remain_len = sample_len;

	//for data is planar,should copy data[0] data[1] to correct buff pos
	if (av_sample_fmt_is_planar((AVSampleFormat)frame->format) == 0) {
		while (remain_len > 0) {
			//cache pcm
			copied_len = min(_sample_size - _sample_in, remain_len);
			if (copied_len) {
				memcpy(_resample_buffer + _sample_in, frame->data[0] + sample_len - remain_len, copied_len);
				_sample_in += copied_len;
				remain_len = remain_len - copied_len;
			}

			//got enough pcm to encoder,resample and mix
			if (_sample_in == _sample_size) {
				_encoder_aac->put(_resample_buffer, _sample_size, frame);

				_sample_in = 0;
			}
		}
	}
	else {//resample size is channels*frame->linesize[0],for 2 channels
		while (remain_len > 0) {
			copied_len = min(_sample_size - _sample_in, remain_len);
			if (copied_len) {
				memcpy(_resample_buffer + _sample_in / 2, frame->data[0] + (sample_len - remain_len) / 2, copied_len / 2);
				memcpy(_resample_buffer + _sample_size / 2 + _sample_in / 2, frame->data[1] + (sample_len - remain_len) / 2, copied_len / 2);
				_sample_in += copied_len;
				remain_len = remain_len - copied_len;
			}

			if (_sample_in == _sample_size) {
				_encoder_aac->put(_resample_buffer, _sample_size, frame);

				_sample_in = 0;
			}
		}
	}
}

void on_aresample_error(int error,int index) {

}

void save_aac() {
	std::string input_id, input_name, out_id, out_name;

	am::device_audios::get_default_input_device(input_id, input_name);

	am::device_audios::get_default_ouput_device(out_id, out_name);

#if 0
	al_info("use default \r\noutput aduio device name:%s \r\noutput audio device id:%s ",
		out_name.c_str(), out_id.c_str());

	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_audio);
	_recorder_audio->init(out_name, out_id, false);
#else
	al_info("use default \r\ninput aduio device name:%s \r\ninput audio device id:%s ",
		input_name.c_str(), input_id.c_str());

	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_audio);
	_recorder_audio->init(input_name, input_id, true);
#endif

	_recorder_audio->registe_cb(on_pcm_data, on_pcm_error, 0);

	_encoder_aac = new am::encoder_aac();
	_encoder_aac->init(A_SAMPLE_CHANNEL, A_SAMPLE_RATE, AV_SAMPLE_FMT_FLTP, A_BIT_RATE);

	am::SAMPLE_SETTING src, dst = { 0 };
	src = {
		_encoder_aac->get_nb_samples(),
		av_get_default_channel_layout(_recorder_audio->get_channel_num()),
		_recorder_audio->get_channel_num(),
		_recorder_audio->get_fmt(),
		_recorder_audio->get_sample_rate()
	};
	dst = {
		_encoder_aac->get_nb_samples(),
		av_get_default_channel_layout(A_SAMPLE_CHANNEL),
		A_SAMPLE_CHANNEL,
		AV_SAMPLE_FMT_FLTP,
		A_SAMPLE_RATE
	};

	_resample_pcm = new am::resample_pcm();
	_resample_pcm->init(&src, &dst, &_resample_size);
	_resample_buffer = new uint8_t[_resample_size];

	_sample_size = av_samples_get_buffer_size(NULL, A_SAMPLE_CHANNEL, _encoder_aac->get_nb_samples(), _recorder_audio->get_fmt(), 1);
	_sample_buffer = new uint8_t[_sample_size];

	_filter_aresample = new am::filter_aresample();
	_filter_aresample->init({
		NULL,NULL,
		_recorder_audio->get_time_base(),
		_recorder_audio->get_sample_rate(),
		_recorder_audio->get_fmt(),
		_recorder_audio->get_channel_num(),
		av_get_default_channel_layout(_recorder_audio->get_channel_num())
	}, {
		NULL,NULL,
		{ 1,AV_TIME_BASE },
		A_SAMPLE_RATE,
		AV_SAMPLE_FMT_FLTP,
		A_SAMPLE_CHANNEL,
		av_get_default_channel_layout(A_SAMPLE_CHANNEL)
	},0);
	_filter_aresample->registe_cb(on_aresample_data, on_aresample_error);


	_filter_aresample->start();
	_encoder_aac->start();
	_recorder_audio->start();

	getchar();

	_recorder_audio->stop();
	_filter_aresample->stop();

	delete _recorder_audio;
	delete _encoder_aac;
	delete _resample_pcm;
	delete _filter_aresample;

	delete[] _sample_buffer;
	delete[] _resample_buffer;
}

void test_audio() 
{
	std::string input_id, input_name, out_id, out_name;

	am::device_audios::get_default_input_device(input_id, input_name);

	al_info("use default \r\ninput aduio device name:%s \r\ninput audio device id:%s ", 
		input_name.c_str(), input_id.c_str());

	am::device_audios::get_default_ouput_device(out_id, out_name);

	al_info("use default \r\noutput aduio device name:%s \r\noutput audio device id:%s ", 
		out_name.c_str(), out_id.c_str());

	record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_speaker);
	_recorder_speaker->init(am::utils_string::ascii_utf8("Default"), am::utils_string::ascii_utf8("Default"), false);


	//record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_microphone);
	//_recorder_microphone->init(input_name,input_id, true);

	_recorder_speaker->start();

	//_recorder_microphone->start();

	getchar();

	_recorder_speaker->stop();
	//_recorder_microphone->stop();
}


void on_remux_progress(const char *src, int progress, int total)
{
	al_debug("on remux progress:%s %d %d", src, progress, total);
}

void on_remux_state(const char *src, int state, int error) {
	al_debug("on remux state:%s %d %d", src, state, error);
}

void test_remux() {
#if TEST_MULTI_THREAD
	for (int i = 0; i < 20; i++) {
		am::REMUXER_PARAM param = { 0 };
		sprintf_s(param.src, 260, "%d", i);
		am::remuxer_ffmpeg::instance()->create_remux(param);
	}
#else
	am::REMUXER_PARAM param = { 0 };

	sprintf_s(param.src, 260, "%s", am::utils_string::ascii_utf8("..\\..\\save.mkv").c_str());
	sprintf_s(param.dst, 260, "%s", am::utils_string::ascii_utf8("..\\..\\save.mp4").c_str());

	param.cb_progress = on_remux_progress;

	param.cb_state = on_remux_state;
	
	am::remuxer_ffmpeg::instance()->create_remux(param);
#endif
}

void test_scale() {
	static const double scaled_vals[] = { 1.0,         1.25, (1.0 / 0.75), 1.5,
		(1.0 / 0.6), 1.75, 2.0,          2.25,
		2.5,         2.75, 3.0,          0.0 };

	auto get_valid_out_resolution = [](int src_width, int src_height, int * out_width, int * out_height)
	{
		int scale_cx = src_width;
		int scale_cy = src_height;

		int i = 0;

		while (((scale_cx * scale_cy) > (1920 * 1080)) && scaled_vals[i] > 0.0) {
			double scale = scaled_vals[i++];
			scale_cx = uint32_t(double(src_width) / scale);
			scale_cy = uint32_t(double(src_height) / scale);
		}

		if (scale_cx % 2 != 0) {
			scale_cx += 1;
		}

		if (scale_cy % 2 != 0) {
			scale_cy += 1;
		}


		*out_width = scale_cx;
		*out_height = scale_cy;

		al_info("get valid output resolution from %dx%d to %dx%d,with scale:%lf", src_width, src_height, scale_cx, scale_cy, scaled_vals[i]);
	};

	int src_width=2736, src_height=1824;
	int dst_width, dst_height;

	get_valid_out_resolution(src_width, src_height, &dst_width, &dst_height);
}

int main(int argc, char **argv)
{
	al_info("record start...");

	am::winversion_info ver = { 0 };

	am::system_version::get_win(&ver);

	bool is_win8_or_above = am::system_version::is_win8_or_above();

	bool is_ia32 = am::system_version::is_32();

	al_info("win version: %d.%d.%d.%d", ver.major, ver.minor, ver.build, ver.revis);
	al_info("is win8 or above: %s", is_win8_or_above ? "true" : "false");

	//auto hw_encoders = am::hardware_acceleration::get_supported_video_encoders();

	//show_devices();

	//test_audio();

	test_recorder();

	//test_remux();

	//save_aac();

	//test_scale();


	al_info("press any key to exit...");
	system("pause");

	return 0;
}
