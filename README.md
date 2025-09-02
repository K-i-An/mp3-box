# MP3 Box - Children's Music Player

An Arduino-based MP3 player system designed for children, featuring RFID-triggered playback, WiFi remote control, and themed music collections.

## Features

- **RFID Card Integration**: 46 RFID cards trigger different themed music folders
- **WiFi Web Interface**: Remote control via web browser with play/pause, volume, and folder selection
- **Physical Controls**: Button for pause/play and "stop-dance" mode (random pauses for dancing)
- **Auto-Sleep**: Automatically pauses after 1.5 hours of inactivity to save power
- **MDNS Discovery**: Easy network access using `http://mp3box.local` (configurable)
- **Sequential Playback**: Plays through folder contents and loops back to the beginning

## Hardware Requirements

- ESP8266 microcontroller (NodeMCU or Wemos D1 Mini)
- DFPlayer Mini MP3 module
- MFRC522 RFID reader module
- SD card with MP3 files organized in folders
- Push button
- Speaker or audio amplifier
- Power supply (5V recommended)

### Pin Connections

```
ESP8266     | DFPlayer Mini | MFRC522 RFID | Button
------------|---------------|--------------|--------
D13 (RX)    | TX           |              |
D15 (TX)    | RX           |              |
D5          |               | RST         |
D4          |               | SDA/SS      |
D3          |               |              | Button Pin
GND         | GND          | GND         | GND
5V          | VCC          | 3.3V        | (optional)
```

*Note: MFRC522 operates at 3.3V, ensure proper voltage level shifting if needed.*

## Software Setup

### Prerequisites

- Arduino IDE with ESP8266 board support
- Required libraries:
  - `DFRobotDFPlayerMini`
  - `MFRC522`
  - `ESP8266WiFi`
  - `ESP8266WebServer`
  - `ESP8266mDNS`

### Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/K-i-An/mp3-box.git
   cd mp3-box
   ```

2. Open the desired version in Arduino IDE:
   - `website_wifi_v1_3_select/website_wifi_v1_3_select.ino` (loops entire folder)
   - `website_wifi_v1_4_withoutLoopFolder/website_wifi_v1_4_withoutLoopFolder.ino` (sequential playback)

3. Configure WiFi credentials:
   ```cpp
   const char* ssids[] = {"Your_WiFi_Name", "Alternative_SSID"};
   const char* password = "Your_WiFi_Password";
   ```

4. Prepare SD card:
   - Format as FAT32
   - Create folders 01-46 with MP3 files
   - Folder structure: `/01/track001.mp3`, `/02/track001.mp3`, etc.

5. Upload sketch to ESP8266

## Usage

### RFID Setup

Each RFID card is mapped to a specific folder. The mapping is defined in the `saveRFIDTable()` function. To add new cards:

1. Scan the card UID using the serial monitor
2. Add the UID to the `listUID` array with desired folder ID

### Web Interface

1. Connect to the same WiFi network as the MP3 Box
2. Open browser and navigate to the MDNS address (shown in serial monitor) or IP address
3. Use the web interface to:
   - Play/Pause music
   - Adjust volume
   - Select folders manually
   - View status information

### Physical Controls

- **Short Button Press**: Toggle play/pause
- **Long Button Press**: Toggle "stop-dance" mode (random 10-30 second pauses)

## Music Folders

The system supports up to 46 folders for organizing MP3 files. Each folder can contain multiple audio tracks that will play sequentially. The folders are mapped to RFID cards, allowing users to trigger specific collections by tapping their card.

Example folder organization:
- **01-20**: Children's stories and songs
- **21-30**: Educational content and nursery rhymes
- **31-36**: Special themes (worship, workout, bedtime stories)
- **37-46**: Custom folders for additional content

*Note: Folder names and contents should comply with copyright laws. Use royalty-free music or content you have rights to.*

## Troubleshooting

### Common Issues

**WiFi Connection Failed**
- Check SSID and password in code
- Ensure ESP8266 is within WiFi range
- Try different WiFi channels (ESP8266 has issues with some channels)

**MP3 Files Not Playing**
- Verify SD card format (FAT32)
- Check file naming (001.mp3, 002.mp3, etc.)
- Ensure proper folder structure (/01/, /02/, etc.)

**RFID Not Responding**
- Check wiring connections
- Verify RFID card compatibility (13.56MHz MIFARE cards)
- Check serial output for UID readings

**Web Interface Not Accessible**
- Confirm device is connected to WiFi
- Try accessing via IP address instead of MDNS name
- Check firewall settings

### Serial Debugging

Connect to ESP8266 via serial monitor (9600 baud) for debugging information including:
- WiFi connection status
- RFID card UIDs
- MP3 player errors
- File playback status

## Version Differences

### v1.3 (website_wifi_v1_3_select)
- Uses `loopFolder()` - repeats entire folder continuously
- Simpler playback logic
- Basic folder looping functionality

### v1.4 (website_wifi_v1_4_withoutLoopFolder)
- Uses `playFolder()` - sequential file playback with auto-advance
- Handles end-of-folder and file errors gracefully
- Shows current/max file numbers in status page
- Improved error handling for missing files

## Potential Extensions and Future Improvements

### Hardware Enhancements
- **Battery Power**: Add LiPo battery with charging circuit and deep sleep modes
- **LED Indicators**: RGB LEDs for status indication (playing, paused, folder active)
- **Audio Amplifier**: Integrated amplifier for better sound quality
- **Multiple Speakers**: Support for stereo or multi-room audio
- **Touch Screen**: LCD touchscreen for local control interface

### Software Features
- **Mobile App**: Native iOS/Android app for remote control
- **Voice Control**: Integration with Alexa, Google Home, or Siri
- **Custom Playlists**: Web interface for creating custom playlists across folders
- **Timer Functions**: Sleep timer, alarm clock features
- **Equalizer Settings**: Adjustable audio EQ via web interface
- **Multi-Language Support**: Web interface in multiple languages
- **OTA Updates**: Over-the-air firmware updates
- **SD Card Management**: Web interface for uploading/managing MP3 files

### Network Features
- **MQTT Integration**: Connect to home automation systems
- **REST API**: Full REST API for third-party integrations
- **WebSocket Support**: Real-time status updates in web interface
- **Network Streaming**: Stream audio from network sources
- **Multi-Device Sync**: Synchronize playback across multiple MP3 boxes

### Advanced RFID Features
- **Dynamic RFID Mapping**: Web interface for assigning folders to cards
- **Guest Cards**: Temporary RFID cards with limited access
- **RFID Statistics**: Track usage statistics per card/folder
- **NFC Support**: Upgrade to NFC for smartphone integration

### User Experience
- **Themes**: Customizable web interface themes
- **Animations**: CSS animations and transitions
- **Responsive Design**: Mobile-optimized web interface
- **Keyboard Shortcuts**: Web interface keyboard controls
- **Accessibility**: Screen reader support and high contrast modes

### Development Tools
- **Configuration Web Interface**: WiFi setup without code changes
- **Debug Logging**: Enhanced logging with web-accessible logs
- **Performance Monitoring**: System resource usage tracking
- **Backup/Restore**: SD card content backup and restore functionality

## Contributing

Contributions are welcome! Please feel free to submit pull requests for bug fixes, new features, or improvements.

## License

This project is open source. Please check the license file for details.

## Acknowledgments

- Based on DFPlayer Mini library by DFRobot
- MFRC522 library by Miguel Balboa
- ESP8266 core for Arduino

---

*For questions or support, please open an issue on GitHub.*
