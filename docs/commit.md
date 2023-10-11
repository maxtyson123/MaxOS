# Buttons - 3/10/23
- Fixed the error with the mouse being weird and not sending click events properly, Window was passing X,Y to the mouse instead of mouseX, mouseY. (X,Y are keycodes that were included with the keyboard driver)
- Fixed button releasing error