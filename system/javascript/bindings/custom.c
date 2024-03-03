#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>
#include <math.h>

#include "system/debug/debug.h"
#include "system/javascript/javascript.h"

#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/graphics/shader/shader.h"
#include "system/graphics/font/font.h"
#include "system/graphics/image/image.h"
#include "system/graphics/object/object3d.h"
#include "system/graphics/object/basic3dshapes.h"
#include "system/graphics/object/lighting.h"
#include "system/graphics/particle/particle.h"
#include "system/datatypes/memory.h"
#include "system/math/splines/spline.h"
#include "system/ui/window/window.h"
#include "system/timer/timer.h"

#include "bindings.h"

/*
#ifdef ALTIVEC
#include <altivec.h>

#define av_float altivec float
#define av_mul vec_mul
#define av_div vec_div
#define av_add vec_add
#define av_sub vec_sub
#define av_sqrt vec_sqrt
#define av_max vec_max
#define av_abs vec_abs

#else
*/

#define av_float float
#define av_mul(a,b) ((a)*(b))
#define av_div(a,b) ((a)/(b))
#define av_add(a,b) ((a)+(b))
#define av_sub(a,b) ((a)-(b))
#define av_sqrt(a) sqrtf(a)
#define av_max(a,b) getMax((a),(b))
#define av_abs(a) fabs((a))

//#endif

//#version 120

//beers and greetings to iq and mercury for giving better comprehension about the subject

#define M_PI 3.1415926535897932f
#define M_PI2 6.283185307179586f

typedef struct {
    av_float f[3];
} vec3_t;

static vec3_t cameraEye = {{0.0f,0.0f,-2.4f}};
static int rayMaxSteps = 40;
static float rayHitThreshold = 0.02f;
static float zFar = 3.0f;
//static float time = 0.0f;
static float twisterTwists = 0.0f;
static float twisterRotateY = 0.0f;
static float twisterRotateX = 0.0f;

// WARNING: about to fuckup negative numbers but doesn't visually matter
#define SINE_PRECISION 250
static float *sinTable = NULL;
static float *cosTable = NULL;
static void preCalcSineTables() {
    sinTable = (float*)malloc(SINE_PRECISION*sizeof(float));
    assert(sinTable);
    cosTable = (float*)malloc(SINE_PRECISION*sizeof(float));
    assert(cosTable);

    float precInc = M_PI2 / (float)SINE_PRECISION;
    for(int i = 0; i < SINE_PRECISION; i++) {
        sinTable[i] = sinf(precInc * i);
        cosTable[i] = cosf(precInc * i);
    }
}

static inline int getTableIndex(float radians) {
    return (int)((fmodf(fabs(radians), M_PI2) / (M_PI2)) * SINE_PRECISION);
}
static inline float calcSin(int index) {
    return sinTable[index];
}

static inline float calcCos(int index) {
    return cosTable[index];
}

static inline void traceRotateY(vec3_t* v, float radians)
{
    //radians = fmodf(radians, M_PI*2);
    //float radians = degrees*(M_PI/180.0);

    int index = getTableIndex(radians);
    av_float cosrad = calcCos(index);
    av_float sinrad = calcSin(index);

    av_float x = v->f[0];
    //av_float y = v->f[1];
    av_float z = v->f[2];

    v->f[0] = av_add(av_mul(x, cosrad), av_mul(z, sinrad));
    v->f[2] = av_add(av_mul(-x, sinrad), av_mul(z, cosrad));
}

static inline void traceRotateX(vec3_t* v, float radians)
{
    //float radians = degrees*(M_PI/180.0);
    int index = getTableIndex(radians);
    av_float cosrad = calcCos(index);
    av_float sinrad = calcSin(index);

    //av_float x = v->f[0];
    av_float y = v->f[1];
    av_float z = v->f[2];

    v->f[1] = av_sub(av_mul(y, cosrad), av_mul(z, sinrad));
    v->f[2] = av_add(av_mul(y, sinrad), av_mul(z, cosrad));
}

