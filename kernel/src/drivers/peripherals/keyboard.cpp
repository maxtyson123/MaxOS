//
// Created by 98max on 9/10/2022.
//

#include <drivers/peripherals/keyboard.h>



using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::peripherals;
using namespace MaxOS::hardwarecommunication;


///___Handler___

KeyboardEventHandler::KeyboardEventHandler()= default;

KeyboardEventHandler::~KeyboardEventHandler() = default;

/**
 * @brief Handle the key down event
 *
 * @param key_down_code The keycode of the key that was pressed
 * @param key_down_state The state of the keyboard when the key was pressed
 */
void KeyboardEventHandler::on_key_down(KeyCode, KeyboardState)
{
}

/**
 * @brief Handle the key up event
 *
 * @param key_up_code The keycode of the key that was released
 * @param key_up_state The state of the keyboard when the key was released
 */
void KeyboardEventHandler::on_key_up(KeyCode, KeyboardState)
{
}

/**
 * @brief Handle the trigger of an event
 *
 * @param event The event to handle
 * @return The event that was passed with the data modified
 */
Event<KeyboardEvents>* KeyboardEventHandler::on_event(Event<KeyboardEvents> *event) {

    switch (event -> type) {

        case KeyboardEvents::KEYDOWN:
            this->on_key_down(((KeyDownEvent *)event)->key_code,
                            ((KeyDownEvent *)event)->keyboard_state);
            break;

        case KeyboardEvents::KEYUP:
          this->on_key_up(((KeyUpEvent *)event)->key_code,
                          ((KeyUpEvent *)event)->keyboard_state);
            break;

        default:
            break;
    }

    return event;
}



///___Driver___

KeyboardDriver::KeyboardDriver(InterruptManager* manager, IOAPIC* ioapic)
: InterruptHandler(0x21, manager),
  m_data_port(0x60),
  m_command_port(0x64)
{

  // Redirect the interrupt to the keyboard driver
  interrupt_redirect_t keyboardRedirect = {
      .type = 0x1,
      .index = 0x12,
      .interrupt = 0x21,
      .destination = 0x00,
      .flags = 0x00,
      .mask = false,
  };
  ioapic -> set_redirect(&keyboardRedirect);

}

KeyboardDriver::~KeyboardDriver()= default;

/**
 * @brief activate the keyboard driver
 */
void KeyboardDriver::activate() {

    // Wait for user to stop pressing key (this is for the start-up key e.g. hold 'F12' for boot menu or hold 'del' for bios )
    while (m_command_port.read() & 0x1)
      m_data_port.read();

    // Enable keyboard interrupts
    m_command_port.write(0xAE);

    // Get the current state of the keyboard
    m_command_port.write(0x20);
    uint8_t status = (m_data_port.read() | 1)  & ~ 0x10;

    // Reset the keyboard
    m_command_port.write(0x60);
    m_data_port.write(status);

    // activate the keyboard
    m_data_port.write(0xF4);
}

/**
 * @brief deactivate the keyboard driver
 */
void KeyboardDriver::handle_interrupt(){

    // read the scancode from the keyboard
    uint8_t key = m_data_port.read();

    // Pass the scan code to the m_handlers
    for(auto& handler : this -> m_input_stream_event_handlers){
        handler -> on_stream_read(key);
    }
}

/**
 * @brief Get the device name
 * @return The device name
 */
string KeyboardDriver::get_device_name() {
    return "Keyboard";
}

///___State___


KeyboardState::KeyboardState() = default;

KeyboardState::~KeyboardState() = default;

///___Interpreter___

KeyboardInterpreter::KeyboardInterpreter()
: InputStreamEventHandler<uint8_t>()
{

}

KeyboardInterpreter::~KeyboardInterpreter() = default;

void KeyboardInterpreter::onKeyRead(bool released, const KeyboardState& state, KeyCode key_code) {

    // Pass the key event to the handlers
    if(released)
      raise_event(new KeyUpEvent(key_code, state));
    else
      raise_event(new KeyDownEvent(key_code, state));

}

///___Interpreter EN_US___

KeyboardInterpreterEN_US::KeyboardInterpreterEN_US()
: KeyboardInterpreter()
{

}

KeyboardInterpreterEN_US::~KeyboardInterpreterEN_US() = default;

/**
 * @brief Handle the key down event
 *
 * @param scan_code The scan code of the key that was pressed
 */
