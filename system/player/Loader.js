/** @constructor */
var Loader = function()
{
    this.resourceCount = 0;
    this.resourceUniqueList = [];
    this.animationLayers = {};
};

Loader.drawLoadingBar = function(percent)
{
    var w = getScreenWidth();
    var h = getScreenHeight();

    perspective2dBegin(w, h);

    var loaderBarWidth = w * 0.6;
    var loaderBarHeight = h * 0.2;
    var loaderBarLoadedWidth = loaderBarWidth * percent;

    var spacingX = w * 0.01;
    var spacingY = h * 0.01 * getWindowScreenAreaAspectRatio();

    var x1 = w / 2 - loaderBarWidth / 2;
    var x2 = w / 2 + loaderBarWidth / 2;
    var y1 = h / 2 - loaderBarHeight / 2;
    var y2 = h / 2 + loaderBarHeight / 2;
    var dx = x1 + loaderBarLoadedWidth;

    var xa2 = x1 + loaderBarWidth * 0.2;
    var ya1 = y2 + spacingY * 3;
    var ya2 = ya1 + loaderBarHeight;
    var yb1 = y1 - spacingY * 3;
    var yb2 = yb1 - loaderBarHeight;

    var fade_out = 1.0;
    var FADE_THRESHOLD = 0.85;
    if (percent > FADE_THRESHOLD)
    {
        fade_out = Utils.clamp(1.0 - ((percent - FADE_THRESHOLD) / (1.0 - FADE_THRESHOLD)));
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glColor4f(0.2, 0.2, 0.2, fade_out);
    glBegin(GL_QUADS);
    glVertex2i(x1 - spacingX, y1 - spacingY);
    glVertex2i(x2 + spacingX, y1 - spacingY);
    glVertex2i(x2 + spacingX, y2 + spacingY);
    glVertex2i(x1 - spacingX, y2 + spacingY);
    glVertex2i(x1 - spacingX, y1 - spacingY);
    glEnd();

    glColor4f(0.4, 0.4, 0.4, fade_out);
    glBegin(GL_QUADS);
    glVertex2i(x1, y1);
    glVertex2i(dx, y1);
    glVertex2i(dx, y2);
    glVertex2i(x1, y2);
    glEnd();

    if (Settings.demoScript.jumalauta === true)
    {
        glColor4f(0.2, 0.2, 0.2, fade_out);

        glBegin(GL_QUADS);
        glVertex2i(x1 - spacingX, yb2 - spacingY);
        glVertex2i(xa2 + spacingX, yb2 - spacingY);
        glVertex2i(xa2 + spacingX, yb1 + spacingY);
        glVertex2i(x1 - spacingX, yb1 + spacingY);
        glVertex2i(x1 - spacingX, yb2 - spacingY);
        glEnd();

        glBegin(GL_QUADS);
        glVertex2i(x1 - spacingX, ya1 - spacingY);
        glVertex2i(xa2 + spacingX, ya1 - spacingY);
        glVertex2i(xa2 + spacingX, ya2 + spacingY);
        glVertex2i(x1 - spacingX, ya2 + spacingY);
        glVertex2i(x1 - spacingX, ya1 - spacingY);
        glEnd();
    }

    glDisable(GL_BLEND);

    glColor4f(1, 1, 1, 1);

    perspective2dEnd();
};

Loader.prototype.validateResourceLoaded = function(animationDefinition, animationDefinitionRef, errorMessage)
{
    if (animationDefinition === void null || animationDefinitionRef === void null || animationDefinitionRef.ptr === void null)
    {
        this.setAnimationError(animationDefinition, 'RESOURCE', errorMessage);
        return false;
    }

    return true;
};

Loader.prototype.setAnimationError = function(animationDefinition, errorType, errorMessage)
{
    windowSetTitle(errorType + ' ERROR');
    animationDefinition.error = errorMessage;
    debugErrorPrint(errorMessage + ' - JSON: ' + JSON.stringify(animationDefinition, null, 2));
};

Loader.prototype.preprocess3dCoordinateAnimation = function(animStart, animDuration, animEnd, animationDefinition, defaults)
{
    if (animationDefinition !== void null)
    {
        var x = 0;
        var y = 0;
        var z = 0;
        if (defaults !== void null)
        {
            x = defaults.x;
            y = defaults.y;
            z = defaults.z;
        }

        for (var i = 0; i < animationDefinition.length; i++)
        {
            var coordinate = animationDefinition[i];

            if (coordinate.x === void null)
            {
                coordinate.x = x;
            }
            if (coordinate.y === void null)
            {
                coordinate.y = y;
            }
            if (coordinate.z === void null)
            {
                coordinate.z = z;
            }

            x = coordinate.x;
            y = coordinate.y;
            z = coordinate.z;
        }
    }
};

Loader.prototype.setSyncDefaults = function(animationDefinition, syncType)
{
    if (animationDefinition.sync !== void null && animationDefinition.sync[syncType] === void null)
    {
        if (animationDefinition.sync.all === true)
        {
            animationDefinition.sync[syncType] = true;
        }
        else
        {
            animationDefinition.sync[syncType] = false;
        }
    }
};

Loader.prototype.preprocessColorAnimation = function(
    animStart, animDuration, animEnd, animationDefinition, animationDefinitionColor)
{
    this.setSyncDefaults(animationDefinition, 'color');

    var r = 255;
    var g = 255;
    var b = 255;
    var a = 255;
    if (animationDefinitionColor === void null)
    {
        animationDefinitionColor = [{}];
    }

    Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinitionColor);

    for (var i = 0; i < animationDefinitionColor.length; i++)
    {
        var color = animationDefinitionColor[i];

        if (color.r === void null)
        {
            color.r = r;
        }
        if (color.g === void null)
        {
            color.g = g;
        }
        if (color.b === void null)
        {
            color.b = b;
        }
        if (color.a === void null)
        {
            color.a = a;
        }

        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
    }
};

