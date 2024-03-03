/** @constructor */
var Sync = function()
{
};

Sync.syncDefinitions = {};

Sync.addSync = function(syncDefinitions)
{
    var startTime = 0;
    var endTime = 0;
    var durationTime = 0;

    for (var syncI = 0; syncI < syncDefinitions.length; syncI++)
    {
        var syncDefinition = syncDefinitions[syncI];

        Utils.setTimeVariables(syncDefinition, startTime, endTime, durationTime);

        startTime = syncDefinition.start;
        endTime = syncDefinition.end;
        durationTime = endTime - startTime;

        if (syncDefinition.type === 'rocket')
        {
            syncDefinition.ref = syncEditorGetTrack(syncDefinition.name);
        }

        if (syncDefinition.pattern === void null)
        {
            syncDefinition.pattern = [{}];

            if (syncDefinition.ref === void null)
            {
                syncDefinition.pattern = [{'start': startTime, 'duration': durationTime}];
            }
        }

        Utils.preprocessTimeAnimation(startTime, durationTime, endTime, syncDefinition.pattern);
        for (var patternI = 0; patternI < syncDefinition.pattern.length; patternI++)
        {
            var pattern = syncDefinition.pattern[patternI];
            pattern.started = false;
            pattern.counter = 0;
        }

        Sync.syncDefinitions[syncDefinition.name] = syncDefinition;
    }
};

Sync.getSyncValue = function(name)
{
    var sync = Sync.syncDefinitions[name];
    if (sync !== void null)
    {
        if (sync.ref !== void null)
        {
            var value = syncEditorGetTrackCurrentValue(sync.ref.ptr);
            return value;
        }
    }

    return 0;
};

Sync.calculateAnimationSync = function(time, animation)
{
    if (animation.sync !== void null)
    {
        var sync = Sync.syncDefinitions[animation.sync.name];
        if (sync !== void null)
        {
            var syncTime = time % sync.end;

            animation.sync.progress = 0;
            if (sync.ref !== void null)
            {
                //GNU Rocket sync
                animation.sync.progress = syncEditorGetTrackCurrentValue(sync.ref.ptr);

                if (sync.syncStartFunction !== void null && sync.started === void null)
                {
                    sync.started = true;
                    Utils.evaluateVariable(animation, sync.syncStartFunction);
                }

                if (sync.syncRunFunction !== void null)
                {
                    Utils.evaluateVariable(animation, sync.syncRunFunction);
                }

                return;
            }

            var syncPatternLength = sync.pattern.length;
            for (var patternI = 0; patternI < syncPatternLength; patternI++)
            {
                var pattern = sync.pattern[patternI];

                if (syncTime >= pattern.start && syncTime < pattern.end)
                {
                    animation.sync.progress = (syncTime - pattern.start) / pattern.duration;
                    if (animation.sync.progress > 1.0)
                    {
                        animation.sync.progress = 1.0;
                    }

                    if (!pattern.started)
                    {
                        pattern.started = true;
                        pattern.startTime = time;

                        if (pattern.syncStartFunction !== void null)
                        {
                            Utils.evaluateVariable(animation, pattern.syncStartFunction);
                        }
                        else if (sync.syncStartFunction !== void null)
                        {
                            Utils.evaluateVariable(animation, sync.syncStartFunction);
                        }
                    }

                    if (pattern.syncRunFunction !== void null)
                    {
                        Utils.evaluateVariable(animation, pattern.syncRunFunction);
                    }
                    else if (sync.syncRunFunction !== void null)
                    {
                        Utils.evaluateVariable(animation, sync.syncRunFunction);
                    }
                }

                if (pattern.started &&
                    (time >= pattern.startTime + pattern.duration ||
                    pattern.startTime > time))
                {
                    pattern.started = false;
                    animation.sync.progress = 0;

                    if (pattern.syncEndFunction !== void null)
                    {
                        Utils.evaluateVariable(animation, pattern.syncEndFunction);
                    }
                    else if (sync.syncEndFunction !== void null)
                    {
                        Utils.evaluateVariable(animation, sync.syncEndFunction);
                    }
                }
            }
        }
    }
}