void KeyboardInterpreterEN_US::on_stream_read(uint8_t scan_code) {

    // 0 is a regular key, 1 is an extended code, 2 is an extended code with e1CodeBuffer
    int keyType = 0;

    // Check if the key was released
    bool released = (scan_code & 0x80) && (m_current_extended_code_1 || (scan_code != 0xe1)) && (m_next_is_extended_code_0 || (scan_code != 0xe0));

    // Clear the released bit
    if (released)
      scan_code &= ~0x80;

    // Set the e0Code flag to true
    if (scan_code == 0xe0)
    {
      m_next_is_extended_code_0 = true;
      return;
    }

    // If e0Code is true, set keyType to 1 and reset e0Code
    if (m_next_is_extended_code_0)
    {
        keyType = 1;
        m_next_is_extended_code_0 = false;

        // Check if the scan_code represents a shift key and return (fake shift)
        if ((KeyboardInterpreterEN_US::KeyCodeEN_US)scan_code == KeyboardInterpreterEN_US::leftShift || (KeyboardInterpreterEN_US::KeyCodeEN_US)scan_code == KeyboardInterpreterEN_US::rightShift)
            return;
    }

    // If the scan_code is 0xe1, set the e1Code flag to 1 and return
    if (scan_code == 0xe1)
    {
       m_current_extended_code_1 = 1;
       return;
    }

    // If e1Code is 1, set e1Code to 2, store the scan_code in e1CodeBuffer, and return
    if (m_current_extended_code_1 == 1)
    {
      m_current_extended_code_1 = 2;
      m_extended_code_1_buffer = scan_code;
      return;
    }

    // If e1Code is 2, set keyType to 2, reset e1Code, and update e1CodeBuffer
    if (m_current_extended_code_1 == 2)
    {
        keyType = 2;
        m_current_extended_code_1 = 0;
        m_extended_code_1_buffer |= (((uint16_t)scan_code) << 8);
    }

    bool is_shifting = this ->m_keyboard_state.left_shift || this ->m_keyboard_state.right_shift;
    bool should_be_upper_case = is_shifting != this ->m_keyboard_state.caps_lock;


    // TODO: Probably a better way to do this (investigate when adding more keyboard layouts)
    if(keyType == 0)
    switch ((KeyCodeEN_US)scan_code) {

        // First row
        case KeyCodeEN_US::escape:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::escape);
            break;

        case KeyCodeEN_US::f1:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f1);
            break;

        case KeyCodeEN_US::f2:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f2);
            break;

        case KeyCodeEN_US::f3:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f3);
            break;

        case KeyCodeEN_US::f4:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f4);
            break;

        case KeyCodeEN_US::f5:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f5);
            break;

        case KeyCodeEN_US::f6:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f6);
            break;

        case KeyCodeEN_US::f7:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f7);
            break;

        case KeyCodeEN_US::f8:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f8);
            break;

        case KeyCodeEN_US::f9:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f9);
            break;

        case KeyCodeEN_US::f10:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f10);
            break;

        case KeyCodeEN_US::f11:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f11);
            break;

        case KeyCodeEN_US::f12:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::f12);
            break;

        case KeyCodeEN_US::printScreen:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock
                        ? KeyCode::numberPadMultiply : KeyCode::printScreen);
            break;

        case KeyCodeEN_US::scrollLock:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::scrollLock);
            break;

        // Second row
        case KeyCodeEN_US::squigglyLine:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::squigglyLine : KeyCode::slantedApostrophe);
            break;

        case KeyCodeEN_US::one:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::exclamationMark : KeyCode::one);
            break;

        case KeyCodeEN_US::two:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::atSign: KeyCode::two);
            break;

        case KeyCodeEN_US::three:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::hash : KeyCode::three);
            break;

        case KeyCodeEN_US::four:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::dollarSign : KeyCode::four);
            break;

        case KeyCodeEN_US::five:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::percentSign : KeyCode::five);
            break;

        case KeyCodeEN_US::six:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::powerSign : KeyCode::six);
            break;

        case KeyCodeEN_US::seven:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::andSign : KeyCode::seven);
            break;

        case KeyCodeEN_US::eight:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::multiply : KeyCode::eight);
            break;

        case KeyCodeEN_US::nine:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::openBracket : KeyCode::nine);
            break;

        case KeyCodeEN_US::zero:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::closeBracket : KeyCode::zero);
            break;

        case KeyCodeEN_US::minus:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::underscore : KeyCode::minus);
            break;

        case KeyCodeEN_US::equals:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::plus : KeyCode::equals);
            break;

        case KeyCodeEN_US::backspace:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::backspace);
            break;

        case KeyCodeEN_US::insert:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock ? KeyCode::numberPadZero : KeyCode::insert);
            break;

        case KeyCodeEN_US::home:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock
                        ? KeyCode::numberPadSeven  : KeyCode::home);
            break;

        case KeyCodeEN_US::pageUp:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock ? KeyCode::numberPadNine : KeyCode::pageUp);
            break;

        case KeyCodeEN_US::numberPadLock:

            // Ensure this is not a repeat
            if(!released){
                this ->m_keyboard_state.number_pad_lock = !this ->m_keyboard_state.number_pad_lock;
            }
            onKeyRead(released, this ->m_keyboard_state, KeyCode::numberPadLock);
            break;

        case KeyCodeEN_US::numberPadForwardSlash:

            // Check if number pad lock is on
            if(this ->m_keyboard_state.number_pad_lock){
                onKeyRead(released, this ->m_keyboard_state, KeyCode::numberPadForwardSlash);
            }else{

                // Normal Forward Slash
                onKeyRead(released, this ->m_keyboard_state,
                          should_be_upper_case ? KeyCode::questionMark : KeyCode::forwardSlash);
            }
            break;

        // Number Pad Multiply is same as print screen

        case KeyCodeEN_US::numberPadMinus:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::numberPadMinus);
            break;

        // Third row
        case KeyCodeEN_US::tab:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::tab);
            break;

        case KeyCodeEN_US::Q:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::Q : KeyCode::q);
            break;

        case KeyCodeEN_US::W:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::W : KeyCode::w);
            break;

        case KeyCodeEN_US::E:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::E : KeyCode::e);
            break;

        case KeyCodeEN_US::R:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::R : KeyCode::r);
            break;

        case KeyCodeEN_US::T:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::T : KeyCode::t);
            break;

        case KeyCodeEN_US::Y:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::Y : KeyCode::y);
            break;

        case KeyCodeEN_US::U:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::U : KeyCode::u);
            break;

        case KeyCodeEN_US::I:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::I : KeyCode::i);
            break;

        case KeyCodeEN_US::O:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::O : KeyCode::o);
            break;

        case KeyCodeEN_US::P:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::P : KeyCode::p);
            break;

        case KeyCodeEN_US::openSquareBracket:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::openCurlyBracket : KeyCode::openSquareBracket);
            break;

        case KeyCodeEN_US::closeSquareBracket:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::closeCurlyBracket : KeyCode::closeSquareBracket);
            break;

        case KeyCodeEN_US::backslash:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::lineThing : KeyCode::backslash);
            break;

        case KeyCodeEN_US::deleteKey:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock
                        ? KeyCode::numberPadFullStop : KeyCode::deleteKey);
            break;

        case KeyCodeEN_US::end:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock ? KeyCode::numberPadOne : KeyCode::end);
            break;

        case KeyCodeEN_US::pageDown:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock
                        ? KeyCode::numberPadThree : KeyCode::pageDown);
            break;

        // Number pad 7 is same as home

        case KeyCodeEN_US::numberPadEight:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock
                        ? KeyCode::numberPadEight : KeyCode::upArrow);
            break;

        // Number pad 9 is same as page up

        case KeyCodeEN_US::numberPadPlus:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::numberPadPlus);
            break;

        // Fourth row

        case KeyCodeEN_US::capsLock:
            // Ensure this is not a repeat
            if(!released){
                this ->m_keyboard_state.caps_lock = !this ->m_keyboard_state.caps_lock;
            }

            onKeyRead(released, this ->m_keyboard_state, KeyCode::capsLock);
            break;

        case KeyCodeEN_US::A:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::A : KeyCode::a);
            break;

        case KeyCodeEN_US::S:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::S : KeyCode::s);
            break;

        case KeyCodeEN_US::D:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::D : KeyCode::d);
            break;

        case KeyCodeEN_US::F:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::F : KeyCode::f);
            break;

        case KeyCodeEN_US::G:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::G : KeyCode::g);
            break;

        case KeyCodeEN_US::H:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::H : KeyCode::h);
            break;

        case KeyCodeEN_US::J:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::J : KeyCode::j);
            break;

        case KeyCodeEN_US::K:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::K : KeyCode::k);
            break;

        case KeyCodeEN_US::L:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::L : KeyCode::l);
            break;

        case KeyCodeEN_US::semicolon:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::colon : KeyCode::semicolon);
            break;

        case KeyCodeEN_US::apostrophe:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::quotationMark : KeyCode::apostrophe);
            break;

        case KeyCodeEN_US::enter:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock
                        ? KeyCode::numberPadEnter : KeyCode::enter);
            break;

        case KeyCodeEN_US::numberPadFour:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock ? KeyCode::numberPadFour : KeyCode::leftArrow);
            break;

        case KeyCodeEN_US::numberPadFive:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::numberPadFive);
            break;

        case KeyCodeEN_US::numberPadSix:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock ? KeyCode::numberPadSix : KeyCode::rightArrow);
            break;

        // Fifth row
        case KeyCodeEN_US::leftShift:
            this ->m_keyboard_state.left_shift = !this ->m_keyboard_state.left_shift;
            onKeyRead(released, this ->m_keyboard_state, KeyCode::leftShift);
            break;

        case KeyCodeEN_US::Z:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::Z : KeyCode::z);
            break;

        case KeyCodeEN_US::X:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::X : KeyCode::x);
            break;

        case KeyCodeEN_US::C:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::C : KeyCode::c);
            break;

        case KeyCodeEN_US::V:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::V : KeyCode::v);
            break;

        case KeyCodeEN_US::B:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::B : KeyCode::b);
            break;

        case KeyCodeEN_US::N:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::N : KeyCode::n);
            break;

        case KeyCodeEN_US::M:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::M : KeyCode::m);
            break;

        case KeyCodeEN_US::comma:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::lessThan : KeyCode::comma);
            break;

        case KeyCodeEN_US::fullStop:
            onKeyRead(released, this ->m_keyboard_state,
                    should_be_upper_case ? KeyCode::greaterThan : KeyCode::fullStop);
            break;

        // Forward slash is same as number pad forward slash

        case KeyCodeEN_US::rightShift:
            // Check if this is a repeat
            if(!released){
                this ->m_keyboard_state.right_shift = !this ->m_keyboard_state.right_shift;
            }

            onKeyRead(released, this ->m_keyboard_state, KeyCode::rightShift);
            break;

        // Up Arrow is the same as number pad 8

        // Number pad 1 is the same as end

        case KeyCodeEN_US::numberPadTwo:
            onKeyRead(released, this ->m_keyboard_state, this ->m_keyboard_state.number_pad_lock ? KeyCode::numberPadTwo : KeyCode::downArrow);
            break;

        // Number pad 3 is the same as page down

        // Number pad enter is the same as enter

        // Sixth row
        case KeyCodeEN_US::leftControl:
            // Check if this is a repeat
            if(!released){
                this ->m_keyboard_state.left_control = !this ->m_keyboard_state.left_control;
                this ->m_keyboard_state.right_control = !this ->m_keyboard_state.right_control;
            }

            onKeyRead(released, this ->m_keyboard_state, KeyCode::leftControl);
            break;

        case KeyCodeEN_US::leftOS:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::leftOS);
            break;

        case KeyCodeEN_US::leftAlt:
            // Check if this is a repeat
            if(!released){
                this ->m_keyboard_state.left_alt = !this ->m_keyboard_state.left_alt;
                this ->m_keyboard_state.right_alt = !this ->m_keyboard_state.right_alt;
            }

            onKeyRead(released, this ->m_keyboard_state, KeyCode::leftAlt);
            break;

        case KeyCodeEN_US::space:
            onKeyRead(released, this ->m_keyboard_state, KeyCode::space);
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

KeyDownEvent::KeyDownEvent(KeyCode keyCode, const KeyboardState& keyboardState)
: Event<KeyboardEvents>(KeyboardEvents::KEYDOWN),
  key_code(keyCode),
  keyboard_state(keyboardState)
{
}

KeyDownEvent::~KeyDownEvent() = default;

KeyUpEvent::KeyUpEvent(KeyCode key_code, const KeyboardState& keyboard_state)
: Event<KeyboardEvents>(KeyboardEvents::KEYUP),
  key_code(key_code),
  keyboard_state(keyboard_state)
{
}

KeyUpEvent::~KeyUpEvent() = default;