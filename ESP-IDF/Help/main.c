
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <complex.h>

// Include the ALSA .H file that defines ALSA functions/data
#include <alsa/asoundlib.h>
#include <math.h>

#include "make_wav.h" 
#include "transform.h" 


#define S_RATE  (65536)
#define BUF_SIZE (S_RATE*4) /* 5 second buffer */

#define S_RATE_TONE  (48000)
#define BUF_SIZE_TONE (S_RATE_TONE*5)

#define S_RATE_TONE_ULTRA  (96000)



#pragma pack (1)
/////////////////////// WAVE File Stuff /////////////////////
// An IFF file header looks like this
typedef struct _FILE_head
{
	unsigned char	ID[4];	// could be {'R', 'I', 'F', 'F'} or {'F', 'O', 'R', 'M'}
	unsigned int	Length;	// Length of subsequent file (including remainder of header). This is in
									// Intel reverse byte order if RIFF, Motorola format if FORM.
	unsigned char	Type[4];	// {'W', 'A', 'V', 'E'} or {'A', 'I', 'F', 'F'}
} FILE_head;


// An IFF chunk header looks like this
typedef struct _CHUNK_head
{
	unsigned char ID[4];	// 4 ascii chars that is the chunk ID
	unsigned int	Length;	// Length of subsequent data within this chunk. This is in Intel reverse byte
							// order if RIFF, Motorola format if FORM. Note: this doesn't include any
							// extra byte needed to pad the chunk out to an even size.
} CHUNK_head;

// WAVE fmt chunk
typedef struct _FORMAT {
	short				wFormatTag;
	unsigned short	wChannels;
	unsigned int	dwSamplesPerSec;
	unsigned int	dwAvgBytesPerSec;
	unsigned short	wBlockAlign;
	unsigned short	wBitsPerSample;
  // Note: there may be additional fields here, depending upon wFormatTag
} FORMAT;
#pragma pack()



#define DURATION 5


// Size of the audio card hardware buffer. Here we want it
// set to 1024 16-bit sample points. This is relatively
// small in order to minimize latency. If you have trouble
// with underruns, you may need to increase this, and PERIODSIZE
// (trading off lower latency for more stability)
#define BUFFERSIZE	(2*1024)

// How many sample points the ALSA card plays before it calls
// our callback to fill some more of the audio card's hardware
// buffer. Here we want ALSA to call our callback after every
// 64 sample points have been played
#define PERIODSIZE	(2*64)

// Handle to ALSA (audio card's) playback port
snd_pcm_t				*PlaybackHandle;

// Handle to our callback thread
snd_async_handler_t	* CallbackHandle;

// Points to loaded WAVE file's data
unsigned char			*WavePtr;

// Size (in frames) of loaded WAVE file's data
snd_pcm_uframes_t		WaveSize;

// Sample rate
unsigned short			WaveRate;

// Bit resolution
unsigned char			WaveBits;

// Number of channels in the wave file
unsigned char			WaveChannels;

// The name of the ALSA port we output to. In this case, we're
// directly writing to hardware card 0,0 (ie, first set of audio
// outputs on the first audio card)
//static const char		SoundCardPortName[] = "plughw:1,1";

static const char		SoundCardPortName[] = "default";

// For WAVE file loading
static const unsigned char Riff[4]	= { 'R', 'I', 'F', 'F' };
static const unsigned char WaveF[4] = { 'W', 'A', 'V', 'E' };
static const unsigned char Fmt[4] = { 'f', 'm', 't', ' ' };
static const unsigned char Data[4] = { 'd', 'a', 't', 'a' };

//Flybird
	static char * option();
	static void loadNplay(char *name);
//
/********************** compareID() *********************
 * Compares the passed ID str (ie, a ptr to 4 Ascii
 * bytes) with the ID at the passed ptr. Returns TRUE if
 * a match, FALSE if not.
 */

static unsigned char compareID(const unsigned char * id, unsigned char * ptr)
{
	register unsigned char i = 4;

	while (i--)
	{
		if ( *(id)++ != *(ptr)++ ) return(0);
	}
	return(1);
}





/********************** waveLoad() *********************
 * Loads a WAVE file.
 *
 * fn =			Filename to load.
 *
 * RETURNS: 0 if success, non-zero if not.
 *
 * NOTE: Sets the global "WavePtr" to an allocated buffer
 * containing the wave data, and "WaveSize" to the size
 * in sample points.
 */

