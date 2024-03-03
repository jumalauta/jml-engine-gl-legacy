/** @constructor */
var Player = function()
{
};

Player.prototype.calculate3dCoordinateAnimation = function(time, animation, defaults)
{
    var obj = {
        'x': 0.0,
        'y': 0.0,
        'z': 0.0
    };
    if (defaults !== void null)
    {
        obj.x = defaults.x;
        obj.y = defaults.y;
        obj.z = defaults.z;

        if (defaults.sync === void null && animation.sync !== void null)
        {
            defaults.sync = animation.sync;
        }
    }

    if (animation !== void null)
    {
        obj.x = Utils.evaluateVariable(animation, animation[0].x);
        obj.y = Utils.evaluateVariable(animation, animation[0].y);
        obj.z = Utils.evaluateVariable(animation, animation[0].z);

        var timeAdjusted = time;
        if (animation.sync !== void null && defaults.sync === true)
        {
            timeAdjusted = animation.start + animation.duration * animation.sync.progress;
            if (animation.sync.progress == 0)
            {
                return obj;
            }
        }

        var _interpolate = interpolate;
        var length = animation.length;
        for (var i = 0; i < length; i++)
        {
            var coordinate = animation[i];
            if (timeAdjusted >= coordinate.start)
            {
                var p = (timeAdjusted - coordinate.start) / coordinate.duration;

                obj.x = _interpolate(p, obj.x, Utils.evaluateVariable(animation, coordinate.x));
                obj.y = _interpolate(p, obj.y, Utils.evaluateVariable(animation, coordinate.y));
                obj.z = _interpolate(p, obj.z, Utils.evaluateVariable(animation, coordinate.z));
            }
        }
    }

    return obj;
};

Player.prototype.calculateScaleAnimation = function(time, animation)
{
    var obj = {
        'x': 1.0,
        'y': 1.0,
        'z': 1.0
    };

    return this.calculate3dCoordinateAnimation(time, animation.scale, obj);
};

Player.prototype.calculatePositionAnimation = function(time, animation)
{
    var obj = {
        'x': 0.0,
        'y': 0.0,
        'z': 0.0
    };

    return this.calculate3dCoordinateAnimation(time, animation.position, obj);
};

Player.prototype.calculatePivotAnimation = function(time, animation)
{
    var obj = {
        'x': 0.0,
        'y': 0.0,
        'z': 0.0
    };

    return this.calculate3dCoordinateAnimation(time, animation.pivot, obj);
};

Player.prototype.calculatePerspectiveAnimation = function(time, animation)
{
    var obj = {
        'fov': 45.0,
        'aspect': getWindowScreenAreaAspectRatio(),
        'near': 1.0,
        'far': 1000.0
    };

    if (animation.perspective !== void null)
    {
        obj.fov = Utils.evaluateVariable(animation, animation.perspective[0].fov);
        obj.aspect = Utils.evaluateVariable(animation, animation.perspective[0].aspect);
        obj.near = Utils.evaluateVariable(animation, animation.perspective[0].near);
        obj.far = Utils.evaluateVariable(animation, animation.perspective[0].far);

        var timeAdjusted = time;
        if (animation.sync !== void null && animation.sync.perspective === true)
        {
            timeAdjusted = animation.start + animation.duration * animation.sync.progress;
            if (animation.sync.progress == 0)
            {
                return obj;
            }
        }

        var _interpolate = interpolate;
        var length = animation.perspective.length;
        for (var i = 0; i < length; i++)
        {
            var perspective = animation.perspective[i];
            if (timeAdjusted >= perspective.start)
            {
                var p = (timeAdjusted - perspective.start) / perspective.duration;

                obj.fov = _interpolate(p, obj.fov, Utils.evaluateVariable(animation, perspective.fov));
                obj.aspect = _interpolate(p, obj.aspect, Utils.evaluateVariable(animation, perspective.aspect));
                obj.near = _interpolate(p, obj.near, Utils.evaluateVariable(animation, perspective.near));
                obj.far = _interpolate(p, obj.far, Utils.evaluateVariable(animation, perspective.far));
            }
        }
    }

    return obj;
};