Loader.prototype.preprocessAngleAnimation = function(animStart, animDuration, animEnd, animationDefinition)
{
    if (animationDefinition.angle !== void null)
    {
        this.setSyncDefaults(animationDefinition, 'angle');

        Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.angle);
        this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.angle,
            {'x': 1.0, 'y': 1.0, 'z': 1.0});

        var degreesX = 0;
        var degreesY = 0;
        var degreesZ = 0;
        for (var i = 0; i < animationDefinition.angle.length; i++)
        {
            var angle = animationDefinition.angle[i];

            if (angle.degreesX === void null)
            {
                angle.degreesX = degreesX;
            }
            if (angle.degreesY === void null)
            {
                angle.degreesY = degreesY;
            }
            if (angle.degreesZ === void null)
            {
                angle.degreesZ = degreesZ;
            }

            if (angle.pivot !== void null)
            {
                this.setAnimationError(animationDefinition, 'PARSE', 'angle.pivot is deprecated. move pivot under animation.');
            }

            degreesX = angle.degreesX;
            degreesY = angle.degreesY;
            degreesZ = angle.degreesZ;
        }
    }
};

Loader.prototype.preprocessPerspectiveAnimation = function(animStart, animDuration, animEnd, animationDefinition)
{
    this.setSyncDefaults(animationDefinition, 'perspective');

    if (animationDefinition.perspective === void null)
    {
        animationDefinition.perspective = [{}];
    }

    Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.perspective);

    var fov = 45.0;
    var aspect = getWindowScreenAreaAspectRatio();
    var near = 1.0;
    var far = 1000.0;

    for (var i = 0; i < animationDefinition.perspective.length; i++)
    {
        var perspective = animationDefinition.perspective[i];

        if (perspective.fov === void null)
        {
            perspective.fov = fov;
        }
        if (perspective.aspect === void null)
        {
            perspective.aspect = aspect;
        }
        if (perspective.near === void null)
        {
            perspective.near = near;
        }
        if (perspective.far === void null)
        {
            perspective.far = far;
        }

        fov = perspective.fov;
        aspect = perspective.aspect;
        near = perspective.near;
        far = perspective.far;
    }
};


