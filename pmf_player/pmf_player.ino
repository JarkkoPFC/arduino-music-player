#include "pmf_player.h"

//============================================================================
// music data
//============================================================================
static const uint8_t PROGMEM s_pmf_file[]=
{
#include "music.h"
};
//----------------------------------------------------------------------------


//============================================================================
// globals
//============================================================================
static pmf_player s_player;
static unsigned s_effect_channel=0;
//----------------------------------------------------------------------------


//============================================================================
// row_callback_test
//============================================================================
void row_callback_test(void *custom_data_, uint8_t channel_idx_, uint8_t &note_idx_, uint8_t &inst_idx_, uint8_t &volume_, uint8_t &effect_, uint8_t &effect_data_)
{
  if(channel_idx_==s_effect_channel)
  {
    static unsigned s_counter=1;
    if(--s_counter==0)
    {
      note_idx_=0+5*12; // C-5 (note+octave*12, note: 0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F, 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B)
      inst_idx_=2;      // sample 2
      volume_=63;       // volume 63 (max)
      s_counter=8;      // hit note every 8th row
    }
  }
}
//----------------------------------------------------------------------------


//============================================================================
// setup
//============================================================================
void setup()
{
#ifdef PMF_SERIAL_LOGS
  // setup serial logging
  Serial.begin(9600);
  delay(1000);
#endif

  s_player.load(s_pmf_file);

/*
  // Uncomment this code block to demo code-controlled effect. The code adds 13th channel to Aryx and plays drum beat every 8th row on the channel
  s_effect_channel=s_player.num_playback_channels();
  s_player.enable_playback_channels(s_player.num_playback_channels()+1); // add one extra audio channel for audio effects
  s_player.set_row_callback(&row_callback_test); // setup row callback for the effect
*/

  s_player.start();
}
//----------------------------------------------------------------------------


//============================================================================
// loop
//============================================================================
void loop()
{
  s_player.update(); // keep updating the audio buffer...
}
//----------------------------------------------------------------------------
