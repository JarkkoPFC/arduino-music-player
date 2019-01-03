#include "pmf_player.h"

static const uint8_t PROGMEM s_pmf_file[]=
{
#include "music.h"
};

static pmf_player s_player;

void setup()
{
  s_player.start(s_pmf_file);
}

void loop()
{
  s_player.update();
}
