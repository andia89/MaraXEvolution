// =================================================================
// --- USER CONFIGURATION & DEFINES ---
// =================================================================
// #define HAS_PRESSURE_GAUGE
// #define HAS_SCALE
// #define HAS_SCREEN

// =================================================================
// --- LIBRARIES ---
// =================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "PID_v1.h"
#include <AsyncMqttClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#ifdef HAS_SCREEN
#include <esp_now.h>
#endif
#include <esp_pm.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#ifdef HAS_SCALE
#include "SimpleKalmanFilter.h"
#endif
#ifdef HAS_PRESSURE_GAUGE
#include "dimmable_light.h"
#endif

// =================================================================
// --- HARDWARE PIN DEFINITIONS ---
// =================================================================
const int BOILER_LEVEL = D1;
const int WATER_DETECTOR = D3;
const int THREE_WAY_SWITCH1 = D9;
const int THREE_WAY_SWITCH2 = D11;
const int TWO_WAY_SWITCH = D10;
const int BREW_SWITCH = A6;
const int ENABLE_LM1830 = D0;
const int BUZZER = A1;
const int LEDMAIN = D6;
const int LEDHEATER = A2;
const int LEDWATER = D5;
const int PUMP_RELAY = A3;
const int COFFEE_RELAY = D7;
const int HEATER_SSR = A7;
const int ZERO_CROSS_PIN = D13; // green
const int PUMP_TRIAC_PIN = A0;  // yellow
const int ADS_SCLK_PIN = D4;    // scale
const int ADS_DOUT_PIN = D12;   // scale

// Analog input channels for ADS1115
const int BOILER_TEMP = 0;
const int HX_TEMP = 1;
const int PRESSURE = 3;

// Pin Arrays for setup loops
const int digitalOutputs[] = {ENABLE_LM1830, LEDMAIN, LEDHEATER, LEDWATER, PUMP_RELAY, COFFEE_RELAY, HEATER_SSR, BUZZER};
const int startupOutputPins[] = {ENABLE_LM1830, LEDHEATER, LEDWATER, PUMP_RELAY, COFFEE_RELAY, HEATER_SSR, BUZZER};
const int digitalInputs[] = {BOILER_LEVEL, WATER_DETECTOR, THREE_WAY_SWITCH1, THREE_WAY_SWITCH2, TWO_WAY_SWITCH, BREW_SWITCH};

// =================================================================
// --- PIN LOOKUP TABLE (FOR TELNET) ---
// =================================================================
struct PinLookup
{
  const char *name;
  int pin;
};

static const PinLookup pinMap[] = {
    {"boiler_level", BOILER_LEVEL},
    {"water_detector", WATER_DETECTOR},
    {"three_way_switch1", THREE_WAY_SWITCH1},
    {"three_way_switch2", THREE_WAY_SWITCH2},
    {"two_way_switch", TWO_WAY_SWITCH},
    {"brew_switch", BREW_SWITCH},
    {"enable_lm1830", ENABLE_LM1830},
    {"buzzer", BUZZER},
    {"ledmain", LEDMAIN},
    {"ledheater", LEDHEATER},
    {"ledwater", LEDWATER},
    {"pump_relay", PUMP_RELAY},
    {"coffee_relay", COFFEE_RELAY},
    {"heater_ssr", HEATER_SSR}};

static const int pinMapSize = sizeof(pinMap) / sizeof(pinMap[0]);

// =================================================================
// --- NETWORK & COMMUNICATION CONFIGURATION ---
// =================================================================

// --- MQTT Broker Configuration ---
char mqtt_server[40] = "";
int mqtt_port = 1883;
char mqtt_user[33] = "";
char mqtt_password[33] = "";
const char *mqtt_client_id = "MaraXV2CustomEspresso";

// --- MQTT Topics ---
// Status Topics
const char *mqtt_topic_state = "espresso/status/state";
const char *mqtt_topic_brew_mode = "espresso/status/brew_mode";
const char *mqtt_topic_steam_boost = "espresso/status/steam_boost";
const char *mqtt_topic_boiler_temp = "espresso/sensor/boiler_temp";
const char *mqtt_topic_hx_temp = "espresso/sensor/hx_temp";
#ifdef HAS_PRESSURE_GAUGE
const char *mqtt_topic_pressure = "espresso/sensor/pressure";
#endif
const char *mqtt_topic_heater = "espresso/sensor/heater";
const char *mqtt_topic_pump = "espresso/sensor/pump";
const char *mqtt_topic_lever = "espresso/sensor/lever";
const char *mqtt_topic_pidoutput = "espresso/sensor/pidoutput";
const char *mqtt_topic_pterm = "espresso/sensor/pterm";
const char *mqtt_topic_iterm = "espresso/sensor/iterm";
const char *mqtt_topic_dterm = "espresso/sensor/dterm";
#ifdef HAS_SCALE
const char *mqtt_topic_weight = "espresso/sensor/weight";
const char *mqtt_topic_flow_rate = "espresso/sensor/flow_rate";
#endif
const char *mqtt_topic_mqtt_server = "espresso/settings/status/mqtt_server";
const char *mqtt_topic_mqtt_port = "espresso/settings/status/mqtt_port";
const char *mqtt_topic_mqtt_user = "espresso/settings/status/mqtt_user";
const char *mqtt_topic_mqtt_pass = "espresso/settings/status/mqtt_pass";
// Settings Status Topics
const char *mqtt_topic_set_temp_brew = "espresso/settings/status/tempsetbrew";
const char *mqtt_topic_set_temp_steam = "espresso/settings/status/tempsetsteam";
const char *mqtt_topic_set_temp_steam_boost = "espresso/settings/status/tempsetsteamboost";
const char *mqtt_topic_set_kp_temperature = "espresso/settings/status/kp_temperature";
const char *mqtt_topic_set_ki_temperature = "espresso/settings/status/ki_temperature";
const char *mqtt_topic_set_kd_temperature = "espresso/settings/status/kd_temperature";
const char *mqtt_topic_set_profiling_mode = "espresso/settings/status/profiling_mode";
const char *mqtt_topic_set_profiling_source = "espresso/settings/status/profiling_source";
const char *mqtt_topic_set_profiling_target = "espresso/settings/status/profiling_target";
const char *mqtt_topic_profile_data = "espresso/settings/status/profile_data";
const char *mqtt_topic_profile_flat = "espresso/settings/status/profiling_flat_value";

const char *mqtt_topic_set_kp_pressure = "espresso/settings/status/kp_pressure";
const char *mqtt_topic_set_ki_pressure = "espresso/settings/status/ki_pressure";
const char *mqtt_topic_set_kd_pressure = "espresso/settings/status/kd_pressure";
const char *mqtt_topic_set_kp_flow = "espresso/settings/status/kp_flow";
const char *mqtt_topic_set_ki_flow = "espresso/settings/status/ki_flow";
const char *mqtt_topic_set_kd_flow = "espresso/settings/status/kd_flow";
#ifdef HAS_SCALE
// Kalman Filter Topics
const char *mqtt_topic_set_flow_kalman_me = "espresso/settings/status/flow_kalman_me";
const char *mqtt_topic_set_flow_kalman_e = "espresso/settings/status/flow_kalman_e";
const char *mqtt_topic_set_flow_kalman_q = "espresso/settings/status/flow_kalman_q";
const char *mqtt_topic_set_weight_kalman_me = "espresso/settings/status/weight_kalman_me";
const char *mqtt_topic_set_weight_kalman_e = "espresso/settings/status/weight_kalman_e";
const char *mqtt_topic_set_weight_kalman_q = "espresso/settings/status/weight_kalman_q";
#endif
// Settings Control Topic
const char *mqtt_topic_set = "espresso/settings/set";

// --- OTA (Over-the-Air Updates) ---
const char *ota_hostname = "esp32-arduino";
const char *ota_password = "1234";

// --- Global Connection State ---
bool isOnline = false;
WiFiManager wm;

// =================================================================
// --- SYSTEM & LIBRARY OBJECTS ---
// =================================================================
WiFiServer telnetServer(23);
WiFiClient telnetClient;
AsyncMqttClient mqttClient;
Preferences preferences;
Adafruit_ADS1115 ads;
#ifdef HAS_PRESSURE_GAUGE
DimmableLight pumpDimmer(PUMP_TRIAC_PIN);
#endif
#ifdef HAS_SCREEN
typedef struct struct_message
{
  char payload[250];
} struct_message;
typedef struct struct_pairing
{
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
  char identifier[10];
} struct_pairing;
const char *espIdentifier = "espresso";
struct_message myData;
struct_pairing pairingData;
uint8_t screenMacAddress[6];
bool isScreenPaired = false;
int myChannel = 0;
esp_now_peer_info_t peerInfo;
#define PAIR_REQUEST 1
#define PAIR_RESPONSE 2

// --- ESP-NOW Buffering ---
const int MAX_ESPNOW_BUFFER = 250;
char espNowMessageBuffer[MAX_ESPNOW_BUFFER] = {0}; // Initialize with nulls
int espNowCurrentLength = 0;
const unsigned long ESP_NOW_SEND_INTERVAL_MS = 250;
static unsigned long lastEspNowSendTime = 0;
#endif
#ifdef HAS_SCALE
// =================================================================
// --- SCALE (ADS1232) CONFIGURATION & GLOBALS ---
// =================================================================
// --- I2C Configuration ---
const int PCF8574_ADDRESS = 0x38;
const int SCALE_CHANNEL = 2;

// --- PCF8574 Pin Mapping ---
const byte PCF_PDWN_BIT = 7;
const byte PCF_SPEED_BIT = 4;
const byte PCF_GAIN0_BIT = 6;
const byte PCF_GAIN1_BIT = 5;
const byte PCF_TEMP_BIT = 0;
const byte PCF_A0_BIT = 2;

// --- Calibration Values ---
long COMBINED_OFFSET = 0;
float COMBINED_SCALE = 1.0;

// --- Global Variables ---
volatile boolean newDataReady = false;
portMUX_TYPE scaleMux = portMUX_INITIALIZER_UNLOCKED;
byte pcfState = 0;
long loadCellValue = 0;
float currentWeight = 0.0;
float calibrationWeight = 0.0;
static float lastRawWeight = 0.0;
static float pendingWeight = 0.0;
static bool isWeightPending = false;
static bool isFirstScaleReading = true;
const float SPIKE_THRESHOLD_G = 25.0;
const int SCALE_BLANKING_PERIOD_MS = 500;

// --- Kalman Filter for Flow Rate ---
float flowKalmanMe = 30.0;
float flowKalmanE = 2.0;
float flowKalmanQ = 0.1;
SimpleKalmanFilter flowKalmanFilter(flowKalmanMe, flowKalmanE, flowKalmanQ);
float weightKalmanMe = 8.0;
float weightKalmanE = 2.0;
float weightKalmanQ = 0.1;
SimpleKalmanFilter weightKalmanFilter(weightKalmanMe, weightKalmanE, weightKalmanQ);

// --- Flow Rate Calculation ---
float flowRate = 0.0f;
float previousWeightForFlowCalc = 0.0f;
unsigned long previousTimeForFlowCalc = 0;
#endif
// --- Profiling State Tracking ---
int currentProfileStepIndex = 0;
float currentStepStartX = 0.0f;
float prevStepTargetY = 0.0f;
unsigned long shotStartTime = 0;

// =================================================================
// --- FLOW PROFILING ---
// =================================================================

#define MAX_PROFILE_STEPS 32

struct ProfileStep
{
  float trigger;  // X-axis value (Time in seconds OR Weight in grams)
  float setpoint; // Y-axis value (Pressure in bar OR Flow in g/s)
};

struct EspressoProfile
{
  char name[65];
  bool isStepped;
  uint8_t numSteps;
  ProfileStep steps[MAX_PROFILE_STEPS];
};

EspressoProfile currentProfile;

int numProfilePoints = 0;

// =================================================================
// --- STATE MACHINE ---
// =================================================================
// --- State Definitions ---
enum MachineState
{
  WATER_EMPTY,
  BOILER_EMPTY,
  HEATING,
  BREWING,
  STANDBY,
  IDLE,
  CLEANING_START,
  CLEANING_PUMPING,
  CLEANING_PAUSE,
  STEAM_BOOST,
  COOLING_FLUSH,
  ERROR,
  DEBUG,
  INIT,
  CALIBRATION_EMPTY,
  CALIBRATION_TEST_WEIGHT
};

// --- State Control Variables ---
MachineState currentState;
MachineState calibrationReturnState = HEATING;
char lastError[128] = "";
unsigned long lastStateTransitionTime = 0;
unsigned long idleEntryTime = 0;
unsigned long flushStartTime = 0;
unsigned long boilerFullTimestamp = 0;
unsigned long steamBoostEntryTime = 0;
long coolingFlushEndTime = 0;
unsigned long programmaticFlushEndTime = 0;

// Timer to track post-shot dripping
unsigned long shotEndTime = 0;
const unsigned long SHOT_POST_DRIP_DURATION_MS = 3000;

// --- Input State Variables ---
bool waterLevelTripped = false;
volatile bool isBoilerEmpty = false;
bool brewLeverLifted = false;
bool threeWaySwitch1High = false;
bool threeWaySwitch2High = false;
bool leverLiftedInStandby = false;
bool isBoilerPinHigh = false;
unsigned long lastBoilerCheckTime = 0;
unsigned long lastLeverChangedTime = 0;
unsigned long lastWaterLevelChangedTime = 0;
const unsigned long debounceDelay = 100;

// --- Override Flags ---
bool ignoreBrewSwitch = false;
bool ignoreTempSwitch = false;
bool firstMqttConnection = true;

// =================================================================
// --- OPERATIONAL PARAMETERS & SETTINGS ---
// =================================================================
// --- Brewing & Steaming Temperatures ---
char brewMode[7];
bool enableSteamBoost = true;
float tempSetBrew = 94.0;
float tempSetSteamBoost = 124.0;
float tempSetSteam = 136.0;
float tempSetSteamHeating = 28;
double calculatedBoilerTemp = 0.0;
const double FF_ONLY_THRESHOLD = 4.0;
const double BOILER_WAY_TOO_HOT = 10.0;

// --- System Timings & Constants ---
const unsigned long STANDBY_TIMEOUT_MS = 15 * 60 * 1000;
const unsigned long COOLING_FLUSH_DURATION_MS = 4000;
const unsigned long PERIODIC_FLUSH_INTERVAL_MS = 5 * 60 * 1000;
const unsigned long BOILER_OVERFILL_DURATION_MS = 200;
const unsigned long STEAM_BOOST_DURATION_MS = 60000;
const unsigned long SENSOR_READ_PAUSE_MS = 200;

// --- Cleaning Cycle ---
int cleaningRepetitionCounter = 0;
unsigned long cleaningPumpStartTime = 0;
int cleaningBeepState = 0;
unsigned long cleaningBeepTimer = 0;
const int TOTAL_CLEANING_REPETITIONS = 10;
MachineState cleaningStateToResume = IDLE;

// --- Profiling Settings ---
char profilingMode[10] = "manual";
char profilingSource[10] = "";
char profilingTarget[10] = "time";
float profilingFlatValue = 100.0;

// --- General Flags & Variables ---
const bool BUZZER_ENABLE = true;
bool pumpRunning = false;
unsigned long lastPumpStateChangeTime = 0;
bool sendRawDebugData = false;

// --- Global Beep Variables ---
unsigned long beepStopTime = 0;
bool isBeeping = false;

// =================================================================
// --- PID & HEATER CONTROL ---
// =================================================================
// --- PID Tuning Parameters ---
double kp_temperature = 0.169;
double ki_temperature = 0.00002216;
double kd_temperature = 0;

const double ASSUMED_AMBIENT_TEMP = 20.0;
const double c1Brew = 0.000363;
const double c2Brew = 5.623e-12;
const double c1Boiler = 0.000041;
const double c2Boiler = 4.984e-12;

// --- PID Control Variables ---
double pidSetpoint;
double pidInput;
double pidOutput;
PID heaterPID(&pidInput, &pidOutput, &pidSetpoint, kp_temperature, ki_temperature, kd_temperature, DIRECT);

// --- Slow PWM & Manual Control ---
bool manualHeaterControl = false;
double manualHeaterPercentage = 0.0;
const unsigned long PWM_WINDOW_SIZE = 5000;
bool heaterOn = false;

// --- Temperature Stability Check ---
const float TEMP_STABILITY_TOLERANCE = 0.2;
const unsigned long TEMP_STABILITY_DURATION_MS = 120000;
unsigned long stableTempStartTime = 0;

// Control Variables
double pumpSetpoint;
double pumpInput;
double pumpOutput;

#ifdef HAS_PRESSURE_GAUGE
// --- PUMP PID (PRESSURE) ---
// Tuning parameters
double kp_pressure = 0.05;
double ki_pressure = 22;
double kd_pressure = 0.0;

// Initialize PID (DIRECT mode: more output = more pressure)
PID pressurePID(&pumpInput, &pumpOutput, &pumpSetpoint, kp_pressure, ki_pressure, kd_pressure, DIRECT);
#endif

#ifdef HAS_SCALE
double kp_flow = 1.0;
double ki_flow = 0.5;
double kd_flow = 0.0;
PID flowPID(&pumpInput, &pumpOutput, &pumpSetpoint, kp_flow, ki_flow, kd_flow, DIRECT);
#endif

// =================================================================
// --- SENSOR CONFIGURATION & DATA ---
// =================================================================
// --- NTC Thermistor LUT (Look-Up Table) Generation ---
// Configuration for the OLD system
const int OLD_ADC_MAX = 1023;
const float VCC_OLD = 5.0;
const float R_SERIES_OHMS_OLD = 7150.0;

const int NEW_ADC_MAX = 32767;
const float VCC_NEW = 3;
const float R_SERIES_OHMS_NEW = 10000.0;
const float V_REF_NEW = 4.096;