Player.prototype.calculateColorAnimation = function(time, animation, animationColor)
{
    var obj = {
        'r': 255,
        'g': 255,
        'b': 255,
        'a': 255
    };

    if (animationColor !== void null)
    {
        obj.r = Utils.evaluateVariable(animationColor, animationColor[0].r);
        obj.g = Utils.evaluateVariable(animationColor, animationColor[0].g);
        obj.b = Utils.evaluateVariable(animationColor, animationColor[0].b);
        obj.a = Utils.evaluateVariable(animationColor, animationColor[0].a);

        var timeAdjusted = time;
        if (animation.sync !== void null && animation.sync.color === true)
        {
            timeAdjusted = animation.start + animation.duration * animation.sync.progress;
            if (animation.sync.progress == 0)
            {
                return obj;
            }
        }

        var _interpolate = interpolate;
        var length = animationColor.length;
        for (var i = 0; i < animationColor.length; i++)
        {
            var color = animationColor[i];
            if (timeAdjusted >= color.start)
            {
                var p = (timeAdjusted - color.start) / color.duration;

                obj.r = _interpolate(p, obj.r, Utils.evaluateVariable(animation, color.r));
                obj.g = _interpolate(p, obj.g, Utils.evaluateVariable(animation, color.g));
                obj.b = _interpolate(p, obj.b, Utils.evaluateVariable(animation, color.b));
                obj.a = _interpolate(p, obj.a, Utils.evaluateVariable(animation, color.a));
            }
        }
    }

    return obj;
};

Player.prototype.calculateAngleAnimation = function(time, animation)
{
    var obj = {
        'degreesX': 0,
        'degreesY': 0,
        'degreesZ': 0,
        'x': 1,
        'y': 1,
        'z': 1
    };

    if (animation.angle !== void null)
    {
        obj.degreesX = Utils.evaluateVariable(animation, animation.angle[0].degreesX);
        obj.degreesY = Utils.evaluateVariable(animation, animation.angle[0].degreesY);
        obj.degreesZ = Utils.evaluateVariable(animation, animation.angle[0].degreesZ);
        obj.x = Utils.evaluateVariable(animation, animation.angle[0].x);
        obj.y = Utils.evaluateVariable(animation, animation.angle[0].y);
        obj.z = Utils.evaluateVariable(animation, animation.angle[0].z);

        var timeAdjusted = time;
        if (animation.sync !== void null && animation.sync.angle === true)
        {
            timeAdjusted = animation.start + animation.duration * animation.sync.progress;
            if (animation.sync.progress == 0)
            {
                return obj;
            }
        }

        var _interpolate = interpolate;
        var length = animation.angle.length;
        for (var i = 0; i < length; i++)
        {
            var angle = animation.angle[i];
            if (timeAdjusted >= angle.start)
            {
                var p = (timeAdjusted - angle.start) / angle.duration;

                obj.degreesX = _interpolate(p, obj.degreesX, Utils.evaluateVariable(animation, angle.degreesX));
                obj.degreesY = _interpolate(p, obj.degreesY, Utils.evaluateVariable(animation, angle.degreesY));
                obj.degreesZ = _interpolate(p, obj.degreesZ, Utils.evaluateVariable(animation, angle.degreesZ));
                obj.x = _interpolate(p, obj.x, Utils.evaluateVariable(animation, angle.x));
                obj.y = _interpolate(p, obj.y, Utils.evaluateVariable(animation, angle.y));
                obj.z = _interpolate(p, obj.z, Utils.evaluateVariable(animation, angle.z));
            }
        }
    }

    return obj;
};

