#include "export.h"

#include "device_audios.h"
#include "encoder_video_define.h"

#include "record_audio_factory.h"
#include "record_desktop_factory.h"

#include "muxer_define.h"
#include "muxer_ffmpeg.h"

#include "remuxer_ffmpeg.h"

#include "error_define.h"
#include "log_helper.h"
#include "utils_string.h"

#ifdef _WIN32
#include "system_version.h"
#endif

#include <string>
#include <atomic>
#include <mutex>

#define USE_DSHOW 0

namespace am {
typedef std::lock_guard<std::mutex> lock_guard;

static const double scaled_vals[] = { 1.0,         1.25, (1.0 / 0.75), 1.5,
	(1.0 / 0.6), 1.75, 2.0,          2.25,
	2.5,         2.75, 3.0,          0.0 };

class recorder {
private:
	recorder();

	~recorder();

public:
	static recorder *instance();

	static void release();

	int init(const AMRECORDER_SETTING & setting, const AMRECORDER_CALLBACK &callbacks);

	int start();

	void stop();

	int pause();

	int resume();

	void set_preview_enabled(bool enable);

private:
	void on_preview_yuv(const uint8_t *data, int size, int width, int height, int type);
	void get_valid_out_resolution(int src_width, int src_height, int *out_width, int *out_height);
private:
	AMRECORDER_SETTING _setting;
	AMRECORDER_CALLBACK _callbacks;

	record_audio *_recorder_speaker;
	record_audio *_recorder_mic;
	record_desktop *_recorder_desktop;

	muxer_file *_muxer;

	std::atomic_bool _inited;
	std::mutex _mutex;
};

static recorder *_g_instance = nullptr;
static std::mutex _g_mutex;

recorder::recorder() {
	memset(&_setting, 0, sizeof(_setting));
	memset(&_callbacks, 0, sizeof(_callbacks));

	_recorder_speaker = nullptr;
	_recorder_mic = nullptr;
	_recorder_desktop = nullptr;

	_inited = false;
	_muxer = nullptr;
}

recorder::~recorder() {
	if (_muxer)
		delete _muxer;

	if (_recorder_desktop)
		delete _recorder_desktop;

	if (_recorder_mic)
		delete _recorder_mic;

	if (_recorder_speaker)
		delete _recorder_speaker;
}

recorder * recorder::instance() {
	lock_guard lock(_g_mutex);

	if (_g_instance == nullptr) _g_instance = new recorder();

	return _g_instance;
}

void recorder::release()
{
	lock_guard lock(_g_mutex);

	if (_g_instance)
		delete _g_instance;

	_g_instance = nullptr;
}

int recorder::init(const AMRECORDER_SETTING & setting, const AMRECORDER_CALLBACK & callbacks)
{
	lock_guard lock(_mutex);
	if (_inited == true)
		return AE_NO;

	int error = AE_NO;
	int audio_num = 0;

	_setting = setting;
	_callbacks = callbacks;

	am::record_audio *audios[2] = { 0 };

#if USE_DSHOW

	error = record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_DSHOW, &_recorder_speaker);
	AMERROR_CHECK(error);

	error = _recorder_speaker->init("audio=virtual-audio-capturer", "audio=virtual-audio-capturer", false);
	AMERROR_CHECK(error);

	error = record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_DSHOW, &_recorder_mic);
	AMERROR_CHECK(error);

	error = _recorder_mic->init(std::string("audio=") + std::string(setting.a_mic.name), std::string("audio=") + std::string(setting.a_mic.name), true);
	AMERROR_CHECK(error);

	audios = { _recorder_speaker,_recorder_mic };
#else
	if (utils_string::utf8_ascii(setting.a_speaker.name).length() && utils_string::utf8_ascii(setting.a_speaker.id).length()) {
		error = record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_speaker);
		AMERROR_CHECK(error);

		error = _recorder_speaker->init(setting.a_speaker.name, setting.a_speaker.id, false);
		AMERROR_CHECK(error);

		audios[audio_num] = _recorder_speaker;
		audio_num++;
	}



	if (utils_string::utf8_ascii(setting.a_mic.name).length() && utils_string::utf8_ascii(setting.a_mic.id).length()) {
		error = record_audio_new(RECORD_AUDIO_TYPES::AT_AUDIO_WAS, &_recorder_mic);
		AMERROR_CHECK(error);

		error = _recorder_mic->init(setting.a_mic.name, setting.a_mic.id, true);
		AMERROR_CHECK(error);

		audios[audio_num] = _recorder_mic;
		audio_num++;
	}
