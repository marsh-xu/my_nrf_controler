#ifndef MUSIC_H_
#define MUSIC_H_

typedef enum music_control_cmd_e
{
    MUSIC_PLAY_PAUSE    = 0x01,
    MUSIC_PREVIOUS_SONG = 0x02,
    MUSIC_NEXT_SONG     = 0x03,
    MUSIC_VOL_PLUS      = 0x04,
    MUSIC_VOL_SUB       = 0x05,
} music_control_cmd_t;

void music_control_init(void);

void music_control(music_control_cmd_t cmd);

#endif // MUSIC_H_
