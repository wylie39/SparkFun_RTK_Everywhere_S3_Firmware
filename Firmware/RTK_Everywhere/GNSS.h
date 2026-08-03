/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GNSS.h

  Declarations and definitions for the GNSS layer
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

#ifndef __GNSS_H__
#define __GNSS_H__

// GNSS receiver type detected in Facet FP
typedef enum
{
    GNSS_RECEIVER_LG290P = 0,
    GNSS_RECEIVER_MOSAIC_X5,
    GNSS_RECEIVER_X20P,
    GNSS_RECEIVER_UM980,
    GNSS_RECEIVER_F9P,
    // Add new values above this line
    GNSS_RECEIVER_UNKNOWN,
} gnssReceiverType_e;

// Every virtual method below has a default (mostly no-op / neutral-value) body given
// directly here, except for the handful that already have a real implementation in
// GNSS.ino (comPortRefresh, hasGnssSpecificConfiguration, isAntennaShorted, isAntennaOpen,
// menuGnssSpecificConfiguration, setGnssSpecificConfiguration, supportsAntennaShortOpen).
// This matters beyond documentation: under the Itanium C++ ABI, GCC emits a class's
// vtable only in the translation unit that defines the class's "key function" (its
// first declared virtual method that is neither pure nor given an inline body). If any
// virtual method here were left as a bare declaration with no definition anywhere, that
// method - or whichever bodyless one came first - would become a key function with no
// definition, and the GNSS vtable would never be emitted, which shows up as a link
// error ("undefined reference to vtable for GNSS") that can appear or disappear
// depending on unrelated code changes and inliner heuristics. Giving every method here
// an inline body removes GNSS's key function entirely, so its vtable gets safe,
// automatic (vague/weak) linkage wherever it's needed - regardless of how many
// "new GNSS_<driver>()" call sites exist or how aggressively the compiler inlines them.
// Do not add a new virtual method here without also giving it a body (or a real
// out-of-line definition in GNSS.ino).
class GNSS
{
  protected:
    double _altitude;          // Altitude in meters
    double _geoidalSeparation; // Geoidal separation in meters
    float _horizontalAccuracy; // Horizontal position accuracy in meters
    double _latitude;          // Latitude in degrees
    double _longitude;         // Longitude in degrees

    uint8_t _day;   // Day number
    uint8_t _month; // Month number
    uint16_t _year;
    uint8_t _hour; // Hours for 24 hour clock
    uint8_t _minute;
    uint8_t _second;
    uint8_t _leapSeconds;
    uint16_t _millisecond; // Limited to first two digits
    uint32_t _nanosecond;

    uint8_t _satellitesInView;
    uint8_t _fixType;
    uint8_t _carrierSolution;

    bool _validDate; // True when date is valid
    bool _validTime; // True when time is valid
    bool _confirmedDate;
    bool _confirmedTime;
    bool _fullyResolved;
    uint32_t _tAcc;

    unsigned long _pvtArrivalMillis;
    bool _pvtUpdated;

    bool _corrRadioExtPortEnabled = false;

    unsigned long _autoBaseStartTimer; // Tracks how long the base auto / averaging mode has been running

  public:
    // Constructor
    GNSS() : _leapSeconds(18), _pvtArrivalMillis(0), _pvtUpdated(0), _satellitesInView(0)
    {
    }

    // If we have decryption keys, configure module
    virtual void applyPointPerfectKeys() {}

    // Set RTCM for base mode to defaults (1005/1074/1084/1094/1124 1Hz & 1230 0.1Hz)
    virtual void baseRtcmDefault() {}

    // Reset to Low Bandwidth Link (1074/1084/1094/1124 0.5Hz & 1005/1230 0.1Hz)
    virtual void baseRtcmLowDataRate() {}

    // Check if a given baud rate is supported by this module
    virtual bool baudIsAllowed(uint32_t baudRate) { return false; }
    virtual uint32_t baudGetMinimum() { return 0; }
    virtual uint32_t baudGetMaximum() { return 0; }

    // Connect to GNSS and identify particulars
    virtual void begin() {}

    // Setup TM2 time stamp input as need
    // Outputs:
    //   Returns true when an external event occurs and false if no event
    virtual bool beginExternalEvent() { return false; }

    virtual bool checkNMEARates() { return false; }

    virtual bool checkPPPRates() { return false; }

    // On platforms that support / need it (i.e. mosaic-X5), refresh the
    // COM port by sending an escape sequence or similar to make the
    // GNSS snap out of it...
    // Outputs:
    //   Returns true if successful and false upon failure
    virtual bool comPortRefresh();

    // Setup the general configuration of the GNSS
    // Not Rover or Base specific (ie, baud rates)
    // Outputs:
    //   Returns true if successfully configured and false upon failure
    virtual bool configure() { return false; }

