/** @constructor */
var Shader = function(animation)
{
};

Shader.increaseLoaderResourceCountWithShaders = function()
{
    if (Settings.demoScript.shaders !== void null)
    {
        setResourceCount(Settings.demoScript.shaders.length);
    }

    if (Settings.demoScript.shaderPrograms !== void null)
    {
        setResourceCount(Settings.demoScript.shaderPrograms.length);
    }
};

Shader.compileAndLinkShaders = function()
{
    Shader.increaseLoaderResourceCountWithShaders();

    if (Settings.demoScript.shaders !== void null)
    {
        for (var shaderI = 0; shaderI < Settings.demoScript.shaders.length; shaderI++)
        {
            if (isUserExit())
            {
                return;
            }

            var shader = Settings.demoScript.shaders[shaderI];
            if (shader.skip === true)
            {
                continue;
            }
            shader.ref = shaderLoad(shader.name, shader.filename);

            notifyResourceLoaded();
        }
    }

    if (Settings.demoScript.shaderPrograms !== void null)
    {
        for (var programI = 0; programI < Settings.demoScript.shaderPrograms.length; programI++)
        {
            if (isUserExit())
            {
                return;
            }

            var shaderProgram = Settings.demoScript.shaderPrograms[programI];
            if (shaderProgram.skip === true)
            {
                continue;
            }

            shaderProgram.ref = shaderProgramLoad(shaderProgram.name);
            for (var shaderI = 0; shaderI < shaderProgram.shaders.length; shaderI++)
            {
                var shader = shaderProgram.shaders[shaderI];
                shaderProgramAddShaderByName(shaderProgram.name, shader.name);
            }

            shaderProgramAttachAndLink(shaderProgram.ref.ptr);
            notifyResourceLoaded();
        }
    }
};

Shader.load = function(shader)
{
    var shaderProgram = getShaderProgramFromMemory(shader.programName);
    if (shaderProgram.ptr === void null)
    {
        shaderProgram = shaderProgramLoad(shader.programName);
        for (var i = 0; i < shader.name.length; i++)
        {
            var shaderFilename = shader.name[i];
            var loadedShader = shaderLoad(shaderFilename, shaderFilename);
            if (loadedShader.ok == 1)
            {
                shaderProgramAddShaderByName(shader.programName, shaderFilename);
            }
        }
        shaderProgramAttachAndLink(shaderProgram.ptr);
    }

    return shaderProgram;
};

Shader.enableShader = function(animation)
{
    if (animation.shader !== void null)
    {
        shaderProgramUse(animation.shader.ref.ptr);

        if (animation.shader.variable !== void null)
        {
            var _getUniformLocation = getUniformLocation;
            var _glUniformi = glUniformi;
            var _glUniformf = glUniformf;
            var _setUniformFunction;

            var length = animation.shader.variable.length;
            for (var i = 0; i < length; i++)
            {
                var variable = animation.shader.variable[i];
                var name = _getUniformLocation(variable.name);

                _setUniformFunction = _glUniformf;
                if (variable.type === 'int')
                {
                    _setUniformFunction = _glUniformi;
                }

                var value = [];
                if (Utils.isString(variable.value) === true)
                {
                    value = Utils.evaluateVariable(animation, variable.value);
                }
                else
                {
                    var valueLength = variable.value.length;
                    for (var j = 0; j < valueLength; j++)
                    {
                        value.push(Utils.evaluateVariable(animation, variable.value[j]));
                    }
                }

                var valueLength = value.length;

                switch (valueLength)
                {
                    case 1:
                        _setUniformFunction(name, value[0]);
                        break;
                    case 2:
                        _setUniformFunction(name, value[0], value[1]);
                        break;
                    case 3:
                        _setUniformFunction(name, value[0], value[1], value[2]);
                        break;
                    case 4:
                        _setUniformFunction(name, value[0], value[1], value[2], value[3]);
                        break;
                    default:
                        break;
                }
            }
        }
    }
};

Shader.disableShader = function(animation)
{
    if (animation.shader !== void null)
    {
        disableShaderProgram();
    }
}
