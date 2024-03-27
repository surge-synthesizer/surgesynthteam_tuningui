# IMPORTANT NOTICE

This version at "https://github.com/FulopNandor/surgesynthteam_tuningui" is a fork 
from the original work published at "https://github.com/surge-synthesizer/surgesynthteam_tuningui", 
containing the following modifications:
- The original version contains column IDs between 0 up to 3 for columns
titled as "Note", "Name", "Freq (hz)" and "log2(f/8.17)", and the value 0
used as a column ID causes an assertion in debug mode.
- This modified version contains column IDs between 1 up to 4, according to
the JUCE API (https://docs.juce.com/master/classTableHeaderComponent.html), which
states that "The ID can be any number _apart from 0_, but every column must have a unique ID."
to avoid the assertion mentioned above. 
- The "hz" is changed to "Hz" in the related column name.
- Sorting icons and popup menu are disabled in table header because the related functionalities has not been implemented yet.
- The member ''table'' in constructor ''TuningTableListBoxModel() is initialized to ''nullptr'' to avoid compiler warning ''C26495'' indicated by MSVC.

## WARNING

The content of this repository is intended for current development work related to the Dexed project ("https://github.com/FulopNandor/dexed", originated from "https://github.com/asb2m10/dexed"). Once this work is completed, this repository may no longer be maintained. Therefore, please always use and refer to the original repository at "https://github.com/surge-synthesizer/surgesynthteam_tuningui", instead of this one. Thank you!