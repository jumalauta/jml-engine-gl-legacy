#ifndef EXH_SYSTEM_GRAPHICS_CAMERA_H_
#define EXH_SYSTEM_GRAPHICS_CAMERA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Camera singleton data
 */
typedef struct {
	point3d_t position;
	point3d_t lookAt;
	point3d_t up;
	object3d_t *positionObject;
	object3d_t *targetObject;
	double fovy, aspect, zNear, zFar;
} camera_t;

extern camera_t* getCamera();

extern void setCameraPositionObject(object3d_t *object);
extern void setCameraTargetObject(object3d_t *object);
extern void setCameraPerspective(double fovy, double aspect, double zNear, double zFar);
extern void setCameraPosition(float x, float y, float z);
extern void setCameraLookAt(float x, float y, float z);
extern void setCameraUpVector(float x, float y, float z);
extern void cameraInit();

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*EXH_SYSTEM_GRAPHICS_CAMERA_H_*/
