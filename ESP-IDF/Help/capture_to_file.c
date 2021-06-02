#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

//snd_pcm_format_t format = SND_PCM_FORMAT_U8;

/*
 * https://www.alsa-project.org/wiki/ALSA_Library_API
 */

snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

int main (int argc, char *argv[]) {

	snd_pcm_t *capture_handle;

	int result = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0);
	if (result < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
			 argv[1],
			 snd_strerror (result));
			 exit(EXIT_FAILURE);
	}

	result = snd_pcm_set_params(capture_handle,
					  format,
					  SND_PCM_ACCESS_RW_INTERLEAVED,
					  1,
					  48000,
					  1,
					  500000);   /* 0.5 sec */
	if (result < 0) {
		fprintf(stderr, "snd_pcm_set_params: %s\n",
			snd_strerror(result));
			exit(EXIT_FAILURE);
    }

	result = snd_pcm_prepare(capture_handle);
	if (result < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	FILE *fd = fopen(argv[2], "w");
	if (fd == NULL) {
		fprintf (stderr, "Error openning file \"%s\": (%s)\n",
			argv[2], strerror(errno));
		exit(EXIT_FAILURE);
	}

	int frame_size = snd_pcm_format_width(format) / 8;

	short buf[1024];

	for (int i = 0; i < 200; ++i) {
		snd_pcm_sframes_t read_frames =
			snd_pcm_readi(capture_handle, buf, sizeof buf / frame_size);
		if (read_frames < 0) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					snd_strerror(read_frames));
			exit(EXIT_FAILURE);
		}
		size_t wrote_frames = fwrite(buf, frame_size, read_frames, fd);
		if (wrote_frames != read_frames && ferror(fd)) {
			fprintf (stderr, "Error writing to file %s: (%s)\n", argv[2],
					strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	fclose(fd);

	snd_pcm_close (capture_handle);
}
