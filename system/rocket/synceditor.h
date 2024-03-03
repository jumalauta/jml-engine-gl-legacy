#ifndef EXH_SYSTEM_ROCKET_SYNCEDITOR_H_
#define EXH_SYSTEM_ROCKET_SYNCEDITOR_H_

extern void syncEditorSetRowsPerBeat(int _rowsPerBeat);
extern int syncEditorGetRowsPerBeat(void);
extern void* syncEditorGetTrack(const char *trackName);
extern double syncEditorGetTrackCurrentValue(void *trackPointer);
extern int isSyncEditor(void);

extern int syncEditorInit(void);
extern void syncEditorRun(void);
extern void syncEditorDeinit(void);

#endif /*EXH_SYSTEM_ROCKET_SYNCEDITOR_H_*/