/*
//doesn't seem to halp with speed
static float sqrtQuake3(const float x)
{
    const float xhalf = 0.5f*x;

    union // get bits for floating value
    {
        float x;
        int i;
    } u;
    u.x = x;
    u.i = 0x5f3759df - (u.i >> 1);  // gives initial guess y0
    return x*u.x*(1.5f - xhalf*u.x*u.x);// Newton step, repeating increases accuracy 
}   
*/

//https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
static inline av_float vecLength (vec3_t* p) {
    return av_sqrt(av_add(av_add(av_mul(p->f[0], p->f[0]), av_mul(p->f[1], p->f[1])), av_mul(p->f[2], p->f[2])));
}

static inline vec3_t* vecNormalize (vec3_t* p) {
    av_float l = vecLength(p);
    p->f[0] = av_div(p->f[0], l);
    p->f[1] = av_div(p->f[1], l);
    p->f[2] = av_div(p->f[2], l);

    return p;
}

static inline vec3_t* vecMax (vec3_t* p, av_float value) {
    p->f[0] = av_max(p->f[0], value);
    p->f[1] = av_max(p->f[1], value);
    p->f[2] = av_max(p->f[2], value);
    return p;
}

static inline vec3_t* vecAbs (vec3_t* p) {
    p->f[0] = av_abs(p->f[0]);
    p->f[1] = av_abs(p->f[1]);
    p->f[2] = av_abs(p->f[2]);
    return p;
}

static inline vec3_t* vecSub (vec3_t* p1, vec3_t* p2) {
    p1->f[0] = av_sub(p1->f[0], p2->f[0]);
    p1->f[1] = av_sub(p1->f[1], p2->f[1]);
    p1->f[2] = av_sub(p1->f[2], p2->f[2]);
    return p1;
}

/*static inline float udBox( vec3_t *p, vec3_t *b )
{
    return vecLength(vecMax(vecSub(vecAbs(p), b), 0.0));
}*/
static inline float udBox( vec3_t *p )
{
    p->f[0] = av_sub(av_abs(p->f[0]), 1.0f);
    p->f[1] = av_sub(av_abs(p->f[1]), 1.0f);
    p->f[2] = av_sub(av_abs(p->f[2]), 1.0f);

    return vecLength(vecMax(p, 0.0));
}

static inline float calculateDistanceMap(vec3_t* p)
{
    traceRotateY(p, twisterRotateY+p->f[1]*twisterTwists);
    traceRotateX(p, twisterRotateX+p->f[0]*twisterTwists);
    //vec3_t size = {1.0, 1.0, 1.0};
    //return udBox(p, &size);
    return udBox(p);
}

static vec3_t *cameraLookUp = NULL;

#define PASS_TRACE_INIT 1
static int passTrace = PASS_TRACE_INIT;
static vec3_t raymarch(vec3_t* cameraTarget)
{
    //twisterTwists = getSceneTimeFromStart();
    //vec3_t coord = {u,v,1.0};
    //vec3_t* cameraTarget = vecNormalize(&coord);

    vec3_t color = {{0.0f,0.0f,0.0f}};

    float rayDistance = 1.0f;
    vec3_t rayPosition;
    for(int i = 0; i < rayMaxSteps; i++)
    {
        rayPosition.f[0] = /*cameraEye.f[0]+*/cameraTarget->f[0]*rayDistance;
        rayPosition.f[1] = /*cameraEye.f[1]+*/cameraTarget->f[1]*rayDistance;
        rayPosition.f[2] = cameraEye.f[2]+cameraTarget->f[2]*rayDistance;
        float zetaa = rayPosition.f[2];

        float distanceToSolid = calculateDistanceMap(&rayPosition);
        if (distanceToSolid < rayHitThreshold)
        {
            passTrace = PASS_TRACE_INIT;
            float val = fabs(zetaa/zFar)*2.1;            
            color.f[0] = val;
            color.f[1] = val;
            color.f[2] = val;
            break;
        }

        rayDistance += distanceToSolid;
        if (rayDistance >= zFar)
        {
            passTrace=PASS_TRACE_INIT*3;
            break;
        }
    }

    return color;
}

