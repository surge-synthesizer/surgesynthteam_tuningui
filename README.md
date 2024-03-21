# surgesynthteam_tuningui

This is a JUCE module which provides the various UI things we've set up for tuning.
Currently it contains the table model which takes a tuning from the 
tuning-library and bridges it to a TableListBoxModel. We use this 
to make the tuning-workbench-synth table and (soon) the dexed show 
tuning table also.

To use it, I generally add it as a submodule. So

```
mkdir lib
cd lib
git submodule add http://github.com/surge-synthesizer/surgesynnthteam_tuningui
```

the open the projucer and add a module and voila.

I will write documentation here one day I promise. For now just go look how we use
this in TWSTuningGrid.h in the tuning-workbench-synth

## IMPORTANT NOTES 
(1) This version at ''https://github.com/FulopNandor/surgesynthteam_tuningui'' is a fork 
from the original work published at ''https://github.com/surge-synthesizer/surgesynthteam_tuningui'' @ 54f9a74, 
containing the following modifications:
- The original version contains column IDs from 0 up to 3 for columns
titled as ''Note'', ''Name'', ''Freq (hz)'' and ''log2(f/8.17)''. Because the value 0 is
used as a column ID for the first column, a call to ''jassert()'' causes assertion in debug mode.
This modified version contains column IDs renumbered from 1 up to 4, according to
the JUCE API (''https://docs.juce.com/master/classTableHeaderComponent.html''), which
states that _''The ID can be any number_ **apart from 0**_, but every column must have a unique ID.''_,
to avoid the assertion mentioned. 
- The substring ''hz'' is changed to ''Hz'' in the related column name.
  
(2) The goal of this version is for some current development work related to the project Dexed
(''https://github.com/FulopNandor/dexed'', originated from ''https://github.com/asb2m10/dexed'');
when it is finished, this repository might not be maintained. 
