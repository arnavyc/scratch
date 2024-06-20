/**
 * SPDX-FileCopyrightText: 2021 arnavyc
 *
 * SPDX-License-Identifier: 0BSD
 *
 * Distributed under the BSD Zero Clause License
 * (See accompanying file LICENSE)
 */

#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ALSA_DEVICE "default"

void get_alsa_hw_info(snd_pcm_t *pcm_handle, snd_pcm_hw_params_t *params) {
  fprintf(stderr, "PCM properties:\n");

  /* get name of ALSA device */
  fprintf(stderr, "\tPCM name: %s\n", snd_pcm_name(pcm_handle));

  /* get name of state of the ALSA device */
  fprintf(stderr, "\tPCM state: %s\n",
          snd_pcm_state_name(snd_pcm_state(pcm_handle)));

  /* get number of channels */
  unsigned int channels = 0;
  snd_pcm_hw_params_get_channels(params, &channels);
  fprintf(stderr, "\tNumber of channels: %u\n", channels);

  snd_pcm_uframes_t no_of_frames_1period;
  snd_pcm_hw_params_get_period_size(params, &no_of_frames_1period, 0);
  fprintf(stderr, "\tNumber of frames in 1 period: %lu\n",
          no_of_frames_1period);

  unsigned int period_time;
  snd_pcm_hw_params_get_period_time(params, &period_time, 0);
  fprintf(stderr, "\tPeriod time: %u\n", period_time);

  unsigned int samplerate;
  snd_pcm_hw_params_get_rate(params, &samplerate, 0);
  fprintf(stderr, "\tSample rate: %u Hz\n", samplerate);
}

void play_sound(SNDFILE *inFile, SF_INFO inFileInfo) {
  snd_pcm_t *pcm_handle;
  unsigned int errcode =
      snd_pcm_open(&pcm_handle, ALSA_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
  if (pcm_handle < 0) {
    printf("Couldn't open '%s' device: %s", ALSA_DEVICE, snd_strerror(errcode));
    exit(1);
  }

  snd_pcm_hw_params_t *params;
  snd_pcm_hw_params_alloca(&params);

  snd_pcm_hw_params_any(pcm_handle, params);

  errcode = snd_pcm_hw_params_set_access(pcm_handle, params,
                                         SND_PCM_ACCESS_RW_INTERLEAVED);
  if (errcode < 0) {
    printf("Can't set interleaved mode. %s\n", snd_strerror(errcode));
    exit(1);
  }

  errcode =
      snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_FLOAT_LE);
  if (errcode < 0) {
    printf("Can't set format. %s\n", snd_strerror(errcode));
    exit(1);
  }

  int channels = inFileInfo.channels;
  errcode = snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
  if (errcode < 0) {
    printf("Can't set number of channels. %s\n", snd_strerror(errcode));
    exit(1);
  }

  unsigned int samplerate = inFileInfo.samplerate;
  errcode = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &samplerate, 0);
  if (errcode < 0) {
    printf("Can't set rate. %s\n", snd_strerror(errcode));
    exit(1);
  }

  errcode = snd_pcm_hw_params(pcm_handle, params);
  if (errcode < 0) {
    printf("Can't set hardware parameters. %s\n", snd_strerror(errcode));
    exit(1);
  }

  get_alsa_hw_info(pcm_handle, params);

  snd_pcm_uframes_t frames;
  snd_pcm_hw_params_get_period_size(params, &frames, 0);

  int buff_size = frames * channels * sizeof(float);
  float *buff = (float *)malloc(buff_size);

  size_t count;
  int pcm;
  while ((count = sf_readf_float(inFile, buff, frames)) != 0) {
    if ((pcm = snd_pcm_writei(pcm_handle, buff, count)) == -EPIPE) {
      fprintf(stderr, "XRUN.\n");
      snd_pcm_prepare(pcm_handle);
    } else if (pcm < 0) {
      fprintf(stderr, "ERROR: Can't write to PCM device. %s\n",
              snd_strerror(pcm));
    }
  }

  snd_pcm_drain(pcm_handle);
  snd_pcm_close(pcm_handle);
  free(buff);
}

int main(int argc, char *argv[]) {
  setvbuf(stderr, NULL, _IONBF, 0);
  char *inFileName;
  SNDFILE *inFile;
  SF_INFO inFileInfo;

  if (argc < 2) {
    fprintf(stderr, "syntax: %s <filename>\n", argv[0]);
    return 1;
  }

  inFileName = argv[1];

  inFile = sf_open(inFileName, SFM_READ, &inFileInfo);

  play_sound(inFile, inFileInfo);
  sf_close(inFile);

  return 0;
}