    // Configure the Base
    // Outputs:
    //   Returns true if successfully configured and false upon failure
    virtual bool configureBase() { return false; }

    // Configure specific aspects of the receiver for NTP mode
    virtual bool configureNtpMode() { return false; }

    // Configure the Rover
    // Outputs:
    //   Returns true if successfully configured and false upon failure
    virtual bool configureRover() { return false; }

    // Responds with the messages supported on this platform
    // Inputs:
    //   returnText: String to receive message names
    // Returns message names in the returnText string
    virtual void createMessageList(String &returnText) {}

    // Responds with the RTCM/Base messages supported on this platform
    // Inputs:
    //   returnText: String to receive message names
    // Returns message names in the returnText string
    virtual void createMessageListBase(String &returnText) {}

    virtual void debuggingDisable() {}

    virtual void debuggingEnable() {}

    // Restore the GNSS to the factory settings
    virtual void factoryReset() {}

    virtual uint16_t fileBufferAvailable() { return 0; }

    virtual uint16_t fileBufferExtractData(uint8_t *fileBuffer, int fileBytesToRead) { return 0; }

    // Start the base using fixed coordinates
    // Outputs:
    //   Returns true if successfully started and false upon failure
    virtual bool fixedBaseStart() { return false; }

    virtual bool fixRateIsAllowed(uint32_t fixRateMs) { return false; }

    // Return min/max rate in ms
    virtual uint32_t fixRateGetMinimumMs() { return 0; }

    virtual uint32_t fixRateGetMaximumMs() { return 0; }

    // Return the number of active/enabled messages
    virtual uint8_t getActiveMessageCount() { return 0; }

    // Return the number of active/enabled RTCM messages
    virtual uint8_t getActiveRtcmMessageCount() { return 0; }

    // Get the altitude
    // Outputs:
    //   Returns the altitude in meters or zero if the GNSS is offline
    virtual double getAltitude() { return 0; }

    // Returns the carrier solution or zero if not online
    virtual uint8_t getCarrierSolution() { return 0; }

    virtual uint32_t getDataBaudRate() { return 0; }

    // Returns the day number or zero if not online
    virtual uint8_t getDay() { return 0; }

    // Return the number of milliseconds since GNSS data was last updated
    virtual uint16_t getFixAgeMilliseconds() { return 0; }

    // Returns the fix type or zero if not online
    virtual uint8_t getFixType() { return 0; }

    // Returns the geoidal separation
    virtual double getGeoidalSeparation() { return 0; }

    // Returns the hours of 24 hour clock or zero if not online
    virtual uint8_t getHour() { return 0; }

    // Get the horizontal position accuracy
    // Outputs:
    //   Returns the horizontal position accuracy or zero if offline
    virtual float getHorizontalAccuracy() { return 0; }

    virtual const char *getId() { return ""; }

    // Get the latitude value
    // Outputs:
    //   Returns the latitude value or zero if not online
    virtual double getLatitude() { return 0; }

    // Query GNSS for current leap seconds
    virtual uint8_t getLeapSeconds() { return 0; }

    // Return the type of logging that matches the enabled messages - drives the logging icon
    virtual uint8_t getLoggingType() { return 0; }

    // Get the longitude value
    // Outputs:
    //   Returns the longitude value or zero if not online
    virtual double getLongitude() { return 0; }

    // Returns two digits of milliseconds or zero if not online
    virtual uint8_t getMillisecond() { return 0; }

    // Returns minutes or zero if not online
    virtual uint8_t getMinute() { return 0; }

    // Returns the current mode: Base/Rover/etc
    virtual uint8_t getMode() { return 0; }

    // Returns month number or zero if not online
    virtual uint8_t getMonth() { return 0; }

    // Returns nanoseconds or zero if not online
    virtual uint32_t getNanosecond() { return 0; }

    virtual uint32_t getRadioBaudRate() { return 0; }

    // Returns the seconds between solutions
    virtual double getRateS() { return 0; }

    virtual const char *getRtcmDefaultString() { return ""; }

    virtual const char *getRtcmLowDataRateString() { return ""; }

    // Returns the number of satellites in view or zero if offline
    virtual uint8_t getSatellitesInView() { return 0; }

    // Returns seconds or zero if not online
    virtual uint8_t getSecond() { return 0; }

    // Get the survey-in mean accuracy
    // Outputs:
    //   Returns the mean accuracy or zero (0)
    virtual float getSurveyInMeanAccuracy() { return 0; }

    // Return the number of seconds the survey-in process has been running
    virtual int getSurveyInObservationTime() { return 0; }

    // Returns timing accuracy or zero if not online
    virtual uint32_t getTimeAccuracy() { return 0; }