const int LEGACY_TEMP_LOOKUP_TABLE[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 15, 18, 20, 25, 30, 35, 40, 45, 48, 50, 55, 58, 60, 65, 70, 75, 78, 80, 85, 88, 90, 95, 98, 100, 105, 108, 110, 115, 118, 120, 123, 125, 128, 130, 135, 138, 140, 143, 145, 148, 150, 155, 158, 160, 163, 165, 168, 170, 173, 175, 178, 180, 183, 185, 188, 190, 193, 195, 198, 200, 203, 205, 207, 209, 210, 213, 215, 218, 220, 223, 224, 225, 228, 230, 233, 235, 237, 239, 240, 243, 245, 247, 249, 250, 253, 255, 257, 259, 260, 263, 265, 267, 269, 270, 273, 275, 277, 279, 280, 283, 285, 287, 289, 290, 292, 294, 295, 297, 299, 300, 303, 305, 307, 309, 310, 312, 313, 315, 317, 319, 320, 322, 324, 325, 327, 329, 330, 332, 334, 335, 337, 339, 340, 342, 344, 345, 347, 349, 350, 352, 354, 355, 356, 358, 359, 360, 362, 364, 365, 367, 369, 370, 372, 374, 375, 376, 378, 379, 380, 382, 384, 385, 386, 388, 389, 390, 392, 394, 395, 396, 398, 399, 400, 402, 404, 405, 406, 408, 409, 410, 412, 414, 415, 416, 418, 419, 420, 422, 424, 425, 426, 428, 429, 430, 432, 434, 435, 436, 438, 439, 440, 442, 443, 444, 445, 446, 447, 449, 450, 452, 454, 455, 456, 458, 459, 460, 462, 463, 464, 465, 466, 468, 469, 470, 472, 474, 475, 476, 478, 479, 480, 482, 484, 485, 486, 487, 488, 489, 490, 492, 493, 494, 495, 496, 498, 499, 500, 502, 503, 504, 505, 506, 508, 509, 510, 512, 513, 514, 515, 516, 518, 519, 520, 521, 523, 524, 525, 526, 528, 529, 530, 532, 533, 534, 535, 536, 537, 538, 539, 540, 542, 543, 544, 545, 547, 548, 549, 550, 551, 553, 554, 555, 556, 557, 559, 560, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 573, 574, 575, 577, 578, 579, 580, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 604, 605, 607, 608, 609, 610, 611, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 630, 631, 632, 634, 635, 637, 638, 639, 640, 641, 642, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 657, 658, 659, 660, 662, 663, 664, 665, 666, 667, 669, 670, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 690, 691, 693, 694, 695, 696, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 710, 712, 713, 714, 715, 716, 717, 718, 719, 720, 722, 723, 724, 725, 726, 727, 728, 730, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 755, 757, 758, 759, 760, 762, 763, 764, 765, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 780, 781, 782, 783, 785, 786, 787, 789, 790, 791, 792, 793, 795, 796, 797, 799, 800, 801, 802, 804, 805, 806, 807, 808, 809, 810, 812, 813, 814, 815, 816, 817, 818, 820, 821, 822, 824, 825, 826, 827, 829, 830, 832, 833, 834, 835, 836, 838, 839, 840, 841, 843, 844, 845, 847, 848, 849, 850, 852, 853, 854, 855, 857, 858, 859, 860, 861, 862, 864, 865, 866, 868, 869, 870, 871, 873, 874, 875, 876, 877, 878, 880, 882, 884, 885, 886, 887, 889, 890, 891, 893, 894, 895, 897, 898, 899, 900, 901, 903, 905, 906, 908, 909, 910, 912, 914, 915, 916, 917, 918, 919, 920, 922, 923, 924, 925, 927, 928, 930, 932, 933, 935, 936, 937, 939, 940, 941, 943, 944, 945, 947, 948, 949, 950, 952, 953, 955, 956, 957, 959, 960, 962, 964, 965, 966, 968, 970, 972, 974, 975, 977, 978, 979, 980, 982, 983, 985, 986, 987, 989, 990, 991, 993, 995, 997, 998, 1000, 1002, 1003, 1005, 1006, 1008, 1009, 1010, 1013, 1015, 1017, 1019, 1020, 1022, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1033, 1035, 1036, 1038, 1040, 1042, 1043, 1044, 1045, 1047, 1048, 1050, 1052, 1054, 1055, 1056, 1058, 1060, 1063, 1065, 1067, 1069, 1070, 1071, 1073, 1075, 1076, 1078, 1080, 1082, 1084, 1085, 1086, 1088, 1090, 1091, 1093, 1095, 1097, 1099, 1100, 1103, 1105, 1107, 1109, 1110, 1112, 1113, 1115, 1116, 1118, 1120, 1123, 1125, 1126, 1128, 1130, 1133, 1135, 1137, 1138, 1140, 1143, 1145, 1147, 1149, 1150, 1151, 1153, 1155, 1157, 1160, 1163, 1165, 1167, 1169, 1170, 1172, 1174, 1175, 1177, 1180, 1182, 1185, 1188, 1190, 1192, 1194, 1195, 1197, 1200, 1203, 1205, 1207, 1209, 1210, 1213, 1215, 1217, 1220, 1223, 1225, 1228, 1230, 1233, 1235, 1237, 1240, 1243, 1245, 1248, 1250, 1252, 1255, 1257, 1259, 1260, 1263, 1265, 1267, 1270, 1275, 1278, 1280, 1283, 1285, 1288, 1290, 1293, 1295, 1297, 1300, 1303, 1305, 1308, 1310, 1313, 1315, 1320, 1322, 1325, 1328, 1330, 1335, 1337, 1340, 1343, 1345, 1348, 1350, 1353, 1355, 1360, 1365, 1368, 1370, 1373, 1375, 1378, 1380, 1385, 1387, 1390, 1393, 1395, 1400, 1402, 1405, 1410, 1413, 1417, 1420, 1425, 1430, 1433, 1437, 1440, 1443, 1447, 1450, 1455, 1460, 1463, 1467, 1470, 1475, 1480, 1483, 1487, 1490, 1495, 1500, 1503, 1507, 1510, 1515, 1520, 1523, 1527, 1530, 1535, 1540, 1545, 1550};
const int LEGACY_TEMP_LOOKUP_TABLE_SIZE = sizeof(LEGACY_TEMP_LOOKUP_TABLE) / sizeof(int);

float remappedAdcValues[LEGACY_TEMP_LOOKUP_TABLE_SIZE];
float remappedTemperatureValues[LEGACY_TEMP_LOOKUP_TABLE_SIZE];

// --- Pressure Sensor Configuration ---
#define PRESSURE_VOLTAGE_MIN 0.392
#define PRESSURE_VOLTAGE_MAX 3.683
#define PRESSURE_BAR_MAX 16.0

// --- General Sensor Constants & Safety Limits ---
const int16_t ADC_RAILED_THRESHOLD = 500;
const int16_t ADC_MAX_VALUE = 32767;
const float MAX_SAFE_TEMPERATURE = 150.0;
const float MAX_ALLOWED_BOILER_TEMP = 133.0;
const int16_t BOILER_CHECK_PERIOD = 3000;
const int16_t BOILER_CHECK_DURATION = 500;

// --- Global Sensor Reading Variables ---
float boilerTemp = 0.0;
float hxTemp = 0.0;
float pressure = 0.0;
int16_t boilerTempADC = 0;
int16_t hxTempADC = 0;

// --- Sensor Smoothing (Moving Average) ---
const int SENSOR_SMOOTHING_SAMPLES = 10;

// =================================================================
// --- FORWARD DECLARATIONS ---f
// =================================================================
// --- Networking & Communication ---
template <typename T>
void printToAll(const T &message);
void printToAll(double value, int precision);
template <typename T>
void printlnToAll(const T &message);
void printlnToAll(double value, int precision);
void publishData(const char *topic, const char *payload, bool retained, bool espNowSendNow = true, bool sendToMqtt = true, bool sendToESP = true);
void handleTelnet();
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void handleIncomingSetting(char *message);
#ifdef HAS_SCREEN
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void sendEspNowBuffer();
#endif
// --- Command Processing & Utilities ---
int getPinByName(const char *pinName);
void processCommand(char *command);
void printStatus();

// --- State Machine ---
void transitionToState(MachineState newState);
const char *stateToString(MachineState state);
void handleStateEntry(MachineState newState);
void allStop();
#ifdef HAS_SCALE
void startCalibration();
void handleCalibrationStep(float weight = 0.0);
#endif
void triggerBeep(int duration);

// --- Hardware Control ---
void setHeater(bool on);
void setBoilerFillValve(bool on);
void setPump(bool on);
void setPumpPower(int percentage);
void enableWaterLevelSensor(bool on);

// --- Interrupts & Input Handling ---
void pollDigitalInputs();
int checkLM1830();
void periodicBoilerLevelCheck();

// --- Sensor Reading & Processing ---
void updateSensorReadings();
bool readPin(const int pin);
bool detectBoilerLevel();
int16_t readADSADC(const int pin);
float convertADCToTemp(int16_t adc);
float linearInterpolation(float xValues[], float yValues[], int numValues, float pointX, bool trim);
double feedForwardHeater(double c1, double c2, double steadyStateTemp, double ambientTemp);
double getTempFromPower(double targetPower, double c1, double c2, double ambientTemp);
bool checkCriticalSensorFailure();
void updateCalculatedBoilerTemp();

// --- Scale (ADS1232) Functions ---
#ifdef HAS_SCALE
void IRAM_ATTR dataReadyISR();
void updatePcf();
long readADCScale();
void setGain(uint8_t gain);
void setSpeed(bool high_speed);
void powerDown(bool power_down);
void selectChannelScale(uint8_t channel);
void handleScale();
long getStableCombinedReadingADS1232(int times = 16);
void tareScale();
void calculateFlowRate();
#endif

// --- User Feedback (LED & Buzzer) ---
void errorBuzzer();
void waterEmptyBuzzer();
void ledError();
void ledWaterEmpty();
void ledHeating();
void ledIdle();
void ledBrew();
void ledStandby();

// --- Machine Operation & Logic ---
void runHeaterPID();
bool isStable();
void runPumpProfile();
float getTargetAt(float currentX);
void updateBrewMode();
void updateTempSwitch();
bool standbyTimoutReached();
bool isProgrammaticFlushActive();
void startProgrammaticFlush(unsigned long duration);

// --- Settings & Configuration ---
void saveSettings();
void loadSettings();
void publishSettings();
void publishSingleSetting(const char *key, bool forceFlush = true);
void publishProfile();
void publishState();
void generateSparseMap();
void setup();
void loop();
void startNetworkServices();

// =================================================================
// --- FUNCTION DEFINITIONS ---
// =================================================================

// ----------------------------------------------------------------
// --- Networking & Communication Functions ---
// ----------------------------------------------------------------
template <typename T>
void printToAll(const T &message)
{
  if (telnetClient && telnetClient.connected())
  {
    telnetClient.print(message);
  }
}

void printToAll(double value, int precision)
{
  if (telnetClient && telnetClient.connected())
  {
    telnetClient.print(value, precision);
  }
}

template <typename T>
void printlnToAll(const T &message)
{
  if (telnetClient && telnetClient.connected())
  {
    telnetClient.println(message);
  }
}

void printlnToAll(double value, int precision)
{
  if (telnetClient && telnetClient.connected())
  {
    telnetClient.println(value, precision);
  }
}

void publishData(const char *topic, const char *payload, bool retained, bool espNowSendNow, bool sendToMqtt, bool sendToESP)
{
  if (sendToMqtt && isOnline)
  {
    if (mqttClient.connected())
    {
      mqttClient.publish(topic, 0, retained, payload);
    }
  }
#ifdef HAS_SCREEN
  if (!sendToESP)
  {
    return;
  }
  if (!isScreenPaired)
  {
    return;
  }

  const char *keyStart = strrchr(topic, '/');

  if (keyStart == NULL)
  {
    keyStart = topic;
  }
  else
  {
    keyStart = keyStart + 1;
  }
  int keyLen = strlen(keyStart);
  int valLen = strlen(payload);
  int neededSpace = keyLen + 1 + valLen;

  if (espNowCurrentLength > 0)
  {
    neededSpace++;
  }

  if (espNowCurrentLength + neededSpace >= MAX_ESPNOW_BUFFER - 1)
  {
    sendEspNowBuffer();
  }

  if (espNowCurrentLength > 0)
  {
    strcat(espNowMessageBuffer, "|");
    espNowCurrentLength++;
  }
  strcat(espNowMessageBuffer, keyStart);
  strcat(espNowMessageBuffer, "=");
  strcat(espNowMessageBuffer, payload);
  espNowCurrentLength = strlen(espNowMessageBuffer);
  if (espNowSendNow)
  {
    sendEspNowBuffer();
  }
#endif
}

void handleTelnet()
{
  if (telnetServer.hasClient())
  {
    if (telnetClient && telnetClient.connected())
    {
      telnetClient.println("\nAnother client is connecting, disconnecting you.");
      telnetClient.stop();
    }
    telnetClient = telnetServer.available();
    if (telnetClient)
    {
      telnetClient.println("\nWelcome to the ESP32 Telnet Serial Monitor!");
      telnetClient.println("--------------------------------------------");

      // --- TELNET NEGOTIATION ---
      telnetClient.write(255); // IAC (Interpret As Command)
      telnetClient.write(251); // WILL
      telnetClient.write(1);   // ECHO
      telnetClient.write(255); // IAC
      telnetClient.write(251); // WILL
      telnetClient.write(3);   // SUPPRESS GO AHEAD

      telnetClient.print("> ");
    }
  }

  static char telnetBuffer[128];
  static int telnetIndex = 0;
  if (telnetClient && telnetClient.connected() && telnetClient.available())
  {
    const char IAC = 255;
    char c = telnetClient.read();

    // --- TELNET COMMAND HANDLING ---
    if (c == IAC)
    {
      if (telnetClient.available() >= 2)
      {
        telnetClient.read();
        telnetClient.read();
      }
    }

    // --- BACKSPACE HANDLING ---
    else if (c == 8 || c == 127)
    {
      if (telnetIndex > 0)
      {
        telnetIndex--;
        telnetBuffer[telnetIndex] = '\0'; // Null-terminate just in case

        // Visual backspace for the user's terminal
        telnetClient.write('\b');
        telnetClient.write(' ');
        telnetClient.write('\b');
      }
    }

    // --- COMMAND EXECUTION on NEWLINE ---
    else if (c == '\r') // Only trigger on Carriage Return
    {
      telnetBuffer[telnetIndex] = '\0'; // Null-terminate the char array

      // Move cursor to next line for the user
      telnetClient.println();
      // OR use your helper: printlnToAll("");

      if (telnetIndex > 0)
      {
        processCommand(telnetBuffer);
      }

      // Reset prompt and buffer
      telnetClient.print("> ");
      telnetIndex = 0;
      telnetBuffer[0] = '\0';
    }

    // --- IGNORE NEWLINE (The second half of Enter) ---
    else if (c == '\n')
    {
      // Do nothing. This prevents the double prompt.
    }

    else if (isPrintable(c))
    {
      if (telnetIndex < sizeof(telnetBuffer) - 1)
      {
        telnetBuffer[telnetIndex] = c;
        telnetIndex++;
        telnetBuffer[telnetIndex] = '\0';

        telnetClient.write(c);
      }
    }
  }
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  if (len > 2048)
  {
    printlnToAll("Error: MQTT message too large for stack buffer.");
    return;
  }
  char message[len + 1];
  memcpy(message, payload, len);
  message[len] = '\0';
  char logBuf[128];
  snprintf(logBuf, sizeof(logBuf), "MQTT received on %s: %s", topic, message);
  printlnToAll(logBuf);
  handleIncomingSetting(message);
}

void onMqttConnect(bool sessionPresent)
{
  printlnToAll("MQTT connected!");

  mqttClient.subscribe(mqtt_topic_set, 0); // 0 = QoS 0
  publishSettings();
  if (firstMqttConnection)
  {
    printlnToAll("First connection: Performing initial state sync.");
    publishState();
    firstMqttConnection = false;
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  printToAll("Disconnected from MQTT. Reason: ");
  printlnToAll((int)reason);
}
#ifdef HAS_SCREEN
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  if (len == sizeof(struct_pairing))
  {
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    if (pairingData.id == PAIR_REQUEST && strcmp(pairingData.identifier, espIdentifier) == 0)
    {
      if (isScreenPaired && memcmp(mac_addr, screenMacAddress, 6) != 0)
      {
        printlnToAll("New pairing request received. Dropping old screen.");
        esp_err_t del_status = esp_now_del_peer(screenMacAddress);
        if (del_status != ESP_OK)
        {
          printlnToAll("Warning: Failed to delete old peer. Continuing anyway...");
        }
      }
      else if (isScreenPaired)
      {
        printlnToAll("Pairing request from already-paired screen. Re-syncing.");
      }
      else
      {
        printlnToAll("Pairing request received from a new screen.");
      }
      memcpy(screenMacAddress, mac_addr, 6);
      int screenChannel = pairingData.channel;

      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, screenMacAddress, 6);
      peerInfo.channel = 0;
      peerInfo.ifidx = WIFI_IF_STA;
      peerInfo.encrypt = false;
      if (esp_now_add_peer(&peerInfo) != ESP_OK)
      {
        if (esp_now_mod_peer(&peerInfo) != ESP_OK)
        {
          printlnToAll("Failed to add or modify new screen as peer.");
          return;
        }
        printlnToAll("Screen peer modified successfully.");
      }
      else
      {
        printlnToAll("Screen peer added successfully.");
      }

      pairingData.id = PAIR_RESPONSE;
      pairingData.channel = 0;
      WiFi.macAddress(pairingData.macAddr);
      strcpy(pairingData.identifier, espIdentifier);
      esp_now_send(screenMacAddress, (uint8_t *)&pairingData, sizeof(pairingData));

      isScreenPaired = true;
      printlnToAll("Syncing all settings and state to new screen.");
      publishSettings();
      publishState();
    }
  }

  else if (len == sizeof(struct_message))
  {

    if (!isScreenPaired || memcmp(mac_addr, screenMacAddress, 6) != 0)
    {
      printlnToAll("Data received from unknown/unpaired MAC. Ignoring.");
      return;
    }

    memcpy(&myData, incomingData, sizeof(myData));
    myData.payload[sizeof(myData.payload) - 1] = '\0';
    char *token = strtok(myData.payload, "|");

    while (token != NULL)
    {
      handleIncomingSetting(token);
      token = strtok(NULL, "|");
    }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status != ESP_NOW_SEND_SUCCESS)
  {
    printlnToAll("ESP-NOW message failed to send.");
  }
}