#endif 

#ifdef _WIN32
#if USE_MAG
  if (_recorder_desktop == nullptr) {
                error =
                    record_desktop_new(RECORD_DESKTOP_TYPES::DT_DESKTOP_WIN_MAG,
                                  &_recorder_desktop);
    if (error == AE_NO) {
      error = _recorder_desktop->init(
          {setting.v_left, setting.v_top,
            setting.v_width + setting.v_left,
            setting.v_height + setting.v_top},
          setting.v_frame_rate);

      if (error != AE_NO)
        record_desktop_destroy(&_recorder_desktop);
    }
  }
#endif

	// windows support wgc since win10.1803
  if (_recorder_desktop == nullptr && system_version::is_win10_or_above(17134)) {
    error = record_desktop_new(RECORD_DESKTOP_TYPES::DT_DESKTOP_WIN_WGC,
        &_recorder_desktop);
    if (error == AE_NO) {
      error = _recorder_desktop->init({setting.v_left, setting.v_top,
                                        setting.v_width + setting.v_left,
                                        setting.v_height + setting.v_top},
                                      setting.v_frame_rate);

      if (error != AE_NO)
        record_desktop_destroy(&_recorder_desktop);
    }
	}

  if (_recorder_desktop == nullptr &&
      system_version::is_win8_or_above()) {
		error = record_desktop_new(RECORD_DESKTOP_TYPES::DT_DESKTOP_WIN_DUPLICATION, &_recorder_desktop);
		if (error == AE_NO) {

			error = _recorder_desktop->init(
			{
				setting.v_left,setting.v_top,setting.v_width + setting.v_left,setting.v_height + setting.v_top
			},
				setting.v_frame_rate
			);

			if (error != AE_NO)
				record_desktop_destroy(&_recorder_desktop);
		}
	}

	if (_recorder_desktop == nullptr) {
		error = record_desktop_new(RECORD_DESKTOP_TYPES::DT_DESKTOP_WIN_GDI, &_recorder_desktop);
		AMERROR_CHECK(error);

		error = _recorder_desktop->init(
		{
			setting.v_left,setting.v_top,setting.v_width + setting.v_left,setting.v_height + setting.v_top
		},
			setting.v_frame_rate
		);

		AMERROR_CHECK(error);
	}
#endif // _WIN32

	am::MUX_SETTING mux_setting;
	mux_setting.v_frame_rate = setting.v_frame_rate;
	mux_setting.v_bit_rate = setting.v_bit_rate;
	mux_setting.v_width = setting.v_width;
	mux_setting.v_height = setting.v_height;
	mux_setting.v_qb = setting.v_qb;
	mux_setting.v_encoder_id = (am::ENCODER_VIDEO_ID)setting.v_enc_id;

	get_valid_out_resolution(setting.v_width, setting.v_height, &mux_setting.v_out_width, &mux_setting.v_out_height);

	mux_setting.a_nb_channel = 2;
	mux_setting.a_sample_fmt = AV_SAMPLE_FMT_FLTP;
	mux_setting.a_sample_rate = 44100;
	mux_setting.a_bit_rate = 128000;



	_muxer = new muxer_ffmpeg();

	_muxer->registe_yuv_data(std::bind(
		&recorder::on_preview_yuv,
		this,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		std::placeholders::_4,
		std::placeholders::_5
	));

	error = _muxer->init(setting.output, _recorder_desktop, audios, audio_num, mux_setting);
	AMERROR_CHECK(error);

	_inited = true;

	return error;
}

int recorder::start()
{
	lock_guard lock(_mutex);
	if (_inited == false)
		return AE_NEED_INIT;

	int error = _muxer->start();

	return error;
}

void recorder::stop()
{
	lock_guard lock(_mutex);
	if (_inited == false)
		return;

	_muxer->stop();
}

int recorder::pause()
{
	lock_guard lock(_mutex);
	if (_inited == false)
		return AE_NEED_INIT;

	return _muxer->pause();
}

int recorder::resume()
{
	lock_guard lock(_mutex);
	if (_inited == false)
		return AE_NEED_INIT;

	return _muxer->resume();
}

void recorder::set_preview_enabled(bool enable)
{
	lock_guard lock(_mutex);
	if (_inited == false)
		return;

	_muxer->set_preview_enabled(enable);
}