    // Sets the four version number parts
    virtual bool getVersion(uint16_t &major, uint8_t &minor, uint8_t &patch, uint8_t &revision) { return false; }

    // Returns full year, ie 2023, not 23.
    virtual uint16_t getYear() { return 0; }

    // Helper functions for the current mode as read from the GNSS receiver
    // Not to be confused with inRoverMode() and inBaseMode() used in States.ino
    virtual bool gnssInBaseFixedMode() { return false; }
    virtual bool gnssInBaseSurveyInMode() { return false; }
    virtual bool gnssInRoverMode() { return false; }

    // Indicate if there are any additional settings specific to this GNSS
    // This governs setGnssSpecificConfiguration() and menuGnssSpecificConfiguration()
    virtual bool hasGnssSpecificConfiguration();

    // Antenna Short / Open detection
    virtual bool isAntennaShorted();
    virtual bool isAntennaOpen();

    virtual bool isBlocking() { return false; }

    // Date is confirmed once we have GNSS fix
    virtual bool isConfirmedDate() { return false; }

    // Date is confirmed once we have GNSS fix
    virtual bool isConfirmedTime() { return false; }

    // Returns true if data is arriving on the Radio Ext port
    virtual bool isCorrRadioExtPortActive() { return false; }

    // Return true if GNSS receiver has a higher quality DGPS fix than 3D
    virtual bool isDgpsFixed() { return false; }

    // Some functions merely need to know if we have an RTK Float.
    // This function checks to see if the given platform has reached sufficient
    // fix type to be considered valid.
    virtual bool isFixed() { return false; }

    // Used in tpISR() for time pulse synchronization
    virtual bool isFullyResolved() { return false; }

    virtual bool isPppConverged() { return false; }

    virtual bool isPppConverging() { return false; }

    // Some functions merely need to know if we have an RTK Float.
    // This function checks to see if the given platform has reached sufficient
    // fix type to be considered valid.
    virtual bool isRTKFix() { return false; }

    // Some functions merely need to know if we have an RTK Float.
    // This function checks to see if the given platform has reached sufficient
    // fix type to be considered valid.
    virtual bool isRTKFloat() { return false; }

    // Determine if the survey-in operation is complete
    // Outputs:
    //   Returns true if the survey-in operation is complete and false
    //   if the operation is still running
    virtual bool isSurveyInComplete() { return false; }

    // Date will be valid if the RTC is reporting (regardless of GNSS fix)
    virtual bool isValidDate() { return false; }

    // Time will be valid if the RTC is reporting (regardless of GNSS fix)
    virtual bool isValidTime() { return false; }

    // Controls the constellations that are used to generate a fix and logged
    virtual void menuConstellations() {}

    // Configure any settings specific to this GNSS
    virtual void menuGnssSpecificConfiguration();

    virtual void menuMessageBaseRtcm() {}

    // Control the messages that get broadcast over Bluetooth and logged (if enabled)
    virtual void menuMessages() {}

    // Print the module type and firmware version
    virtual void printModuleInfo() {}

    // Send correction data to the GNSS
    // Inputs:
    //   dataToSend: Address of a buffer containing the data
    //   dataLength: The number of valid data bytes in the buffer
    // Outputs:
    //   Returns the number of correction data bytes written
    virtual int pushRawData(uint8_t *dataToSend, int dataLength) { return 0; }

    // Hardware or software reset the GNSS receiver
    virtual bool reset() { return false; }

    virtual uint16_t rtcmBufferAvailable() { return 0; }

    // If LBand is being used, ignore any RTCM that may come in from the GNSS
    virtual void rtcmOnGnssDisable() {}

    // If L-Band is available, but encrypted, allow RTCM through other sources (radio, ESP-NOW) to GNSS receiver
    virtual void rtcmOnGnssEnable() {}

    virtual uint16_t rtcmRead(uint8_t *rtcmBuffer, int rtcmBytesToRead) { return 0; }

    virtual bool setBaudRate(uint8_t uartNumber, uint32_t baudRate) { return false; }

    virtual bool setBaudRateComm(uint32_t baud) { return false; }

    virtual bool setBaudRateData(uint32_t baud) { return false; }

    virtual bool setBaudRateRadio(uint32_t baud) { return false; }

    // Save the current configuration
    // Outputs:
    //   Returns true when the configuration was saved and false upon failure
    virtual bool saveConfiguration() { return false; }

    // Enable all the valid constellations and bands for this platform
    virtual bool setConstellations() { return false; }

    // Enable / disable corrections protocol(s) on the Radio External port
    // Always update if force is true. Otherwise, only update if enable has changed state
    virtual bool setCorrRadioExtPort(bool enable, bool force) { return false; }