static unsigned char waveLoad(const char *fn)
{
	const char				*message;
	FILE_head				head;
	register int			inHandle;

	if ((inHandle = open(fn, O_RDONLY)) == -1)
		message = "didn't open";

	// Read in IFF File header
	else
	{
		if (read(inHandle, &head, sizeof(FILE_head)) == sizeof(FILE_head))
		{
			// Is it a RIFF and WAVE?
			if (!compareID(&Riff[0], &head.ID[0]) || !compareID(&WaveF[0], &head.Type[0]))
			{
				message = "is not a WAVE file";
				goto bad;
			}

			// Read in next chunk header
			while (read(inHandle, &head, sizeof(CHUNK_head)) == sizeof(CHUNK_head))
			{
				// ============================ Is it a fmt chunk? ===============================
				if (compareID(&Fmt[0], &head.ID[0]))
				{
					FORMAT	format;

					// Read in the remainder of chunk
					if (read(inHandle, &format.wFormatTag, sizeof(FORMAT)) != sizeof(FORMAT)) break;

					// Can't handle compressed WAVE files
					if (format.wFormatTag != 1)
					{
						message = "compressed WAVE not supported";
						goto bad;
					}

					WaveBits = (unsigned char)format.wBitsPerSample;
					WaveRate = (unsigned short)format.dwSamplesPerSec;
					WaveChannels = format.wChannels;
				}

				// ============================ Is it a data chunk? ===============================
				else if (compareID(&Data[0], &head.ID[0]))
				{
					// Size of wave data is head.Length. Allocate a buffer and read in the wave data
					if (!(WavePtr = (unsigned char *)malloc(head.Length)))
					{
						message = "won't fit in RAM";
						goto bad;
					}

					if (read(inHandle, WavePtr, head.Length) != head.Length)
					{
						free(WavePtr);
						break;
					}

					// Store size (in frames)
					WaveSize = (head.Length * 8) / ((unsigned int)WaveBits * (unsigned int)WaveChannels);

					close(inHandle);
					return(0);
				}

				// ============================ Skip this chunk ===============================
				else
				{
					if (head.Length & 1) ++head.Length;  // If odd, round it up to account for pad byte
					lseek(inHandle, head.Length, SEEK_CUR);
				}
			}
		}

		message = "is a bad WAVE file";
bad:	close(inHandle);
	}
	
	printf("%s %s\n", fn, message);
	return(1);
}



/********************** play_audio() **********************
 * Plays the loaded waveform.
 *
 * NOTE: ALSA sound card's handle must be in the global
 * "PlaybackHandle". A pointer to the wave data must be in
 * the global "WavePtr", and its size of "WaveSize".
 */

static void play_audio(void)
{
	register snd_pcm_uframes_t		count, frames;

	// Output the wave data
	count = 0;
	do
	{
		frames = snd_pcm_writei(PlaybackHandle, WavePtr + count, WaveSize - count);

		// If an error, try to recover from it
		if (frames < 0)
			frames = snd_pcm_recover(PlaybackHandle, frames, 0);
		if (frames < 0)
		{
			printf("Error playing wave: %s\n", snd_strerror(frames));
			break;
		}

		// Update our pointer
		count += frames;

	} while (count < WaveSize);

	// Wait for playback to completely finish
	if (count == WaveSize)
		snd_pcm_drain(PlaybackHandle);
		
}





/*********************** free_wave_data() *********************
 * Frees any wave data we loaded.
 *
 * NOTE: A pointer to the wave data be in the global
 * "WavePtr".
 */

static void free_wave_data(void)
{
	
	if (WavePtr) free(WavePtr);
	WavePtr = 0;
}





int main(int argc, char **argv)
{
	// No wave data loaded yet
	
	char *name;
		
		name = option(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), 0);
			//	printf("Name: %s",name);
		loadNplay(name);
		

	return(0);
}

static void loadNplay(char *name){
	WavePtr = 0;
	//printf("IM here %s\n\n", name);
	
			// Load the wave file 
		if (!waveLoad(name)) 
		{
			register int		err;
	
			// Open audio card we wish to use for playback
			if ((err = snd_pcm_open(&PlaybackHandle, &SoundCardPortName[0], SND_PCM_STREAM_PLAYBACK, 0)) < 0)
				printf("Can't open audio %s: %s\n", &SoundCardPortName[0], snd_strerror(err));
			else
			{
				switch (WaveBits)
				{
					case 8:
						err = SND_PCM_FORMAT_U8;
						break;
				
					case 16:
						err = SND_PCM_FORMAT_S16;
						break;
				
					case 24:
						err = SND_PCM_FORMAT_S24;
						break;
				
					case 32:
						err = SND_PCM_FORMAT_S32;
						break;
				}
			
			
				// Set the audio card's hardware parameters (sample rate, bit resolution, etc)					///alterar o waverate para o caso do ultrassom
				if ((!(strcmp(name, "ultra.wav") ) && ((err = snd_pcm_set_params(PlaybackHandle, err, SND_PCM_ACCESS_RW_INTERLEAVED, WaveChannels, 65536, 1, 500000)) < 0) ))
					printf("Can't set sound parameters: %s\n", snd_strerror(err));
				else if ( (strcmp(name, "ultra.wav") ) && ((err = snd_pcm_set_params(PlaybackHandle, err, SND_PCM_ACCESS_RW_INTERLEAVED, WaveChannels, WaveRate, 1, 500000)) < 0 ))
					printf("Can't set sound parameters: %s\n", snd_strerror(err));
				
				
				// Play the waveform
				else
					play_audio();
	
				// Close sound card
				snd_pcm_close(PlaybackHandle);
			}
		}
	
		// Free the WAVE data
		free_wave_data();
	//printf("IM here\n\n");
	
	return;
	
	}