static texture_t* rayMarcherTexture = NULL;
#define TEX_WIDTH 200
#define TEX_HEIGHT 200
//#define TEX_CHANNELS 4
//#define TEX_FORMAT GL_RGBA
#define TEX_CHANNELS 2
#define TEX_FORMAT GL_LUMINANCE_ALPHA

static unsigned char *texData = NULL;

static int duk_initRayMarching(duk_context *ctx)
{
    texData = (unsigned char*)malloc(sizeof(unsigned char)*(TEX_HEIGHT*TEX_WIDTH*TEX_CHANNELS+TEX_CHANNELS));
    assert(texData);

    cameraLookUp = (vec3_t*)malloc(TEX_HEIGHT*TEX_WIDTH*sizeof(vec3_t));
    assert(cameraLookUp);
    for(int y = 0; y < TEX_HEIGHT; y++)
    {
        float v = (y/(float)TEX_HEIGHT)*2.0-1.0;
        for(int x = 0; x < TEX_WIDTH; x++)
        {
            float u = (x/(float)TEX_WIDTH)*2.0-1.0;

            vec3_t *p = &cameraLookUp[y*TEX_WIDTH+x];
            p->f[0] = u;
            p->f[1] = v;
            p->f[2] = 1.0;
            vecNormalize(p);
        }
    }

    rayMarcherTexture = imageCreateTexture("raymarcher", 0, TEX_FORMAT, TEX_WIDTH, TEX_HEIGHT);
    assert(rayMarcherTexture);

    return 0;
}

static int duk_deinitRayMarching(duk_context *ctx)
{
    if (rayMarcherTexture) {
        textureDeinit(rayMarcherTexture);
        rayMarcherTexture = NULL;
    }
    if (cameraLookUp) {
        free(cameraLookUp);
        cameraLookUp = NULL;
    }
    if (texData) {
        free(texData);
        texData = NULL;
    }
    if (sinTable) {
        free(sinTable);
        sinTable = NULL;
    }
    if (cosTable) {
        free(cosTable);
        cosTable = NULL;
    }
    return 0;
}

static void renderRayMarchingToTexture()
{
    int y1 = 0;
    int x1 = 0;
    static int drawPass = 0;
    #define INTERLACES 3
    for(int y = ((drawPass++)%INTERLACES); y < TEX_HEIGHT; y+=INTERLACES)
    {
        for(int x = x1; x < TEX_WIDTH-2; x+=passTrace)
        {
            int ti = (y*TEX_WIDTH + x) * TEX_CHANNELS;

            vec3_t color = raymarch(&cameraLookUp[y*TEX_WIDTH+x]);
            if (color.f[0] == 0.0) {
                for (int i = ti; i < ti + passTrace*TEX_CHANNELS; i+=TEX_CHANNELS)
                {
                    texData[i + 0] = 0x00;
#if TEX_CHANNELS == 2
                    texData[i + 1] = 0x00;
#elif TEX_CHANNELS == 4
                    texData[i + 1] = 0x00;
                    texData[i + 2] = 0x00;
                    texData[i + 3] = 0x00;
#endif
                }
                continue;
            }

            x++;
            texData[ti + 0] = color.f[0] * 0xFF;
#if TEX_CHANNELS == 1
            texData[ti + 1] = texData[ti + 0];
#elif TEX_CHANNELS == 2
            texData[ti + 1] = 0xFF;
            texData[ti + 2] = texData[ti + 0];
            texData[ti + 3] = 0xFF;
#elif TEX_CHANNELS == 4
            texData[ti + 1] = color.f[1] * 0xFF;
            texData[ti + 2] = color.f[2] * 0xFF;
            texData[ti + 3] = 0xFF;
            texData[ti + 4] = texData[ti + 0];
            texData[ti + 5] = texData[ti + 1];
            texData[ti + 6] = texData[ti + 2];
            texData[ti + 7] = texData[ti + 3];
#else
            #error "Channel amount not supported you fucking fucker"
#endif
        }
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rayMarcherTexture->id);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rayMarcherTexture->w, rayMarcherTexture->h, TEX_FORMAT, GL_UNSIGNED_BYTE, (const void *)texData);

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (drawPass < INTERLACES-1) {
        // prebake the first rounds of interlacing
        renderRayMarchingToTexture();
    }
}