Player.prototype.drawImageAnimation = function(time, animation)
{
    if (animation.blend !== void null)
    {
        setTextureBlendFunc(animation.ref.ptr, animation.blend.src, animation.blend.dst);
    }

    var perspective3d = 0;
    if (animation.perspective === '3d')
    {
        perspective3d = 1;
    }
    setTexturePerspective3d(animation.ref.ptr, perspective3d);

    if (animation.canvasWidth !== void null && animation.canvasHeight !== void null)
    {
        setTextureCanvasDimensions(animation.ref.ptr,
            Utils.evaluateVariable(animation, animation.canvasWidth),
            Utils.evaluateVariable(animation, animation.canvasHeight));
    }

    if (animation.uv !== void null)
    {
        setTextureUvDimensions(animation.ref.ptr,
            Utils.evaluateVariable(animation, animation.uv.uMin),
            Utils.evaluateVariable(animation, animation.uv.vMin),
            Utils.evaluateVariable(animation, animation.uv.uMax),
            Utils.evaluateVariable(animation, animation.uv.vMax));
    }

    for (var multiTexI = 1; multiTexI < animation.multiTexRef.length; multiTexI++)
    {
        setTextureUnitTexture(animation.ref.ptr, multiTexI, animation.multiTexRef[multiTexI].ptr);
    }

    if (animation.pivot !== void null)
    {
        var pivot = this.calculatePivotAnimation(time, animation);
        setTexturePivot(animation.ref.ptr, pivot.x, pivot.y, pivot.z);
    }

    if (animation.angle !== void null)
    {
        var angle = this.calculateAngleAnimation(time, animation);
        setTextureRotation(animation.ref.ptr, angle.degreesX, angle.degreesY, angle.degreesZ, angle.x, angle.y, angle.z);
    }

    var scale = this.calculateScaleAnimation(time, animation);
    setTextureScale(animation.ref.ptr, scale.x, scale.y);

    if (animation.position !== void null)
    {
        var position = this.calculatePositionAnimation(time, animation);
        var x = position.x;
        var y = position.y;
        var z = position.z;
        if (Settings.demoScript.kanttuCompatibility)
        {
            y = getScreenHeight() - position.y;
        }
        setTexturePosition(animation.ref.ptr, x, y, z);
    }

    if (animation.align !== void null)
    {
        setTextureCenterAlignment(animation.ref.ptr, animation.align);
    }

    var color = this.calculateColorAnimation(time, animation, animation.color);
    glColor4ub(color.r, color.g, color.b, color.a);

    for (var videoI = 0; videoI < animation.multiTexRef.length; videoI++)
    {
        var multiTexRef = animation.multiTexRef[videoI];
        if (multiTexRef.video !== void null)
        {
            loggerInfo(JSON.stringify(animation),null,2);
            videoSetStartTime(multiTexRef.video.ref.ptr, animation.start);

            if (multiTexRef.video.speed !== void null)
            {
                videoSetSpeed(multiTexRef.video.ref.ptr, Utils.evaluateVariable(animation, multiTexRef.video.speed));
            }

            if (multiTexRef.video.fps !== void null)
            {
                videoSetFps(multiTexRef.video.ref.ptr, Utils.evaluateVariable(animation, multiTexRef.video.fps));
            }

            if (multiTexRef.video.loop !== void null)
            {
                videoSetLoop(multiTexRef.video.ref.ptr, Utils.evaluateVariable(animation, multiTexRef.video.loop));
            }

            if (multiTexRef.video.length !== void null)
            {
                videoSetLength(multiTexRef.video.ref.ptr, Utils.evaluateVariable(animation, multiTexRef.video.length));
            }

            if (multiTexRef.video.playing === void null)
            {
                videoPlay(multiTexRef.video.ref.ptr);
                multiTexRef.video.playing = true;
            }
        }
    }

    drawTexture(animation.ref.ptr);
    setTextureDefaults(animation.ref.ptr);
};

Player.prototype.drawTextAnimation = function(time, animation)
{
    setTextDefaults();

    setDrawTextString(animation.text.string);

    if (animation.pivot !== void null)
    {
        var pivot = this.calculatePivotAnimation(time, animation);
        setTextPivot(pivot.x, pivot.y, pivot.z);
    }

    if (animation.angle !== void null)
    {
        var angle = this.calculateAngleAnimation(time, animation);
        setTextRotation(angle.degreesX, angle.degreesY, angle.degreesZ);
    }

    var scale = this.calculateScaleAnimation(time, animation);
    setTextSize(scale.x, scale.y);

    if (animation.text.perspective === '2d')
    {
        if (animation.position !== void null)
        {
            var position = this.calculatePositionAnimation(time, animation);
            var x = position.x;
            var y = position.y;
            if (Settings.demoScript.kanttuCompatibility)
            {
                y = getScreenHeight() - (position.y - yFixed);
            }
            setTextPosition(x, y, 0);
        }

        if (animation.align !== void null)
        {
            setTextCenterAlignment(animation.align);
        }
    }
    else
    {
        if (animation.position !== void null)
        {
            var position = this.calculatePositionAnimation(time, animation);
            setTextPosition(position.x, position.y, position.z);
        }
    }

    if (animation.text.name !== void null)
    {
        setTextFont(animation.text.name);
    }

    var color = this.calculateColorAnimation(time, animation, animation.color);
    glColor4ub(color.r, color.g, color.b, color.a);

    if (animation.text.perspective === '2d')
    {
        drawText2d();
    }
    else
    {
        if (animation.clearDepthBuffer)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        drawText3d();
    }

    setTextDefaults();
};

