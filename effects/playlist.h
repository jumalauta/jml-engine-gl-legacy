#ifndef EXH_EFFECTS_PLAYLIST_H_
#define EXH_EFFECTS_PLAYLIST_H_

extern const char *getPlaylistMusic(void);
extern float getPlaylistLength(void);
extern void playlistInit(void);
extern void setPlaylistMusic(const char *file);
extern void setPlaylistLength(const char *length);

#endif /*EXH_EFFECTS_PLAYLIST_H_*/