static int duk_drawRayMarching(duk_context *ctx)
{
    twisterTwists = (double)duk_get_number(ctx, 0);
    twisterRotateX = (double)duk_get_number(ctx, 1);
    twisterRotateY = (double)duk_get_number(ctx, 2);

    renderRayMarchingToTexture();

    float sw = 512.0f*0.83f;
    float sh = 288.0f*0.83f;
    perspective2dBegin(sw,sh);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, rayMarcherTexture->id);

    float x=sw/2.0f - rayMarcherTexture->w/2.0f;
    float y=sh/2.0f - rayMarcherTexture->h/2.0f;

    glBegin(GL_QUADS);
    glMultiTexCoord2f(GL_TEXTURE0, rayMarcherTexture->uMax,rayMarcherTexture->vMax);
    glVertex3f(x+rayMarcherTexture->w,y+rayMarcherTexture->h,0);
    glMultiTexCoord2f(GL_TEXTURE0, rayMarcherTexture->uMin,rayMarcherTexture->vMax);
    glVertex3f(x,y+rayMarcherTexture->h,0);
    glMultiTexCoord2f(GL_TEXTURE0, rayMarcherTexture->uMin,rayMarcherTexture->vMin);
    glVertex3f(x,y,0);
    glMultiTexCoord2f(GL_TEXTURE0, rayMarcherTexture->uMax,rayMarcherTexture->vMin);
    glVertex3f(x+rayMarcherTexture->w,y,0);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    perspective2dEnd();

    return 0;
}