Player.prototype.drawObjectAnimation = function(time, animation)
{
    var isPushPop = false;
    if (animation.ref !== void null)
    {
        if (animation.position !== void null)
        {
            var position = this.calculatePositionAnimation(time, animation);
            setObjectPosition(animation.ref.ptr, position.x, position.y, position.z);
        }
        if (animation.pivot !== void null)
        {
            var pivot = this.calculatePivotAnimation(time, animation);
            setObjectPivot(animation.ref.ptr, pivot.x, pivot.y, pivot.z);
        }

        if (animation.angle !== void null)
        {
            var angle = this.calculateAngleAnimation(time, animation);
            setObjectRotation(animation.ref.ptr, angle.degreesX, angle.degreesY, angle.degreesZ, angle.x, angle.y, angle.z);
        }
        if (animation.scale !== void null)
        {
            var scale = this.calculateScaleAnimation(time, animation);
            setObjectScale(animation.ref.ptr, scale.x, scale.y, scale.z);
        }
        if (animation.color !== void null)
        {
            var color = this.calculateColorAnimation(time, animation, animation.color);
            setObjectColor(animation.ref.ptr, color.r / 255, color.g / 255, color.b / 255, color.a / 255);
        }

        var camera = animation.camera;
        var fps = (time - animation.start) * animation.fps;
        if (animation.frame !== void null)
        {
            fps = animation.frame;
        }
        var clearDepthBuffer = animation.clearDepthBuffer;

        if (animation.shape !== void null)
        {
            if (animation.shape.type === 'CUSTOM')
            {
                isPushPop = true;
            }
        }

        if (isPushPop)
        {
            glPushMatrix();
        }

        if (clearDepthBuffer)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        drawObject(animation.ref.ptr, camera, fps, clearDepthBuffer);
    }

    if (animation.objectFunction !== void null)
    {
        Utils.evaluateVariable(animation, animation.objectFunction);
    }

    if (isPushPop)
    {
        glPopMatrix();
    }
};

Player.prototype.drawFboAnimation = function(time, animation)
{
    if (animation.fbo.dimension !== void null)
    {
        var obj = {
            'x': 1.0,
            'y': 1.0,
            'z': 1.0
        };

        var dimension = this.calculate3dCoordinateAnimation(time, animation.fbo.dimension, obj);
        fboSetRenderDimensions(animation.ref.ptr, dimension.x, dimension.y);
    }


    if (animation.fbo.action === 'begin')
    {
        fboBind(animation.ref.ptr);
        fboUpdateViewport(animation.ref.ptr);
    }
    else if (animation.fbo.action === 'end')
    {
        fboBind();
        fboUpdateViewport();

        fboBindTextures(animation.ref.ptr);

        glColor4f(1, 1, 1, 1);
        if (animation.ref.color !== void null)
        {
            setTextureSizeToScreenSize(animation.ref.color.ptr);
        }
        if (animation.ref.depth !== void null)
        {
            setTextureSizeToScreenSize(animation.ref.depth.ptr);
        }
        drawTexture(animation.ref.color.ptr);

        fboBindTextures();
    }
    else if (animation.fbo.action === 'unbind')
    {
        fboBind();
        fboUpdateViewport();
    }
    else if (animation.fbo.action === 'draw')
    {
        fboBindTextures(animation.ref.ptr);

        glColor4f(1, 1, 1, 1);
        if (animation.ref.color !== void null)
        {
            setTextureSizeToScreenSize(animation.ref.color.ptr);
        }
        if (animation.ref.depth !== void null)
        {
            setTextureSizeToScreenSize(animation.ref.depth.ptr);
        }
        drawTexture(animation.ref.color.ptr);

        fboBindTextures();
    }
};

