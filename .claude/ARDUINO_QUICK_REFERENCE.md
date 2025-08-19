# Arduino Export Quick Reference

## 🚀 Quick Start

### 1. Basic Export Process
1. **Design Circuit** in wiRedPanda
2. **File → Export → Arduino Code**
3. **Choose target board** (Uno/Mega/Leonardo)
4. **Upload to Arduino** and test

### 2. Supported Elements

| Element | Support Level | Notes |
|---------|---------------|-------|
| **AND/OR/NOT Gates** | ✅ Full | Perfect compatibility |
| **D Flip-Flop** | ✅ Full | Interrupt-based edge detection |
| **JK Flip-Flop** | ✅ Full | Includes toggle mode |
| **SR Flip-Flop** | ✅ Full | Handles forbidden states |
| **T Flip-Flop** | ✅ Full | Toggle functionality |
| **Latches** | ⚠️ Limited | Level-triggered only |
| **Multiplexers** | ✅ Full | Efficient implementation |
| **7-Seg Display** | ✅ Full | Multiplexed output |
| **Clock** | ✅ Full | Hardware timer support |

## 📋 Board Specifications

### Arduino Uno
- **RAM**: 2KB (manage usage carefully)
- **Pins**: 20 total (2 interrupt pins: 2,3)
- **Best for**: Simple circuits (≤4 flip-flops)

### Arduino Mega  
- **RAM**: 8KB (generous headroom)
- **Pins**: 70 total (6 interrupt pins: 2,3,18,19,20,21)
- **Best for**: Complex circuits (≤20 flip-flops)

### Arduino Leonardo
- **RAM**: 2.5KB 
- **Pins**: 23 total (5 interrupt pins: 0,1,2,3,7)
- **Best for**: Medium circuits (≤6 flip-flops)

## ⚠️ Common Issues & Solutions

### "Insufficient Interrupt Pins"
**Problem**: More flip-flops than interrupt pins
**Solutions**:
- Use shared clock domains
- Upgrade to Arduino Mega
- Use hardware timers for clocks

### "Memory Usage Exceeds Limit"
**Problem**: Circuit too complex for board RAM
**Solutions**:
- Simplify circuit design
- Upgrade to Arduino Mega  
- Optimize variable usage

### "Sequential Logic Not Working"
**Problem**: Timing issues with clocks
**Solutions**:
- Check mechanical switch debouncing
- Verify clock frequencies
- Use debug mode for troubleshooting

## 🔧 Configuration Options

### Generator Settings
```cpp
generator.setTargetBoard("Arduino Uno");     // Board selection
generator.setEnableDebouncing(true);         // Input debouncing
generator.setDebounceDelay(50);             // 50ms debounce
generator.setDebugMode(true);               // Serial debugging
```

### Pin Assignments
- **Interrupt pins**: Automatically assigned to clocks
- **Digital pins**: Assigned to inputs/outputs  
- **Analog pins**: Used for additional inputs
- **Conflicts**: Automatically detected and resolved

## 📊 Performance Guidelines

### Circuit Complexity Limits

| Board | Simple | Medium | Complex |
|-------|--------|--------|---------|
| **Uno** | 1-4 FFs | 5-8 FFs | Not recommended |
| **Mega** | 1-8 FFs | 9-16 FFs | 17+ FFs |
| **Leonardo** | 1-4 FFs | 5-10 FFs | 11+ FFs |

### Clock Frequency Limits
- **Mechanical switches**: ≤10 Hz (with debouncing)
- **Push buttons**: ≤100 Hz  
- **Hardware timers**: Up to 16 MHz/prescaler
- **Recommended max**: 1 kHz for educational circuits

## 🧪 Testing Checklist

### Before Deployment
- [ ] Resource analysis shows <90% usage
- [ ] No pin conflicts detected
- [ ] Clock frequencies within limits
- [ ] Reset logic properly implemented

### After Upload
- [ ] Serial monitor shows initialization
- [ ] LED outputs match expected states
- [ ] Clock edges trigger state changes
- [ ] No erratic behavior observed

## 🔍 Debugging Tools

### Serial Debug Output
```cpp
// Automatically added in debug mode
Serial.print("FF_state: Q=");
Serial.print(ff.Q ? "1" : "0");
Serial.println();
```

### Resource Monitor
```cpp
// Check resource usage
ResourceProfile profile = generator.analyzeResources();
qDebug() << "RAM:" << profile.ramUsage << "/" << BOARD_RAM_LIMITS[board];
qDebug() << "Pins:" << profile.pinCount << "/" << BOARD_PIN_COUNTS[board];
```

### Test Framework
```cpp
// Run comprehensive tests
ArduinoTestBench testBench("MyCircuit");
testBench.generateDFlipFlopTests();
bool success = testBench.runAllTests();
```

## 📝 Code Examples

### Simple D Flip-Flop
```cpp
// Minimal working example
const int CLOCK_PIN = 2;    // Interrupt pin
const int DATA_PIN = 4;     // Digital input  
const int Q_PIN = 8;        // Digital output

struct DFlipFlopState {
    bool Q = false;
    bool triggered = false;
    bool data_captured = false;
} ff;

void clock_ISR() {
    ff.data_captured = digitalRead(DATA_PIN);
    ff.triggered = true;
}

void setup() {
    pinMode(CLOCK_PIN, INPUT_PULLUP);
    pinMode(DATA_PIN, INPUT_PULLUP);
    pinMode(Q_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), clock_ISR, RISING);
}

void loop() {
    if (ff.triggered) {
        ff.Q = ff.data_captured;
        ff.triggered = false;
        digitalWrite(Q_PIN, ff.Q);
    }
}
```

### Hardware Clock Generation
```cpp
// 1Hz clock using Timer1
void setupTimer1() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 15624; // 16MHz/(1024*1Hz) - 1
    TCCR1B |= (1 << WGM12);           // CTC mode
    TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 prescaler  
    TIMSK1 |= (1 << OCIE1A);          // Enable interrupt
    sei();
}

ISR(TIMER1_COMPA_vect) {
    // Generate clock edge for connected flip-flops
    triggerClockEdge();
}
```

## 🎯 Best Practices

### Circuit Design
1. **Start Simple**: Begin with basic circuits, add complexity gradually
2. **Use Reset**: Always include reset logic for initialization
3. **Share Clocks**: Minimize number of different clock domains
4. **Consider Timing**: Design for real-world timing constraints

### Hardware Setup  
1. **Stable Power**: Use quality power supply (avoid USB power for complex circuits)
2. **Good Grounding**: Ensure proper ground connections
3. **Debounce Inputs**: Use hardware debouncing for mechanical switches
4. **Monitor Current**: Check power consumption within Arduino limits

### Software Development
1. **Enable Debug**: Use debug mode during development
2. **Test Incrementally**: Validate each circuit section separately  
3. **Check Resources**: Monitor RAM and pin usage
4. **Version Control**: Save working versions before modifications

## 📞 Support & Resources

### Getting Help
- **Issues**: Report bugs on GitHub repository
- **Questions**: Use wiRedPanda community forums
- **Documentation**: Refer to complete user guide
- **Examples**: Check example circuits in distribution

### Additional Resources
- **Arduino Documentation**: Official Arduino reference
- **Timing Analysis**: Hardware timing constraint guides  
- **Circuit Design**: Digital logic textbooks
- **Hardware Debugging**: Oscilloscope and logic analyzer guides

---
*This quick reference covers the essential information for using wiRedPanda's improved Arduino export. For comprehensive details, see the complete documentation.*