static int duk_drawCube(duk_context *ctx)
{
    glBegin(GL_QUADS);
    //top
    glNormal3f( 0, 1, 0 );
    glTexCoord2f(1.0,1.0);
    glVertex3f( 0.5, 0.5, -0.5);
    glNormal3f( 0, 1, 0 );
    glTexCoord2f(0.0,1.0);
    glVertex3f(-0.5, 0.5, -0.5);
    glNormal3f( 0, 1, 0 );
    glTexCoord2f(0.0,0.0);
    glVertex3f(-0.5, 0.5,  0.5);
    glNormal3f( 0, 1, 0 );
    glTexCoord2f(1.0,0.0);
    glVertex3f( 0.5, 0.5,  0.5);

    //bottom
    glNormal3f( 0, -1, 0 );
    glTexCoord2f(1.0,1.0);
    glVertex3f( 0.5, -0.5,  0.5);
    glNormal3f( 0, -1, 0 );
    glTexCoord2f(0.0,1.0);
    glVertex3f(-0.5, -0.5,  0.5);
    glNormal3f( 0, -1, 0 );
    glTexCoord2f(0.0,0.0);
    glVertex3f(-0.5, -0.5, -0.5);
    glNormal3f( 0, -1, 0 );
    glTexCoord2f(1.0,0.0);
    glVertex3f( 0.5, -0.5, -0.5);

    //front
    glNormal3f( 0, 0, 1 );
    glTexCoord2f(1.0,1.0);
    glVertex3f( 0.5,  0.5, 0.5);
    glNormal3f( 0, 0, 1 );
    glTexCoord2f(0.0,1.0);
    glVertex3f(-0.5,  0.5, 0.5);
    glNormal3f( 0, 0, 1 );
    glTexCoord2f(0.0,0.0);
    glVertex3f(-0.5, -0.5, 0.5);
    glNormal3f( 0, 0, 1 );
    glTexCoord2f(1.0,0.0);
    glVertex3f( 0.5, -0.5, 0.5);

    //back
    glNormal3f( 0, 0, -1 );
    glTexCoord2f(1.0,1.0);
    glVertex3f( 0.5, -0.5, -0.5);
    glNormal3f( 0, 0, -1 );
    glTexCoord2f(0.0,1.0);
    glVertex3f(-0.5, -0.5, -0.5);
    glNormal3f( 0, 0, -1 );
    glTexCoord2f(0.0,0.0);
    glVertex3f(-0.5,  0.5, -0.5);
    glNormal3f( 0, 0, -1 );
    glTexCoord2f(1.0,0.0);
    glVertex3f( 0.5,  0.5, -0.5);

    //left
    glNormal3f( -1, 0, 0 );
    glTexCoord2f(1.0,1.0);
    glVertex3f(-0.5,  0.5,  0.5);
    glNormal3f( -1, 0, 0 );
    glTexCoord2f(0.0,1.0);
    glVertex3f(-0.5,  0.5, -0.5);
    glNormal3f( -1, 0, 0 );
    glTexCoord2f(0.0,0.0);
    glVertex3f(-0.5, -0.5, -0.5);
    glNormal3f( -1, 0, 0 );
    glTexCoord2f(1.0,0.0);
    glVertex3f(-0.5, -0.5,  0.5);

    //right
    glNormal3f( 1, 0, 0 );
    glTexCoord2f(1.0,1.0);
    glVertex3f(0.5,  0.5, -0.5);
    glNormal3f( 1, 0, 0 );
    glTexCoord2f(0.0,1.0);
    glVertex3f(0.5,  0.5,  0.5);
    glNormal3f( 1, 0, 0 );
    glTexCoord2f(0.0,0.0);
    glVertex3f(0.5, -0.5,  0.5);
    glNormal3f( 1, 0, 0 );
    glTexCoord2f(1.0,0.0);
    glVertex3f(0.5, -0.5, -0.5);

    glEnd();

    return 0;
}

static int duk_drawCross(duk_context *ctx)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-0.1,-0.3,-1);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(0.1,-0.3,-1);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(0.1,0.3,-1);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-0.1,0.3,-1);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-0.3,-0.1,-1);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(-0.1,-0.1,-1);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(-0.1,0.1,-1);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-0.3,0.1,-1);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(0.3,-0.1,-1);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.1,-0.1,-1);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.1,0.1,-1);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(0.3,0.1,-1);
    glEnd();

    return 0;
}

static int duk_drawFan(duk_context *ctx)
{
    float r1 = (float)duk_get_number(ctx, 0);
    float r2 = (float)duk_get_number(ctx, 1);
    float startP = (float)duk_get_number(ctx, 2);
    float endP = (float)duk_get_number(ctx, 3);
    float precision = (float)duk_get_number(ctx, 4);

    r2 = r1 + r2;

    int start_i = precision*startP;
    int end_i = start_i + precision*endP;
    glBegin(GL_TRIANGLES);
    for (int i=start_i; i < end_i; i++)
    {
        float z = -3;
        float angle = (i+precision)*2*M_PI/precision;
        float cosa = cosf(angle);
        float sina = sinf(angle);
        float ax1 = cosa * r1;
        float ay1 = sina * r1;
        float bx1 = cosa * r2;
        float by1 = sina * r2;

        angle = ((i+1)+precision)*2*M_PI/precision;
        cosa = cosf(angle);
        sina = sinf(angle);
        float ax2 = cosa * r1;
        float ay2 = sina * r1;
        float bx2 = cosa * r2;
        float by2 = sina * r2;

        glVertex3f(ax1,ay1,z);
        glVertex3f(bx1,by1,z);
        glVertex3f(ax2,ay2,z);

        glVertex3f(ax2,ay2,z);
        glVertex3f(bx2,by2,z);
        glVertex3f(bx1,by1,z);
    }
    glEnd();

    return 0;
}