Loader.prototype.preprocessScaleAnimation = function(animStart, animDuration, animEnd, animationDefinition)
{
    this.setSyncDefaults(animationDefinition, 'scale');

    if (animationDefinition.scale === void null)
    {
        animationDefinition.scale = [{}];
    }

    for (var i = 0; i < animationDefinition.scale.length; i++)
    {
        var scale = animationDefinition.scale[i];
        if (scale.uniform3d !== void null)
        {
            scale.x = scale.uniform3d;
            scale.y = scale.uniform3d;
            scale.z = scale.uniform3d;
        }
        else if (scale.uniform2d !== void null)
        {
            scale.x = scale.uniform2d;
            scale.y = scale.uniform2d;
        }
    }


    Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.scale);
    this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.scale,
        {'x': 1.0, 'y': 1.0, 'z': 1.0});
};

Loader.prototype.preprocessDimensionAnimation = function(animStart, animDuration, animEnd, animationDefinition)
{
    this.setSyncDefaults(animationDefinition, 'dimension');

    if (animationDefinition.dimension === void null)
    {
        animationDefinition.dimension = [{}];
    }

    Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.dimension);
    this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.dimension,
        {'x': 1.0, 'y': 1.0, 'z': 1.0});
};

Loader.prototype.preprocessPositionAnimation = function(animStart, animDuration, animEnd, animationDefinition, defaults)
{
    //position initialization
    if (animationDefinition.position !== void null)
    {
        this.setSyncDefaults(animationDefinition, 'position');

        Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.position);
        this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.position, defaults);
    }
};

Loader.prototype.preprocessPivotAnimation = function(animStart, animDuration, animEnd, animationDefinition, defaults)
{
    //pivot initialization
    if (animationDefinition.pivot !== void null)
    {
        this.setSyncDefaults(animationDefinition, 'pivot');

        Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.pivot);
        this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.pivot, defaults);
    }
};

Loader.prototype.preprocessAnimationDefinitions = function(animStart, animDuration, animEnd, animationDefinition)
{
    var startTime = animStart;
    var endTime = animEnd;

    animStart = startTime;
    animEnd = startTime;
    animDuration = animEnd - animStart;
    this.preprocessColorAnimation(animStart, animDuration, animEnd, animationDefinition, animationDefinition.color);

    animStart = startTime;
    animEnd = startTime;
    animDuration = animEnd - animStart;
    this.preprocessAngleAnimation(animStart, animDuration, animEnd, animationDefinition);

    animStart = startTime;
    animEnd = startTime;
    animDuration = animEnd - animStart;
    this.preprocessScaleAnimation(animStart, animDuration, animEnd, animationDefinition);

    animStart = startTime;
    animEnd = startTime;
    animDuration = animEnd - animStart;
    this.preprocessPositionAnimation(animStart, animDuration, animEnd, animationDefinition, {'x': 0.0, 'y': 0.0, 'z': 0.0});

    animStart = startTime;
    animEnd = startTime;
    animDuration = animEnd - animStart;
    this.preprocessPivotAnimation(animStart, animDuration, animEnd, animationDefinition, {'x': 0.0, 'y': 0.0, 'z': 0.0});
};

Loader.prototype.addNotifyResource = function(name)
{
    if (name !== void null && this.resourceUniqueList[name] === void null)
    {
        this.resourceCount++;
        this.resourceUniqueList[name] = false;

        setResourceCount(1);

        return true;
    }

    return false;
};

Loader.prototype.notifyResourceLoaded = function(name)
{
    if (name !== void null && this.resourceUniqueList[name] === false)
    {
        this.resourceUniqueList[name] = true;
        notifyResourceLoaded();

        return true;
    }

    return false;
};

Loader.prototype.sortArray = function(animationLayers)
{
    return Object.keys(animationLayers).sort().reduce(function(result, key) {
        result[key] = animationLayers[key];
        return result;
    }, {});
};

Loader.prototype.getLayerString = function(layer)
{
    if (Utils.isString(layer))
    {
        return layer;
    }
    else if (Utils.isNumeric(layer))
    {
        var layerString = '00000' + layer;
        return layerString.substring(layerString.length - 5); //number 21 to string "00021" etc...
    }

    return undefined;
};