    // Set the elevation in degrees
    // Inputs:
    //   elevationDegrees: The elevation value in degrees
    virtual bool setElevation(uint8_t elevationDegrees) { return false; }

    // Configure any additional settings specific to this GNSS
    virtual bool setGnssSpecificConfiguration();

    virtual bool setPppService() { return false; }

    // Configure any logging settings - currently mosaic-X5 specific
    virtual bool setLogging() { return false; }

    // Enable/disable messages according to the NMEA array
    virtual bool setMessagesNMEA() { return false; }

    // Enable/disable messages according to the RTCM Base array
    virtual bool setMessagesRTCMBase() { return false; }

    // Enable/disable messages according to the NMEA array
    virtual bool setMessagesRTCMRover() { return false; }

    // Turn on all the enabled Extra/Other messages
    virtual bool setMessagesOther() { return false; }

    // Set the minimum satellite signal level for navigation.
    virtual bool setMinCN0(uint8_t cnoValue) { return false; }

    // Set the dynamic model to use for RTK
    // Inputs:
    //   modelNumber: Number of the model to use, provided by radio library
    virtual bool setModel(uint8_t modelNumber) { return false; }

    virtual bool setMultipathMitigation(bool enableMultipathMitigation) { return false; }

    virtual bool setNmeaMessageRateByName(const char *msgName, uint8_t msgRate) { return false; }

    // Configure the Pulse-per-second pin based on user settings
    virtual bool setPPS() { return false; }

    // Specify the interval between solutions
    // Inputs:
    //   secondsBetweenSolutions: Number of seconds between solutions
    // Outputs:
    //   Returns true if the rate was successfully set and false upon
    //   failure
    virtual bool setRate(double secondsBetweenSolutions) { return false; }

    // Enable/disable any output needed for tilt compensation
    virtual bool setTilt() { return false; }

    virtual bool standby() { return false; }

    // Antenna Short / Open detection
    virtual bool supportsAntennaShortOpen();

    // Reset the survey-in operation
    // Outputs:
    //   Returns true if the survey-in operation was reset successfully
    //   and false upon failure
    virtual bool surveyInReset() { return false; }

    // Start the survey-in operation
    // Outputs:
    //   Return true if successful and false upon failure
    virtual bool surveyInStart() { return false; }

    // Poll routine to update the GNSS state
    virtual void update() {}
};

// Update the constellations following a set command
bool gnssCmdUpdateConstellations(const char *settingName, void *settingData, int settingType);

// Update the message rates following a set command
bool gnssCmdUpdateMessageRates(const char *settingName, void *settingData, int settingType);

// Restore the GNSS to the factory settings
void gnssFactoryReset();

// Determine if the GNSS receiver is present
typedef bool (*GNSS_PRESENT)();

// Create the GNSS class instance
typedef void (*GNSS_NEW_CLASS)();

// List available settings, their type in CSV, and value
typedef bool (*GNSS_COMMAND_LIST)(RTK_Settings_Types type, int settingsIndex, bool inCommands, int qualifier,
                                  char *settingName, char *settingValue);

// Add types to a JSON array
typedef void (*GNSS_COMMAND_TYPE_JSON)(JsonArray &command_types);

// Create string for settings
typedef bool (*GNSS_CREATE_STRING)(RTK_Settings_Types type, int settingsIndex, char *newSettings);

// Return setting value as a string
typedef bool (*GNSS_GET_SETTING_VALUE)(RTK_Settings_Types type, const char *suffix, int settingsIndex, int qualifier,
                                       char *settingValueStr);

// Update a setting value
typedef bool (*GNSS_NEW_SETTING_VALUE)(struct Settings * tempSettings, RTK_Settings_Types type, const char *suffix, int qualifier, double d);

// Write settings to a file
typedef bool (*GNSS_SETTING_TO_FILE)(char * line, size_t lineSize, RTK_Settings_Types type, int settingsIndex);

typedef struct _GNSS_SUPPORT_ROUTINES
{
    const char *name;
    const char *gnssModelIdentifier;
    gnssReceiverType_e _receiver;
    GNSS_PRESENT _present;
    int8_t _presentPriority; // -1 : no priority; 0 : highest priority; 1 : next highest
    GNSS_NEW_CLASS _newClass;
    GNSS_COMMAND_LIST _commandList;
    GNSS_COMMAND_TYPE_JSON _commandTypeJson;
    GNSS_CREATE_STRING _createString;
    GNSS_GET_SETTING_VALUE _getSettingValue;
    GNSS_NEW_SETTING_VALUE _newSettingValue;
    GNSS_SETTING_TO_FILE _settingToFile;
} GNSS_SUPPORT_ROUTINES;

#endif // __GNSS_H__