//in theory noise texture size could be lowered
//without significant quality reduction
//but 512x512 is fine for now (or until more memory required)
#define NOISE_TEX_HEIGHT 512
#define NOISE_TEX_WIDTH 512
#define NOISE_UV_COUNT 60
#define NOISE_TEX_FORMAT GL_LUMINANCE
#define NOISE_TEX_CHANNELS 1
#if NOISE_TEX_CHANNELS != 1
#error "Invalid channel amount, you silly retard"
#endif

static texture_t* noiseTexture = NULL;
static float noiseTextureTexCoords[NOISE_UV_COUNT*2];

static int duk_initNoise(duk_context *ctx)
{
    int size = NOISE_TEX_HEIGHT*NOISE_TEX_WIDTH*NOISE_TEX_CHANNELS;
    unsigned char *noiseTexData = (unsigned char*)malloc(sizeof(unsigned char)*(size));
    assert(noiseTexData);

    noiseTexture = imageCreateTexture("noisetex", 1, NOISE_TEX_FORMAT, NOISE_TEX_WIDTH, NOISE_TEX_HEIGHT);
    assert(noiseTexture);

    for(int i = 0; i < size; i+=NOISE_TEX_CHANNELS)
    {
        noiseTexData[i] = (rand()%0xFF);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, noiseTexture->id);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, noiseTexture->w, noiseTexture->h, NOISE_TEX_FORMAT, GL_UNSIGNED_BYTE, (const void *)noiseTexData);

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    free(noiseTexData);

    for(int i = 0; i < NOISE_UV_COUNT*2; i++)
    {
        noiseTextureTexCoords[i] = (rand()%1000)/1000.0f;
    }

    return 0;
}
static int duk_deinitNoise(duk_context *ctx)
{
    if (noiseTexture) {
        textureDeinit(noiseTexture);
        noiseTexture = NULL;
    }

    return 0;
}
static int duk_drawNoise(duk_context *ctx)
{
    float alpha = (double)duk_get_number(ctx, 0);
    float size = (double)duk_get_number(ctx, 1);

    float w = getScreenWidth();
    float h = getScreenHeight();
    perspective2dBegin(w, h);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, noiseTexture->id);

    glColor4f(1.0f,1.0f,1.0f,alpha);
    const float x=0.0f;
    const float y=0.0f;
    static int noiseTexCoordIter = 0;

    noiseTexCoordIter = (noiseTexCoordIter+1)%NOISE_UV_COUNT;
    noiseTexture->uMin = noiseTextureTexCoords[noiseTexCoordIter * 2];
    noiseTexture->vMin = noiseTextureTexCoords[noiseTexCoordIter * 2 + 1];
    noiseTexture->uMax = noiseTexture->uMin + size;
    noiseTexture->vMax = noiseTexture->vMin + size;

    glBegin(GL_QUADS);
    glMultiTexCoord2f(GL_TEXTURE0, noiseTexture->uMax,noiseTexture->vMax);
    glVertex3f(x+w,y+h,0);
    glMultiTexCoord2f(GL_TEXTURE0, noiseTexture->uMin,noiseTexture->vMax);
    glVertex3f(x,y+h,0);
    glMultiTexCoord2f(GL_TEXTURE0, noiseTexture->uMin,noiseTexture->vMin);
    glVertex3f(x,y,0);
    glMultiTexCoord2f(GL_TEXTURE0, noiseTexture->uMax,noiseTexture->vMin);
    glVertex3f(x+w,y,0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);

    perspective2dEnd();

    return 0;
}

#define RTT_TEXTURES_COUNT 2
static texture_t* rttTextures[RTT_TEXTURES_COUNT] = {NULL, NULL};

