//
// Created by 98max on 9/10/2022.
//

#include <drivers/peripherals/keyboard.h>



using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;
using namespace maxOS::hardwarecommunication;


///___Handler___

KeyboardEventHandler::KeyboardEventHandler(){

}

KeyboardEventHandler::~KeyboardEventHandler() {

};

void KeyboardEventHandler::onKeyDown(KeyCode keyDownCode, KeyboardState keyDownState)
{
}

void KeyboardEventHandler::onKeyUp(KeyCode keyUpCode, KeyboardState keyUpState)
{
}

void KeyboardEventHandler::onEvent(Event<KeyboardEvents> *event) {

    switch (event -> type) {

        case KeyboardEvents::KEYDOWN:
            this->onKeyDown(((KeyDownEvent*)event)->keyCode, ((KeyDownEvent*)event)->keyboardState);
            break;

        case KeyboardEvents::KEYUP:
            this->onKeyUp(((KeyUpEvent*)event)->keyCode, ((KeyUpEvent*)event)->keyboardState);
            break;

        default:
            break;
    }

}



///___Driver___

KeyboardDriver::KeyboardDriver(InterruptManager* manager)
        : InterruptHandler(0x21, manager),
          dataPort(0x60),
          commandPort(0x64)
{
}
KeyboardDriver::~KeyboardDriver(){

}

/**
 * @details Activate the keyboard driver
 */
void KeyboardDriver::activate() {
    while (commandPort.Read() & 0x1)    //Wait for user to stop pressing key (this is for the start-up key eg.. hold 'F12' for boot menu or hold 'del' for bios ), The wait is needed as the keyboard controller won't send anymore characters until the buffer has been read
        dataPort.Read();

    commandPort.Write(0xAE);                            // Tell: PIC to send keyboard interrupt [or] tell keyboard to send interrupts to PIC
    commandPort.Write(0x20);                            // Tell: get current state
    uint8_t status = (dataPort.Read() | 1)  & ~ 0x10;        // Read current state then set rightmost bit to 1 becuase this will be the new state and clear the bit
    commandPort.Write(0x60);                            // Tell: change current state
    dataPort.Write(status);                             // Write back the current state

    dataPort.Write(0xF4);                               // Final Activation of keyboard

    //Keyboard Controller Commands :
    //
    //0xAE : Enable Keyboard
    //0x20 : Read command byte , after that read the status from data port
    //0x60 : Write command byte , after that change the state of the data port
}

/**
 * @details Handle the keyboard interrupt
 *
 * @param esp  The stack pointer
 * @return returns the passed esp
 */
uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp){

    // Read the scancode from the keyboard
    uint8_t key = dataPort.Read();      //NOTE: The 8th bit is set to 1 if key is released and cleared to 0 if key is pressed

    // Pass the scan code to the handlers
    for(Vector<InputStreamEventHandler<uint8_t>*>::iterator streamEventHandler = inputStreamEventHandlers.begin(); streamEventHandler != inputStreamEventHandlers.end(); streamEventHandler++)
        (*streamEventHandler)->onStreamRead(key);

    return esp;

}

string KeyboardDriver::getDeviceName() {
    return "Keyboard";
}

///___State___

/**
 * @details Initialise the keyboard stat with the default values being false
 */
KeyboardState::KeyboardState() {

    this -> leftShift = false;
    this -> rightShift = false;
    this -> leftControl = false;
    this -> rightControl = false;
    this -> leftAlt = false;
    this -> rightAlt = false;

    this -> capsLock = false;
    this -> numberPadLock = false;
    this -> scrollLock = false;


}

KeyboardState::~KeyboardState() {

}

///___Interpreter___

KeyboardInterpreter::KeyboardInterpreter()
: InputStreamEventHandler<uint8_t>()
{

    // Extended codes are none by default
    this -> nextIsExtendedCode0 = false;
    this -> currentExtendedCode1 = 0;
    this -> extendedCode1Buffer = 0;

}

KeyboardInterpreter::~KeyboardInterpreter() {

}

void KeyboardInterpreter::onKeyRead(bool released, KeyboardState state, KeyCode keyCode) {

    // Check if the key is released or pressed
    if(released){

        // Pass the release event to the handlers
        raiseEvent(new KeyUpEvent(keyCode, state));

        // Event handled
        return;
    }

    // Pass the press event to the handlers
    raiseEvent(new KeyDownEvent(keyCode, state));

}