Loader.prototype.addAnimation = function(animationDefinitions)
{
    var layer = '00001';
    for (var animationI = 0; animationI < animationDefinitions.length; animationI++)
    {
        var animationDefinition = animationDefinitions[animationI];
        if (animationDefinition.layer !== void null)
        {
            layer = this.getLayerString(animationDefinition.layer);
        }

        animationDefinition.layer = layer;

        if (this.animationLayers[layer] === void null)
        {
            this.animationLayers[layer] = new Array();
        }

        if (animationDefinition.shader !== void null)
        {
            /*
                If multiple shaders defined as an Array
                then split&duplicate Array to animationDefinition per shader

                If image/fbo type is defined then image/fbo names should be replaced with passToFbo names
            */
            if (animationDefinition.shader.constructor === Array)
            {
                if (animationDefinition.passToFbo === void null)
                {
                    this.setAnimationError(animationDefinition, 'PARSE',
                        'passToFbo must be declared when declaring animation shaders as an Array.');
                    continue;
                }

                var previousFboName = void null;
                for (var i = 0; i < animationDefinition.shader.length; i++)
                {
                    var animationDefinitionDuplicate = Utils.deepCopyJson(animationDefinition);
                    animationDefinitionDuplicate.shader = Utils.deepCopyJson(animationDefinition.shader[i]);

                    if (i > 0 && previousFboName !== void null)
                    {
                        if (animationDefinitionDuplicate.fbo !== void null)
                        {
                            if (animationDefinitionDuplicate.fbo.name !== void null)
                            {
                                animationDefinitionDuplicate.fbo.name = previousFboName;
                            }
                        }
                        else if (animationDefinitionDuplicate.image !== void null)
                        {
                            var colorFboSuffix = '.color.fbo';
                            var depthFboSuffix = '.depth.fbo';

                            var imageName = animationDefinitionDuplicate.image;
                            if (imageName.indexOf(colorFboSuffix) >= 0)
                            {
                                imageName = previousFboName + colorFboSuffix;
                            }
                            else if (imageName.indexOf(depthFboSuffix) >= 0)
                            {
                                imageName = previousFboName + depthFboSuffix;
                            }

                            animationDefinitionDuplicate.image = imageName;
                        }
                    }

                    if (i < animationDefinition.shader.length - 1)
                    {
                        animationDefinitionDuplicate.passToFbo.name += '.pass.' + i;
                        this.addAnimation([animationDefinitionDuplicate]);
                    }
                    else
                    {
                        animationDefinition.shader = animationDefinitionDuplicate.shader;
                        if (animationDefinition.fbo !== void null)
                        {
                            animationDefinition.fbo.name = animationDefinitionDuplicate.fbo.name;
                        }
                        if (animationDefinition.image !== void null)
                        {
                            animationDefinition.image = animationDefinitionDuplicate.image;
                        }
                    }

                    previousFboName = animationDefinitionDuplicate.passToFbo.name;
                }
            }

            if (animationDefinition.shader.name.constructor !== Array)
            {
                animationDefinition.shader.name = [animationDefinition.shader.name];
            }

            var name = animationDefinition.shader.name[animationDefinition.shader.name.length - 1];
            if (animationDefinition.shader.programName === void null)
            {
                animationDefinition.shader.programName = name;
            }

            this.addNotifyResource(animationDefinition.shader.programName);
        }

        if (animationDefinition.passToFbo !== void null)
        {
            var fboAnimationDefinition = {'fbo': {}};

            if (animationDefinition.start !== void null)
            {
                fboAnimationDefinition.start = animationDefinition.start;
            }
            if (animationDefinition.end !== void null)
            {
                fboAnimationDefinition.end = animationDefinition.end;
            }
            if (animationDefinition.duration !== void null)
            {
                fboAnimationDefinition.duration = animationDefinition.duration;
            }

            fboAnimationDefinition.layer = animationDefinition.layer;
            if (animationDefinition.passToFbo.beginLayer !== void null)
            {
                fboAnimationDefinition.layer = animationDefinition.passToFbo.beginLayer;
            }
            fboAnimationDefinition.fbo.name = animationDefinition.passToFbo.name;
            fboAnimationDefinition.fbo.action = 'begin';
            if (animationDefinition.passToFbo.beginAction !== void null)
            {
                fboAnimationDefinition.fbo.action = animationDefinition.passToFbo.beginAction;
            }

            this.addAnimation([fboAnimationDefinition]);
        }

        if (animationDefinition.object !== void null)
        {
            this.addNotifyResource(animationDefinition.object);
        }
        else if (animationDefinition.image !== void null)
        {
            if (this.addNotifyResource(animationDefinition.image))
            {
                imageLoadImageAsync(animationDefinition.image);
            }
        }
        else if (animationDefinition.fbo !== void null)
        {
            this.addNotifyResource(animationDefinition.fbo.name);
        }

        if (animationDefinition.initFunction !== void null)
        {
            this.addNotifyResource(animationDefinition.initFunction);
        }

        this.animationLayers[layer].push(animationDefinition);

        if (animationDefinition.passToFbo !== void null)
        {
            var fboAnimationDefinition = {'fbo': {}};

            if (animationDefinition.start !== void null)
            {
                fboAnimationDefinition.start = animationDefinition.start;
            }
            if (animationDefinition.end !== void null)
            {
                fboAnimationDefinition.end = animationDefinition.end;
            }
            if (animationDefinition.duration !== void null)
            {
                fboAnimationDefinition.duration = animationDefinition.duration;
            }

            fboAnimationDefinition.layer = animationDefinition.layer;
            if (animationDefinition.passToFbo.endLayer !== void null)
            {
                fboAnimationDefinition.layer = animationDefinition.passToFbo.endLayer;
            }
            fboAnimationDefinition.fbo.name = animationDefinition.passToFbo.name;
            fboAnimationDefinition.fbo.action = 'unbind';
            if (animationDefinition.passToFbo.endAction !== void null)
            {
                fboAnimationDefinition.fbo.action = animationDefinition.passToFbo.endAction;
            }

            this.addAnimation([fboAnimationDefinition]);
        }
    }

    this.animationLayers = this.sortArray(this.animationLayers);
};

