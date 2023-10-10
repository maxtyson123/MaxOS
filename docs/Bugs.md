
# Known Bugs
| Status | #Issue Code |                       Info                        |
|:------:|:-----------:|:-------------------------------------------------:|
| Fixing |      X      |                      Example                      |
| Known  |             | Event handlers only trigger for one event handler |
| Known  |             |   Windows have to be weirdly clicked to resize    |


# Annoying
Bugs that were simple to fix but took forever to find

| Time Spent |                   Bug                    |                        Fix                        |                                           Fix Commit                                            |
|:----------:|:----------------------------------------:|:-------------------------------------------------:|:-----------------------------------------------------------------------------------------------:|
|    2:00    | amd network driver not sending/reciveing | - Change 0x7FF -> 0xF7FF AND write(1) -> write(2) | [Github](https://github.com/maxtyson123/max-os/commit/4a0a080a271a9bf27d9cc3701c7d32b40aa2bab3) |
|    3:44    |       amd network driver not init        |   - PCI read/write check fail AND driver issue    | [Github](https://github.com/maxtyson123/max-os/commit/4a0a080a271a9bf27d9cc3701c7d32b40aa2bab3) |