///___Interpreter EN_US___

KeyboardInterpreterEN_US::KeyboardInterpreterEN_US()
: KeyboardInterpreter()
{

}

KeyboardInterpreterEN_US::~KeyboardInterpreterEN_US() {

}

void KeyboardInterpreterEN_US::onStreamRead(uint8_t scanCode) {

    // TODO: Extended ScanCodes E0, E1

    // Check if the key is released or pressed (The 8th bit is set to 1 if key is released and cleared to 0 if key is pressed)
    bool released = (scanCode & 0x8);

    bool isShifting = this -> keyBoardState.leftShift || this -> keyBoardState.rightShift;
    bool shouldBeUpperCase = isShifting != this -> keyBoardState.capsLock;

    switch ((KeyCodeEN_US)scanCode) {

        // First row
        case KeyCodeEN_US::escape:
            onKeyRead(released, this -> keyBoardState, KeyCode::escape);
            break;

        case KeyCodeEN_US::f1:
            onKeyRead(released, this -> keyBoardState, KeyCode::f1);
            break;

        case KeyCodeEN_US::f2:
            onKeyRead(released, this -> keyBoardState, KeyCode::f2);
            break;

        case KeyCodeEN_US::f3:
            onKeyRead(released, this -> keyBoardState, KeyCode::f3);
            break;

        case KeyCodeEN_US::f4:
            onKeyRead(released, this -> keyBoardState, KeyCode::f4);
            break;

        case KeyCodeEN_US::f5:
            onKeyRead(released, this -> keyBoardState, KeyCode::f5);
            break;

        case KeyCodeEN_US::f6:
            onKeyRead(released, this -> keyBoardState, KeyCode::f6);
            break;

        case KeyCodeEN_US::f7:
            onKeyRead(released, this -> keyBoardState, KeyCode::f7);
            break;

        case KeyCodeEN_US::f8:
            onKeyRead(released, this -> keyBoardState, KeyCode::f8);
            break;

        case KeyCodeEN_US::f9:
            onKeyRead(released, this -> keyBoardState, KeyCode::f9);
            break;

        case KeyCodeEN_US::f10:
            onKeyRead(released, this -> keyBoardState, KeyCode::f10);
            break;

        case KeyCodeEN_US::f11:
            onKeyRead(released, this -> keyBoardState, KeyCode::f11);
            break;

        case KeyCodeEN_US::f12:
            onKeyRead(released, this -> keyBoardState, KeyCode::f12);
            break;

        case KeyCodeEN_US::printScreen:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadMultiply : KeyCode::printScreen);
            break;

        case KeyCodeEN_US::scrollLock:
            onKeyRead(released, this -> keyBoardState, KeyCode::scrollLock);
            break;

        /*
         * TODO: Implement pause/break, it conflicts with numlock, I prefer numlock so that is why it is not implemented
        case KeyCodeEN_US::pauseBreak:
            onKeyRead(released, this -> keyBoardState, KeyCode::pauseBreak);
            break;
        */

        // Second row
        case KeyCodeEN_US::squigglyLine:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::squigglyLine : KeyCode::slantedApostrophe);
            break;

        case KeyCodeEN_US::one:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::exclamationMark : KeyCode::one);
            break;

        case KeyCodeEN_US::two:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::atSign: KeyCode::two);
            break;

        case KeyCodeEN_US::three:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::hash : KeyCode::three);
            break;

        case KeyCodeEN_US::four:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::dollarSign : KeyCode::four);
            break;

        case KeyCodeEN_US::five:
            onKeyRead(released, this ->  keyBoardState, shouldBeUpperCase ? KeyCode::percentSign : KeyCode::five);
            break;

        case KeyCodeEN_US::six:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::powerSign : KeyCode::six);
            break;

        case KeyCodeEN_US::seven:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::andSign : KeyCode::seven);
            break;

        case KeyCodeEN_US::eight:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::multiply : KeyCode::eight);
            break;

        case KeyCodeEN_US::nine:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::openBracket : KeyCode::nine);
            break;

        case KeyCodeEN_US::zero:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::closeBracket : KeyCode::zero);
            break;

        case KeyCodeEN_US::minus:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::underscore : KeyCode::minus);
            break;

        case KeyCodeEN_US::equals:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::plus : KeyCode::equals);
            break;

        case KeyCodeEN_US::backspace:
            onKeyRead(released, this -> keyBoardState, KeyCode::backspace);
            break;

        case KeyCodeEN_US::insert:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadZero : KeyCode::insert);
            break;

        case KeyCodeEN_US::home:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadSeven  : KeyCode::home);
            break;

        case KeyCodeEN_US::pageUp:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadNine : KeyCode::pageUp);
            break;

        case KeyCodeEN_US::numberPadLock:

            // Ensure this is not a repeat
            if(!released){
                this -> keyBoardState.numberPadLock = !this -> keyBoardState.numberPadLock;
            }
            onKeyRead(released, this -> keyBoardState, KeyCode::numberPadLock);
            break;

        case KeyCodeEN_US::numberPadForwardSlash:

            // Check if number pad lock is on
            if(this -> keyBoardState.numberPadLock){
                onKeyRead(released, this -> keyBoardState, KeyCode::numberPadForwardSlash);
            }else{

                // Normal Forward Slash
                onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::questionMark : KeyCode::forwardSlash);
            }
            break;

        // Number Pad Multiply is same as print screen

        case KeyCodeEN_US::numberPadMinus:
            onKeyRead(released, this -> keyBoardState, KeyCode::numberPadMinus);
            break;

        // Third row
        case KeyCodeEN_US::tab:
            onKeyRead(released, this -> keyBoardState, KeyCode::tab);
            break;

        case KeyCodeEN_US::Q:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::Q : KeyCode::q);
            break;

        case KeyCodeEN_US::W:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::W : KeyCode::w);
            break;

        case KeyCodeEN_US::E:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::E : KeyCode::e);
            break;

        case KeyCodeEN_US::R:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::R : KeyCode::r);
            break;

        case KeyCodeEN_US::T:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::T : KeyCode::t);
            break;

        case KeyCodeEN_US::Y:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::Y : KeyCode::y);
            break;

        case KeyCodeEN_US::U:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::U : KeyCode::u);
            break;

        case KeyCodeEN_US::I:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::I : KeyCode::i);
            break;

        case KeyCodeEN_US::O:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::O : KeyCode::o);
            break;

        case KeyCodeEN_US::P:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::P : KeyCode::p);
            break;

        case KeyCodeEN_US::openSquareBracket:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::openCurlyBracket : KeyCode::openSquareBracket);
            break;

        case KeyCodeEN_US::closeSquareBracket:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::closeCurlyBracket : KeyCode::closeSquareBracket);
            break;

        case KeyCodeEN_US::backslash:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::lineThing : KeyCode::backslash);
            break;

        case KeyCodeEN_US::deleteKey:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadFullStop : KeyCode::deleteKey);
            break;

        case KeyCodeEN_US::end:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadOne : KeyCode::end);
            break;

        case KeyCodeEN_US::pageDown:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadThree : KeyCode::pageDown);
            break;

        // Number pad 7 is same as home

        case KeyCodeEN_US::numberPadEight:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadEight : KeyCode::upArrow);
            break;

        // Number pad 9 is same as page up

        case KeyCodeEN_US::numberPadPlus:
            onKeyRead(released, this -> keyBoardState, KeyCode::numberPadPlus);
            break;

        // Fourth row

        case KeyCodeEN_US::capsLock:
            // Ensure this is not a repeat
            if(!released){
                this -> keyBoardState.capsLock = !this -> keyBoardState.capsLock;
            }

            onKeyRead(released, this -> keyBoardState, KeyCode::capsLock);
            break;

        case KeyCodeEN_US::A:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::A : KeyCode::a);
            break;

        case KeyCodeEN_US::S:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::S : KeyCode::s);
            break;

        case KeyCodeEN_US::D:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::D : KeyCode::d);
            break;

        case KeyCodeEN_US::F:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::F : KeyCode::f);
            break;

        case KeyCodeEN_US::G:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::G : KeyCode::g);
            break;

        case KeyCodeEN_US::H:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::H : KeyCode::h);
            break;

        case KeyCodeEN_US::J:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::J : KeyCode::j);
            break;

        case KeyCodeEN_US::K:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::K : KeyCode::k);
            break;

        case KeyCodeEN_US::L:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::L : KeyCode::l);
            break;

        case KeyCodeEN_US::semicolon:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::colon : KeyCode::semicolon);
            break;

        case KeyCodeEN_US::apostrophe:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::quotationMark : KeyCode::apostrophe);
            break;

        case KeyCodeEN_US::enter:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadEnter : KeyCode::enter);
            break;

        case KeyCodeEN_US::numberPadFour:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadFour : KeyCode::leftArrow);
            break;

        case KeyCodeEN_US::numberPadFive:
            onKeyRead(released, this -> keyBoardState, KeyCode::numberPadFive);
            break;

        case KeyCodeEN_US::numberPadSix:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadSix : KeyCode::rightArrow);
            break;

        // Fifth row
        case KeyCodeEN_US::leftShift:
            // Check if this is a repeat
            if(!released){
                this -> keyBoardState.leftShift = !this -> keyBoardState.leftShift;
            }

            onKeyRead(released, this -> keyBoardState, KeyCode::leftShift);
            break;

        case KeyCodeEN_US::Z:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::Z : KeyCode::z);
            break;

        case KeyCodeEN_US::X:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::X : KeyCode::x);
            break;

        case KeyCodeEN_US::C:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::C : KeyCode::c);
            break;

        case KeyCodeEN_US::V:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::V : KeyCode::v);
            break;

        case KeyCodeEN_US::B:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::B : KeyCode::b);
            break;

        case KeyCodeEN_US::N:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::N : KeyCode::n);
            break;

        case KeyCodeEN_US::M:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::M : KeyCode::m);
            break;

        case KeyCodeEN_US::comma:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::lessThan : KeyCode::comma);
            break;

        case KeyCodeEN_US::fullStop:
            onKeyRead(released, this -> keyBoardState, shouldBeUpperCase ? KeyCode::greaterThan : KeyCode::fullStop);
            break;

        // Forward slash is same as number pad forward slash

        case KeyCodeEN_US::rightShift:
            // Check if this is a repeat
            if(!released){
                this -> keyBoardState.rightShift = !this -> keyBoardState.rightShift;
            }

            onKeyRead(released, this -> keyBoardState, KeyCode::rightShift);
            break;

        // Up Arrow is the same as number pad 8

        // Number pad 1 is the same as end

        case KeyCodeEN_US::numberPadTwo:
            onKeyRead(released, this -> keyBoardState, this -> keyBoardState.numberPadLock ? KeyCode::numberPadTwo : KeyCode::downArrow);
            break;

        // Number pad 3 is the same as page down

        // Number pad enter is the same as enter

        // Sixth row
        case KeyCodeEN_US::leftControl:
            // Check if this is a repeat
            if(!released){
                this -> keyBoardState.leftControl = !this -> keyBoardState.leftControl;
                this -> keyBoardState.rightControl = !this -> keyBoardState.rightControl;
            }

            onKeyRead(released, this -> keyBoardState, KeyCode::leftControl);
            break;

        case KeyCodeEN_US::leftOS:
            onKeyRead(released, this -> keyBoardState, KeyCode::leftOS);
            break;

        case KeyCodeEN_US::leftAlt:
            // Check if this is a repeat
            if(!released){
                this -> keyBoardState.leftAlt = !this -> keyBoardState.leftAlt;
                this -> keyBoardState.rightAlt = !this -> keyBoardState.rightAlt;
            }

            onKeyRead(released, this -> keyBoardState, KeyCode::leftAlt);
            break;

        case KeyCodeEN_US::space:
            onKeyRead(released, this -> keyBoardState, KeyCode::space);
            break;

        // Right Alt is the same as left alt

        // Right Control is the same as left control

        // Left Arrow is the same as number pad 4

        // Down Arrow is the same as number pad 2

        // Right Arrow is the same as number pad 6

        // Number pad 0 is the same as insert

        // Number pad full stop is the same as delete

        default:
            break;
        
    }
    
}

KeyDownEvent::KeyDownEvent(KeyCode keyCode, KeyboardState keyboardState)
: Event<KeyboardEvents>(KeyboardEvents::KEYDOWN){
    this -> keyCode = keyCode;
    this -> keyboardState = keyboardState;
}

KeyDownEvent::~KeyDownEvent() {

}

KeyUpEvent::KeyUpEvent(KeyCode keyCode, KeyboardState keyboardState)
: Event<KeyboardEvents>(KeyboardEvents::KEYUP){
    this -> keyCode = keyCode;
    this -> keyboardState = keyboardState;
}

KeyUpEvent::~KeyUpEvent() {

}