static int duk_initRtt(duk_context *ctx)
{
    preCalcSineTables();

    rttTextures[0] = imageCreateTexture("rtt1", 1, GL_LUMINANCE_ALPHA, 16, 16);
    assert(rttTextures[0]);
    //rttTextures[1] = imageCreateTexture("rtt2", 1, GL_RGB, getScreenWidth(), getScreenHeight());
    rttTextures[1] = imageCreateTexture("rtt2", 1, GL_RGBA, 256, 256);
    assert(rttTextures[1]);

    return 0;
}

static int duk_deinitRtt(duk_context *ctx)
{
    return 0;
    for (int i = 0; i < RTT_TEXTURES_COUNT; i++) {
        if (rttTextures[i]) {
            textureDeinit(rttTextures[i]);
            rttTextures[i] = NULL;
        }
    }

    return 0;
}
static int duk_renderToTexture(duk_context *ctx)
{
    unsigned int rttIndex = (unsigned int)duk_get_uint(ctx, 0);
    assert(rttIndex < RTT_TEXTURES_COUNT);
    
    int width = (int)duk_get_int(ctx, 1);
    int height = (int)duk_get_int(ctx, 2);

    texture_t* rttTexture = rttTextures[rttIndex];

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rttTexture->id);
    //glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenPositionX(), getScreenPositionY(), width,height);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width,height,0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    return 0;
}

static int duk_drawRtt(duk_context *ctx)
{
    unsigned int rttIndex = (unsigned int)duk_get_uint(ctx, 0);
    assert(rttIndex < RTT_TEXTURES_COUNT);


    texture_t* rttTexture = rttTextures[rttIndex];
    float alpha = 1.0f;
    float w = getScreenWidth();
    float h = getScreenHeight();
    perspective2dBegin(w, h);
    //rttTexture->vMax = getScreenHeight()/512.0f;

    rttTexture->uMin = 0.0f;
    rttTexture->uMax = 1.0f;//getScreenWidth()/2.0f/512.0f;
    rttTexture->vMin = 0.0f;
    rttTexture->vMax = 1.0f;//getScreenHeight()/2.0f/512.0f;

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rttTexture->id);

    glColor4f(1.0f,1.0f,1.0f,alpha);
    const float x=0.0f;
    const float y=0.0f;
    static int noiseTexCoordIter = 0;

    glBegin(GL_QUADS);
    glMultiTexCoord2f(GL_TEXTURE0, rttTexture->uMax,rttTexture->vMax);
    glVertex3f(x+w,y+h,0);
    glMultiTexCoord2f(GL_TEXTURE0, rttTexture->uMin,rttTexture->vMax);
    glVertex3f(x,y+h,0);
    glMultiTexCoord2f(GL_TEXTURE0, rttTexture->uMin,rttTexture->vMin);
    glVertex3f(x,y,0);
    glMultiTexCoord2f(GL_TEXTURE0, rttTexture->uMax,rttTexture->vMin);
    glVertex3f(x+w,y,0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);

    perspective2dEnd();

    return 0;
}

static float vtYSpeed = 4.0f;
static float vtXSpeed = 4.0f;
static float vtYMag = 3.0f;
static float vtXMag = 3.0f;
static float vtGlobalMag = 1.0f;
static int vtColor = 1.0;

static int duk_objectSetVertexTransformVariables(duk_context *ctx)
{
    vtYSpeed = (float)duk_get_number(ctx, 0);
    vtXSpeed = (float)duk_get_number(ctx, 1);
    vtYMag = (float)duk_get_number(ctx, 2);
    vtXMag = (float)duk_get_number(ctx, 3);
    vtGlobalMag = (float)duk_get_number(ctx, 4);
    vtColor = (int)duk_get_int(ctx, 5);
    return 0;
}