void sendEspNowBuffer()
{
  if (espNowCurrentLength == 0)
    return;

  if (!isScreenPaired)
  {
    espNowCurrentLength = 0;
    espNowMessageBuffer[0] = '\0';
    return;
  }

  struct_message espnow_message;
  strncpy(espnow_message.payload, espNowMessageBuffer, sizeof(espnow_message.payload) - 1);
  espnow_message.payload[sizeof(espnow_message.payload) - 1] = '\0';

  esp_err_t result = ESP_FAIL;
  int attempts = 0;
  const int MAX_RETRIES = 3;

  while (result != ESP_OK && attempts <= MAX_RETRIES)
  {
    if (attempts > 0)
    {
      delay(10);
    }
    result = esp_now_send(screenMacAddress, (uint8_t *)&espnow_message, sizeof(espnow_message));
    attempts++;
  }

  if (result != ESP_OK)
  {
    printToAll("ERROR: ESP-NOW failed code: ");
    printlnToAll(result);
  }
  espNowCurrentLength = 0;
  espNowMessageBuffer[0] = '\0';
}
#endif
void handleIncomingSetting(char *message)
{
  char *separator = strchr(message, '=');
  if (separator == NULL)
  {
    printToAll("Invalid format. Expected key=value.");
    return;
  }
  *separator = '\0';
  char *key = message;
  char *value = separator + 1;

  bool settingsChanged = false;
  bool connectionSettingsChanged = false;

  if (strcasecmp(key, "mqtt_server") == 0)
  {
    strlcpy(mqtt_server, value, sizeof(mqtt_server));
    connectionSettingsChanged = true;
  }
  else if (strcasecmp(key, "mqtt_port") == 0)
  {
    mqtt_port = atoi(value);
    connectionSettingsChanged = true;
  }
  else if (strcasecmp(key, "mqtt_user") == 0)
  {
    strlcpy(mqtt_user, value, sizeof(mqtt_user));
    connectionSettingsChanged = true;
  }
  else if (strcasecmp(key, "mqtt_password") == 0)
  {
    strlcpy(mqtt_password, value, sizeof(mqtt_password));
    connectionSettingsChanged = true;
  }

  else if (strcasecmp(key, "tempsetbrew") == 0)
  {
    if (strcasecmp(value, "auto") == 0)
    {
      printlnToAll("Brew Temp returned to 3-Way Switch control.");
      settingsChanged = true;
      ignoreTempSwitch = false;
    }
    else
    {
      float val = atof(value);
      if (val > 0)
      {
        float oldTemp = tempSetBrew;
        tempSetBrew = val;
        updateCalculatedBoilerTemp();
        ignoreTempSwitch = true;
        settingsChanged = true;

        if (currentState == IDLE && abs(tempSetBrew - oldTemp) > 0.1)
        {
          transitionToState(HEATING);
        }
      }
    }
  }
  else if (strcasecmp(key, "tempsetsteam") == 0)
  {
    tempSetSteam = atof(value);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "tempsetsteamboost") == 0)
  {
    tempSetSteamBoost = atof(value);
    settingsChanged = true;
  }

  // --- PID ---
  else if (strcasecmp(key, "kp_temperature") == 0)
  {
    kp_temperature = atof(value);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "ki_temperature") == 0)
  {
    ki_temperature = atof(value);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "kd_temperature") == 0)
  {
    kd_temperature = atof(value);
    settingsChanged = true;
  }
#ifdef HAS_PRESSURE_GAUGE
  else if (strcasecmp(key, "kp_pressure") == 0)
  {
    kp_pressure = atof(value);
    pressurePID.SetTunings(kp_pressure, ki_pressure, kd_pressure);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "ki_pressure") == 0)
  {
    ki_pressure = atof(value);
    pressurePID.SetTunings(kp_pressure, ki_pressure, kd_pressure);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "kd_pressure") == 0)
  {
    kd_pressure = atof(value);
    pressurePID.SetTunings(kp_pressure, ki_pressure, kd_pressure);
    settingsChanged = true;
  }
#endif
#ifdef HAS_SCALE
  else if (strcasecmp(key, "kp_flow") == 0)
  {
    kp_flow = atof(value);
    flowPID.SetTunings(kp_flow, ki_flow, kd_flow);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "ki_flow") == 0)
  {
    ki_flow = atof(value);
    flowPID.SetTunings(kp_flow, ki_flow, kd_flow);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "kd_flow") == 0)
  {
    kd_flow = atof(value);
    flowPID.SetTunings(kp_flow, ki_flow, kd_flow);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "flow_kalman_me") == 0)
  {
    flowKalmanMe = atof(value);
    flowKalmanFilter.setMeasurementError(flowKalmanMe);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "flow_kalman_e") == 0)
  {
    flowKalmanE = atof(value);
    flowKalmanFilter.setEstimateError(flowKalmanE);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "flow_kalman_q") == 0)
  {
    flowKalmanQ = atof(value);
    flowKalmanFilter.setProcessNoise(flowKalmanQ);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "weight_kalman_me") == 0)
  {
    weightKalmanMe = atof(value);
    weightKalmanFilter.setMeasurementError(weightKalmanMe);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "weight_kalman_e") == 0)
  {
    weightKalmanE = atof(value);
    weightKalmanFilter.setEstimateError(weightKalmanE);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "weight_kalman_q") == 0)
  {
    weightKalmanQ = atof(value);
    weightKalmanFilter.setProcessNoise(weightKalmanQ);
    settingsChanged = true;
  }

  else if (strcasecmp(key, "tare_scale") == 0)
  {
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0)
      tareScale();
  }
  else if (strcasecmp(key, "calibratescale") == 0)
  {
    startCalibration();
  }
  else if (strcasecmp(key, "calibration_step") == 0)
  {
    handleCalibrationStep(atof(value));
  }
#endif
  else if (strcasecmp(key, "profiling_mode") == 0)
  {
    strlcpy(profilingMode, value, sizeof(profilingMode));
    settingsChanged = true;
  }
  else if (strcasecmp(key, "profiling_source") == 0)
  {
    strlcpy(profilingSource, value, sizeof(profilingSource));
    settingsChanged = true;
  }
  else if (strcasecmp(key, "profiling_target") == 0)
  {
    strlcpy(profilingTarget, value, sizeof(profilingTarget));
    settingsChanged = true;
  }
  else if (strcasecmp(key, "profiling_flat_value") == 0)
  {
    profilingFlatValue = atof(value);
    settingsChanged = true;
  }
  else if (strcasecmp(key, "profile_data") == 0)
  {
    printlnToAll("Parsing new profile JSON...");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, value);
    if (!error)
    {
      strlcpy(currentProfile.name, doc["n"] | "Imported", sizeof(currentProfile.name));
      currentProfile.isStepped = (doc["m"] == 1);
      JsonArray steps = doc["s"];
      currentProfile.numSteps = 0;
      for (JsonVariant step : steps)
      {
        if (currentProfile.numSteps >= MAX_PROFILE_STEPS)
          break;
        currentProfile.steps[currentProfile.numSteps].setpoint = step[0].as<float>();
        currentProfile.steps[currentProfile.numSteps].trigger = step[1].as<float>();
        currentProfile.numSteps++;
      }
      settingsChanged = true;
    }
    else
    {
      printlnToAll("JSON Error");
    }
  }

  else if (strcasecmp(key, "start_cleaning") == 0)
  {
    if ((strcmp(value, "true") == 0 || strcmp(value, "1") == 0) &&
        (currentState == IDLE || (currentState == HEATING && hxTemp >= 80.0)))
    {
      cleaningRepetitionCounter = 0;
      cleaningStateToResume = IDLE;
      transitionToState(CLEANING_START);
    }
  }
  else if (strcasecmp(key, "brewmode") == 0)
  {

    if (strcasecmp(value, "coffee") == 0)
    {
      ignoreBrewSwitch = true;
      strcpy(brewMode, "COFFEE");
      settingsChanged = true;
    }
    else if (strcasecmp(value, "steam") == 0)
    {
      ignoreBrewSwitch = true;
      strcpy(brewMode, "STEAM");
      settingsChanged = true;
    }
    else if (strcasecmp(value, "auto") == 0)
    {
      ignoreBrewSwitch = false;
      printlnToAll("Brew Mode returned to Physical Switch control.");
    }
  }
  else if (strcasecmp(key, "steamboost") == 0 || strcasecmp(key, "enablesteamboost") == 0)
  {
    ignoreBrewSwitch = true;
    if (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0)
    {
      enableSteamBoost = true;
      settingsChanged = true;
    }
    else if (strcasecmp(value, "false") == 0 || strcmp(value, "0") == 0)
    {
      enableSteamBoost = false;
      settingsChanged = true;
    }
  }
  else if (strcasecmp(key, "request") == 0)
  {
    if (strcasecmp(value, "true") == 0 || strlen(value) == 0)
      publishSettings();
    else
      publishSingleSetting(value, true);
  }
  else
  {
    printToAll("Unknown setting key: ");
    printlnToAll(key);
  }

  if (settingsChanged || connectionSettingsChanged)
  {
    saveSettings();
  }
  if (settingsChanged)
  {
    publishSingleSetting(key, true);
    printToAll("Setting updated: ");
    printToAll(key);
    printToAll("=");
    printlnToAll(value);
  }
  if (connectionSettingsChanged)
  {
    mqttClient.setServer(mqtt_server, mqtt_port);
    if (strcmp(mqtt_user, "") != 0)
    {
      mqttClient.setCredentials(mqtt_user, mqtt_password);
    }
    else
    {
      mqttClient.setCredentials(nullptr, nullptr);
    }
    mqttClient.connect();
  }
}

// ----------------------------------------------------------------
// --- Command Processing & Utilities ---
// ----------------------------------------------------------------
/**
 * @brief Gets a pin number from a string name using a fast lookup table.
 * * This function is heap-safe, using no String allocations.
 * @param pinName A pointer to a C-string (null-terminated)
 * @return The pin number, or -1 if not found.
 */
int getPinByName(const char *pinName)
{
  for (int i = 0; i < pinMapSize; i++)
  {
    if (strcmp(pinName, pinMap[i].name) == 0)
    {
      return pinMap[i].pin;
    }
  }

  int pinNum = atoi(pinName);
  if (pinNum != 0 || strcmp(pinName, "0") == 0)
  {
    return pinNum;
  }

  return -1;
}

void processCommand(char *command)
{
  char *cmd = strtok(command, " ");
  char *args = strtok(NULL, "");

  if (cmd == NULL)
    return;

  if (strcasecmp(cmd, "help") == 0)
  {
    printlnToAll("--- GENERAL COMMANDS ---");
    printlnToAll("  help              - Shows this message.");
    printlnToAll("  status            - Prints system status.");
    printlnToAll("  exit              - Closes the Telnet connection.");
    printlnToAll("  reboot            - Restarts the device.");
    printlnToAll("  macaddress        - Prints WiFi MAC.");
    printlnToAll("  lasterror         - Shows last critical error.");
    printlnToAll("  debug             - Toggle DEBUG mode.");

    printlnToAll("");
    printlnToAll("--- CONTROL & SETTINGS ---");
    printlnToAll("  set <key>=<val>   - Update settings.");
    printlnToAll("  flush <ms>        - Run pump (Idle/Heating only).");
#ifdef HAS_PRESSURE_GAUGE
    printlnToAll("  pumppid           - View Pump PID constants.");
#endif
#ifdef HAS_SCREEN
    printlnToAll("  espnow <msg>      - Send test message.");
#endif

#ifdef HAS_SCALE
    printlnToAll("");
    printlnToAll("--- SCALE COMMANDS ---");
    printlnToAll("  tare_scale        - Tare the scale.");
    printlnToAll("  calibratescale    - Start calibration wizard.");
    printlnToAll("  calibratenext <g> - Next step (tare or weigh).");
    printlnToAll("  updatepcf         - Force update the pin state of the PCF8575");
#endif

    if (currentState == DEBUG)
    {
      printlnToAll("");
      printlnToAll("--- DEBUG HARDWARE COMMANDS ---");
      printlnToAll("  heater on|off          - Manually toggle Heater SSR.");
      printlnToAll("  pump on|off            - Manually toggle Pump Relay.");
      printlnToAll("  fillvalve on|off       - Manually toggle Fill Valve.");
      printlnToAll("  buzzer on|off          - Manually toggle Buzzer.");
      printlnToAll("  led[main|heater|water] on|off - Toggle specific LEDs.");
      printlnToAll("  dimmer <0-100>         - Set pump dimmer brightness %.");
      printlnToAll("  pidoutput <0-100|auto> - Set manual heater duty cycle %.");
      printlnToAll("  checkic                - Test LM1830 water sensor logic.");
      printlnToAll("  writepin <name> <hi|lo> - Write HIGH/LOW to any output pin.");
      printlnToAll("  readpin <name>         - Read state of any input pin.");

      printlnToAll("");
      printlnToAll("--- DEBUG SENSORS ---");
      printlnToAll("  readweight             - Read filtered, raw, and ADC weight values.");
      printlnToAll("  readadc <0-3>          - Read raw value from ADS1115 channel.");
      printlnToAll("  readhxtemp             - Read current HX temperature.");
      printlnToAll("  readboilertemp         - Read current Boiler temperature.");
      printlnToAll("  computedboiler         - Show calculated steady-state boiler target.");
      printlnToAll("  restartreason          - Show reason for last ESP reset.");
#ifdef HAS_PRESSURE_GAUGE
      printlnToAll("  readpress              - Read current pressure (bar).");
#endif
#ifdef HAS_SCALE
      printlnToAll("  rawdata [on|off]       - Stream raw/filtered scale data via ESP-NOW.");

#endif
    }
  }
  else if (strcasecmp(cmd, "reboot") == 0)
  {
    printlnToAll("Rebooting device...");
    delay(100);
    ESP.restart();
  }
  else if (strcasecmp(cmd, "exit") == 0)
  {
    printlnToAll("Goodbye!");
    telnetClient.stop();
  }
  else if (strcasecmp(cmd, "lasterror") == 0)
  {
    if (lastError[0] != '\0')
    {
      printToAll("Last recorded error: ");
      printlnToAll(lastError);
    }
    else
    {
      printlnToAll("No critical error has been recorded.");
    }
  }
  else if (strcasecmp(cmd, "status") == 0)
  {
    printStatus();
  }
  else if (strcasecmp(cmd, "macaddress") == 0)
  {
    printlnToAll("MAC Address: ");
    printlnToAll(WiFi.macAddress());
  }
  else if (strcasecmp(cmd, "espnow") == 0)
  {
#ifdef HAS_SCREEN
    if (args != NULL)
    {
      struct_message tempMessage;
      strlcpy(tempMessage.payload, args, sizeof(tempMessage.payload));
      esp_err_t result = esp_now_send(NULL, (uint8_t *)&tempMessage, sizeof(tempMessage));
      if (result != ESP_OK)
      {
        printToAll("Error queueing ESP-NOW message. Code: ");
        printlnToAll((int)result);
      }
      else
      {
        printlnToAll("ESP-NOW message queued.");
      }
    }
    else
    {
      printlnToAll("Usage: espnow <message>");
    }
#else
    printlnToAll("ESP-NOW not enabled.");
#endif
  }
  else if (strcasecmp(cmd, "debug") == 0)
  {
    if (currentState == DEBUG)
    {
      printlnToAll("Exiting DEBUG mode. Returning to HEATING state.");
      transitionToState(HEATING);
    }
    else
    {
      transitionToState(DEBUG);
    }
  }
  else if (strcasecmp(cmd, "flush") == 0)
  {
    if (args != NULL)
    {
      unsigned long duration = strtoul(args, NULL, 10);
      startProgrammaticFlush(duration);
    }
    else
    {
      printlnToAll("Usage: flush <milliseconds>");
    }
  }
  else if (strcasecmp(cmd, "set") == 0)
  {
    if (args != NULL)
      handleIncomingSetting(args);
    else
      printlnToAll("Usage: set <key>=<value>");
  }
#ifdef HAS_PRESSURE_GAUGE
  else if (strcasecmp(cmd, "pumppid") == 0)
  {
    printToAll("Pump PID values:");
    printToAll(kp_pressure, 2);
    printToAll(",");
    printToAll(ki_pressure, 2);
    printToAll(",");
    printToAll(kd_pressure, 2);
    printlnToAll(",");
  }
#endif
#ifdef HAS_SCALE
  else if (strcasecmp(cmd, "calibratescale") == 0)
  {
    if (args == NULL)
    {
      printlnToAll("Error: Missing argument. Usage: calibratescale <weight_in_grams>");
      printlnToAll("Example: calibratescale 100.5");
      return;
    }
    startCalibration();
  }
  else if (strcasecmp(cmd, "calibratenext") == 0)
  {
    float weight = 0.0;
    if (args != NULL)
    {
      weight = atof(args);
    }
    handleCalibrationStep(weight);
  }
  else if (strcasecmp(cmd, "tare_scale") == 0)
  {
    tareScale();
  }
  else if (strcasecmp(cmd, "updatepcf") == 0)
  {
    updatePcf();
  }
#endif
  else if (currentState == DEBUG)
  {
    if (strcasecmp(cmd, "heater") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        setHeater(true);
      else if (args && strcasecmp(args, "off") == 0)
        setHeater(false);
      else
        printlnToAll("Usage: heater on|off");
    }
    else if (strcasecmp(cmd, "pump") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        setPump(true);
      else if (args && strcasecmp(args, "off") == 0)
        setPump(false);
      else
        printlnToAll("Usage: pump on|off");
    }
    else if (strcasecmp(cmd, "fillvalve") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        setBoilerFillValve(true);
      else if (args && strcasecmp(args, "off") == 0)
        setBoilerFillValve(false);
      else
        printlnToAll("Usage: fillvalve on|off");
    }
    else if (strcasecmp(cmd, "dimmer") == 0)
    {
      if (args != NULL)
      {
        int val = atoi(args);
        if (val >= 0 && val <= 100)
        {
          setPumpPower(val);
          printToAll("Dimmer set to ");
          printlnToAll(val);
        }
        else
        {
          printlnToAll("Usage: dimmer <0-100>");
        }
      }
    }
    else if (strcasecmp(cmd, "checkic") == 0)
    {
      int lm1830State = checkLM1830();
      if (lm1830State == 1)
        printlnToAll("OK: Pin toggled correctly.");
      else if (lm1830State == 0)
        printlnToAll("Inconclusive. Pin high in both states.");
      else
        printlnToAll("FAIL: IC failed (Pin high when OFF).");
    }
#ifdef HAS_SCALE
    else if (strcasecmp(cmd, "readweight") == 0)
    {
      long currentRawADC;
      int attempts = 0;
      const int MAX_ATTEMPTS = 200;
      do
      {
        currentRawADC = readADCScale();

        if (currentRawADC == -1 || currentRawADC == -2)
        {
          attempts++;
          delay(2);
        }
      } while ((currentRawADC == -1 || currentRawADC == -2) && attempts < MAX_ATTEMPTS);

      if (currentRawADC == -1 || currentRawADC == -2)
      {
        printlnToAll("--- SCALE ERROR ---");
        printlnToAll("Timeout: Could not retrieve a valid reading.");
        printlnToAll("Check wiring or if scale is powered.");
        return;
      }

      float currentInstantWeight = (float)(currentRawADC - COMBINED_OFFSET) / COMBINED_SCALE;

      printlnToAll("--- SCALE DIAGNOSTIC ---");

      printToAll("Filtered Weight (Kalman): ");
      printToAll(currentWeight, 2);
      printlnToAll(" g");

      printToAll("Instant Weight (Unfiltered): ");
      printToAll(currentInstantWeight, 2);
      printlnToAll(" g");

      printToAll("Raw ADC Value: ");
      printlnToAll(currentRawADC);

      printToAll("Calibration Offset: ");
      printToAll(COMBINED_OFFSET);
      printToAll(" | Scale Factor: ");
      printlnToAll(COMBINED_SCALE, 6);
      printlnToAll("--------------------------");
    }
#endif
    else if (strcasecmp(cmd, "pidoutput") == 0)
    {
      if (args && strcasecmp(args, "auto") == 0)
      {
        manualHeaterControl = false;
        heaterPID.SetMode(AUTOMATIC);
        printlnToAll("Heater control returned to automatic PID.");
      }
      else if (args)
      {
        float percentage = atof(args);
        if (percentage >= 0 && percentage <= 100)
        {
          manualHeaterPercentage = percentage;
          manualHeaterControl = true;
          heaterPID.SetMode(MANUAL);
          printToAll("Manual heater output: ");
          printToAll(manualHeaterPercentage, 1);
          printlnToAll("%");
        }
        else
        {
          printlnToAll("Usage: pidoutput <0-100|auto>");
        }
      }
    }
    else if (strcasecmp(cmd, "buzzer") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        digitalWrite(BUZZER, HIGH);
      else
        digitalWrite(BUZZER, LOW);
    }
    else if (strcasecmp(cmd, "ledmain") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        digitalWrite(LEDMAIN, HIGH);
      else
        digitalWrite(LEDMAIN, LOW);
    }
    else if (strcasecmp(cmd, "ledheater") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        digitalWrite(LEDHEATER, HIGH);
      else
        digitalWrite(LEDHEATER, LOW);
    }
    else if (strcasecmp(cmd, "ledwater") == 0)
    {
      if (args && strcasecmp(args, "on") == 0)
        digitalWrite(LEDWATER, HIGH);
      else
        digitalWrite(LEDWATER, LOW);
    }
    else if (strcasecmp(cmd, "writepin") == 0)
    {
      if (args)
      {
        char *pinName = strtok(args, " ");
        char *state = strtok(NULL, " ");
        if (pinName && state)
        {
          int pin = getPinByName(pinName);
          if (pin != -1)
          {
            bool isValid = false;
            for (int p : digitalOutputs)
              if (p == pin)
                isValid = true;

            if (isValid)
            {
              if (strcasecmp(state, "high") == 0)
                digitalWrite(pin, HIGH);
              else
                digitalWrite(pin, LOW);
              printlnToAll("Pin state updated.");
            }
            else
            {
              printlnToAll("Error: Not a configured output pin.");
            }
          }
          else
          {
            printlnToAll("Error: Unknown pin.");
          }
        }
        else
        {
          printlnToAll("Usage: writepin <name|pin> <high|low>");
        }
      }
    }
    else if (strcasecmp(cmd, "readpin") == 0)
    {
      if (args)
      {
        int pin = getPinByName(args);
        if (pin != -1)
        {
          bool isValid = false;
          for (int p : digitalInputs)
            if (p == pin)
              isValid = true;
          if (isValid)
          {
            printToAll("Pin ");
            printToAll(args);
            printToAll(" is ");
            printlnToAll(digitalRead(pin) == HIGH ? "HIGH" : "LOW");
          }
          else
          {
            printlnToAll("Error: Not a configured input pin.");
          }
        }
        else
        {
          printlnToAll("Error: Unknown pin.");
        }
      }
    }
    else if (strcasecmp(cmd, "readadc") == 0)
    {
      if (args)
      {
        int pin = atoi(args);
        if (pin >= 0 && pin <= 3)
        {
          int16_t adcValue = readADSADC(pin);
          float voltage = (float)adcValue / NEW_ADC_MAX * V_REF_NEW;
          printToAll("ADC[");
          printToAll(pin);
          printToAll("] Raw: ");
          printToAll(adcValue);
          printToAll(", Volts: ");
          printToAll(voltage, 3);
          printlnToAll("V");
        }
        else
        {
          printlnToAll("Usage: readadc <0-3>");
        }
      }
    }
    else if (strcasecmp(cmd, "readhxtemp") == 0)
    {
      int16_t rawADC = readADSADC(HX_TEMP);
      float voltage = (float)rawADC / NEW_ADC_MAX * V_REF_NEW;
      float temp = convertADCToTemp(rawADC);
      printToAll("HX Temp: ");
      printToAll(temp, 2);
      printToAll("C (Raw: ");
      printToAll(rawADC);
      printToAll(", ");
      printToAll(voltage, 3);
      printlnToAll("V)");
    }
    else if (strcasecmp(cmd, "restartreason") == 0)
    {
      printToAll("Restart Reason Code: ");
      printlnToAll((int)esp_reset_reason());
    }
    else if (strcasecmp(cmd, "computedboiler") == 0)
    {
      printToAll("Computed Boiler Target: ");
      printlnToAll(calculatedBoilerTemp);
    }
    else if (strcasecmp(cmd, "readboilertemp") == 0)
    {
      int16_t rawADC = readADSADC(BOILER_TEMP);
      float voltage = (float)rawADC / NEW_ADC_MAX * V_REF_NEW;
      float temp = convertADCToTemp(rawADC);
      printToAll("Boiler Temp: ");
      printToAll(temp, 2);
      printToAll("C (Raw: ");
      printToAll(rawADC);
      printToAll(", ");
      printToAll(voltage, 3);
      printlnToAll("V)");
#ifdef HAS_PRESSURE_GAUGE
    }
    else if (strcasecmp(cmd, "readpress") == 0)
    {
      int16_t rawADC = readADSADC(PRESSURE);
      float voltage = (float)rawADC / NEW_ADC_MAX * V_REF_NEW;
      float constrainedVoltage = constrain(voltage, PRESSURE_VOLTAGE_MIN, PRESSURE_VOLTAGE_MAX);
      float press = (constrainedVoltage - PRESSURE_VOLTAGE_MIN) * (PRESSURE_BAR_MAX / (PRESSURE_VOLTAGE_MAX - PRESSURE_VOLTAGE_MIN));
      printToAll("Pressure: ");
      printToAll(press, 2);
      printToAll(" bar (Raw ADC: ");
      printToAll(rawADC);
      printToAll(", Volts: ");
      printToAll(voltage, 3);
      printlnToAll("V)");
#endif
#ifdef HAS_SCALE
    }

    else if (strcasecmp(cmd, "rawdata") == 0)
    {
      if (args)
      {
        if (strcasecmp(args, "on") == 0)
          sendRawDebugData = true;
        else if (strcasecmp(args, "off") == 0)
          sendRawDebugData = false;
        else
          sendRawDebugData = !sendRawDebugData;
      }
      else
      {
        sendRawDebugData = !sendRawDebugData;
      }
      printToAll("ESP-NOW raw data: ");
      printlnToAll(sendRawDebugData ? "ON" : "OFF");
#endif
    }
    else
    {
      printlnToAll("Unknown debug command. Type 'help'.");
    }
  }
  else
  {
    printToAll("Unknown command: '");
    printToAll(cmd);
    printlnToAll("'");
    printlnToAll("Type 'help' for available commands.");
  }
}