void recorder::on_preview_yuv(const uint8_t * data, int size, int width, int height, int type)
{
	if (_callbacks.func_preview_yuv != NULL)
		_callbacks.func_preview_yuv(data, size, width, height, type);
}
void recorder::get_valid_out_resolution(int src_width, int src_height, int * out_width, int * out_height)
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
}
}

AMRECORDER_API const char * recorder_err2str(int error)
{
	return am::utils_string::ascii_utf8(err2str(error)).c_str();
}

AMRECORDER_API int recorder_init(const AMRECORDER_SETTING & setting, const AMRECORDER_CALLBACK & callbacks)
{
	return am::recorder::instance()->init(setting, callbacks);
}

AMRECORDER_API void recorder_release()
{
	return am::recorder::instance()->release();
}

AMRECORDER_API int recorder_start()
{
	return am::recorder::instance()->start();
}

AMRECORDER_API void recorder_stop()
{
	return am::recorder::instance()->stop();
}

AMRECORDER_API int recorder_pause()
{
	return am::recorder::instance()->pause();
}

AMRECORDER_API int recorder_resume()
{
	return am::recorder::instance()->resume();
}

AMRECORDER_API int recorder_get_speakers(AMRECORDER_DEVICE ** devices)
{
	std::list<am::DEVICE_AUDIOS> device_list;

	int error = am::device_audios::get_output_devices(device_list);
	if (error != AE_NO) return -error;

	int count = device_list.size();

	*devices = new AMRECORDER_DEVICE[count];

	int index = 0;
	for each (auto device in device_list)
	{
		al_info("audio input name:%s id:%s", device.name.c_str(), device.id.c_str());

		(*devices)[index].is_default = device.is_default;
		sprintf_s((*devices)[index].id, 260, "%s", device.id.c_str());
		sprintf_s((*devices)[index].name, 260, "%s", device.name.c_str());

		index++;
	}

	return count;
}

AMRECORDER_API int recorder_get_mics(AMRECORDER_DEVICE ** devices)
{
	std::list<am::DEVICE_AUDIOS> device_list;

	int error = am::device_audios::get_input_devices(device_list);
	if (error != AE_NO) return -error;

	int count = device_list.size();

	*devices = new AMRECORDER_DEVICE[count];

	int index = 0;
	for each (auto device in device_list)
	{
		al_info("audio output name:%s id:%s", device.name.c_str(), device.id.c_str());

		(*devices)[index].is_default = device.is_default;
		sprintf_s((*devices)[index].id, 260, "%s", device.id.c_str());
		sprintf_s((*devices)[index].name, 260, "%s", device.name.c_str());

		index++;
	}

	return count;
}

AMRECORDER_API int recorder_get_cameras(AMRECORDER_DEVICE ** devices)
{
	return -AE_UNSUPPORT;
}

AMRECORDER_API int recorder_get_vencoders(AMRECORDER_ENCODERS ** encoders)
{
	auto hw_encoders = am::hardware_acceleration::get_supported_video_encoders();

	int count = hw_encoders.size() + 1;
	*encoders = new AMRECORDER_ENCODERS[count];

	AMRECORDER_ENCODERS *ptr = *encoders;
	ptr->id = am::EID_VIDEO_X264;
	sprintf_s(ptr->name, 260, am::utils_string::ascii_utf8("Soft.X264").c_str());

	for each (auto hw_encoder in hw_encoders)
	{
		ptr++;
		ptr->id = hw_encoder.type;
		sprintf_s(ptr->name, 260, "%s", hw_encoder.name);
	}

	return count;
}

AMRECORDER_API void recorder_free_array(void * array_address)
{
	if (array_address != nullptr)
		delete[]array_address;
}

AMRECORDER_API int recorder_remux(const char * src, const char * dst, AMRECORDER_FUNC_REMUX_PROGRESS func_progress, AMRECORDER_FUNC_REMUX_STATE func_state)
{
	am::REMUXER_PARAM param = { 0 };

	sprintf_s(param.src, 260, "%s", am::utils_string::utf8_ascii(src).c_str());
	sprintf_s(param.dst, 260, "%s", am::utils_string::utf8_ascii(dst).c_str());

	param.cb_progress = func_progress;

	param.cb_state = func_state;

	return am::remuxer_ffmpeg::instance()->create_remux(param);
}

AMRECORDER_API void recorder_set_preview_enabled(int enable)
{
	am::recorder::instance()->set_preview_enabled(enable == 1);
}

AMRECORDER_API void recorder_set_logpath(const char * path)
{
	AMLog *log = AMLog::get(path);
}
