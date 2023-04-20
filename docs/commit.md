# Fix Bugs - Committed on 4/20/2023
- Kernel not going into endless loop causing the program to finsh, this made the GUI not render as the clock interrupt was no longer being handled.
- General Protection Fault was being called when trying to write into the am79c973's output stream as it was pointing to 0 as there was no console stream yet
- Colour conversion by the graphics context was wrong so everything was rendered as black
- Connected the console mouse event handler
- VGA now calls GetFrameBufferSegment()
- Wouldn't compile due to video.h referencing graphicscontext.h instead of graphicsContext.h