void printStatus()
{
  bool currentBrewLever = (digitalRead(BREW_SWITCH) == LOW);
  bool currentSwitch1 = (digitalRead(THREE_WAY_SWITCH1) == HIGH);
  bool currentSwitch2 = (digitalRead(THREE_WAY_SWITCH2) == HIGH);
  bool currentWaterLevel = (digitalRead(WATER_DETECTOR) == LOW);
  bool currentTwoWaySwitch = (digitalRead(TWO_WAY_SWITCH) == HIGH);
  bool currentBoilerLevel = !detectBoilerLevel();

  printlnToAll("--- STATUS ---");
  printToAll("WiFi Channel: ");
  printlnToAll(WiFi.channel());
  printToAll("WiFi RSSI: ");
  printToAll(WiFi.RSSI());
  printlnToAll(" dBm");
  printToAll("Current State: ");
  printlnToAll(stateToString(currentState));
  printToAll("Brew Mode: ");
  printToAll(brewMode);
  printToAll(", Steam Boost: ");
  printlnToAll((enableSteamBoost ? "Enabled" : "Disabled"));
  printToAll("Boiler: ");
  printToAll(boilerTemp);
  printToAll("C, HX: ");
  printToAll(hxTemp);
#ifdef HAS_PRESSURE_GAUGE
  printToAll("C, Pressure: ");
  printToAll(pressure);
  printlnToAll(" bar");
#else
  printlnToAll("C");
#endif
  printToAll("Heater: ");
  printlnToAll(heaterOn ? "ON" : "OFF");
  printlnToAll("--- INPUTS ---");
  printToAll("Brew Lever: ");
  printlnToAll(currentBrewLever ? "LIFTED" : "DOWN");
  printToAll("2-Way Switch: ");
  printlnToAll(currentTwoWaySwitch ? "STEAM (HIGH)" : "COFFEE (LOW)");
  printToAll("3-Way Switch1: ");
  printToAll(currentSwitch1 ? "HIGH" : "LOW");
  printToAll(", Switch2: ");
  printlnToAll(currentSwitch2 ? "HIGH" : "LOW");
  printToAll("Water Tank: ");
  printToAll(currentWaterLevel ? "EMPTY" : "OK");
  printToAll(", Boiler Empty: ");
  printlnToAll(currentBoilerLevel ? "YES" : "NO");
  printlnToAll("--- SETTINGS ---");
  printToAll("Temps: Brew=");
  printToAll(tempSetBrew, 2);
  printToAll("C, Steam=");
  printToAll(tempSetSteam, 2);
  printToAll("C, Boost=");
  printToAll(tempSetSteamBoost, 2);
  printlnToAll("C");
  printToAll("PID Temp: P=");
  printToAll(kp_temperature, 4);
  printToAll(" I=");
  printToAll(ki_temperature, 8);
  printToAll(" D=");
  printlnToAll(kd_temperature, 2);
#ifdef HAS_PRESSURE_GAUGE
  printToAll("PID Pressure: P=");
  printToAll(kp_pressure, 4);
  printToAll(" I=");
  printToAll(ki_pressure, 4);
  printToAll(" D=");
  printlnToAll(kd_pressure, 4);
#endif
#ifdef HAS_SCALE
  printToAll("PID Flow: P=");
  printToAll(kp_flow, 4);
  printToAll(" I=");
  printToAll(ki_flow, 4);
  printToAll(" D=");
  printlnToAll(kd_flow, 4);
  printToAll("Kalman W: Me=");
  printToAll(weightKalmanMe, 2);
  printToAll(" E=");
  printToAll(weightKalmanE, 2);
  printToAll(" Q=");
  printlnToAll(weightKalmanQ, 2);

  printToAll("Kalman F: Me=");
  printToAll(flowKalmanMe, 2);
  printToAll(" E=");
  printToAll(flowKalmanE, 2);
  printToAll(" Q=");
  printlnToAll(flowKalmanQ, 2);
#endif
  printToAll("Profiling: Mode=");
  printToAll(profilingMode);
  printToAll(", Source=");
  printlnToAll(profilingSource);

  printlnToAll("--- CURRENT PROFILE ---");
  printToAll("Name: ");
  printlnToAll(currentProfile.name);
  printToAll("Type: ");
  printlnToAll(currentProfile.isStepped ? "Stepped" : "Ramped");
  printToAll("Steps: ");
  printlnToAll(currentProfile.numSteps);
  for (int i = 0; i < currentProfile.numSteps; i++)
  {
    printToAll("  Step ");
    printToAll(i);
    printToAll(": Target=");
    printToAll(currentProfile.steps[i].setpoint, 1);
    printToAll(", Trigger=");
    printlnToAll(currentProfile.steps[i].trigger, 1);
  }
  printlnToAll("-----------------------");
}

// ----------------------------------------------------------------
// --- State Machine Functions ---
// ----------------------------------------------------------------
void transitionToState(MachineState newState)
{
  if (currentState == newState)
  {
    return;
  }
  publishData(mqtt_topic_state, stateToString(newState), true, true);
  lastStateTransitionTime = millis();
  digitalWrite(BUZZER, LOW);
  if (currentState == DEBUG)
  {
    if (manualHeaterControl)
    {
      manualHeaterControl = false;
      heaterPID.SetMode(AUTOMATIC);
      printlnToAll("Manual heater control disabled. PID is now AUTOMATIC.");
    }
  }
  if (currentState == STANDBY)
  {
    leverLiftedInStandby = false;
  }

  if (currentState == HEATING && newState == IDLE)
  {
    if (BUZZER_ENABLE)
    {
      triggerBeep(150);
    }
  }

  if (currentState == HEATING || currentState == IDLE)
  {
    if (newState != HEATING && newState != IDLE)
    {
      stableTempStartTime = 0;
    }
  }

  currentState = newState;
  printToAll("Transitioning to state: ");
  printlnToAll(stateToString(newState));
  handleStateEntry(newState);
}

/**
 * @brief Handles all "on entry" actions for a new state.
 * This function is called by transitionToState() *after*
 * the new state has been set.
 * @param newState The state we are *entering*.
 */
void handleStateEntry(MachineState newState)
{
  switch (newState)
  {
  case ERROR:
  case STANDBY:
    allStop();
    break;

  case WATER_EMPTY:
    allStop();
    break;

  case BOILER_EMPTY:
    setHeater(false);
    enableWaterLevelSensor(true);
    setPumpPower(100);
    boilerFullTimestamp = 0;
    break;

  case HEATING:
    heaterPID.SetMode(MANUAL);
    heaterPID.SetMode(AUTOMATIC);
    stableTempStartTime = 0;
#ifdef HAS_SCALE
    updatePcf();
#endif
    break;

  case COOLING_FLUSH:
    setBoilerFillValve(false);
    setPumpPower(100);
    setPump(true);
    flushStartTime = millis();
    break;

  case IDLE:
    setPump(false);
    setBoilerFillValve(false);
    idleEntryTime = millis();
#ifdef HAS_SCALE
    updatePcf();
#endif
    break;

  case BREWING:
    setBoilerFillValve(false);
    setPump(true);
    currentProfileStepIndex = 0;
    currentStepStartX = 0.0f;
    prevStepTargetY = 0.0f;
    shotStartTime = millis();
#ifdef HAS_PRESSURE_GAUGE
    pressurePID.SetMode(MANUAL);
#endif
#ifdef HAS_SCALE
    flowPID.SetMode(MANUAL);
#endif
#ifdef HAS_SCALE
    updatePcf();
#endif
    break;

  case STEAM_BOOST:
    setPump(false);
    setBoilerFillValve(false);
    steamBoostEntryTime = millis();
#ifdef HAS_SCALE
    updatePcf();
#endif
    break;

  case CLEANING_START:
  case CLEANING_PAUSE:
    setPump(false);
    setBoilerFillValve(false);
    break;

  case CLEANING_PUMPING:
    setPumpPower(100);
    setPump(true);
    setBoilerFillValve(false);
    cleaningPumpStartTime = millis();
    cleaningBeepState = 0;
    break;
#ifdef HAS_SCALE
  case CALIBRATION_EMPTY:
  case CALIBRATION_TEST_WEIGHT:
    setPump(false);
    setBoilerFillValve(false);
    if (newState == CALIBRATION_EMPTY)
    {
      printlnToAll("\nStep 1: Tare Weight");
      printlnToAll("Ensure the scale is empty, then send 'calibratenext' (Telnet) or 'calibration_step' (MQTT/ESP-NOW).");
    }
    else
    {
      printlnToAll("\nStep 2: Calibration Weight");
      printToAll("Place your ");
      printToAll(calibrationWeight);
      printlnToAll("g weight on the scale, then send 'calibratenext <weight>' (Telnet) or 'calibration_step <weight>' (MQTT/ESP-NOW).");
    }
    break;
#endif
  case DEBUG:
  case INIT:
    break;
  }
}

/**
 * @brief Converts a MachineState enum value to a read-only string literal.
 * * This function is memory-safe as it returns a 'const char*' pointer
 * to a string literal in program memory, avoiding all heap allocations.
 * * @param state The MachineState enum value to convert.
 * @return A const char* pointer to the string name of the state.
 */
const char *stateToString(MachineState state)
{
  switch (state)
  {
  case WATER_EMPTY:
    return "WATER_EMPTY";
  case BOILER_EMPTY:
    return "BOILER_EMPTY";
  case HEATING:
    return "HEATING";
  case BREWING:
    return "BREWING";
  case STANDBY:
    return "STANDBY";
  case IDLE:
    return "IDLE";
  case CLEANING_START:
    return "CLEANING_START";
  case CLEANING_PUMPING:
    return "CLEANING_PUMPING";
  case CLEANING_PAUSE:
    return "CLEANING_PAUSE";
  case STEAM_BOOST:
    return "STEAM_BOOST";
  case COOLING_FLUSH:
    return "COOLING_FLUSH";
  case ERROR:
    return "ERROR";
  case DEBUG:
    return "DEBUG";
  case INIT:
    return "INIT";
  case CALIBRATION_EMPTY:
    return "CALIBRATION_EMPTY";
  case CALIBRATION_TEST_WEIGHT:
    return "CALIBRATION_TEST_WEIGHT";
  }
  return "UNKNOWN_STATE";
}

void allStop()
{
  setBoilerFillValve(false);
  setPump(false);
  setHeater(false);
#ifdef HAS_PRESSURE_GAUGE
  pressurePID.SetMode(MANUAL);
#endif
#ifdef HAS_SCALE
  flowPID.SetMode(MANUAL);
#endif
  pumpOutput = 0;
}

void triggerBeep(int duration)
{
  if (BUZZER_ENABLE)
  {
    digitalWrite(BUZZER, HIGH);
    beepStopTime = millis() + duration;
    isBeeping = true;
  }
}

// ----------------------------------------------------------------
// --- Hardware Control Functions ---
// ----------------------------------------------------------------
void setHeater(bool on)
{
  if (on != heaterOn)
  {
    heaterOn = on;
    digitalWrite(HEATER_SSR, heaterOn ? HIGH : LOW);
    if (mqttClient.connected())
    {
      publishData(mqtt_topic_heater, heaterOn ? "ON" : "OFF", false);
    }
  }
}

void setBoilerFillValve(bool on)
{
  static bool lastFillValveState = !on;
  if (on != lastFillValveState)
  {
    digitalWrite(PUMP_RELAY, on ? HIGH : LOW);
    lastFillValveState = on;
  }
}

void setPump(bool on)
{
  if (on != pumpRunning)
  {
    pumpRunning = on;
    digitalWrite(COFFEE_RELAY, pumpRunning ? HIGH : LOW);
    lastPumpStateChangeTime = millis();
    if (mqttClient.connected())
    {
      publishData(mqtt_topic_pump, pumpRunning ? "ON" : "OFF", false);
    }
  }
}

void setPumpPower(int percentage)
{
#ifdef HAS_PRESSURE_GAUGE
  int brightness = (int)(percentage * (255.0 / 100.0));
  pumpDimmer.setBrightness(brightness);
#else
  return;
#endif
}

void enableWaterLevelSensor(bool on)
{
  static bool lastEnableState = !on;
  if (on != lastEnableState)
  {
    digitalWrite(ENABLE_LM1830, on ? HIGH : LOW);
    lastEnableState = on;
  }
}