void vertexTransformCustom(float* x, float* y, float* z) {
    double now = timerGetTime();

    float s = 1.0+(((calcSin(getTableIndex(now*vtYSpeed+(*y)*vtYMag)))+(calcCos(getTableIndex(now*vtXSpeed+(*x)*vtXMag)))))/4.0f*vtGlobalMag;
    
    *x *= s;
    *z *= s;

    if (vtColor) {
        float c = 2.0-s;
        float color[4]={c, c, c, 1.0};
        glMaterialfv(GL_FRONT, GL_AMBIENT, color);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
        glMaterialfv(GL_FRONT, GL_SPECULAR, color);
    }
}


static int duk_objectSetVertexTransform(duk_context *ctx)
{
    object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
    int type = (int)duk_get_int(ctx, 1);

    switch(type) {
        case 1:
            object->vertexTransform = vertexTransformCustom;
            break;
        case 0:
        default:
            object->vertexTransform = NULL;
            break;
    }

    return 0;
}


static int duk_drawRttPostProc(duk_context *ctx)
{
    unsigned int rttIndex = (unsigned int)duk_get_uint(ctx, 0);
    assert(rttIndex < RTT_TEXTURES_COUNT);
    int postProc = (int)duk_get_int(ctx, 1);


    texture_t* rttTexture = rttTextures[rttIndex];
    float alpha = 1.0f;
    float w = 100;//getScreenWidth();
    float h = 20;//getScreenHeight();
    perspective2dBegin(w, h);

    //glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rttTexture->id);

    glColor4f(1.0f,1.0f,1.0f,alpha);
    static int noiseTexCoordIter = 0;

    glBegin(GL_QUADS);

    float u1 = 0.0;
    float v1 = 0.0;
    float u2 = getScreenWidth()/512.0f;
    float v2 = 0.0;
    float x=0.0f;
    float y=0.0f;
    float w1=w;
    float h1=1.0f;
    float uSkew = 0.0f;
    float vSkew = 0.0f;
    float vInc = (getScreenHeight()/512.0f) / h;
    assert(sinTable);
    for(int i = 0; i < h; i++) {
        v2 += vInc;
        uSkew = calcSin(rand()%SINE_PRECISION)*0.015f;
        vSkew = uSkew;
        //uSkew = calcSin(i%SINE_PRECISION);

        glMultiTexCoord2f(GL_TEXTURE0, u2+uSkew,v2+vSkew);
        glVertex3f(x+w1,y+h1,0);
        glMultiTexCoord2f(GL_TEXTURE0, u1+uSkew,v2+vSkew);
        glVertex3f(x,y+h1,0);
        glMultiTexCoord2f(GL_TEXTURE0, u1+uSkew,v1+vSkew);
        glVertex3f(x,y,0);
        glMultiTexCoord2f(GL_TEXTURE0, u2+uSkew,v1+vSkew);
        glVertex3f(x+w1,y,0);
        v1 += vInc;
        y += 1.0f;
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    //glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);

    perspective2dEnd();

    return 0;
}

void bindJsCustomFunctions(duk_context *ctx)
{
    bindCFunctionToJs(initRayMarching, 0);
    bindCFunctionToJs(deinitRayMarching, 0);
    bindCFunctionToJs(drawRayMarching, 3);
    bindCFunctionToJs(drawCube, 0);
    bindCFunctionToJs(drawCross, 0);
    bindCFunctionToJs(drawFan, 5);

    bindCFunctionToJs(initNoise, 0);
    bindCFunctionToJs(deinitNoise, 0);
    bindCFunctionToJs(drawNoise, 2);

    bindCFunctionToJs(initRtt, 0);
    bindCFunctionToJs(deinitRtt, 0);
    bindCFunctionToJs(drawRtt, 1);
    bindCFunctionToJs(drawRttPostProc, 2);
    bindCFunctionToJs(renderToTexture, 3);

    bindCFunctionToJs(objectSetVertexTransform, 2);
    bindCFunctionToJs(objectSetVertexTransformVariables, 6);
}
