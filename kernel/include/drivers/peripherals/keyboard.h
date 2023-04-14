//
// Created by 98max on 9/10/2022.
//

#ifndef MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H
#define MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H

#include <common/types.h>
#include <common/vector.h>
#include <common/inputStream.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxOS
{
    namespace drivers {

        namespace peripherals{

            enum KeyCode : common::uint16_t{

                // Alphabet
                A = 'A', a = 'a',
                B = 'B', b = 'b',
                C = 'C', c = 'c',
                D = 'D', d = 'd',
                E = 'E', e = 'e',
                F = 'F', f = 'f',
                G = 'G', g = 'g',
                H = 'H', h = 'h',
                I = 'I', i = 'i',
                J = 'J', j = 'j',
                K = 'K', k = 'k',
                L = 'L', l = 'l',
                M = 'M', m = 'm',
                N = 'N', n = 'n',
                O = 'O', o = 'o',
                P = 'P', p = 'p',
                Q = 'Q', q = 'q',
                R = 'R', r = 'r',
                S = 'S', s = 's',
                T = 'T', t = 't',
                U = 'U', u = 'u',
                V = 'V', v = 'v',
                W = 'W', w = 'w',
                X = 'X', x = 'x',
                Y = 'Y', y = 'y',
                Z = 'Z', z = 'z',

                // Numbers
                zero = '0',
                one = '1',
                two = '2',
                three = '3',
                four = '4',
                five = '5',
                six = '6',
                seven = '7',
                eight = '8',
                nine = '9',

                // Symbols
                comma = ',',
                fullStop = '.',
                exclamationMark = '!',
                questionMark = '?',
                quotationMark = '\"',
                semicolon = ';',
                colon = ':',
                apostrophe = '\'',
                slantedApostrophe = '`',

                // Signs
                powerSign = '^',
                dollarSign = '$',
                percentSign = '%',
                andSign = '&',
                atSign = '@',

                // Special Characters
                underscore = '_',
                lineThing = '|',
                hash = '#',
                backslash = '\\',
                forwardSlash = '/',
                squigglyLine = '~',

                // Math Symbols
                plus = '+',
                minus = '-',
                equals = '=',
                multiply = '*',
                lessThan = '<',
                greaterThan = '>',

                // Brackets
                openBracket = '(',
                closeBracket = ')',
                openSquareBracket = '[',
                closeSquareBracket = ']',
                openCurlyBracket = '{',
                closeCurlyBracket = '}',

                // Writing
                space = ' ',
                tab = '\t',
                enter = '\n',
                backspace = '\b',

                /// OTHER CODES THAT ARE NOT CHARACTERS ///

                // Functions
                f1 = 1000,      //force it to start at 1000 so it doesn't conflict with ascii
                f2,
                f3,
                f4,
                f5,
                f6,
                f7,
                f8,
                f9,
                f10,
                f11,
                f12,

                // Top Row
                escape,
                printScreen,
                scrollLock,
                pauseBreak,

                // Arrow s
                upArrow,
                downArrow,
                leftArrow,
                rightArrow,

                // Keys Above Arrows
                insert,
                home,
                pageUp,
                deleteKey,
                end,
                pageDown,

                // Left Side
                capsLock,
                leftShift,
                leftControl,
                leftOS,                 //weird ass windows key or command on mac
                leftAlt,

                // Right Side
                rightAlt,
                functionKey,
                rightControl,
                rightShift,

                // Number Pad
                numberPadLock,
                numberPadForwardSlash,
                numberPadMultiply,
                numberPadMinus,
                numberPadPlus,
                numberPadEnter,
                numberPadZero,
                numberPadOne,
                numberPadTwo,
                numberPadThree,
                numberPadFour,
                numberPadFive,
                numberPadSix,
                numberPadSeven,
                numberPadEight,
                numberPadNine,
                numberPadFullStop,

                // Numper Pad (Non Number Lock)
                numberPadHome,
                numberPadPageDown,
                numberPadPageUp,
                numberPadEnd,
                numberPadInsert,
                numberPadUpArrow,
                numberPadDownArrow,
                numberPadLeftArrow,
                numberPadRightArrow,

                //TODO: Probably should add more but these are the ones on my keyboard

            };

            class KeyboardState{
            public:
                KeyboardState();
                ~KeyboardState();

                // Left and Right
                bool leftShift;
                bool rightShift;
                bool leftControl;
                bool rightControl;
                bool leftAlt;
                bool rightAlt;

                // Other Stuff
                bool capsLock;
                bool numberPadLock;
                bool scrollLock;

            };

            class KeyboardEventHandler {
            public:
                KeyboardEventHandler();
                ~KeyboardEventHandler();

                virtual void onKeyDown(KeyCode keyDownCode, KeyboardState keyDownState);
                virtual void onKeyUp(KeyCode keyUpCode, KeyboardState keyUpState);
            };

            class KeyboardInterpreter : public common::InputStreamEventHandler<common::uint8_t>{

            protected:
                KeyboardState keyBoardState;
                common::Vector<KeyboardEventHandler*> keyboardEventHandlers;

                bool nextIsExtendedCode0;                                   // Some keyboard codes are 2 bytes long
                common::uint8_t currentExtendedCode1;
                common::uint16_t extendedCode1Buffer;

            public:
                KeyboardInterpreter();
                ~KeyboardInterpreter();

                void connectEventHandler(KeyboardEventHandler* keyboardEventHandler);
                void onKeyRead(bool released, KeyboardState state, KeyCode keyCode);

            };

            class KeyboardInterpreterEN_US : public KeyboardInterpreter{

            public:

                enum KeyCodeEN_US {
                    // First Row
                    escape      = 0x01,
                    f1          = 0x3B,
                    f2          = 0x3C,
                    f3          = 0x3D,
                    f4          = 0x3E,
                    f5          = 0x3F,
                    f6          = 0x40,
                    f7          = 0x41,
                    f8          = 0x42,
                    f9          = 0x43,
                    f10         = 0x44,
                    f11         = 0x57,
                    f12         = 0x58,
                    printScreen = 0x37,
                    scrollLock  = 0x46,
                    pauseBreak  = 0x45,

                    // Second Row
                    squigglyLine            = 0x29,
                    one                     = 0x02,
                    two                     = 0x03,
                    three                   = 0x04,
                    four                    = 0x05,
                    five                    = 0x06,
                    six                     = 0x07,
                    seven                   = 0x08,
                    eight                   = 0x09,
                    nine                    = 0x0A,
                    zero                    = 0x0B,
                    minus                   = 0x0C,
                    equals                  = 0x0D,
                    backspace               = 0x0E,
                    insert                  = 0x52,
                    home                    = 0x47,
                    pageUp                  = 0x49,
                    numberPadLock           = 0x45,
                    numberPadForwardSlash   = 0x35,
                    numberPadMultiply       = 0x37,
                    numberPadMinus          = 0x4A,

                    // Third Row
                    tab                 = 0x0F,
                    Q                   = 0x10,
                    W                   = 0x11,
                    E                   = 0x12,
                    R                   = 0x13,
                    T                   = 0x14,
                    Y                   = 0x15,
                    U                   = 0x16,
                    I                   = 0x17,
                    O                   = 0x18,
                    P                   = 0x19,
                    openSquareBracket   = 0x1A,
                    closeSquareBracket  = 0x1B,
                    backslash           = 0x2B,
                    deleteKey           = 0x53,
                    end                 = 0x4F,
                    pageDown            = 0x51,
                    numberPadSeven      = 0x47,
                    numberPadEight      = 0x48,
                    numberPadNine       = 0x49,
                    numberPadPlus       = 0x4E,

                    // Fourth Row
                    capsLock        = 0x3A,
                    A               = 0x1E,
                    S               = 0x1F,
                    D               = 0x20,
                    F               = 0x21,
                    G               = 0x22,
                    H               = 0x23,
                    J               = 0x24,
                    K               = 0x25,
                    L               = 0x26,
                    semicolon       = 0x27,
                    apostrophe      = 0x28,
                    enter           = 0x1C,
                    numberPadFour   = 0x4B,
                    numberPadFive   = 0x4C,
                    numberPadSix    = 0x4D,

                    // Fifth Row
                    leftShift       = 0x2A,
                    Z               = 0x2C,
                    X               = 0x2D,
                    C               = 0x2E,
                    V               = 0x2F,
                    B               = 0x30,
                    N               = 0x31,
                    M               = 0x32,
                    comma           = 0x33,
                    fullStop          = 0x34,
                    forwardSlash    = 0x35,
                    rightShift      = 0x36,
                    upArrow         = 0x48,
                    numberPadOne    = 0x4F,
                    numberPadTwo    = 0x50,
                    numberPadThree  = 0x51,
                    numberPadEnter  = 0x1C,

                    // Sixth Row
                    leftControl     = 0x1D,
                    leftOS          = 0x5B,
                    leftAlt         = 0x38,
                    space           = 0x39,
                    rightAlt        = 0x38,
                    function        = 0x5D,
                    rightControl    = 0x1D,
                    leftArrow       = 0x4B,
                    downArrow       = 0x50,
                    rightArrow      = 0x4D,
                    numberPadZero   = 0x52,
                    numberPadFullStop = 0x53
                };

                KeyboardInterpreterEN_US();
                ~KeyboardInterpreterEN_US();

                void onStreamRead(common::uint8_t scanCode);

            };

            class KeyboardDriver : public hardwarecommunication::InterruptHandler, public Driver, public common::GenericInputStream<common::uint8_t>{
                hardwarecommunication::Port8Bit dataPort;
                hardwarecommunication::Port8Bit commandPort;


            public:
                KeyboardDriver(hardwarecommunication::InterruptManager *manager, KeyboardEventHandler *keyboardEventHandler);

                ~KeyboardDriver();

                virtual common::uint32_t HandleInterrupt(common::uint32_t esp);

                virtual void Activate();
            };

        }


    }
}

#endif //MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H