// ----------------------------------------------------------------
// --- Interrupts & Input Handling ---
// ----------------------------------------------------------------
void pollDigitalInputs()
{
  static unsigned long lastDebounceTimeLever = 0;
  static unsigned long lastDebounceTimeWater = 0;
  static bool lastRawLeverState = false;
  static bool lastRawWaterState = false;

  bool currentRawLeverState = (digitalRead(BREW_SWITCH) == LOW);

  if (currentRawLeverState != lastRawLeverState)
  {
    lastDebounceTimeLever = millis();
  }

  if ((millis() - lastDebounceTimeLever) > debounceDelay)
  {
    if (currentRawLeverState != brewLeverLifted)
    {
      if (currentRawLeverState == false)
      {
        if (currentState == BREWING || currentState == HEATING || currentState == COOLING_FLUSH)
        {
          shotEndTime = millis();
          printlnToAll("Lever down. Logging drips for 3s.");
        }
      }
      brewLeverLifted = currentRawLeverState;
      lastLeverChangedTime = millis();
      if (mqttClient.connected())
      {
        publishData(mqtt_topic_lever, brewLeverLifted ? "LIFTED" : "DOWN", false);
      }
    }
  }

  lastRawLeverState = currentRawLeverState;
  bool currentRawWaterState = (digitalRead(WATER_DETECTOR) == LOW);
  if (currentRawWaterState != lastRawWaterState)
  {
    lastDebounceTimeWater = millis();
  }
  if ((millis() - lastDebounceTimeWater) > debounceDelay)
  {
    if (currentRawWaterState != waterLevelTripped)
    {
      waterLevelTripped = currentRawWaterState;
      lastWaterLevelChangedTime = millis();
    }
  }
  lastRawWaterState = currentRawWaterState;

  threeWaySwitch1High = (digitalRead(THREE_WAY_SWITCH1) == HIGH);
  threeWaySwitch2High = (digitalRead(THREE_WAY_SWITCH2) == HIGH);
}

int checkLM1830()
{
  digitalWrite(ENABLE_LM1830, LOW);
  delay(500);
  bool pinBefore = digitalRead(BOILER_LEVEL);
  digitalWrite(ENABLE_LM1830, HIGH);
  delay(500);
  bool pinAfter = digitalRead(BOILER_LEVEL);
  digitalWrite(ENABLE_LM1830, LOW);
  if (pinBefore)
  {
    return -1;
  }
  if (!pinBefore && pinAfter)
  {
    return 1;
  }
  return 0;
}

void periodicBoilerLevelCheck()
{
  if (currentState == BOILER_EMPTY)
    return;
  if (pumpRunning)
  {
    enableWaterLevelSensor(false);
    return;
  }
  static unsigned long lm1830EnableTime = 0;
  static bool isLm1830Enabled = false;
  static bool previousBoilerReadingEmpty = false;

  if ((!isLm1830Enabled && (millis() - lastBoilerCheckTime > BOILER_CHECK_PERIOD)) && (currentState != DEBUG))
  {
    isBoilerPinHigh = digitalRead(BOILER_LEVEL);
    enableWaterLevelSensor(true);
    isLm1830Enabled = true;
    lm1830EnableTime = millis();
    lastBoilerCheckTime = millis();
  }

  if ((isLm1830Enabled && (millis() - lm1830EnableTime > BOILER_CHECK_DURATION)) && (currentState != DEBUG))
  {
    bool currentBoilerReadingEmpty = !digitalRead(BOILER_LEVEL);

    if (currentBoilerReadingEmpty && previousBoilerReadingEmpty)
    {
      isBoilerEmpty = true;
    }
    else if (!currentBoilerReadingEmpty)
    {
      isBoilerEmpty = false;
    }

    previousBoilerReadingEmpty = currentBoilerReadingEmpty;

    enableWaterLevelSensor(false);
    isLm1830Enabled = false;
  }
}

// ----------------------------------------------------------------
// --- Sensor Reading & Processing ---
// ----------------------------------------------------------------
void updateSensorReadings()
{
  static float boilerTempSamples[SENSOR_SMOOTHING_SAMPLES];
  static float hxTempSamples[SENSOR_SMOOTHING_SAMPLES];

  static float boilerTempTotal = 0;
  static float hxTempTotal = 0;
#ifdef HAS_PRESSURE_GAUGE
  static float pressureTotal = 0;
  static float pressureSamples[SENSOR_SMOOTHING_SAMPLES];
#endif
  static int sampleIndex = 0;
  static bool filterInitialized = false;

  if (!filterInitialized)
  {
    unsigned long bootReadStartTime = millis();
    do
    {
      boilerTempADC = readADSADC(BOILER_TEMP);
      if (millis() % 50 == 0)
        delay(1);

      if (millis() - bootReadStartTime > 250)
      {
        break;
      }
    } while (boilerTempADC < ADC_RAILED_THRESHOLD);

    bootReadStartTime = millis();
    do
    {
      hxTempADC = readADSADC(HX_TEMP);
      if (millis() % 50 == 0)
        delay(1);

      if (millis() - bootReadStartTime > 250)
      {
        break;
      }
    } while (hxTempADC < ADC_RAILED_THRESHOLD);

    float initialBoilerTemp = convertADCToTemp(boilerTempADC);
    float initialHxTemp = convertADCToTemp(hxTempADC);
#ifdef HAS_PRESSURE_GAUGE
    int16_t initialPressureADC = readADSADC(PRESSURE);
    float initialVoltage = (float)initialPressureADC / NEW_ADC_MAX * V_REF_NEW;
    float initialConstrainedVoltage = constrain(initialVoltage, PRESSURE_VOLTAGE_MIN, PRESSURE_VOLTAGE_MAX);
    float initialPressure = (initialConstrainedVoltage - PRESSURE_VOLTAGE_MIN) * (PRESSURE_BAR_MAX / (PRESSURE_VOLTAGE_MAX - PRESSURE_VOLTAGE_MIN));
#endif
    for (int i = 0; i < SENSOR_SMOOTHING_SAMPLES; i++)
    {
      boilerTempSamples[i] = initialBoilerTemp;
      hxTempSamples[i] = initialHxTemp;
#ifdef HAS_PRESSURE_GAUGE
      pressureSamples[i] = initialPressure;
#endif
    }

    boilerTempTotal = initialBoilerTemp * SENSOR_SMOOTHING_SAMPLES;
    hxTempTotal = initialHxTemp * SENSOR_SMOOTHING_SAMPLES;
#ifdef HAS_PRESSURE_GAUGE
    pressureTotal = initialPressure * SENSOR_SMOOTHING_SAMPLES;
#endif

    filterInitialized = true;
  }

  bool isFirstRunInSetup = (lastStateTransitionTime == 0);

  if (!isFirstRunInSetup && (millis() - lastStateTransitionTime < SENSOR_READ_PAUSE_MS || millis() - lastLeverChangedTime < SENSOR_READ_PAUSE_MS || millis() - lastWaterLevelChangedTime < SENSOR_READ_PAUSE_MS || millis() - lastPumpStateChangeTime < SENSOR_READ_PAUSE_MS))
  {
    return;
  }
  boilerTempTotal -= boilerTempSamples[sampleIndex];
  hxTempTotal -= hxTempSamples[sampleIndex];
#ifdef HAS_PRESSURE_GAUGE
  pressureTotal -= pressureSamples[sampleIndex];
#endif

  boilerTempADC = readADSADC(BOILER_TEMP);
  hxTempADC = readADSADC(HX_TEMP);
#ifdef HAS_PRESSURE_GAUGE
  int16_t pressureADC = readADSADC(PRESSURE);
#endif

  float newBoilerTemp = convertADCToTemp(boilerTempADC);
  float newHxTemp = convertADCToTemp(hxTempADC);
#ifdef HAS_PRESSURE_GAUGE
  float voltage = (float)pressureADC / NEW_ADC_MAX * V_REF_NEW;
  float constrainedVoltage = constrain(voltage, PRESSURE_VOLTAGE_MIN, PRESSURE_VOLTAGE_MAX);
  float newPressure = (constrainedVoltage - PRESSURE_VOLTAGE_MIN) * (PRESSURE_BAR_MAX / (PRESSURE_VOLTAGE_MAX - PRESSURE_VOLTAGE_MIN));
#endif
  boilerTempSamples[sampleIndex] = newBoilerTemp;
  hxTempSamples[sampleIndex] = newHxTemp;
#ifdef HAS_PRESSURE_GAUGE
  pressureSamples[sampleIndex] = newPressure;
#endif

  boilerTempTotal += newBoilerTemp;
  hxTempTotal += newHxTemp;
#ifdef HAS_PRESSURE_GAUGE
  pressureTotal += newPressure;
#endif
  boilerTemp = boilerTempTotal / SENSOR_SMOOTHING_SAMPLES;
  hxTemp = hxTempTotal / SENSOR_SMOOTHING_SAMPLES;
#ifdef HAS_PRESSURE_GAUGE
  pressure = pressureTotal / SENSOR_SMOOTHING_SAMPLES;
#endif
  sampleIndex++;
  if (sampleIndex >= SENSOR_SMOOTHING_SAMPLES)
  {
    sampleIndex = 0;
  }
}

bool readPin(const int pin)
{
  return (digitalRead(pin) == HIGH);
}

bool detectBoilerLevel()
{
  if (pumpRunning)
  {
    return false;
  }
  enableWaterLevelSensor(true);
  delay(500);
  bool waterLevel = readPin(BOILER_LEVEL);
  enableWaterLevelSensor(false);
  return (waterLevel);
}

int16_t readADSADC(const int pin)
{
  return ads.readADC_SingleEnded(pin);
}

float convertADCToTemp(int16_t adc)
{
  if (adc < 0 || adc >= 32767)
  {
    return 0.0;
  }
  return linearInterpolation(remappedAdcValues, remappedTemperatureValues, LEGACY_TEMP_LOOKUP_TABLE_SIZE, (float)adc, false);
}

float linearInterpolation(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
  if (trim)
  {
    if (pointX <= xValues[0])
      return yValues[0];
    if (pointX >= xValues[numValues - 1])
      return yValues[numValues - 1];
  }

  auto i = 0;
  float rst = 0;
  if (pointX <= xValues[0])
  {
    i = 0;
    auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
    rst = yValues[i] * (1 - t) + yValues[i + 1] * t;
  }
  else if (pointX >= xValues[numValues - 1])
  {
    auto t = (pointX - xValues[numValues - 2]) / (xValues[numValues - 1] - xValues[numValues - 2]);
    rst = yValues[numValues - 2] * (1 - t) + yValues[numValues - 1] * t;
  }
  else
  {
    while (pointX >= xValues[i + 1])
      i++;
    auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
    rst = yValues[i] * (1 - t) + yValues[i + 1] * t;
  }
  return rst;
}

double feedForwardHeater(double c1, double c2, double steadyStateTempC, double ambientTempC)
{
  double steadyStateTempK = steadyStateTempC + 273.15;
  double ambientTempK = ambientTempC + 273.15;

  double linearLoss = c1 * (steadyStateTempC - ambientTempC);

  double radiativeLoss = c2 * (pow(steadyStateTempK, 4) - pow(ambientTempK, 4));

  double powerOut = linearLoss + radiativeLoss;
  return powerOut * 100;
}

double getTempFromPower(double targetPower, double c1, double c2, double ambientTemp)
{
  if (targetPower <= 0.0)
    return ambientTemp;

  double T_guess = 100.0;
  double ambientK = ambientTemp + 273.15;

  const int MAX_ITERATIONS = 20;
  const double TOLERANCE = 0.01;

  for (int i = 0; i < MAX_ITERATIONS; i++)
  {
    double T_guessK = T_guess + 273.15;

    double linearLoss = c1 * (T_guess - ambientTemp);
    double radiativeLoss = c2 * (pow(T_guessK, 4) - pow(ambientK, 4));
    double calculatedPower = (linearLoss + radiativeLoss) * 100.0;

    double f_T = calculatedPower - targetPower;

    if (abs(f_T) < TOLERANCE)
    {
      return T_guess;
    }
    double f_prime_T = 100.0 * (c1 + (4.0 * c2 * pow(T_guessK, 3)));
    if (f_prime_T == 0.0)
      break;
    T_guess = T_guess - (f_T / f_prime_T);
  }

  return T_guess;
}

void updateCalculatedBoilerTemp()
{
  double requiredPower = feedForwardHeater(c1Brew, c2Brew, tempSetBrew, ASSUMED_AMBIENT_TEMP);
  calculatedBoilerTemp = getTempFromPower(requiredPower, c1Boiler, c2Boiler, ASSUMED_AMBIENT_TEMP);
}

bool checkCriticalSensorFailure()
{
  if (boilerTempADC < ADC_RAILED_THRESHOLD || boilerTempADC > (ADC_MAX_VALUE - ADC_RAILED_THRESHOLD))
  {
    snprintf(lastError, sizeof(lastError), "Boiler sensor disconnected/shorted. ADC: %d", boilerTempADC);
    return true;
  }
  if (hxTempADC < ADC_RAILED_THRESHOLD || hxTempADC > (ADC_MAX_VALUE - ADC_RAILED_THRESHOLD))
  {
    snprintf(lastError, sizeof(lastError), "HX sensor disconnected/shorted. ADC: %d", hxTempADC);
    return true;
  }

  if (isBoilerPinHigh)
  {
    snprintf(lastError, sizeof(lastError), "LM1830 Pin High when switch turned OFF");
    return true;
  }

  if (boilerTemp > MAX_SAFE_TEMPERATURE)
  {
    snprintf(lastError, sizeof(lastError), "Boiler temperature unsafe: %.1f C", boilerTemp);
    return true;
  }
  return false;
}

// ----------------------------------------------------------------
// --- User Feedback (LED & Buzzer) ---
// ----------------------------------------------------------------
void errorBuzzer()
{
  if (!BUZZER_ENABLE)
  {
    return;
  }
  static unsigned long lastBeepTime = 0;
  const int beepInterval = 500;

  if (millis() - lastBeepTime > beepInterval)
  {
    lastBeepTime = millis();
    digitalWrite(BUZZER, !digitalRead(BUZZER));
  }
}

void waterEmptyBuzzer()
{
  if (!BUZZER_ENABLE)
  {
    return;
  }
  static int beepState = 0;
  static unsigned long lastStateChangeTime = 0;

  const int BEEP_DURATION = 150;
  const int SHORT_PAUSE = 150;
  const int LONG_PAUSE = 3500;

  unsigned long currentTime = millis();

  switch (beepState)
  {
  case 0:
    digitalWrite(BUZZER, HIGH);
    lastStateChangeTime = currentTime;
    beepState = 1;
    break;

  case 1:
    if (currentTime - lastStateChangeTime > BEEP_DURATION)
    {
      digitalWrite(BUZZER, LOW);
      lastStateChangeTime = currentTime;
      beepState = 2;
    }
    break;

  case 2:
    if (currentTime - lastStateChangeTime > SHORT_PAUSE)
    {
      digitalWrite(BUZZER, HIGH);
      lastStateChangeTime = currentTime;
      beepState = 3;
    }
    break;

  case 3:
    if (currentTime - lastStateChangeTime > BEEP_DURATION)
    {
      digitalWrite(BUZZER, LOW);
      lastStateChangeTime = currentTime;
      beepState = 4;
    }
    break;

  case 4:
    if (currentTime - lastStateChangeTime > LONG_PAUSE)
    {
      beepState = 0;
    }
    break;
  }
}

void ledError()
{
  static unsigned long lastBlinkTime = 0;
  const int blinkInterval = 500;

  if (millis() - lastBlinkTime > blinkInterval)
  {
    lastBlinkTime = millis();
    digitalWrite(LEDMAIN, !digitalRead(LEDMAIN));
    digitalWrite(LEDHEATER, !digitalRead(LEDHEATER));
    digitalWrite(LEDWATER, !digitalRead(LEDWATER));
  }
}

void ledWaterEmpty()
{
  digitalWrite(LEDHEATER, LOW);
  digitalWrite(LEDMAIN, HIGH);

  static unsigned long lastBlinkTime = 0;
  const int blinkInterval = 500;

  if (millis() - lastBlinkTime > blinkInterval)
  {
    lastBlinkTime = millis();
    digitalWrite(LEDWATER, !digitalRead(LEDWATER));
  }
}

void ledHeating()
{
  digitalWrite(LEDWATER, HIGH);
  digitalWrite(LEDMAIN, HIGH);

  static unsigned long lastBlinkTime = 0;
  const int blinkInterval = 500;

  if (millis() - lastBlinkTime > blinkInterval)
  {
    lastBlinkTime = millis();
    digitalWrite(LEDHEATER, !digitalRead(LEDHEATER));
  }
}

void ledIdle()
{
  digitalWrite(LEDWATER, HIGH);
  digitalWrite(LEDMAIN, HIGH);
  digitalWrite(LEDHEATER, HIGH);
}

void ledBrew()
{
  digitalWrite(LEDWATER, HIGH);
  digitalWrite(LEDMAIN, HIGH);
  digitalWrite(LEDHEATER, HIGH);
}

void ledStandby()
{
  digitalWrite(LEDHEATER, LOW);
  digitalWrite(LEDWATER, LOW);

  static unsigned long lastBlinkTime = 0;
  const unsigned long BLINK_CYCLE = 5000;
  const unsigned long BLINK_ON_DURATION = 500;

  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime > BLINK_CYCLE)
  {
    lastBlinkTime = currentTime;
  }

  if (currentTime - lastBlinkTime < BLINK_ON_DURATION)
  {
    digitalWrite(LEDMAIN, HIGH);
  }
  else
  {
    digitalWrite(LEDMAIN, LOW);
  }
}

// ----------------------------------------------------------------
// --- Machine Operation & Logic ---
// ----------------------------------------------------------------

