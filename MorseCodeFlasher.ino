/*
    RMIT University Vietnam
    Course: EEET2505|EEET2654
    Semester: 2025A
    Assessment: Group Assessment - Mourse Code Flasher 
    Group: Group 07
    Members: 
        s3999925, Hoang Minh Thang
        s4044642, Nguyen Thanh Lam
        s3928624, Nguyen Ngoc Thien Ngan
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#define GREEN_LED PORTB2 // Pin for green LED, which indicate the dash (long mark) only
#define BLUE_LED PORTB1 // Pin for blue LED , which indicate the dot (short mark) only
#define RED_LED PORTB0 // Pin for red LED, which indicate the main message
#define SEGMENT_LED PORTD // Port for 7-segment LEDs
#define BUTTON_PIN PINB3  // Pin for button input
#define DEBOUNCE_TIME 50 // Debounce time in  50 milliseconds

// Global variables
// Debounce FSM (Finite State Machine) variables
volatile uint8_t db_last_raw = 1;       // Last raw sample (1 = not pressed, 0 = pressed)
volatile uint8_t db_state = 1;          // Current debounced state (1 = not pressed, 0 = pressed)
volatile uint8_t db_counter = 0;        // Milliseconds count for stable raw input
volatile uint8_t press_pending = 0;     // Flag set when a clean press is detected
volatile uint8_t release_detected = 0;  // Flag set when a release is detected after a press

// Create Morse type
typedef struct {
    char letter;
    const char* code;
} Morse;

Morse morseTable[] = {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."},
    {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"},
    {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, {'N', "-."}, {'O', "---"},
    {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
    {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"}, {'Y', "-.--"},
    {'Z', "--.."}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5',"....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
    {'0', "-----"},
};

// Waits for a duration of 0.5 seconds using Timer 1
void waitOneUnit() {
    DDRB |= (1 << PB5); // Set PB5 as output for LED
    // Reset the timer counter
    TCNT1 = 0;
    // Set Timer 1 with prescaler of 256 (CTC mode is omitted for simplicity)
    TCCR1B = (1 << CS12); // Prescaler 256
    // Wait until the timer count reaches the desired value for 0.5 seconds
    while (TCNT1 < 31249); // Waits for 0 to 31,248 -> 31,249 tick
    // Timer automatically rolls over, thus no need to clear the flag
}

// Initializes Timer0 in CTC (Clear Timer on Compare Match) mode
void initTimer0() {
    // Configure Timer0 for CTC (Clear Timer on Compare Match) mode
    // Set the prescaler to 64, which results in a timer frequency of 250kHz
    // This is calculated based on the CPU frequency (e.g., 16MHz / 64 = 250kHz)
    TCCR0A = (1 << WGM01); // Set WGM01 bit to enable CTC mode (WGM = 2)
    
    // Set the prescaler to 64
    TCCR0B = (1 << CS01) | (1 << CS00); // CS01 and CS00 bits set for prescaler of 64
    
    // Set the Output Compare Register A (OCR0A) to 249
    // This means the timer will count from 0 to 249 (250 counts total)
    // At 250kHz, this results in a 1ms interrupt (250kHz / 250 = 1kHz)
    OCR0A = 249; // Set the compare value for 1ms interrupts
    
    // Enable Timer0 Output Compare Match A interrupt
    TIMSK0 = (1 << OCIE0A); // Set OCIE0A bit to enable interrupt on compare match
}

// Debounce ISR (1ms tick)
ISR(TIMER0_COMPA_vect) {
    // Read the current button state and check for changes
    uint8_t raw = (PINB & (1 << BUTTON_PIN)) ? 1 : 0;
    // If the button state has changed, reset the debounce counter
    if (raw != db_last_raw) {
        db_last_raw = raw;
        db_counter = 0;
    } 
    // If the state is stable, increment the debounce counter
    else if (db_counter < DEBOUNCE_TIME) {
        if (++db_counter == DEBOUNCE_TIME) {
            // Update the debounced state and set flags for press/release events
            if (db_state != raw) {
                db_state = raw;
                if (db_state == 0) press_pending = 1; // Button pressed
                else if (db_state == 1 && press_pending) release_detected = 1; // Button released
            }
        }
    }
}

// dih signal (short mask) for 0.5 seconds ON and 0.5 second OFF
void dot() {
    PORTB |= (1 << RED_LED); // turn on RED LED
    PORTB |= (1 << BLUE_LED); // turn on BLUE LED
    waitOneUnit(); // ON for 1 time unit (500 ms)
    PORTB &= ~(1 << RED_LED); // turn off RED LED
    PORTB &= ~(1 << BLUE_LED); // turn off BLUE LED
    waitOneUnit(); // Inner-symbol gap (500ms)
}

// dah signal (long mask) for 1.5 seconds ON and 0.5 second OFF
void dash() {
    PORTB |= (1 << RED_LED); // turn on RED LED
    PORTB |= (1 << GREEN_LED); // turn on GREEN LED
    waitOneUnit(); // ON for 3 time units (1500 ms)
    waitOneUnit();
    waitOneUnit();

    PORTB &= ~(1 << RED_LED); // turn off RED LED
    PORTB &= ~(1 << GREEN_LED); // turn off GREEN LED
    waitOneUnit(); // Inner-symbol gap (500ms)
}

// Create a short gap between letters in Morse code signaling
void letterGap() {
    // Wait total 3 time units (1.5s)
    for(int i = 0; i < 2; i++) { // Note: already wait 1 time units -> iterate 2 more times
        waitOneUnit();  
    }
}

// Create a medium gap between words in Morse code signaling
void wordGap() {
   // Wait total 7 time units (3.5s)
    for(int i = 0; i < 6; i++) { // Note: already wait 1 time unit  -> iterate 6 more times
        waitOneUnit();  
    }
}

// signalStart() sends the prosign “KA” (-.-.-) before every Morse message.
void signalStart() {
    // Start prosign = "KA" ( -.-.-) (transmitted as one merged Morse symbol, without a letter gap between the K and A)
    dash(); // -
    dot();  // .
    dash(); // -
    dot();  // .
    dash(); // -
    letterGap(); 
}

// signalStop() sends the prosign “AR” (.-.-.) after each message.
void signalStop(){
    // Stop prosign = "AR" (.-.-.) (transmitted as one merged Morse symbol, without a letter gap between the A and R.)
    letterGap(); // 3 time units (1.5s)
    dot();    // .
    dash();   // -
    dot();    // .
    dash();   // -
    dot();    // .
}

// Get morse code for a letter
const char* getMorseCode(char letter) {
    letter = toupper(letter); //convert letter to UPPERCASE
    for (int i = 0; i < sizeof(morseTable) / sizeof(Morse); i++) {
        if (morseTable[i].letter == letter) {
            return morseTable[i].code;
        }
    }
    return ""; // Unknown character
}

// Send the Morse Code of a give message (group members' name or 'SOS')
void sendMorseMessage(const char* message) {
    for (int i = 0; message[i] != '\0'; i++) { // Loop through each character in the message
        char c = message[i]; // Get the current character
        // Word gap [medium gap] (wait 3.5 seconds) if space
        if (c == ' ') {
            wordGap();
            continue;
        }
        const char* code = getMorseCode(c); // Get Morse code for the character
        for (int j = 0; code[j] != '\0'; j++) { // Loop through each symbol in the Morse code
            if (code[j] == '.') {
                dot();
            } else if (code[j] == '-') {
                dash();
            }
        }
        // Gap between letters [short gap] (pause 1.5 seconds)
        if (message[i + 1] != '\0' && message[i + 1] != ' ') {
            letterGap();
        }
    }
}

// Display number on the 7-segment LED
void displayNumber(uint8_t num) {
    // Lookup table for 0-6 on a common cathode 7-segment display
    const uint8_t segmentMap[7] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
    };

    if (num < 7 && (num != 0)) { // Only display 1 -> 6 on 7segment LEDs 
        PORTD = segmentMap[num];
    } else {
        PORTD = 0x00; // Clear display if out of range
    }
}

// Main program
int main() {
    // Set all three LEDs pin as output
    DDRB |= (1 << RED_LED);
    DDRB |= (1 << BLUE_LED);
    DDRB |= (1 << GREEN_LED);
    
    DDRD |= 0xFF;           // Set all pins of PORTD (7-segment LED) as output
    DDRB &= ~(1 << PINB3);  // Set BUTTON_PIN (pin 3) as input

    // Initially turn OFF all 3 LEDs
    PORTB &= ~(1 << RED_LED);
    PORTB &= ~(1 << BLUE_LED);
    PORTB &= ~(1 << GREEN_LED);
    PORTD &= ~0xFF;     // Initialize all PORTD pins to low

    // Local variable
    int mode = 1;                       // Initialize mode variable
    uint8_t button_press_count = 0;     // Initialize button press count

    initTimer0();   // Initialize Timer0 for timing operations
    sei();          // Enable global interrupts

    while (1) { // Infinite loop for continuous operation
        // Reset flags for the next button press cycle
        press_pending = 0;
        release_detected = 0;

        while (!release_detected); // Wait until a release is detected (spin-wait)
        // Increment the button press count, wrapping around after 6
        button_press_count = (button_press_count % 6) + 1;
        displayNumber(button_press_count); // Display the current button press count
        
        // Behavior according to button press count
        if ((button_press_count == 2) && (mode == 1)) {
            signalStart();  // Start singnaling 'KA'
            sendMorseMessage("THANG LAM NGAN");   // Send group members' names in Morse code
            signalStop();   // Stop singnaling 'AR'
            mode++; // Increment to prevent running this message twice

        } else if ((button_press_count == 4) && (mode == 2)) { 
            signalStart();  // Start singnaling 'KA'
            sendMorseMessage("SOS");    // Send 'SOS' in Morse code
            signalStop();   // Stop singnaling 'AR'
            mode++;
        }
    }
    return 0; // Success (not reach in infinite loop)
}
