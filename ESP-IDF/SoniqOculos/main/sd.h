#include "main.h"

#include "nvs_tools.h"

#define MOUNT_POINT "/sdcard"
#define FILE_NAME "rec"

#define WRITE "wb"
#define READ "rb"

#define SD_CLK_PIN GPIO_NUM_16 // RX2
#define SD_MISO_PIN GPIO_NUM_4
#define SD_MOSI_PIN GPIO_NUM_2
#define SD_CS_PIN GPIO_NUM_15

#define SD_DET_PIN GPIO_NUM_35

#define SD_DET_DELAY 50 // ms

#define SD_DET_STACK_DEPTH 3 * 1024

//* WAV File

/*//Standard values for CD-quality audio
#define SUBCHUNK1SIZE (16)
#define AUDIO_FORMAT (1) //For PCM
#define NUM_CHANNELS (2)
#define SAMPLE_RATE (44100)

#define BITS_PER_SAMPLE (16)

#define BYTE_RATE (SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8)
#define BLOCK_ALIGN (NUM_CHANNELS * BITS_PER_SAMPLE / 8)

typedef struct wavfile_header_s
{
    char ChunkID[4];   //  4
    int32_t ChunkSize; //  4
    char Format[4];    //  4

    char Subchunk1ID[4];   //  4
    int32_t Subchunk1Size; //  4
    int16_t AudioFormat;   //  2
    int16_t NumChannels;   //  2
    int32_t SampleRate;    //  4
    int32_t ByteRate;      //  4
    int16_t BlockAlign;    //  2
    int16_t BitsPerSample; //  2

    char Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;*/

/*typedef struct _wav_header
{
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size = 0;    // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4];      // Contains "fmt " (includes trailing space)
    int fmt_chunk_size = 16; // Should be 16 for PCM
    short audio_format = 1;  // Should be 1 for PCM. 3 for IEEE Float
    short num_channels = 1;
    int sample_rate = 16000;
    int byte_rate = 32000;      // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment = 2; // num_channels * Bytes Per Sample
    short bit_depth = 16;       // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes = 0;  // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes

    _wav_header()
    {
        riff_header[0] = 'R';
        riff_header[1] = 'I';
        riff_header[2] = 'F';
        riff_header[3] = 'F';
        wave_header[0] = 'W';
        wave_header[1] = 'A';
        wave_header[2] = 'V';
        wave_header[3] = 'E';
        fmt_header[0] = 'f';
        fmt_header[1] = 'm';
        fmt_header[2] = 't';
        fmt_header[3] = ' ';
        data_header[0] = 'd';
        data_header[1] = 'a';
        data_header[2] = 't';
        data_header[3] = 'a';
    }
} wav_header_t;*/

bool sd_card_state();

void sd_card_init();
void sd_card_deinit();

void spi_init();
//void spi_deinit();

void sd_open_file(char *file, char *type);
/**
 * @brief Close the file
 * 
 */
void sd_close_file();

/**
 * @brief Write data to SD card
 * 
 * @param data buffer of samples to write
 * @param len buffer length
 */
void sd_write_data(uint8_t *data, size_t *len);

void sd_card_toggle();