void runHeaterPID()
{
  static unsigned long pwmWindowStartTime = millis();
  static double lastPidSetpoint = 0;
  bool heatingModeCoffee = strcmp(brewMode, "STEAM");
  if (manualHeaterControl)
  {
    unsigned long now = millis();
    unsigned long heaterOnTime_ms = (manualHeaterPercentage / 100.0) * PWM_WINDOW_SIZE;

    if (now - pwmWindowStartTime >= PWM_WINDOW_SIZE)
    {
      pwmWindowStartTime = now;
    }
    if (heaterOnTime_ms > (now - pwmWindowStartTime))
    {
      setHeater(true);
    }
    else
    {
      setHeater(false);
    }
    return;
  }

  bool bypassPID = false;
  if (heaterPID.GetKp() != kp_temperature || heaterPID.GetKi() != ki_temperature || heaterPID.GetKd() != kd_temperature)
  {
    heaterPID.SetTunings(kp_temperature, ki_temperature, kd_temperature);
  }

  bool heaterShouldRun = true;
  switch (currentState)
  {
  case HEATING:
  case IDLE:
  case COOLING_FLUSH:
  case CLEANING_START:
  case CLEANING_PUMPING:
  case CLEANING_PAUSE:
  case CALIBRATION_EMPTY:
  case CALIBRATION_TEST_WEIGHT:
    if (!heatingModeCoffee)
    {
      bypassPID = true;
    }
    else
    {
      pidSetpoint = tempSetBrew;
      pidInput = hxTemp;
      if (boilerTemp < tempSetBrew)
      {
        bypassPID = true;
      }
      if (hxTemp + 20 < tempSetBrew)
      {
        bypassPID = true;
      }
    }
    break;

  case STEAM_BOOST:
  case BREWING:
    bypassPID = true;
    break;

  default:
    heaterShouldRun = false;
    break;
  }

  if (pidSetpoint != lastPidSetpoint && heatingModeCoffee)
  {
    heaterPID.SetMode(MANUAL);
    heaterPID.SetMode(AUTOMATIC);
    printToAll("PID Reset: Setpoint changed from ");
    printToAll(lastPidSetpoint, 1);
    printToAll("C to ");
    printToAll(pidSetpoint, 1);
    printlnToAll("C");
    lastPidSetpoint = pidSetpoint;
    if (currentState == IDLE)
    {
      transitionToState(HEATING);
    }
  }
  if (!heatingModeCoffee && currentState != HEATING)
  {
    if (boilerTemp >= tempSetBrew + tempSetSteamHeating)
    {
      heaterShouldRun = false;
    }
  }

  if (boilerTemp >= MAX_ALLOWED_BOILER_TEMP)
  {
    heaterShouldRun = false;
  }

  if (!heaterShouldRun)
  {
    setHeater(false);
    return;
  }

  if (bypassPID)
  {
    setHeater(true);
  }
  else
  {
    double ff_output = feedForwardHeater(c1Brew, c2Brew, pidSetpoint, ASSUMED_AMBIENT_TEMP);
    double total_output = 0.0;

    bool boilerIsTooHot = (boilerTemp > (calculatedBoilerTemp + FF_ONLY_THRESHOLD)) && (hxTemp < tempSetBrew);
    bool boilerIsWayTooHot = (boilerTemp >= calculatedBoilerTemp + BOILER_WAY_TOO_HOT);
    if (heatingModeCoffee && boilerIsWayTooHot)
    {
      total_output = 0;
    }
    else if (heatingModeCoffee && boilerIsTooHot)
    {
      total_output = ff_output;
    }
    else
    {
      bool computedOutput = false;
      if (currentState != BREWING)
      {
        computedOutput = heaterPID.Compute();
      }

      total_output = ff_output + pidOutput;
      if (computedOutput && mqttClient.connected())
      {
        char termBuffer[10];
        dtostrf(heaterPID.GetPIntegrator(), 4, 3, termBuffer);
        publishData(mqtt_topic_pterm, termBuffer, false, false);
        dtostrf(heaterPID.GetIIntegrator(), 4, 3, termBuffer);
        publishData(mqtt_topic_iterm, termBuffer, false, false);
        dtostrf(heaterPID.GetDIntegrator(), 4, 3, termBuffer);
        publishData(mqtt_topic_dterm, termBuffer, false, false);
      }
    }
    total_output = constrain(total_output, 0, 100);

    if (mqttClient.connected())
    {
      char termBuffer[10];
      dtostrf(total_output, 4, 3, termBuffer);
      publishData(mqtt_topic_pidoutput, termBuffer, false, true);
    }

    unsigned long now = millis();
    unsigned long heaterOnTime_ms = (total_output / 100.0) * PWM_WINDOW_SIZE;

    if (now - pwmWindowStartTime >= PWM_WINDOW_SIZE)
    {
      pwmWindowStartTime = now;
    }
    if (heaterOnTime_ms > (now - pwmWindowStartTime))
    {
      setHeater(true);
    }
    else
    {
      setHeater(false);
    }
  }
}

bool isStable()
{
  bool heatingModeCoffee = strcmp(brewMode, "STEAM");

  if (!heatingModeCoffee)
  {
    if (hxTemp >= tempSetBrew)
    {
      return true;
    }
    else
    {
      stableTempStartTime = 0;
      return false;
    }
  }
  else
  {
    if (abs(pidInput - pidSetpoint) <= TEMP_STABILITY_TOLERANCE)
    {
      if (stableTempStartTime == 0)
      {
        stableTempStartTime = millis();
      }

      if (millis() - stableTempStartTime >= TEMP_STABILITY_DURATION_MS)
      {
        return true;
      }
    }
    else
    {
      stableTempStartTime = 0;
    }

    return false;
  }
}

void runPumpProfile()
{
  float currentTargetY = 0.0f;
  bool usePID = false;

  if (strcmp(profilingMode, "manual") == 0)
  {
    setPumpPower(100);
#ifdef HAS_PRESSURE_GAUGE
    pressurePID.SetMode(MANUAL);
#endif
#ifdef HAS_SCALE
    flowPID.SetMode(MANUAL);
#endif
    return;
  }
  else if (strcmp(profilingMode, "flat") == 0)
  {
    currentTargetY = profilingFlatValue;
    usePID = true;
  }
  else if (strcmp(profilingMode, "profile") == 0)
  {
    float currentX;
    usePID = false;
    if (strcmp(profilingTarget, "time") == 0)
    {
      currentX = (millis() - shotStartTime) / 1000.0f;
      usePID = true;
    }
#ifdef HAS_SCALE
    if (strcmp(profilingTarget, "weight") == 0)
    {
      currentX = max(0.0f, currentWeight);
      usePID = true;
    }
#endif
    if (usePID)
    {
      currentTargetY = getTargetAt(currentX);
    }
  }

  if (currentTargetY < 0.1f && (millis() - shotStartTime > 5000))
  {
    setPump(false);
    setPumpPower(0);
#ifdef HAS_PRESSURE_GAUGE
    pressurePID.SetMode(MANUAL);
    pumpOutput = 0;
#endif
#ifdef HAS_SCALE
    flowPID.SetMode(MANUAL);
#endif
    return;
  }

  if (usePID)
  {
    pumpSetpoint = (double)currentTargetY;
    bool controlActive = false;

#ifdef HAS_PRESSURE_GAUGE
    if (strcmp(profilingSource, "pressure") == 0)
    {
#ifdef HAS_SCALE
      if (flowPID.GetMode() != MANUAL)
        flowPID.SetMode(MANUAL);
#endif
      if (pressurePID.GetMode() != AUTOMATIC)
        pressurePID.SetMode(AUTOMATIC);

      pumpInput = (double)pressure;
      pressurePID.Compute();
      controlActive = true;
    }
#endif

#ifdef HAS_SCALE
    if (!controlActive && strcmp(profilingSource, "flow") == 0)
    {
#ifdef HAS_PRESSURE_GAUGE
      if (pressurePID.GetMode() != MANUAL)
        pressurePID.SetMode(MANUAL);
#endif
      if (flowPID.GetMode() != AUTOMATIC)
        flowPID.SetMode(AUTOMATIC);

      pumpInput = (double)flowRate;
      flowPID.Compute();
      controlActive = true;
    }
#endif
    if (controlActive)
    {
      char msgBuffer[10];
      dtostrf(pumpOutput, 4, 1, msgBuffer);
      setPumpPower((int)pumpOutput);
    }
    else
    {
      setPumpPower(100);
#ifdef HAS_PRESSURE_GAUGE
      pressurePID.SetMode(MANUAL);
#endif
#ifdef HAS_SCALE
      flowPID.SetMode(MANUAL);
#endif
    }
  }
  else
  {
    setPumpPower(100);
#ifdef HAS_PRESSURE_GAUGE
    pressurePID.SetMode(MANUAL);
#endif
#ifdef HAS_SCALE
    flowPID.SetMode(MANUAL);
#endif
  }
}

/**
 * @brief Calculates the current target setpoint for the pump based on the active profile.
 * Optimized for sequential, real-time execution during a shot.
 *
 * @param currentX The current progress of the shot (elapsed time in seconds OR total weight in grams).
 * @return The calculated target setpoint (Pressure in bar OR Flow in g/s).
 */
float getTargetAt(float currentX)
{
  if (currentProfile.numSteps == 0)
    return 0.0f;

  while (currentProfileStepIndex < currentProfile.numSteps)
  {
    ProfileStep &step = currentProfile.steps[currentProfileStepIndex];

    float stepDuration = step.trigger;
    float stepEndX = currentStepStartX + stepDuration;

    if (currentX >= stepEndX - 0.001f)
    {
      currentProfileStepIndex++;
      currentStepStartX = stepEndX;
      prevStepTargetY = step.setpoint;
      continue;
    }

    if (currentProfile.isStepped)
    {
      return step.setpoint;
    }

    else
    {
      if (stepDuration <= 0.001f)
        return step.setpoint;

      float ratio = (currentX - currentStepStartX) / stepDuration;

      ratio = constrain(ratio, 0.0f, 1.0f);

      return prevStepTargetY + ratio * (step.setpoint - prevStepTargetY);
    }
  }

  return prevStepTargetY;
}

void updateTempSwitch()
{
  if (ignoreTempSwitch)
    return;

  float targetTemp = 92.0;

  if (digitalRead(THREE_WAY_SWITCH1) == HIGH)
  {
    targetTemp = 90.0;
  }
  else if (digitalRead(THREE_WAY_SWITCH2) == HIGH)
  {
    targetTemp = 94.0;
  }

  if (abs(tempSetBrew - targetTemp) > 0.1)
  {
    tempSetBrew = targetTemp;
    updateCalculatedBoilerTemp();

    char msgBuffer[10];
    dtostrf(tempSetBrew, 4, 1, msgBuffer);
    publishData(mqtt_topic_set_temp_brew, msgBuffer, true);

    printToAll("Brew Temp changed via Switch to: ");
    printlnToAll(tempSetBrew);

    if (currentState == IDLE)
    {
      transitionToState(HEATING);
    }
  }
}

void updateBrewMode()
{
  static char lastBrewMode[7] = "";
  static bool lastSteamBoostState = !enableSteamBoost;

  if (!ignoreBrewSwitch)
  {
    if (digitalRead(TWO_WAY_SWITCH) == HIGH)
    {
      strcpy(brewMode, "STEAM");
      enableSteamBoost = true;
    }
    else
    {
      strcpy(brewMode, "COFFEE");
      enableSteamBoost = false;
    }
  }

  if (strcmp(brewMode, lastBrewMode) != 0)
  {
    publishData(mqtt_topic_brew_mode, brewMode, true);
    strcpy(lastBrewMode, brewMode);
    if (currentState == IDLE)
    {
      transitionToState(HEATING);
    }
  }

  if (enableSteamBoost != lastSteamBoostState)
  {
    publishData(mqtt_topic_steam_boost, enableSteamBoost ? "true" : "false", true);
    lastSteamBoostState = enableSteamBoost;
  }
}

bool standbyTimoutReached()
{
  if (currentState == IDLE && idleEntryTime > 0)
  {
    if (millis() - idleEntryTime >= STANDBY_TIMEOUT_MS)
    {
      return true;
    }
  }
  return false;
}

/**
 * @brief Checks if a programmatic (non-state-changing) flush is active.
 * This function is called every loop and will reset its own timer.
 * @return true if the flush is currently running, false otherwise.
 */
bool isProgrammaticFlushActive()
{
  if (programmaticFlushEndTime == 0)
  {
    return false;
  }

  if (millis() >= programmaticFlushEndTime)
  {
    programmaticFlushEndTime = 0;
    printlnToAll("Programmatic flush finished.");
    return false;
  }

  return true;
}

/**
 * @brief Starts a new programmatic (non-state-changing) cooling flush.
 * @param duration The length of the flush in milliseconds.
 */
void startProgrammaticFlush(unsigned long duration)
{
  if (currentState != HEATING && currentState != IDLE)
  {
    printlnToAll("Flush only allowed in HEATING or IDLE states.");
    return;
  }

  if (brewLeverLifted)
  {
    printlnToAll("Cannot start flush while brew lever is lifted.");
    return;
  }

  if (duration == 0)
  {
    printlnToAll("Flush duration must be greater than 0.");
    return;
  }

  unsigned long newEndTime = millis() + duration;

  if (programmaticFlushEndTime > millis())
  {
    printlnToAll("Flush already in progress. Extending duration.");
  }
  else
  {
    printToAll("Starting programmatic flush for ");
    printToAll(duration);
    printlnToAll("ms.");
  }

  programmaticFlushEndTime = newEndTime;
}

// ----------------------------------------------------------------
// --- Settings & Configuration ---
// ----------------------------------------------------------------

void saveSettings()
{
  printlnToAll("Saving settings to flash memory...");
  preferences.begin("espresso-app", false);

  // --- Save Connection Settings ---
  preferences.putString("mqttServer", mqtt_server);
  preferences.putInt("mqttPort", mqtt_port);
  preferences.putString("mqttUser", mqtt_user);
  preferences.putString("mqttPass", mqtt_password);

  if (ignoreTempSwitch)
  {
    preferences.putFloat("tempSetBrew", tempSetBrew);
  }
  preferences.putFloat("tempSetSteam", tempSetSteam);
  preferences.putFloat("tempSetSteamB", tempSetSteamBoost);

  preferences.putDouble("kp_temperature", kp_temperature);
  preferences.putDouble("ki_temperature", ki_temperature);
  preferences.putDouble("kd_temperature", kd_temperature);
#ifdef HAS_PRESSURE_GAUGE
  preferences.putDouble("kp_pressure", kp_pressure);
  preferences.putDouble("ki_pressure", ki_pressure);
  preferences.putDouble("kd_pressure", kd_pressure);
#endif
#ifdef HAS_SCALE
  preferences.putDouble("kp_flow", kp_flow);
  preferences.putDouble("ki_flow", ki_flow);
  preferences.putDouble("kd_flow", kd_flow);

  preferences.putFloat("flowKalmanMe", flowKalmanMe);
  preferences.putFloat("flowKalmanE", flowKalmanE);
  preferences.putFloat("flowKalmanQ", flowKalmanQ);
  preferences.putFloat("weightKalmanMe", weightKalmanMe);
  preferences.putFloat("weightKalmanE", weightKalmanE);
  preferences.putFloat("weightKalmanQ", weightKalmanQ);
#endif

  // Save MQTT override states
  if (ignoreBrewSwitch)
  {
    preferences.putString("brewMode", brewMode);
  }
  preferences.putBool("steamBoost", enableSteamBoost);
#ifdef HAS_SCALE
  preferences.putLong("scaleOffset", COMBINED_OFFSET);
  preferences.putFloat("scaleScale", COMBINED_SCALE);
#endif
  preferences.putString("profMode", profilingMode);
  preferences.putString("profSource", profilingSource);
  preferences.putString("profTarget", profilingTarget);

  preferences.putInt("prof_count", numProfilePoints);
  preferences.putFloat("profFlatVal", profilingFlatValue);

  preferences.putBytes("profile_blob", &currentProfile, sizeof(EspressoProfile));

  preferences.end();
  printlnToAll("Settings saved.");
}

void loadSettings()
{
  printlnToAll("Loading settings from flash memory...");
  preferences.begin("espresso-app", true);

  // --- Load Connection Settings ---
  if (preferences.getString("mqttServer", mqtt_server, sizeof(mqtt_server)) == 0)
    mqtt_server[0] = '\0';
  if (preferences.getString("mqttUser", mqtt_user, sizeof(mqtt_user)) == 0)
    mqtt_user[0] = '\0';
  if (preferences.getString("mqttPass", mqtt_password, sizeof(mqtt_password)) == 0)
    mqtt_password[0] = '\0';
  mqtt_port = preferences.getInt("mqttPort", 1883);
  enableSteamBoost = preferences.getBool("steamBoost", true);

  if (preferences.isKey("tempSetBrew"))
  {
    tempSetBrew = preferences.getFloat("tempSetBrew", 94.0);
    ignoreTempSwitch = true;
    printlnToAll("Loaded Brew Temp from Flash.");
  }
  else
  {
    ignoreTempSwitch = false;
    printlnToAll("No saved Brew Temp. Defaulting to Switch.");
  }

  updateCalculatedBoilerTemp();
  tempSetSteam = preferences.getFloat("tempSetSteam", 136.0);
  tempSetSteamBoost = preferences.getFloat("tempSetSteamB", 124.0);

  kp_temperature = preferences.getDouble("kp_temperature", 0.16);
  ki_temperature = preferences.getDouble("ki_temperature", 0.0000261);
  kd_temperature = preferences.getDouble("kd_temperature", 0);
#ifdef HAS_PRESSURE_GAUGE

  kp_pressure = preferences.getDouble("kp_pressure", 0.05);
  ki_pressure = preferences.getDouble("ki_pressure", 22);
  kd_pressure = preferences.getDouble("kd_pressure", 0.0);
#endif
#ifdef HAS_SCALE
  kp_flow = preferences.getDouble("kp_flow", 1.0);
  ki_flow = preferences.getDouble("ki_flow", 0.5);
  kd_flow = preferences.getDouble("kd_flow", 0.0);
  flowKalmanMe = preferences.getFloat("flowKalmanMe", 30.0);
  flowKalmanE = preferences.getFloat("flowKalmanE", 2.0);
  flowKalmanQ = preferences.getFloat("flowKalmanQ", 0.1);
  weightKalmanMe = preferences.getFloat("weightKalmanMe", 8.0);
  weightKalmanE = preferences.getFloat("weightKalmanE", 2.0);
  weightKalmanQ = preferences.getFloat("weightKalmanQ", 0.1);
#endif
  if (preferences.isKey("brewMode"))
  {
    preferences.getString("brewMode", brewMode, sizeof(brewMode));
    ignoreBrewSwitch = true;
    printlnToAll("Loaded Brew Mode from Flash.");
  }
  else
  {
    ignoreBrewSwitch = false;
    printlnToAll("No saved Brew Mode. Defaulting to Switch.");
    updateBrewMode();
  }

#ifdef HAS_PRESSURE_GAUGE
  if (preferences.getString("profMode", profilingMode, sizeof(profilingMode)) == 0)
    strcpy(profilingMode, "manual");
  if (preferences.getString("profSource", profilingSource, sizeof(profilingSource)) == 0)
    strcpy(profilingSource, "pressure");
  if (preferences.getString("profTarget", profilingTarget, sizeof(profilingTarget)) == 0)
    strcpy(profilingTarget, "time");
  profilingFlatValue = preferences.getFloat("profFlatVal", 100.0);
#elif defined(HAS_SCALE)
  if (preferences.getString("profMode", profilingMode, sizeof(profilingMode)) == 0)
    strcpy(profilingMode, "manual");
  if (preferences.getString("profSource", profilingSource, sizeof(profilingSource)) == 0)
    strcpy(profilingSource, "flow");
  if (preferences.getString("profTarget", profilingTarget, sizeof(profilingTarget)) == 0)
    strcpy(profilingTarget, "weight");
#else
  if (preferences.getString("profMode", profilingMode, sizeof(profilingMode)) == 0)
    strcpy(profilingMode, "manual");
  profilingSource[0] = '\0';
  profilingTarget[0] = '\0';
#endif
#ifdef HAS_SCALE
  COMBINED_OFFSET = preferences.getLong("scaleOffset", 0);
  COMBINED_SCALE = preferences.getFloat("scaleScale", 1.0);
#endif
  if (preferences.isKey("profile_blob"))
  {
    preferences.getBytes("profile_blob", &currentProfile, sizeof(EspressoProfile));
  }
  else
  {
    strcpy(currentProfile.name, "Standard Profile");
    currentProfile.isStepped = true;
    currentProfile.numSteps = 0;
  }

  printToAll("Profile loaded: ");
  printToAll(currentProfile.name);
  printToAll(" (");
  printToAll(currentProfile.numSteps);
  printlnToAll(" steps)");
  preferences.end();
  printlnToAll("Settings loaded.");
}

