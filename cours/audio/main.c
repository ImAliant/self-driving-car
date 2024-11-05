#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <sndfile.h>

#include "audio.h"
#include "vcd.h"

const size_t sample_rate = Audio__period;

void die(const char *message) {
  fprintf(stderr, message);
  exit(EXIT_FAILURE);
}

SNDFILE *file_out = NULL;

int main(int argc, char** argv)
{
  Audio__main_mem mem;
  Audio__main_out res;
  SDL_AudioSpec spec;
  SDL_AudioDeviceID dev;
  int opt = -1;
  bool quiet = false;
  size_t max_sec = SIZE_MAX;    /* largest value of type size_t */
  const char *filename = NULL;
  Uint32 buffered;

  while ((opt = getopt(argc, argv, "ho:qm:t:")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: %s OPTIONS\n", argv[0]);
      printf("Options:\n");
      printf("  -o <file.wav>   write samples to <file.wav>\n");
      printf("  -q              do not play sound\n");
      printf("  -m <sec>        play for <sec> seconds\n");
      printf("  -t <file.vcd>   dump traces in <file.vcd>\n");
      printf("  -h              display this message\n");
      return 0;
    case 'q':
      quiet = true;
      break;
    case 'o':
      filename = optarg;
      break;
    case 'm':
      max_sec = atoi(optarg);
      break;
    case 't':
      hept_vcd_init(optarg, VCD_TIME_UNIT_US, 20);
      break;
    default:
      fprintf(stderr, "Unknown option '%c'\n", opt);
      exit(EXIT_FAILURE);
    }
  }

  if (SDL_Init(SDL_INIT_AUDIO) < 0)
    die("Could not initialize SDL2\n");

  /* Specification of requested output device. */
  bzero(&spec, sizeof spec);
  spec.freq = sample_rate;      /* Samples per second */
  spec.format = AUDIO_F32;      /* Sample format: IEEE-754 32 bits */
  spec.channels = 2;            /* Two channels */
  spec.samples = 4096;          /* Buffers sized 4 KiB */
  spec.callback = NULL;

  if (!(dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0)))
    die("Could not open audio device\n");

  if (filename != NULL) {
    /* Specification of requested output file, if any. */
    SF_INFO info_out;
    bzero(&info_out, sizeof info_out);
    info_out.channels = 2;                              /* Two channels */
    info_out.samplerate = sample_rate;                  /* Samples per second */
    info_out.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; /* File format */

    if (!(file_out = sf_open(filename, SFM_WRITE, &info_out))) {
      fprintf(stderr, "Could not open WAV file %s for writing\n", argv[1]);
      SDL_Quit();
      exit(EXIT_FAILURE);
    }
  }

  Audio__main_reset(&mem);
  float *buffer = calloc(spec.samples, sizeof *buffer);
  SDL_PauseAudioDevice(dev, 0);

  /* Loop until we've produced the requested amount of samples, that is the
     duration in seconds multiplied by the number of samples per second. This
     number of samples shall be sent on each of both stereo channels.

     Each iteration sends spec.samples stereo samples to the audio device,
     hence we halve it to get the number of generated samples per-channel. */
  for (size_t samples = 0;
       samples < max_sec * sample_rate;
       samples += spec.samples / 2) {

    /* Print sound progress. */
    printf("\rSent %08zu samples", samples);
    if (max_sec != SIZE_MAX) {
      printf(" (%2.0f%)", 100. * (double)samples / (max_sec * sample_rate));
    }
    fflush(stdout);

    /* Exit immediately if requested, e.g., the user pressed Ctrl-C. */
    if (SDL_QuitRequested()) {
      printf("\n");
      return 1;
    }

    /* Step the node as much as necessary to fill a buffer. Each step produces
       one stereo sample. */
    for (size_t i = 0; i < spec.samples; i += 2) {
      Audio__main_step(&res, &mem);
      buffer[i+0] = res.o.l;
      buffer[i+1] = res.o.r;
    }

    /* Send the generated sound to the sound card and/or file. */
    if (!quiet)
      SDL_QueueAudio(dev, buffer, spec.samples * sizeof *buffer);
    if (file_out)
      sf_writef_float(file_out, buffer, spec.samples / 2);

    /* Throttle queued audio, otherwise we will certainly end up consuming all
       available memory. */
    buffered = SDL_GetQueuedAudioSize(dev);
    while (!quiet && buffered >= 1 << 22) {
      SDL_Delay(50);
      buffered = SDL_GetQueuedAudioSize(dev);
    }
  }
  printf("\n");

  /* Wait until the audio buffer is empty. */
  printf("Waiting for queue flush... "); fflush(stdout);
  while ((buffered = SDL_GetQueuedAudioSize(dev)) != 0)
    SDL_Delay(50);
  printf("done.\n");

  free(buffer);
  if (file_out)
    sf_close(file_out);
  SDL_Quit();

  return 0;
}
