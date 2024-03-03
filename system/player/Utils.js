String.prototype.endsWith = function(suffix) {
    return this.indexOf(suffix, this.length - suffix.length) !== -1;
};

/** @constructor */
var Utils = function()
{
};

Utils.clamp = function(value)
{
    return Utils.clampRange(value, 0.0, 1.0);
};

Utils.clampRange = function(value, min, max)
{
    return Math.min(Math.max(value, min), max);
};

Utils.calculateProgress = function(start, duration, noClamp)
{
    var time = getSceneTimeFromStart();
    var p = (time - start) / duration;
    if (noClamp !== true)
    {
        p = Utils.clamp(p);
    }

    return p;
};

Utils.debugPrintStackTrace = function()
{
    var stackTrace = ['stack trace:'];

    //start from stackPosition -3 to skip printing of debugPrintStackTrace()
    for (var i = 0, stackPosition = -3;; stackPosition--)
    {
        var info = Duktape.act(stackPosition);
        if (!info)
        {
            break;
        }

        if (info.function.name == '')
        {
            continue;
        }

        var string = '\tstack: ' + stackPosition;
        string += ', function: ' + info.function.name;
        string += ', line: ' + info.lineNumber;
        string += ', info: ' + Duktape.enc('jx', info);
        stackTrace[++i] = string;
    }

    if (stackTrace.length > 1)
    {
        debugPrint(stackTrace.join('\n'));
    }
};

Utils.isString = function(variable)
{
    if (typeof variable === 'string' || variable instanceof String)
    {
        return true;
    }

    return false;
};

Utils.isNumeric = function(variable)
{
    if (!isNaN(parseFloat(variable)) && isFinite(variable))
    {
        return true;
    }

    return false;
};

Utils.isVideo = function(filename)
{
    if (filename.endsWith('.ogv') || filename.endsWith('.ogg'))
    {
        return true;
    }

    return false;
};

Utils.evaluateVariable = function(animation, variable)
{
    if (Utils.isString(variable) && variable.charAt(0) === '{')
    {
        var func = new Function('animation', variable);
        return func(animation);
    }

    return variable;
};

Utils.deepCopyJson = function(jsonObject)
{
    var object = JSON.parse(JSON.stringify(jsonObject));

    if (jsonObject.ptr !== void null)
    {
        object.ptr = jsonObject.ptr;
    }

    if (jsonObject.ref !== void null)
    {
        if (jsonObject.ref.ptr !== void null)
        {
            object.ref.ptr = jsonObject.ref.ptr;
        }
    }

    return object;
};

Utils.getRandomArrayIndex = function(array)
{
    if (array.length !== void null)
    {
        return Math.floor(random() * array.length);
    }

    return undefined;
};

Utils.setTimeVariables = function(variable, animStart, animEnd, animDuration)
{
    if (Utils.isString(variable.start))
    {
        variable.start = convertTimeToSeconds(variable.start);
    }
    if (Utils.isString(variable.duration))
    {
        variable.duration = convertTimeToSeconds(variable.duration);
    }
    if (Utils.isString(variable.end))
    {
        variable.end = convertTimeToSeconds(variable.end);
    }

    if (variable.start === void null)
    {
        variable.start = animStart;
    }
    if (variable.duration === void null)
    {
        if (variable.end !== void null)
        {
            variable.duration = variable.end - variable.start;
        }
        else
        {
            variable.duration = animDuration;
            variable.end = variable.start + variable.duration;
        }
    }
    if (variable.end === void null)
    {
        variable.end = variable.start + variable.duration;
    }
};

Utils.preprocessTimeAnimation = function(animStart, animDuration, animEnd, animationDefinition)
{
    if (animationDefinition !== void null)
    {
        for (var i = 0; i < animationDefinition.length; i++)
        {
            var time = animationDefinition[i];
            Utils.setTimeVariables(time, animStart, animEnd, animDuration);

            animStart = time.end;
            animDuration = time.duration;
            animEnd = animStart + animDuration;
        }
    }
};