void publishProfile()
{
  if (currentProfile.numSteps == 0)
  {
    return;
  }
  JsonDocument doc;

  doc["n"] = currentProfile.name;
  doc["m"] = currentProfile.isStepped ? 1 : 0;

  JsonArray steps = doc["s"].to<JsonArray>();
  for (int i = 0; i < currentProfile.numSteps; i++)
  {
    JsonArray step = steps.add<JsonArray>();
    step.add(currentProfile.steps[i].setpoint);
    step.add(currentProfile.steps[i].trigger);
  }

  char jsonBuffer[1024];
  size_t n = serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
  publishData(mqtt_topic_profile_data, jsonBuffer, true, true);
}

void publishSingleSetting(const char *key, bool forceFlush)
{
  char msgBuffer[20];

  // --- Temperatures ---
  if (strcasecmp(key, "tempsetbrew") == 0)
  {
    dtostrf(tempSetBrew, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_temp_brew, msgBuffer, true, forceFlush);
  }
  else if (strcasecmp(key, "tempsetsteam") == 0)
  {
    dtostrf(tempSetSteam, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_temp_steam, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "tempsetsteamboost") == 0)
  {
    dtostrf(tempSetSteamBoost, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_temp_steam_boost, msgBuffer, true, forceFlush, true, false);

    // --- Modes ---
  }
  else if (strcasecmp(key, "brewmode") == 0)
  {
    publishData(mqtt_topic_brew_mode, brewMode, true, forceFlush);
  }
  else if (strcasecmp(key, "steamboost") == 0 || strcasecmp(key, "enablesteamboost") == 0)
  {
    publishData(mqtt_topic_steam_boost, enableSteamBoost ? "true" : "false", true, forceFlush);

    // --- PID ---
  }
  else if (strcasecmp(key, "kp") == 0 || strcasecmp(key, "kp_temperature") == 0)
  {
    dtostrf(kp_temperature, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_kp_temperature, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "ki") == 0 || strcasecmp(key, "ki_temperature") == 0)
  {
    dtostrf(ki_temperature, 4, 8, msgBuffer);
    publishData(mqtt_topic_set_ki_temperature, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "kd") == 0 || strcasecmp(key, "kd_temperature") == 0)
  {
    dtostrf(kd_temperature, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_kd_temperature, msgBuffer, true, forceFlush, true, false);
    // --- Pump PIDs (MQTT ONLY) ---
#ifdef HAS_PRESSURE_GAUGE
  }

  else if (strcasecmp(key, "kp_pressure") == 0)
  {
    dtostrf(kp_pressure, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_kp_pressure, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "ki_pressure") == 0)
  {
    dtostrf(ki_pressure, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_ki_pressure, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "kd_pressure") == 0)
  {
    dtostrf(kd_pressure, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_kd_pressure, msgBuffer, true, forceFlush, true, false);
#endif
#ifdef HAS_SCALE
  }
  else if (strcasecmp(key, "kp_flow") == 0)
  {
    dtostrf(kp_flow, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_kp_flow, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "ki_flow") == 0)
  {
    dtostrf(ki_flow, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_ki_flow, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "kd_flow") == 0)
  {
    dtostrf(kd_flow, 4, 3, msgBuffer);
    publishData(mqtt_topic_set_kd_flow, msgBuffer, true, forceFlush, true, false);

    // --- Kalman Filters (MQTT ONLY) ---
  }
  else if (strcasecmp(key, "flow_kalman_me") == 0)
  {
    dtostrf(flowKalmanMe, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_flow_kalman_me, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "flow_kalman_e") == 0)
  {
    dtostrf(flowKalmanE, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_flow_kalman_e, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "flow_kalman_q") == 0)
  {
    dtostrf(flowKalmanQ, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_flow_kalman_q, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "weight_kalman_me") == 0)
  {
    dtostrf(weightKalmanMe, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_weight_kalman_me, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "weight_kalman_e") == 0)
  {
    dtostrf(weightKalmanE, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_weight_kalman_e, msgBuffer, true, forceFlush, true, false);
  }
  else if (strcasecmp(key, "weight_kalman_q") == 0)
  {
    dtostrf(weightKalmanQ, 4, 2, msgBuffer);
    publishData(mqtt_topic_set_weight_kalman_q, msgBuffer, true, forceFlush, true, false);
#endif
    // --- Profiling ---
  }
  else if (strcasecmp(key, "prof_mode") == 0 || strcasecmp(key, "profiling_mode") == 0)
  {
    publishData(mqtt_topic_set_profiling_mode, profilingMode, true, forceFlush);
  }
  else if (strcasecmp(key, "prof_src") == 0 || strcasecmp(key, "profiling_source") == 0)
  {
    publishData(mqtt_topic_set_profiling_source, profilingSource, true, forceFlush);
  }
  else if (strcasecmp(key, "prof_trg") == 0 || strcasecmp(key, "profiling_target") == 0)
  {
    publishData(mqtt_topic_set_profiling_target, profilingTarget, true, forceFlush);
  }
  else if (strcasecmp(key, "prof_flat") == 0 || strcasecmp(key, "profiling_flat_value") == 0)
  {
    dtostrf(profilingFlatValue, 4, 1, msgBuffer);
    publishData(mqtt_topic_profile_flat, msgBuffer, true, forceFlush);
  }
  else if (strcasecmp(key, "mqtt_server") == 0)
  {
    publishData(mqtt_topic_mqtt_server, mqtt_server, true, forceFlush);
  }
  else if (strcasecmp(key, "mqtt_port") == 0)
  {
    itoa(mqtt_port, msgBuffer, 10);
    publishData(mqtt_topic_mqtt_port, msgBuffer, true, forceFlush);
  }
  else if (strcasecmp(key, "mqtt_user") == 0)
  {
    publishData(mqtt_topic_mqtt_user, mqtt_user, true, forceFlush);
  }
  else if (strcasecmp(key, "mqtt_pass") == 0 || strcasecmp(key, "mqtt_password") == 0)
  {
    publishData(mqtt_topic_mqtt_pass, mqtt_password, true, forceFlush);
  }
  else if (strcasecmp(key, "profile") == 0 || strcasecmp(key, "profile_data") == 0)
  {
    publishProfile();
  }
  else
  {
    printToAll("Warning: Unknown setting requested: ");
    printlnToAll(key);
  }
}

void publishSettings()
{
  static unsigned long lastPublishTime = 0;
  if (millis() - lastPublishTime < 5000)
    return;
  lastPublishTime = millis();

  if (!mqttClient.connected())
    return;

  publishSingleSetting("mqtt_server", false);
  publishSingleSetting("mqtt_port", false);
  publishSingleSetting("mqtt_user", false);
  publishSingleSetting("mqtt_pass", true);

  publishSingleSetting("profile");

  publishSingleSetting("tempsetbrew", false);
  publishSingleSetting("tempsetsteam", false);
  publishSingleSetting("tempsetsteamboost", false);
  publishSingleSetting("brewmode", false);
  publishSingleSetting("steamboost", true);

  publishSingleSetting("kp", false);
  publishSingleSetting("ki", false);
  publishSingleSetting("kd", true);

  publishSingleSetting("prof_mode", false);
  publishSingleSetting("prof_src", false);
  publishSingleSetting("prof_trg", false);
  publishSingleSetting("prof_flat", true);

  publishSingleSetting("kp_pressure", false);
  publishSingleSetting("ki_pressure", false);
  publishSingleSetting("kd_pressure", true);

  publishSingleSetting("kp_flow", false);
  publishSingleSetting("ki_flow", false);
  publishSingleSetting("kd_flow", true);

#ifdef HAS_SCALE
  publishSingleSetting("weight_kalman_me", false);
  publishSingleSetting("weight_kalman_e", false);
  publishSingleSetting("weight_kalman_q", true);

  publishSingleSetting("flow_kalman_me", false);
  publishSingleSetting("flow_kalman_e", false);
  publishSingleSetting("flow_kalman_q", true);
#endif

  printlnToAll("Full settings sync complete.");
}

void startNetworkServices()
{
#ifdef HAS_SCREEN
  printlnToAll("Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK)
  {
    printlnToAll("Error initializing ESP-NOW");
    return;
  }
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
#endif
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);

  mqttClient.setServer(mqtt_server, mqtt_port);
  if (strcmp(mqtt_user, "") != 0)
  {
    mqttClient.setCredentials(mqtt_user, mqtt_password);
  }
  mqttClient.setClientId(mqtt_client_id);
  if (strcmp(mqtt_server, "") != 0)
  {
    printlnToAll("Connecting to MQTT broker...");
    mqttClient.connect();
  }
  else
  {
    printlnToAll("MQTT server not set. Skipping initial connection.");
  }

  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();
  printlnToAll("OTA Ready");
}

void publishState()
{
  publishData(mqtt_topic_state, stateToString(currentState), true, false);
  publishData(mqtt_topic_lever, brewLeverLifted ? "LIFTED" : "DOWN", false, false);
  publishData(mqtt_topic_pump, pumpRunning ? "ON" : "OFF", false, false);
  publishData(mqtt_topic_heater, heaterOn ? "ON" : "OFF", false, true);
}

void generateSparseMap()
{
  printlnToAll("Generating sparse map for interpolation on boot...");

  for (int i = 0; i < LEGACY_TEMP_LOOKUP_TABLE_SIZE; i++)
  {
    int old_adc_val = i;
    float temp_times_10 = LEGACY_TEMP_LOOKUP_TABLE[i];

    float v_out_old = (float)old_adc_val * (VCC_OLD / OLD_ADC_MAX);
    float r_thermistor = (R_SERIES_OHMS_OLD * (VCC_OLD - v_out_old)) / v_out_old;

    float v_out_new = VCC_NEW * (R_SERIES_OHMS_NEW / (r_thermistor + R_SERIES_OHMS_NEW));
    float new_adc_val = (v_out_new / V_REF_NEW) * NEW_ADC_MAX;

    remappedAdcValues[i] = new_adc_val;
    remappedTemperatureValues[i] = temp_times_10 / 10.0;
  }
  printlnToAll("Sparse map generation complete.");
}
#ifdef HAS_SCALE
// =================================================================
// --- SCALE (ADS1232) FUNCTIONS ---
// =================================================================

/**
 * @brief Interrupt Service Routine (ISR) for the ADS1232.
 * Sets a flag when new data is ready to be read.
 */
void IRAM_ATTR dataReadyISR()
{
  portENTER_CRITICAL_ISR(&scaleMux);
  newDataReady = true;
  portEXIT_CRITICAL_ISR(&scaleMux);
}

/**
 * @brief Sends the current pcfState byte to the PCF8574 I2C expander.
 */
void updatePcf()
{
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(pcfState);
  Wire.endTransmission();
}

/**
 * @brief Reads the 24-bit raw data from the ADS1232.
 */
