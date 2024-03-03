#ifndef EXH_SYSTEM_TIMER_TIMER_H_
#define EXH_SYSTEM_TIMER_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *name;
	double time;
} timerCounter_t;

extern timerCounter_t *timerCounterStart(const char *counterName);
extern double timerCounterEnd(timerCounter_t *counter);

extern void timerInit(double newEndTime);
extern void timerUpdate(void);
extern int timerIsAddTimeGracePeriod();
extern void timerAddTime(double at);
extern void timerSetTime(double time);
extern int timerIsPause();
extern void timerPause();
extern double timerGetBeatsPerMinute(void);
extern void timerSetBeatsPerMinute(double bpm);
extern double timerGetBeatInSeconds(void);
extern double timerGetCurrentBeat(void);
extern void timerSetTargetFps(double fps);
extern double timerGetTargetFps();
extern double timerGetSeconds(void);
extern double timerGetTime(void);
extern double timerGetEndTime(void);
extern int timerIsEnd(void);
extern void timerAdjustFramerate(void);
extern double timerGetFpsCorrection(void);
extern void timerSetTimeString(char* timeString);

extern double convertTimeToSeconds(const char *time);
extern void convertSecondsToTime(double seconds, char *time);
extern void convertSecondsToAbsoluteTime(double seconds, char *time);
extern void timerSleep(int millis);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /* EXH_SYSTEM_TIMER_TIMER_H_ */