Loader.prototype.processAnimation = function()
{
    threadWaitAsyncCalls();

    var startTime = getSceneStartTime();
    var endTime = getSceneEndTime();
    var durationTime = 0;
    for (var key in this.animationLayers)
    {
        if (this.animationLayers.hasOwnProperty(key))
        {
            var animationLayersLength = this.animationLayers[key].length;
            for (var animationI = 0; animationI < animationLayersLength; animationI++)
            {
                var animationDefinition = this.animationLayers[key][animationI];
                Utils.setTimeVariables(animationDefinition, startTime, endTime, durationTime);

                startTime = animationDefinition.start;
                endTime = animationDefinition.end;
                durationTime = endTime - startTime;

                if (animationDefinition.sync !== void null)
                {
                    if (animationDefinition.sync.all === void null)
                    {
                        animationDefinition.sync.all = true;
                    }
                }

                if (animationDefinition.shader !== void null)
                {
                    animationDefinition.shader.ref = Shader.load(animationDefinition.shader);
                    this.validateResourceLoaded(animationDefinition, animationDefinition.shader.ref,
                        'Could not load shader program ' + animationDefinition.shader.programName);
                    this.notifyResourceLoaded(animationDefinition.shader.programName);
                }

                if (animationDefinition.object !== void null ||
                    animationDefinition.objectFunction !== void null)
                {
                    animationDefinition.type = 'object';

                    if (animationDefinition.object !== void null)
                    {
                        if (animationDefinition.shape !== void null)
                        {
                            if (animationDefinition.shape.type === 'SPHERE')
                            {
                                var radius = 1;
                                if (animationDefinition.shape.radius !== void null)
                                {
                                    radius = animationDefinition.shape.radius;
                                }
                                var lats = 30;
                                if (animationDefinition.shape.lats !== void null)
                                {
                                    lats = animationDefinition.shape.lats;
                                }
                                var longs = 30;
                                if (animationDefinition.shape.longs !== void null)
                                {
                                    longs = animationDefinition.shape.longs;
                                }

                                animationDefinition.ref = setObjectSphereData(animationDefinition.object, radius, lats, longs);
                            }
                            else if (animationDefinition.shape.type === 'CYLINDER')
                            {
                                var base = 1;
                                if (animationDefinition.shape.base !== void null)
                                {
                                    base = animationDefinition.shape.base;
                                }
                                var top = 1;
                                if (animationDefinition.shape.top !== void null)
                                {
                                    top = animationDefinition.shape.top;
                                }
                                var height = 1;
                                if (animationDefinition.shape.height !== void null)
                                {
                                    height = animationDefinition.shape.height;
                                }
                                var slices = 30;
                                if (animationDefinition.shape.slices !== void null)
                                {
                                    slices = animationDefinition.shape.slices;
                                }
                                var stacks = 30;
                                if (animationDefinition.shape.stacks !== void null)
                                {
                                    stacks = animationDefinition.shape.stacks;
                                }

                                animationDefinition.ref = setObjectCylinderData(animationDefinition.object,
                                    base, top, height, slices, stacks);
                            }
                            else if (animationDefinition.shape.type === 'DISK')
                            {
                                var inner = 0;
                                if (animationDefinition.shape.inner !== void null)
                                {
                                    inner = animationDefinition.shape.inner;
                                }
                                var outer = 1;
                                if (animationDefinition.shape.outer !== void null)
                                {
                                    outer = animationDefinition.shape.outer;
                                }
                                var slices = 30;
                                if (animationDefinition.shape.slices !== void null)
                                {
                                    slices = animationDefinition.shape.slices;
                                }
                                var loops = 30;
                                if (animationDefinition.shape.loops !== void null)
                                {
                                    loops = animationDefinition.shape.loops;
                                }

                                animationDefinition.ref = setObjectDiskData(animationDefinition.object,
                                    inner, outer, slices, loops);
                            }
                            else if (animationDefinition.shape.type === 'CUBE')
                            {
                                animationDefinition.ref = setObjectCubeData(animationDefinition.object);
                            }
                            else if (animationDefinition.shape.type === 'MATRIX' || animationDefinition.shape.type === 'CUSTOM')
                            {
                                animationDefinition.ref = loadObjectBasicShape(animationDefinition.object,
                                    animationDefinition.shape.type);
                            }
                        }
                        else
                        {
                            animationDefinition.ref = loadObject(animationDefinition.object);
                        }

                        if (Settings.demoScript.kanttuCompatibility === true)
                        {
                            if (animationDefinition.objectLighting === void null)
                            {
                                animationDefinition.objectLighting = false;
                            }
                            if (animationDefinition.simpleColors === void null)
                            {
                                animationDefinition.simpleColors = true;
                            }
                        }

                        if (this.validateResourceLoaded(animationDefinition, animationDefinition.ref,
                            'Could not load ' + animationDefinition.object))
                        {
                            if (animationDefinition.objectLighting === false)
                            {
                                useObjectLighting(animationDefinition.ref.ptr, 0);
                            }
                            if (animationDefinition.simpleColors === true)
                            {
                                useSimpleColors(animationDefinition.ref.ptr, 1);
                            }
                            if (animationDefinition.objectCamera === false)
                            {
                                useObjectCamera(animationDefinition.ref.ptr, 0);
                            }
                        }
                    }

                    if (animationDefinition.fps === void null)
                    {
                        animationDefinition.fps = 0;
                    }
                    if (animationDefinition.camera === void null)
                    {
                        animationDefinition.camera = 'Camera01';
                    }
                    if (animationDefinition.clearDepthBuffer === void null)
                    {
                        animationDefinition.clearDepthBuffer = 0;
                    }
                    else
                    {
                        animationDefinition.clearDepthBuffer = animationDefinition.clearDepthBuffer === true ? 1 : 0;
                    }

                    var animStart = startTime;
                    var animEnd = endTime;
                    var animDuration = animEnd - animStart;
                    this.preprocessAnimationDefinitions(animStart, animDuration, animEnd, animationDefinition);

                    this.notifyResourceLoaded(animationDefinition.object);
                }
                else if (animationDefinition.image !== void null)
                {
                    animationDefinition.type = 'image';
                    if (animationDefinition.image.constructor !== Array)
                    {
                        animationDefinition.image = [animationDefinition.image];
                    }

                    for (var imageI = 0; imageI < animationDefinition.image.length; imageI++)
                    {
                        if (Utils.isString(animationDefinition.image[imageI]) === true)
                        {
                            animationDefinition.image[imageI] = {'name': animationDefinition.image[imageI]};
                        }
                    }

                    animationDefinition.ref = imageLoadImage(animationDefinition.image[0].name);
                    if (animationDefinition.image[0].video !== void null)
                    {
                        var video = Utils.deepCopyJson(animationDefinition.image[0].video);
                        video.ref = videoLoad(animationDefinition.image[0].name);
                        animationDefinition.ref.video = video;
                    }

                    animationDefinition.multiTexRef = [animationDefinition.ref];
                    for (var imageI = 1; imageI < animationDefinition.image.length; imageI++)
                    {
                        animationDefinition.multiTexRef.push(imageLoadImage(animationDefinition.image[imageI].name));
                        if (animationDefinition.image[imageI].video !== void null)
                        {
                            var video = Utils.deepCopyJson(animationDefinition.image[imageI].video);
                            video.ref = videoLoad(animationDefinition.image[imageI].name);
                            animationDefinition.multiTexRef[imageI].video = video;
                        }
                    }

                    if (animationDefinition.blend !== void null)
                    {
                        if (animationDefinition.blend.src === void null) {
                            animationDefinition.blend.src = GL_SRC_ALPHA;
                        }
                        if (animationDefinition.blend.dst === void null) {
                            animationDefinition.blend.dst = GL_ONE_MINUS_SRC_ALPHA;
                        }
                    }
                    else if (animationDefinition.additive === true)
                    {
                        animationDefinition.blend = {
                            'src': GL_SRC_ALPHA,
                            'dst': GL_ONE
                        };
                    }

                    if (animationDefinition.perspective === void null)
                    {
                        animationDefinition.perspective = '2d';
                    }

                    if (animationDefinition.align === void null && animationDefinition.position === void null)
                    {
                        animationDefinition.align = Constants.Align.CENTER;
                    }

                    var animStart = startTime;
                    var animEnd = endTime;
                    var animDuration = animEnd - animStart;
                    this.preprocessAnimationDefinitions(animStart, animDuration, animEnd, animationDefinition);

                    var message = 'Could not load ' + animationDefinition.image[0].name;
                    this.validateResourceLoaded(animationDefinition, animationDefinition.ref, message);
                    for (var i = 0; i < animationDefinition.multiTexRef.length; i++)
                    {
                        var multiTexRef = animationDefinition.multiTexRef[i];
                        this.validateResourceLoaded(animationDefinition, multiTexRef, message);
                    }

                    this.notifyResourceLoaded(animationDefinition.image[0].name);
                }
                else if (animationDefinition.text !== void null)
                {
                    animationDefinition.type = 'text';
                    if (animationDefinition.text.perspective === void null)
                    {
                        animationDefinition.text.perspective = '2d';
                    }
                    else if (animationDefinition.text.perspective === '3d')
                    {
                        if (animationDefinition.clearDepthBuffer === void null)
                        {
                            animationDefinition.clearDepthBuffer = 0;
                        }
                        else
                        {
                            animationDefinition.clearDepthBuffer = animationDefinition.clearDepthBuffer === true ? 1 : 0;
                        }
                    }

                    if (animationDefinition.text.name !== void null)
                    {
                        setTextFont(animationDefinition.text.name);
                    }

                    if (animationDefinition.align === void null && animationDefinition.position === void null)
                    {
                        animationDefinition.align = Constants.Align.CENTER;
                    }

                    var animStart = startTime;
                    var animEnd = endTime;
                    var animDuration = animEnd - animStart;
                    this.preprocessAnimationDefinitions(animStart, animDuration, animEnd, animationDefinition);
                }
                else if (animationDefinition.fbo !== void null)
                {
                    animationDefinition.type = 'fbo';

                    if (animationDefinition.fbo.name === void null)
                    {
                        animationDefinition.fbo.name = 'fbo';
                    }

                    animationDefinition.ref = fboInit(animationDefinition.fbo.name);
                    if (this.validateResourceLoaded(animationDefinition, animationDefinition.ref,
                        'Could not load ' + animationDefinition.fbo.name))
                    {
                        if (animationDefinition.ref.id === 0)
                        {
                            fboStoreDepth(animationDefinition.ref.ptr, animationDefinition.fbo.storeDepth === true ? 1 : 0);
                            if (animationDefinition.fbo.width !== void null && animationDefinition.fbo.height !== void null)
                            {
                                fboSetDimensions(animationDefinition.ref.ptr,
                                    animationDefinition.fbo.width, animationDefinition.fbo.height);
                            }
                            fboGenerateFramebuffer(animationDefinition.ref.ptr);
                        }
                    }

                    animStart = startTime;
                    animEnd = startTime;
                    animDuration = animEnd - animStart;
                    this.preprocessDimensionAnimation(animStart, animDuration, animEnd, animationDefinition);

                    this.notifyResourceLoaded(animationDefinition.fbo.name);
                }
                else if (animationDefinition.light !== void null)
                {
                    animationDefinition.type = 'light';

                    animStart = startTime;
                    animEnd = startTime;
                    animDuration = animEnd - animStart;
                    this.preprocessPositionAnimation(animStart, animDuration, animEnd, animationDefinition,
                        {'x': 0.0, 'y': 0.0, 'z': 1.0});

                    if (animationDefinition.ambientColor !== void null)
                    {
                        animStart = startTime;
                        animEnd = startTime;
                        animDuration = animEnd - animStart;
                        this.preprocessColorAnimation(animStart, animDuration, animEnd, animationDefinition,
                            animationDefinition.ambientColor);
                    }
                    if (animationDefinition.diffuseColor !== void null)
                    {
                        animStart = startTime;
                        animEnd = startTime;
                        animDuration = animEnd - animStart;
                        this.preprocessColorAnimation(animStart, animDuration, animEnd, animationDefinition,
                            animationDefinition.diffuseColor);
                    }
                    if (animationDefinition.specularColor !== void null)
                    {
                        animStart = startTime;
                        animEnd = startTime;
                        animDuration = animEnd - animStart;
                        this.preprocessColorAnimation(animStart, animDuration, animEnd, animationDefinition,
                            animationDefinition.specularColor);
                    }
                }
                else if (animationDefinition.camera !== void null)
                {
                    animationDefinition.type = 'camera';

                    animStart = startTime;
                    animEnd = startTime;
                    animDuration = animEnd - animStart;
                    this.preprocessPerspectiveAnimation(animStart, animDuration, animEnd, animationDefinition);

                    animStart = startTime;
                    animEnd = startTime;
                    animDuration = animEnd - animStart;
                    this.preprocessPositionAnimation(animStart, animDuration, animEnd, animationDefinition,
                        {'x': 0.0, 'y': 0.0, 'z': 2.0});

                    animStart = startTime;
                    animEnd = startTime;
                    animDuration = animEnd - animStart;
                    if (animationDefinition.sync !== void null && animationDefinition.sync.target === void null)
                    {
                        if (animationDefinition.sync.all === true)
                        {
                            animationDefinition.sync.target = true;
                        }
                        else
                        {
                            animationDefinition.sync.target = false;
                        }
                    }
                    Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.target);
                    this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.target,
                        {'x': 0.0, 'y': 0.0, 'z': 0.0});

                    animStart = startTime;
                    animEnd = startTime;
                    animDuration = animEnd - animStart;
                    if (animationDefinition.sync !== void null && animationDefinition.sync.up === void null)
                    {
                        if (animationDefinition.sync.all === true)
                        {
                            animationDefinition.sync.up = true;
                        }
                        else
                        {
                            animationDefinition.sync.up = false;
                        }
                    }
                    Utils.preprocessTimeAnimation(animStart, animDuration, animEnd, animationDefinition.up);
                    this.preprocess3dCoordinateAnimation(animStart, animDuration, animEnd, animationDefinition.up,
                        {'x': 0.0, 'y': 1.0, 'z': 0.0});
                }

                if (animationDefinition.initFunction !== void null)
                {
                    Utils.evaluateVariable(animationDefinition, animationDefinition.initFunction);
                    notifyResourceLoaded(animationDefinition.initFunction);
                }

                startTime = endTime;
                endTime = startTime + durationTime;
            }
        }
    }
}
