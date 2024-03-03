#ifndef EXH_SYSTEM_UI_INPUT_INPUT_H_
#define EXH_SYSTEM_UI_INPUT_INPUT_H_

extern int isUserExit(void);

extern void inputGetMouseState(int* x, int* y);

#ifdef JAVASCRIPT
extern void jsSetUseInput(int _useInput);
extern int jsIsUseInput(void);
#endif

#endif /*EXH_SYSTEM_UI_INPUT_INPUT_H_*/
