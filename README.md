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

NOTE: this version at "https://github.com/FulopNandor/surgesynthteam_tuningui" is a fork 
from the original work published at "https://github.com/surge-synthesizer/surgesynthteam_tuningui", 
containing the following modifications:
The original version contains column IDs between 0 up to 3 for columns
titled as "Note", "Name", "Freq (hz)" and "log2(f/8.17)", and the value 0
used as a column ID causes an assertion in debug mode.
This modified version contains column IDs between 1 up to 4, according to
the JUCE API (https://docs.juce.com/master/classTableHeaderComponent.html), which
states that "The ID can be any number apart from 0, but every column must have a unique ID."
to avoid the assertion mentioned above. 
Furthermore, "hz" is changed to "Hz" in the column name related.

WARNING: The goal of this version is for some current development work related to the project Dexed
("https://github.com/FulopNandor/dexed", originated from "https://github.com/asb2m10/dexed");
when it is finished, this repository might not be maintained. 