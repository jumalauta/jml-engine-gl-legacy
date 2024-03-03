/** @constructor */
var Effect = function()
{
};

Effect.effects = [];

Effect.init = function(effectName)
{
    var effect = eval('new ' + effectName);

    if (effect.loader === void null)
    {
        effect.loader = new Loader();
    }
    if (effect.player === void null)
    {
        effect.player = new Player();
    }

    Effect.effects[effectName] = effect;

    if (effect.init !== void null)
    {
        effect.init();
    }

    if (effect.postInit !== void null)
    {
        effect.postInit();
    }
    else
    {
        effect.loader.processAnimation();
    }
};

Effect.run = function(effectName)
{
    var effect = Effect.effects[effectName];

    if (effect.run !== void null)
    {
        effect.run();
    }
    else
    {
        effect.player.drawAnimation(effect.loader.animationLayers);
    }
};

Effect.deinit = function(effectName)
{
    var effect = Effect.effects[effectName];

    if (effect.deinit !== void null)
    {
        effect.deinit();
    }

    delete Effect.effects[effectName];
}