Player.prototype.drawLightAnimation = function(time, animation)
{
    if (animation.ambientColor !== void null)
    {
        var color = this.calculateColorAnimation(time, animation, animation.ambientColor);
        lightSetAmbientColor(animation.light.index, color.r / 255, color.g / 255, color.b / 255, color.a / 255);
    }

    if (animation.diffuseColor !== void null)
    {
        var color = this.calculateColorAnimation(time, animation, animation.diffuseColor);
        lightSetDiffuseColor(animation.light.index, color.r / 255, color.g / 255, color.b / 255, color.a / 255);
    }

    if (animation.specularColor !== void null)
    {
        var color = this.calculateColorAnimation(time, animation, animation.specularColor);
        lightSetSpecularColor(animation.light.index, color.r / 255, color.g / 255, color.b / 255, color.a / 255);
    }

    if (animation.position !== void null)
    {
        var position = this.calculate3dCoordinateAnimation(time, animation.position, {'x': 0, 'y': 0, 'z': 1});
        lightSetPosition(animation.light.index, position.x, position.y, position.z);
    }

    lightSetPositionObject(animation.light.index);
    if (animation.positionObject !== void null)
    {
        lightSetPositionObject(animation.light.index, animation.positionObject.ptr);
    }
    else if (animation.lightRelativePosition !== void null)
    {
        animation.positionObject = getObjectFromMemory(animation.lightRelativePosition);
    }

    if (animation.light.action === 'begin')
    {
        lightSetOn(animation.light.index);
    }
    else if (animation.light.action === 'end')
    {
        lightSetOff(animation.light.index);
    }
};

Player.prototype.drawCameraAnimation = function(time, animation)
{
    if (animation.perspective !== void null)
    {
        var perspective = this.calculatePerspectiveAnimation(time, animation);
        setCameraPerspective(perspective.fov, perspective.aspect, perspective.near, perspective.far);
    }
    if (animation.position !== void null)
    {
        var position = this.calculate3dCoordinateAnimation(time, animation.position, {'x': 0, 'y': 0, 'z': 2});
        setCameraPosition(position.x, position.y, position.z);
    }
    if (animation.target !== void null)
    {
        var target = this.calculate3dCoordinateAnimation(time, animation.target, {'x': 0, 'y': 0, 'z': 0});
        setCameraLookAt(target.x, target.y, target.z);
    }
    if (animation.up !== void null)
    {
        var up = this.calculate3dCoordinateAnimation(time, animation.up, {'x': 0, 'y': 1, 'z': 0});
        setCameraUpVector(up.x, up.y, up.z);
    }

    setCameraPositionObject();
    if (animation.positionObject !== void null)
    {
        setCameraPositionObject(animation.positionObject.ptr);
    }
    else if (animation.cameraRelativePosition !== void null)
    {
        animation.positionObject = getObjectFromMemory(animation.cameraRelativePosition);
    }

    setCameraTargetObject();
    if (animation.targetObject !== void null)
    {
        setCameraTargetObject(animation.targetObject.ptr);
    }
    else if (animation.cameraRelativeTarget !== void null)
    {
        animation.targetObject = getObjectFromMemory(animation.cameraRelativeTarget);
    }

    resetViewport();
    viewReset();
};

Player.prototype.drawAnimation = function(animationLayers)
{
    var time = getSceneTimeFromStart();

    for (var key in animationLayers)
    {
        glPushMatrix();

        if (animationLayers.hasOwnProperty(key))
        {
            var animationLayersLength = animationLayers[key].length;
            for (var animationI = 0; animationI < animationLayersLength; animationI++)
            {
                glPushAttrib(GL_CURRENT_BIT);
                var animation = animationLayers[key][animationI];
                if (animation.error !== void null)
                {
                    continue; //skip animations that are in error state
                }

                if (time >= animation.start && time < animation.end)
                {
                    Sync.calculateAnimationSync(time, animation);

                    if (animation.shader !== void null)
                    {
                        Shader.enableShader(animation);
                    }

                    if (animation.type === 'image')
                    {
                        this.drawImageAnimation(time, animation);
                    }
                    else if (animation.type === 'text')
                    {
                        this.drawTextAnimation(time, animation);
                    }
                    else if (animation.type === 'object')
                    {
                        this.drawObjectAnimation(time, animation);
                    }
                    else if (animation.type === 'fbo')
                    {
                        this.drawFboAnimation(time, animation);
                    }
                    else if (animation.type === 'light')
                    {
                        this.drawLightAnimation(time, animation);
                    }
                    else if (animation.type === 'camera')
                    {
                        this.drawCameraAnimation(time, animation);
                    }

                    if (animation.runFunction !== void null)
                    {
                        Utils.evaluateVariable(animation, animation.runFunction);
                    }

                    if (animation.shader !== void null)
                    {
                        Shader.disableShader(animation);
                    }
                }
                glPopAttrib();
            }
        }

        glPopMatrix();
    }
}