long readADCScale()
{
  if (digitalRead(ADS_DOUT_PIN) == HIGH)
    return -2;
  long reading = 0;
  portENTER_CRITICAL(&scaleMux);
  for (int i = 0; i < 24; i++)
  {
    digitalWrite(ADS_SCLK_PIN, HIGH);
    delayMicroseconds(1);
    reading <<= 1;
    if (digitalRead(ADS_DOUT_PIN))
    {
      reading |= 1;
    }
    digitalWrite(ADS_SCLK_PIN, LOW);
    delayMicroseconds(1);
  }
  digitalWrite(ADS_SCLK_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(ADS_SCLK_PIN, LOW);
  portEXIT_CRITICAL(&scaleMux);

  if (reading & 0x800000)
  {
    reading |= 0xFF000000;
  }
  if (reading == 0x7FFFFF)
  {
    updatePcf();
    return -1;
  }

  return reading;
}

// --- PCF8574 Helper Functions ---

void setGain(uint8_t gain)
{
  if (gain == 128)
  {
    bitSet(pcfState, PCF_GAIN1_BIT);
    bitSet(pcfState, PCF_GAIN0_BIT);
  }
  else
  {
    bitClear(pcfState, PCF_GAIN1_BIT);
    bitClear(pcfState, PCF_GAIN0_BIT);
  }
}
void setSpeed(bool high_speed)
{
  if (high_speed)
    bitSet(pcfState, PCF_SPEED_BIT);
  else
    bitClear(pcfState, PCF_SPEED_BIT);
}
void powerDown(bool power_down)
{
  if (power_down)
    bitClear(pcfState, PCF_PDWN_BIT);
  else
    bitSet(pcfState, PCF_PDWN_BIT);
}
void selectChannelScale(uint8_t channel)
{
  if (channel == 2)
    bitSet(pcfState, PCF_A0_BIT);
  else
    bitClear(pcfState, PCF_A0_BIT);
}

/**
 * @brief Non-blocking function to be called from the main loop()
 * to read the scale and calculate the final weight.
 */
void handleScale()
{

  if (newDataReady)
  {
    long raw_data = readADCScale();
    if (raw_data == -2 || raw_data == -1)
    {
      return;
    }
    loadCellValue = raw_data;
    float newRawWeight = (float)(raw_data - COMBINED_OFFSET) / COMBINED_SCALE;

    if (isFirstScaleReading)
    {
      lastRawWeight = newRawWeight;
      currentWeight = weightKalmanFilter.updateEstimate(newRawWeight);
      calculateFlowRate();
      isFirstScaleReading = false;
      isWeightPending = false;
    }
    else if (isWeightPending)
    {

      float confirmDelta = abs(newRawWeight - pendingWeight);

      if (confirmDelta < SPIKE_THRESHOLD_G)
      {
        currentWeight = weightKalmanFilter.updateEstimate(pendingWeight);
        currentWeight = weightKalmanFilter.updateEstimate(newRawWeight);
        calculateFlowRate();
        lastRawWeight = newRawWeight;
        isWeightPending = false;
      }
      else
      {
        float revertDelta = abs(newRawWeight - lastRawWeight);
        if (revertDelta < SPIKE_THRESHOLD_G)
        {
          currentWeight = weightKalmanFilter.updateEstimate(newRawWeight);
          calculateFlowRate();
          lastRawWeight = newRawWeight;
          isWeightPending = false;
        }
        else
        {
          pendingWeight = newRawWeight;
          isWeightPending = true;
        }
      }
    }
    else
    {
      float delta = abs(newRawWeight - lastRawWeight);

      if (delta < SPIKE_THRESHOLD_G)
      {
        currentWeight = weightKalmanFilter.updateEstimate(newRawWeight);
        calculateFlowRate();
        lastRawWeight = newRawWeight;
        isWeightPending = false;
      }
      else
      {
        pendingWeight = newRawWeight;
        isWeightPending = true;
      }
    }

    if (sendRawDebugData)
    {
      char msgBuffer[10];

      dtostrf(newRawWeight, 4, 1, msgBuffer);
      publishData("raw_weight", msgBuffer, false, false, false, true);

      dtostrf(currentWeight, 4, 1, msgBuffer);
      publishData("filtered_weight", msgBuffer, false, true, false, true);
    }
    bool postShotDrip = false;
    if (shotEndTime != 0)
    {
      if (millis() - shotEndTime < SHOT_POST_DRIP_DURATION_MS)
      {
        postShotDrip = true;
      }
      else
      {
        shotEndTime = 0;
      }
    }
    if ((brewLeverLifted && (currentState == BREWING || currentState == HEATING || currentState == IDLE)) || postShotDrip)
    {
      char msgBuffer[10];

      dtostrf(currentWeight, 4, 1, msgBuffer);
      publishData(mqtt_topic_weight, msgBuffer, false, false, false, true);

      dtostrf(flowRate, 4, 1, msgBuffer);
      publishData(mqtt_topic_flow_rate, msgBuffer, false, true, false, true);
    }

    portENTER_CRITICAL(&scaleMux);
    newDataReady = false;
    portEXIT_CRITICAL(&scaleMux);
  }
}

/**
 * @brief Reads a stable combined value from both channels by averaging.
 * This is a BLOCKING function and detaches the ISR.
 * @param times The number of reading cycles to average.
 * @return The averaged combined ADC value as a long.
 */
long getStableCombinedReadingADS1232(int times)
{
  detachInterrupt(digitalPinToInterrupt(ADS_DOUT_PIN));

  long total = 0;

  printlnToAll("  Settling and reading scale channel...");
  selectChannelScale(SCALE_CHANNEL);
  updatePcf();

  for (int i = 0; i < 4; i++)
  {
    while (digitalRead(ADS_DOUT_PIN) == HIGH)
    {
    }
    readADCScale();
  }

  for (int i = 0; i < times; i++)
  {
    while (digitalRead(ADS_DOUT_PIN) == HIGH)
    {
    }
    long val = readADCScale();
    if (val == -1)
    {
      i--;
      continue;
    }
    total += val;
  }
  long avg = total / times;
  printToAll("  Avg ADC: ");
  printlnToAll(avg);
  attachInterrupt(digitalPinToInterrupt(ADS_DOUT_PIN), dataReadyISR, FALLING);
  return avg;
}

void calculateFlowRate()
{
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTimeForFlowCalc;
  static bool resetKalman = false;
  bool postShotDrip = false;
  if (shotEndTime != 0)
  {
    if (millis() - shotEndTime < SHOT_POST_DRIP_DURATION_MS)
    {
      postShotDrip = true;
    }
    else
    {
      shotEndTime = 0;
    }
  }

  bool isShotActive = brewLeverLifted || postShotDrip;

  if (!isShotActive && !resetKalman)
  {
    flowRate = 0.0f;

    flowKalmanFilter.setEstimateError(flowKalmanE);
    flowKalmanFilter.reset();
    resetKalman = true;
  }
  else if (isShotActive)
  {

    float rawFlowRate = 0.0f;
    if (timeDelta > 0)
    {
      float weightDelta = currentWeight - previousWeightForFlowCalc;
      rawFlowRate = (weightDelta / (float)timeDelta) * 1000.0f;
      rawFlowRate = max(0.0f, rawFlowRate);
    }
    resetKalman = false;
    flowRate = flowKalmanFilter.updateEstimate(rawFlowRate);
    previousTimeForFlowCalc = currentTime;
    previousWeightForFlowCalc = currentWeight;
  }

  if (sendRawDebugData)
  {
    char msgBuffer[10];
    dtostrf(flowRate, 4, 1, msgBuffer);
    publishData("filtered_flow", msgBuffer, false, true, false, true);
  }
}

/**
 * @brief A blocking function to be called from Telnet for tare-ing the scale.
 */
void tareScale()
{
  printlnToAll("Starting scale tare...");
  weightKalmanFilter.reset();
  flowKalmanFilter.reset();
  weightKalmanFilter.setEstimateError(weightKalmanE);
  flowKalmanFilter.setEstimateError(flowKalmanE);
  currentWeight = 0.0;
  flowRate = 0.0f;
  lastRawWeight = 0.0;
  isFirstScaleReading = true;
  previousWeightForFlowCalc = 0.0;
  previousTimeForFlowCalc = millis();
  publishData(mqtt_topic_weight, "0.0", false, true);

  isFirstScaleReading = true;
  isWeightPending = false;
  pendingWeight = 0.0;
  lastRawWeight = 0.0;

  long offset_value = getStableCombinedReadingADS1232(16);
  COMBINED_OFFSET = offset_value;

  printToAll("Scale tare complete. New offset: ");
  printlnToAll(COMBINED_OFFSET);
}
/**
 * @brief Prepares the machine for calibration.
 * Can be called from any input source.
 */
void startCalibration()
{
  if (currentState != DEBUG && currentState != IDLE && currentState != HEATING)
  {
    printlnToAll("Error: Calibration can only be started from IDLE, HEATING, or DEBUG states.");
    return;
  }
  calibrationReturnState = currentState;
  transitionToState(CALIBRATION_EMPTY);
}

/**
 * @brief Performs the next step (tare or weigh) in the calibration process.
 * Can be called from any input source.
 */
void handleCalibrationStep(float weight)
{
  if (currentState == CALIBRATION_EMPTY)
  {
    printlnToAll("Taring... This will take a few seconds.");

    long offset_value = getStableCombinedReadingADS1232(16);
    COMBINED_OFFSET = offset_value;
    printToAll("Tare complete. New offset: ");
    printlnToAll(COMBINED_OFFSET);
    transitionToState(CALIBRATION_TEST_WEIGHT);
  }
  else if (currentState == CALIBRATION_TEST_WEIGHT)
  {
    calibrationWeight = weight;
    printlnToAll("--- Starting Scale Calibration ---");
    if (weight <= 0)
    {
      printlnToAll("Error: Invalid weight. Must be a positive number.");
      return;
    }
    printToAll("Known test weight set to: ");
    printToAll(calibrationWeight);
    printlnToAll("g");

    printlnToAll("Weighing... This will take a few seconds.");

    long reading_with_weight = getStableCombinedReadingADS1232(16);
    float scale_value = (float)(reading_with_weight - COMBINED_OFFSET) / calibrationWeight;
    COMBINED_SCALE = scale_value;
    saveSettings();
    printToAll("Weighing complete. New scale value: ");
    printlnToAll(scale_value, 4);
    printlnToAll("----------------------------------------------------");
    printlnToAll("Calibration complete! Returning to HEATING state.");

    updatePcf();
    newDataReady = false;

    transitionToState(calibrationReturnState);
  }
  else
  {
    printlnToAll("Error: 'next' command ignored. Not in calibration mode.");
  }
}
#endif
// ----------------------------------------------------------------
// --- CORE EXECUTION (SETUP & LOOP) ---
// ----------------------------------------------------------------

void setup()
{
  delay(1000);
  printlnToAll("Configuring digital input pins...");
  for (const int pin : digitalInputs)
  {
    pinMode(pin, INPUT);
  }
  pinMode(D0, OUTPUT);
  pinMode(ADS_SCLK_PIN, INPUT_PULLUP);
  pinMode(ADS_DOUT_PIN, OUTPUT);
  digitalWrite(D0, HIGH);
  printlnToAll("Configuring digital output pins...");
  for (const int pin : startupOutputPins)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  pinMode(LEDMAIN, OUTPUT);
  digitalWrite(LEDMAIN, HIGH);

  generateSparseMap();
  loadSettings();
  updateCalculatedBoilerTemp();
  printlnToAll("Initializing ADS1115...");
  Wire.begin(A4, A5);
  if (!ads.begin())
  {
    printlnToAll("Failed to initialize ADS1115. Check wiring.");
  }
  else
  {
    printlnToAll("ADS1115 initialized successfully.");
    ads.setGain(GAIN_ONE);
  }
#ifdef HAS_SCALE
  printlnToAll("Initializing Scale (ADS1232)...");
  Wire.setClock(400000);

  pinMode(ADS_SCLK_PIN, OUTPUT);
  digitalWrite(ADS_SCLK_PIN, LOW);
  pinMode(ADS_DOUT_PIN, INPUT_PULLUP);

  powerDown(true);
  delay(100);
  setSpeed(false);
  setGain(128);
  selectChannelScale(SCALE_CHANNEL);
  bitClear(pcfState, PCF_TEMP_BIT);
  updatePcf();
  delay(100);
  powerDown(true);
  updatePcf();
  delay(100);
  powerDown(false);
  updatePcf();

  attachInterrupt(digitalPinToInterrupt(ADS_DOUT_PIN), dataReadyISR, FALLING);
  printlnToAll("Scale initialized.");
#endif
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(PUMP_TRIAC_PIN, OUTPUT);

#ifdef HAS_PRESSURE_GAUGE
  DimmableLight::setSyncPin(ZERO_CROSS_PIN);
  DimmableLight::setSyncPullup(false);
  DimmableLight::setSyncDir(RISING);
  DimmableLight::begin();
#else
  pinMode(PUMP_TRIAC_PIN, OUTPUT);
  digitalWrite(PUMP_TRIAC_PIN, HIGH);
#endif

  heaterPID.SetSampleTime(1000);
  heaterPID.SetOutputLimits(-100, 100);
  heaterPID.SetMode(AUTOMATIC);

#ifdef HAS_PRESSURE_GAUGE
  pressurePID.SetTunings(kp_pressure, ki_pressure, kd_pressure);
  pressurePID.SetSampleTime(50);
  pressurePID.SetOutputLimits(50, 100);
  pressurePID.SetMode(AUTOMATIC);
#endif
#ifdef HAS_SCALE
  flowPID.SetTunings(kp_flow, ki_flow, kd_flow);
  flowPID.SetSampleTime(50);
  flowPID.SetOutputLimits(50, 100);
  flowPID.SetMode(AUTOMATIC);
  flowKalmanFilter.setMeasurementError(flowKalmanMe);
  flowKalmanFilter.setEstimateError(flowKalmanE);
  flowKalmanFilter.setProcessNoise(flowKalmanQ);
  weightKalmanFilter.setMeasurementError(weightKalmanMe);
  weightKalmanFilter.setEstimateError(weightKalmanE);
  weightKalmanFilter.setProcessNoise(weightKalmanQ);
#endif

  wm.setConfigPortalBlocking(false);
  wm.setTimeout(0);

  if (wm.autoConnect("MaraX-Setup"))
  {
    printToAll("Connected to WiFi: ");
    printlnToAll(WiFi.SSID());
    isOnline = true;
    WiFi.mode(WIFI_AP_STA);
  }
  else
  {
    printlnToAll("WiFi not connected. Running in offline mode (AP might be active in background).");
    isOnline = false;
  }

  if (!isOnline)
  {
    printlnToAll("Offline Mode: Reverting to physical switches.");
    ignoreTempSwitch = false;
    ignoreBrewSwitch = false;
  }

  if (isOnline)
  {
    startNetworkServices();
  }

  telnetServer.begin();
  printToAll("Telnet server started. Connect to ");
  printToAll(WiFi.localIP());
  printlnToAll(" on port 23.");
  printlnToAll("Type 'help' for a list of commands.");
  pollDigitalInputs();
  updateBrewMode();
  updateTempSwitch();
  updateSensorReadings();
#ifdef HAS_SCALE
  tareScale();
#endif

  waterLevelTripped = !readPin(WATER_DETECTOR);
  isBoilerEmpty = !detectBoilerLevel();
  lastBoilerCheckTime = millis();
  setPumpPower(100);

  printlnToAll("Detecting initial state...");
  if (digitalRead(BREW_SWITCH) == LOW)
  {
    transitionToState(DEBUG);
    allStop();
  }
  else if (checkCriticalSensorFailure())
  {
    transitionToState(ERROR);
  }
  else if (waterLevelTripped)
  {
    transitionToState(WATER_EMPTY);
  }
  else if (isBoilerEmpty)
  {
    transitionToState(BOILER_EMPTY);
  }
  else
  {
    transitionToState(INIT);
  }
}

void loop()
{
  unsigned long nowTime = millis();

  wm.process();

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!isOnline)
    {
      printlnToAll("WiFi Connected!");
      isOnline = true;
      startNetworkServices();
    }
  }
  else
  {
    if (isOnline)
    {
      printlnToAll("WiFi Lost!");
      isOnline = false;
    }
  }

  if (isBeeping && nowTime >= beepStopTime)
  {
    digitalWrite(BUZZER, LOW);
    isBeeping = false;
  }
  if (isOnline)
  {
    ArduinoOTA.handle();

#ifdef HAS_SCREEN
    if (nowTime - lastEspNowSendTime > ESP_NOW_SEND_INTERVAL_MS)
    {
      sendEspNowBuffer();
      lastEspNowSendTime = nowTime;
    }
#endif
  }
  handleTelnet();
  pollDigitalInputs();
  updateSensorReadings();
#ifdef HAS_SCALE
  handleScale();
#endif
  periodicBoilerLevelCheck();
  updateBrewMode();
  updateTempSwitch();

  bool errorState = false;
  if (currentState != DEBUG)
  {
    if (checkCriticalSensorFailure())
    {
      transitionToState(ERROR);
      errorState = true;
    }
    else if (waterLevelTripped)
    {
      if (currentState != WATER_EMPTY)
      {
        if (currentState == CLEANING_START || currentState == CLEANING_PUMPING || currentState == CLEANING_PAUSE)
        {
          cleaningStateToResume = currentState;
          printlnToAll("Water empty during cleaning. Pausing cycle...");
        }
        else
        {
          cleaningStateToResume = IDLE;
        }
      }
      transitionToState(WATER_EMPTY);
      errorState = true;
    }
    else if (isBoilerEmpty && !waterLevelTripped)
    {
      transitionToState(BOILER_EMPTY);
      errorState = true;
    }
  }

  switch (currentState)
  {
  case INIT:
    transitionToState(HEATING);
    break;
  case ERROR:
    errorBuzzer();
    ledError();
    if (!checkCriticalSensorFailure() && !errorState)
    {
      digitalWrite(BUZZER, LOW);
      transitionToState(HEATING);
      break;
    }
    break;
  case WATER_EMPTY:
    waterEmptyBuzzer();
    ledWaterEmpty();
    if (!errorState)
    {
      digitalWrite(BUZZER, LOW);
      if (cleaningStateToResume != IDLE)
      {
        printlnToAll("Water refilled. Resuming cleaning cycle.");
        transitionToState(cleaningStateToResume);
        cleaningStateToResume = IDLE;
      }
      else
      {
        transitionToState(HEATING);
      }
      break;
    }
    break;
  case BOILER_EMPTY:
  {
    if (brewLeverLifted)
    {
      setBoilerFillValve(false);
      setPump(false);
      break;
    }
    if (!brewLeverLifted)
    {
      setBoilerFillValve(true);
      setPump(true);
    }
    bool waterDetected = readPin(BOILER_LEVEL);

    if (waterDetected)
    {
      if (boilerFullTimestamp == 0)
      {
        boilerFullTimestamp = nowTime;
      }
      if (!errorState && (nowTime - boilerFullTimestamp >= BOILER_OVERFILL_DURATION_MS))
      {
        transitionToState(HEATING);
      }
    }
    else
    {
      boilerFullTimestamp = 0;
    }
    break;
  }
  case HEATING:
  {
    runHeaterPID();
    ledHeating();
    if (brewLeverLifted)
    {
      if (programmaticFlushEndTime > 0)
      {
        programmaticFlushEndTime = 0;
        printlnToAll("Programmatic flush cancelled by lever.");
      }
      transitionToState(BREWING);
      break;
    }
    bool flushRunning = isProgrammaticFlushActive();
    if (flushRunning)
    {
      setPumpPower(100);
      setBoilerFillValve(false);
      setPump(true);
      break;
    }
    setBoilerFillValve(false);
    setPump(false);
    if (isStable())
    {
      bool heatingModeCoffee = strcmp(brewMode, "STEAM");

      if (heatingModeCoffee)
      {
        printlnToAll("Temperature stable -> IDLE.");
        transitionToState(IDLE);
      }
      else
      {
        transitionToState(COOLING_FLUSH);
      }
      break;
    }
    break;
  }
  case COOLING_FLUSH:
  {
    if (nowTime - flushStartTime >= COOLING_FLUSH_DURATION_MS)
    {
      setBoilerFillValve(false);
      setPump(false);
      transitionToState(IDLE);
      coolingFlushEndTime = nowTime;
      break;
    }
    break;
  }
  case IDLE:
  {
    runHeaterPID();
    ledIdle();
    bool heatingModeCoffee = strcmp(brewMode, "STEAM");
    if (heatingModeCoffee && abs(pidInput - pidSetpoint) > TEMP_STABILITY_TOLERANCE && nowTime - coolingFlushEndTime > 60000)
    {
      printlnToAll("Temperature unstable. Returning to HEATING state.");
      transitionToState(HEATING);
      break;
    }
    if (brewLeverLifted)
    {
      if (programmaticFlushEndTime > 0)
      {
        programmaticFlushEndTime = 0;
        printlnToAll("Programmatic flush cancelled by lever.");
      }
      transitionToState(BREWING);
      break;
    }
    bool flushRunning = isProgrammaticFlushActive();
    if (flushRunning)
    {
      setPumpPower(100);
      setBoilerFillValve(false);
      setPump(true);
      break;
    }
    setBoilerFillValve(false);
    setPump(false);
    if (standbyTimoutReached())
    {
      transitionToState(STANDBY);
      break;
    }
    if (!heatingModeCoffee)
    {
      if (nowTime - idleEntryTime >= PERIODIC_FLUSH_INTERVAL_MS)
      {
        printlnToAll("Periodic flush triggered");
        transitionToState(COOLING_FLUSH);
        break;
      }
    }
    break;
  }
  case BREWING:
    runHeaterPID();
    ledBrew();
    runPumpProfile();

    if (!brewLeverLifted)
    {
      if (enableSteamBoost)
      {
        transitionToState(STEAM_BOOST);
      }
      else
      {
        bool heatingModeCoffee = strcmp(brewMode, "STEAM");

        if (!heatingModeCoffee)
        {
          transitionToState(IDLE);
        }
        else
        {
          transitionToState(HEATING);
        }
      }
    }
    break;
  case STEAM_BOOST:
  {
    runHeaterPID();
    if (nowTime - steamBoostEntryTime >= STEAM_BOOST_DURATION_MS)
    {
      bool heatingModeCoffee = strcmp(brewMode, "STEAM");

      if (!heatingModeCoffee)
      {
        transitionToState(IDLE);
      }
      else
      {
        transitionToState(HEATING);
      }
    }
    break;
  }
  case STANDBY:
    ledStandby();
    if (brewLeverLifted)
    {
      leverLiftedInStandby = true;
    }

    if (leverLiftedInStandby && !brewLeverLifted)
    {
      transitionToState(HEATING);
    }
    break;
  case CLEANING_START:
  {
    runHeaterPID();
    ledIdle();

    if (brewLeverLifted)
    {
      printlnToAll("Cleaning cycle 1/10: Lever lifted, starting pump.");
      cleaningPumpStartTime = nowTime;
      cleaningBeepState = 0;
      transitionToState(CLEANING_PUMPING);
    }
    break;
  }
  case CLEANING_PUMPING:
  {
    runHeaterPID();
    ledIdle();

    const unsigned long PUMP_DURATION_MS = 10000;
    const int BEEP_DURATION_MS = 150;
    const int BEEP_PAUSE_MS = 150;

    if (cleaningBeepState == 0 && (nowTime - cleaningPumpStartTime >= PUMP_DURATION_MS))
    {
      cleaningBeepState = 1;
      cleaningBeepTimer = nowTime;
      if (BUZZER_ENABLE)
        digitalWrite(BUZZER, HIGH);
    }

    if (cleaningBeepState == 1)
    {
      if (nowTime - cleaningBeepTimer >= BEEP_DURATION_MS)
      {
        digitalWrite(BUZZER, LOW);
        cleaningBeepTimer = nowTime;
        if (cleaningRepetitionCounter == 4)
        {
          cleaningBeepState = 2;
        }
        else
        {
          cleaningBeepState = 4;
        }
      }
    }
    else if (cleaningBeepState == 2)
    {
      if (nowTime - cleaningBeepTimer >= BEEP_PAUSE_MS)
      {
        if (BUZZER_ENABLE)
          digitalWrite(BUZZER, HIGH);
        cleaningBeepTimer = nowTime;
        cleaningBeepState = 3;
      }
    }
    else if (cleaningBeepState == 3)
    {
      if (nowTime - cleaningBeepTimer >= BEEP_DURATION_MS)
      {
        digitalWrite(BUZZER, LOW);
        cleaningBeepState = 4;
      }
    }

    if (!brewLeverLifted)
    {
      setPump(false);
      digitalWrite(BUZZER, LOW);
      cleaningRepetitionCounter++;

      if (cleaningRepetitionCounter >= TOTAL_CLEANING_REPETITIONS)
      {
        printlnToAll("Cleaning cycle finished.");
        cleaningRepetitionCounter = 0;
        cleaningStateToResume = IDLE;

        bool heatingModeCoffee = strcmp(brewMode, "STEAM");

        if (!heatingModeCoffee)
        {
          transitionToState(IDLE);
        }
        else
        {
          transitionToState(HEATING);
        }
      }
      else
      {
        printToAll("Cleaning rep ");
        printToAll(cleaningRepetitionCounter);
        printlnToAll(" complete. Pull lever for next.");
        transitionToState(CLEANING_PAUSE);
      }
    }
    break;
  }
  case CLEANING_PAUSE:
  {
    runHeaterPID();
    ledIdle();

    if (brewLeverLifted)
    {
      printToAll("Cleaning cycle ");
      printToAll(cleaningRepetitionCounter + 1);
      printToAll("/");
      printToAll(TOTAL_CLEANING_REPETITIONS);
      printlnToAll(": Lever lifted, starting pump.");
      cleaningPumpStartTime = nowTime;
      cleaningBeepState = 0;
      transitionToState(CLEANING_PUMPING);
    }
    break;
  }
  case CALIBRATION_EMPTY:
  case CALIBRATION_TEST_WEIGHT:
    runHeaterPID();
    break;
  case DEBUG:
    if (manualHeaterControl)
    {
      runHeaterPID();
    }
    break;
  }

  static unsigned long lastPrintTime = 0;
  if (nowTime - lastPrintTime > 1000)
  {

    char msgBuffer[10];
    dtostrf(boilerTemp, 4, 2, msgBuffer);
    publishData(mqtt_topic_boiler_temp, msgBuffer, false, false);
    dtostrf(hxTemp, 4, 2, msgBuffer);
    publishData(mqtt_topic_hx_temp, msgBuffer, false, false);
#ifdef HAS_PRESSURE_GAUGE
    dtostrf(pressure, 4, 2, msgBuffer);
    publishData(mqtt_topic_pressure, msgBuffer, false, false);
#endif
#ifdef HAS_SCALE
    dtostrf(currentWeight, 4, 1, msgBuffer);
    publishData(mqtt_topic_weight, msgBuffer, false, false);
    dtostrf(flowRate, 4, 1, msgBuffer);
    publishData(mqtt_topic_flow_rate, msgBuffer, false, false);
#endif
    publishState();

    lastPrintTime = nowTime;
  }
}
