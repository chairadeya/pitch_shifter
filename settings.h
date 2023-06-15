/* Beberapa kombinasi suara akan gagal diUpload dikarenakan tidak cukupnya ruang di Mikrokontroler chip ATmega328 
 *  Contoh: Start suara (ScaniaV8Start.h) dikombinasikan dengan file (chevyNovaV8.h) Akan Error "Sketch too big, Error compiling for board Arduino Nano"
 *  Solusinya bisa kalian coba satu-persatu kombinasinya dan Start suara (UralV8Start.h) adalah suara start paling kecil dan file (LaFerrari.h) file paling kecil
*/

/* =======================================================================================================
   Suara Start (Hapus tanda "//" pada awal baris untuk memilih) 
   =======================================================================================================*/
//#include "ScaniaV8Start.h" // Scania V8 Start
#include "UralV8Start.h" // Ural 4320 V8 Start
//#include "DefenderV8Start.h" // Land Rover Defender V8 Start

/* =======================================================================================================
   Suara Utama (Hapus tanda "//" pada awal baris untuk memilih) 
   =======================================================================================================*/
//#include "diesel.h"             // Truk Diesel Lawas
//#include "ScaniaV8Idle.h"       // Scania V8
//#include "UralV8Idle.h"         // Ural 4320 V8
//#include "DefenderV8Idle.h"     // Land Rover Defender V8
//#include "v8.h"                 // Generic V8
#include "chevyNovaV8.h"        // Chevy Nova Coupe 1975 
//#include "Mustang68.h"          // Ford Mustang 1968
//#include "MgBGtV8.h"            // MG B GT V8
//#include "LaFerrari.h"          // Ferrari "LaFerrari"
//#include "TrophyTruckIdle.h"      // V8 Trophy Truk


// PWM Throttle range calibration -----------------------------------------------------------------------------------
int16_t pulseZero = 2000; 
int16_t pulseNeutral = 0; 
int16_t pulseSpan = 1000; 
int16_t pulseLimit = 700;
boolean engineManualOnOff = false; 

// Engine RPM range
#define TOP_SPEED_MULTIPLIER 30 // RPM multiplier: semakin besar angkanya semakin lama ketukan RPM nya, 20 - 50 Recommended
const int16_t maxRpm = 1023; 
const int16_t minRpm = 0; 

// Engine mass simulation
const int8_t acc = 9; // Acceleration step per 5ms
const int8_t dec = 6; // Deceleration step per 5ms
