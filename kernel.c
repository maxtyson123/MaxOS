#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

//Foreground and Background
static  inline  uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
        {
            return fg | bg << 4;    //Byte Shift (x << 1)

        }

static inline  uint16_t vga_entry(unsigned char uc, uint8_t color){
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

//Screen Hieght and width
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

//Terminal Varibles
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void)
{
    //Reset Pos
    terminal_row = 0;
    terminal_column = 0;
    //Rest Color
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    //Make the screen blank
    for (size_t y = 0; y < VGA_HEIGHT; y++) {       //For each y col (height)
        for (size_t x = 0; x < VGA_WIDTH; x++) {    //For each x row (width)
            const size_t index = y * VGA_WIDTH + x; //Get pixel index
            terminal_buffer[index] = vga_entry(' ', terminal_color);    //Set to blank
        }
    }
}

//Setting the colour
void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

//Put a character at position on terminal screen
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    //Same to when reseting
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

//Put the character
void terminal_putchar(char c)
{
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
    }
}


//Write a seris of charachters based on length
void terminal_write(const char* data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

//Simplified use of above funct
void terminal_writestring(const char* data)
{
    terminal_write(data, strlen(data));
}

void kernel_main(void)
{
    /* Initialize terminal interface */
    terminal_initialize();

    /* Newline support is left as an exercise. */
    terminal_writestring("Hello, kernel World! -Max");
}


//
// Created by 98max on 9/18/2022.
// Refernce https://wiki.osdev.org/Bare_Bones
//
