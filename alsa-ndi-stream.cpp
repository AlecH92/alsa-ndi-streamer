#include <stdio.h>
#include <stdlib.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <alsa/asoundlib.h>
#include <Processing.NDI.Lib.h>

static std::atomic<bool> exit_loop(false);
static void sigint_handler(int) { exit_loop = true; }

int main(int argc, char* argv[])
{
	if (!NDIlib_initialize())
	{
		// Cannot run NDI. Most likely because the CPU is not sufficient (see SDK documentation).
		// you can check this directly with a call to NDIlib_is_supported_CPU()
		fprintf(stderr, "Cannot run NDI.");
		return 1;
	}
	signal(SIGINT, sigint_handler); // Catch interrupt so that we can shut down gracefully

	long loops;
	int rc;
	int size;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val = 19200;
	int dir;
	snd_pcm_uframes_t frames = 1920;
	char *buffer;

	rc = snd_pcm_open(&handle, argv[1], SND_PCM_STREAM_CAPTURE, 0); // Open PCM device for recording (capture).
	if (rc < 0)
	{
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		return 1;
	}

	snd_pcm_hw_params_alloca(&params); // Allocate a hardware parameters object.
	snd_pcm_hw_params_any(handle, params); // Fill it in with default values

	// Set the desired hardware parameters.
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); // Interleaved mode
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE); // Signed 16-bit little-endian format
	snd_pcm_hw_params_set_channels(handle, params, 1); // Mono audio
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir); // Sampling rate
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	rc = snd_pcm_hw_params(handle, params); // Write the parameters to the driver
	if (rc < 0)
	{
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		return 1;
	}

	snd_pcm_hw_params_get_period_size(params, &frames, &dir); // Retrieve the set frames
	snd_pcm_hw_params_get_period_time(params, &val, &dir); // Retrieve the set sampling rate

	NDIlib_send_create_t NDI_send_create_desc; // Create an NDI source that is called argv[2] and is clocked to the audio.
	NDI_send_create_desc.p_ndi_name = argv[2];
	NDI_send_create_desc.clock_audio = true;

	NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc); // We create the NDI finder
	if (!pNDI_send)
	{
		fprintf(stderr, "Failed to create ndi sender: %s\n", NDI_send_create_desc.p_ndi_name);
		return 1;
	}

	NDIlib_audio_frame_interleaved_16s_t NDI_audio_frame;
	NDI_audio_frame.sample_rate = val; // sampling rate from above
	NDI_audio_frame.no_channels = 1; // mono audio
	NDI_audio_frame.no_samples = frames; // frames from above
	NDI_audio_frame.p_data = (short*)malloc(frames * 2 * sizeof(short));

	while(!exit_loop)
	{
		rc = snd_pcm_readi(handle, NDI_audio_frame.p_data, frames);
		if (rc == -EPIPE)
		{
			fprintf(stderr, "overrun occurred\n"); // EPIPE means overrun
			snd_pcm_prepare(handle);
		}
		else if (rc < 0)
		{
			fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
		}
		else if (rc != (int)frames)
		{
			fprintf(stderr, "short read, read %d frames\n", rc);
		}
		NDIlib_util_send_send_audio_interleaved_16s(pNDI_send, &NDI_audio_frame); // We now submit the frame.
	}

	free(NDI_audio_frame.p_data);// Free the audio frame
	NDIlib_send_destroy(pNDI_send); // Destroy the NDI finder
	NDIlib_destroy(); // Not required, but nice
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	fprintf(stdout, "audio interface closed\n");
	return 0;
}

