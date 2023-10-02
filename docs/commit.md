# Fix Bugs With New VESA - 10/2/23
- BAR code for PCI was causing VESA not to be able to be written to to update pixels
- VESA wouldnt place pixels correctly, this was due to not using the right BPP and PITCH
- Fixed some issues where precompiled objects were causing compile errors not to be thrown
- Desktop is broken tho