
# Known Bugs
| Status | #Issue Code |  Info   |
|:------:|:-----------:|:-------:|
| Fixing |      X      | Example |

# Annoying
Bugs that were simple to fix but took forever to find

| Time Spent |                   Bug                    |                        Fix                        |                                           Fix Commit                                            |
|:----------:|:----------------------------------------:|:-------------------------------------------------:|:-----------------------------------------------------------------------------------------------:|
|    2:00    | amd network driver not sending/reciveing | - Change 0x7FF -> 0xF7FF AND write(1) -> write(2) | [Github](https://github.com/maxtyson123/max-os/commit/4a0a080a271a9bf27d9cc3701c7d32b40aa2bab3) |
|    3:44    |       amd network driver not init        |   - PCI read/write check fail AND driver issue    | [Github](https://github.com/maxtyson123/max-os/commit/4a0a080a271a9bf27d9cc3701c7d32b40aa2bab3) |


What could be problem (level 1)
- [x] Desktop - Fixes Drawing Weirdness
- [x] Graphics Context - Fixes mouse fuckery, colour weirdness 
- [x] VGA - No observable change
- [x] Kernel - No observable change
- [x] Mouse - Fixes mouse movement
- [x] Clock - No observable change
- [ ] Window - Fixes side drawing defects
- [x] Widget - No observable change
- [x] Rectangle - Fixes Weird Positioning
- [x] vector - no observable change 
- [x] coords/ - fix positioning
- [x] font - text is drawn

Problems: Window Not Moving, Most likely a problem with clicking (onMouseButtonPressed is not called for desktop), Text will only be the last text set
Window border not drawing correctly