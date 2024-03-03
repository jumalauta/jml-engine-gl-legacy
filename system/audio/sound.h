#ifndef EXH_SYSTEM_AUDIO_SOUND_H_
#define EXH_SYSTEM_AUDIO_SOUND_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int soundGetOrder(void);
extern int soundGetRow(void);
extern int soundIsSync(int instrument);
extern void soundInit(void);
extern void soundLoadPlaylist(const char *_filename);
extern int soundAddSongToPlaylist(const char *_filename, const char *title, int length);
extern void soundLoadSong(int song_number);
extern double soundGetSongCurrentPlayTime(void);
extern void soundPlaySong(int song_number);
extern void soundPause(void);
extern void soundStop(void);
extern void soundMute(int _mute_sound);
extern int soundIsMute(void);
extern void soundPreviousTrack(void);
extern void soundNextTrack(void);
extern int soundGetPlaylistSize(void);
extern int soundGetCurrentSong(void);
extern int soundGetSongLength(int song_number);
extern const char* soundGetSongFilename(int song_number);
extern const char* soundGetSongName(int song_number);
extern int soundGetTrackNumber(int song_number);
extern int soundIsPlaying(void);
extern void soundCheckPlayer(void);
extern void soundClearPlaylist(void);
extern void soundDeinit(void);
extern void soundSetPosition(double position);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*EXH_SYSTEM_AUDIO_SOUND_H_*/