static float randfloat(void) {
   return (float)rand()/RAND_MAX;
}

// ve qual a opcao inserida pelo utilizador
 static char * option(int opt,int opt2, int opt3, int opt4, int secondTime){
	
	
	char* wavName;
	
	if (opt == 0 ){ 
	
		if (opt4 == 1){
		 
		 wavName = "tone.wav";
			float fs = 96000;
			float Ts = 1/fs;
			float fm = (float)opt2;
			float fm_ultra = (float)opt3;
			
			int i;
			int nSamples = (int)(5*fs);
			
			// Create a mono (1), 32-bit sound and set the duration
			Wave mySound = makeWave((int)fs,1,32);
			waveSetDuration( &mySound, 5 );
			
			double sound[1];
			
			float freq_radians_per_sample = fm*2*3.14159/fs;
			float freq_radians_per_sample_ultra = fm_ultra*2*3.14159/fs;
			float phase = 0.0;
			float phase2 = 0.0;
			
			for (i=0; i<nSamples; i++){
				phase += freq_radians_per_sample;
				phase2 += freq_radians_per_sample_ultra;
				
				sound[0] = 0.5*sin(phase)+ 0.5*sin(phase2);
				
				waveAddSample( &mySound, &sound[0] );
			}
				
				waveToFile( &mySound, wavName );
			waveDestroy( &mySound );

		}else if (opt4 == 2){
			//hilbert
			float fs = 96000.0;
			float Ts = 1/fs;
			float fc = 40000.0;
			float fm = (float)opt2;
			int i;
			int nSamples = (int)(2*fs);
			
			// Create a mono (1), 32-bit sound and set the duration
			Wave mySound = makeWave((int)fs,1,32);
			waveSetDuration( &mySound, 2 );
			
			double sound[nSamples];
			double sound_aux[nSamples];
			double ylower[nSamples];
			
			float freq_radians_per_sample = fm*2*3.14159/fs;
			
			
			for (i=0; i<nSamples; i++){
				sound[i] = sin(freq_radians_per_sample*(float)i);
				
			}
				
			for (i=0; i<nSamples; i++){
				sound_aux[i] = sound[i];
			}
			
			hilbert(sound, 65536);
			
			for (i=0; i<nSamples; i++){
				ylower[i] =  sound_aux[i]*cos(fc*2*3.14159/fs*(float)i) + cimag(sound[i])*sin(fc*2*3.14159/fs*(float)i);
				waveAddSample( &mySound, &ylower[i] );
			}
				
			waveToFile( &mySound, "tone.wav");
			waveDestroy( &mySound );
			
			
		} else {
			
			wavName = "tone.wav";
			
			float fs = S_RATE_TONE;
			float Ts = 1/fs;
			float fm = (float)opt2;
			int i;
			int nSamples = (int)(DURATION*fs);
			// Create a mono (1), 32-bit sound and set the duration
			Wave mySound = makeWave((int)fs,1,32);
			waveSetDuration( &mySound, 5 );
			
			double sound[1];
			
			float freq_radians_per_sample = fm*2*3.14159/fs;
			float phase=0.0;
			for (i=0; i<nSamples; i++){
				phase += freq_radians_per_sample;
				sound[0] = sin(phase);
				waveAddSample( &mySound, &sound[0] );
			}
				
			waveToFile( &mySound, wavName);
			waveDestroy( &mySound );

		}
		
		
	}	else if (opt == 1 ){ 
	
			wavName = "white.wav";
				
			float sampleRate = (float)S_RATE_TONE;
			float duration = 2;       // seconds

				
			int nSamples = (int)(duration*sampleRate);
			
			// Create a mono (1), 32-bit sound and set the duration
			Wave mySound = makeWave((int)sampleRate,1,32);
			waveSetDuration( &mySound, duration );

			// Add all of the data
			int i;
			double frameData[1];
			for(i=0; i<nSamples; i+=1 ){
				
				frameData[0] = randfloat() * 2.0 - 1;
				waveAddSample( &mySound, frameData );
			}

			// Write it to a file and clean up when done
			waveToFile( &mySound, wavName);
			waveDestroy( &mySound );
			
	} 	else if( opt == 2) 	{
		
		if( opt2 == 1 && opt4 == 0) wavName = "./sounds/bird/1.wav";
		if( opt2 == 2 && opt4 == 0) wavName = "./sounds/bird/2.wav";
		if( opt2 == 3 && opt4 == 0) wavName = "./sounds/bird/3.wav";
		
		if( opt3 == 1 && opt4 == 1) wavName = "./sounds/pred/1.wav";
		if( opt3 == 2 && opt4 == 1) wavName = "./sounds/pred/2.wav";
		if( opt3 == 3 && opt4 == 1) wavName = "./sounds/pred/3.wav";
		
	}	

		
	return wavName;
}

