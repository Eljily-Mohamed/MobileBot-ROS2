#include "main.h"
#include <unistd.h>
#include <stdint.h>
#include "VL53L0X.h"

//https://github.com/pololu/vl53l0x-arduino/blob/master/VL53L0X.cpp
//---------------------------------------------------------
// Local variables within this file (private)
//---------------------------------------------------------
uint8_t g_i2cAddr = ADDRESS_DEFAULT;
uint8_t g_stopVariable; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API

uint16_t g_timeoutStartMs;
uint16_t g_ioTimeout;
bool did_timeout;

//---------------------------------------------------------
// Locally used functions (private)
//---------------------------------------------------------
uint8_t VL53L0X_performSingleRefCalibration(uint8_t vhv_init_byte);

uint8_t millis()
{

return 0;
}


//---------------------------------------------------------
// I2C communication Functions
//---------------------------------------------------------
// Write an 8-bit register
void writeReg(uint8_t reg, uint8_t value) {
	i2c1_WriteRegBuffer(0x53,reg,&value,1);
}
// Write a 16-bit register
void writeReg16Bit(uint8_t reg, uint16_t value){
	uint8_t tab[2];
	tab[0]= ((value >> 8));
	tab[1] = ((value ) & 0xFF);
	i2c1_WriteRegBuffer(0x53,reg,tab,2);
}
// Write a 32-bit register
void writeReg32Bit(uint8_t reg, uint32_t value){
	uint8_t tab[4];
		tab[3]= ((value >> 24) & 0xFF);
		tab[2]= ((value >> 16) & 0xFF);
		tab[1]= ((value >> 8) & 0xFF);
		tab[0] = ((value ) & 0xFF);
		i2c1_WriteRegBuffer(0x53,reg,tab,4);
}
// Read an 8-bit register
uint8_t readReg(uint8_t reg) {
  	uint8_t value=0;
  	i2c1_ReadRegBuffer(0x53,reg,&value,1);
  	return value;
}
// Read a 16-bit register
uint16_t readReg16Bit(uint8_t reg) {
	uint8_t tab[2];
	i2c1_ReadRegBuffer(0x53,reg,tab,2);
  	uint16_t value= ((uint16_t)tab[0] << 8) | (uint16_t)tab[1];
  	return value;
}
// Read a 32-bit register
uint32_t readReg32Bit(uint8_t reg) {
  uint8_t tab[4];
  i2c1_ReadRegBuffer(0x53,reg,tab,4);
  uint32_t value= (tab[3] << 24) | (tab[2] << 16 ) | (tab[1] << 8) | tab[0];
  return value;
}
// Write an arbitrary number of bytes from the given array to the sensor,
// starting at the given register
void writeMulti(uint8_t reg, uint8_t const *src, uint8_t count){
  while ( count-- > 0 ) {
    i2c1_WriteRegBuffer(0x53,reg,(uint8_t *)src,1);
  }
}



// Read an arbitrary number of bytes from the sensor, starting at the given
// register, into the given array
void readMulti(uint8_t reg, uint8_t * dst, uint8_t count)
{
	uint8_t value=0;

  while (count-- > 0)
  {
	  i2c1_ReadRegBuffer(0x53,reg,&value,1);
    *(dst++) = value;
  }
}
//===========================================================================
void VL53L0X_setAddress(uint8_t new_addr) {
  writeReg( I2C_SLAVE_DEVICE_ADDRESS, (new_addr>>1) & 0x7F );
  g_i2cAddr = new_addr;
}
//===========================================================================
uint8_t VL53L0X_getAddress() {
  return g_i2cAddr;
}
//===========================================================================
uint8_t VL53L0X_validateInterface()
{
	uint16_t var=0;
	var = readReg(0xC0);
	if ( var == 0xEE)
	{
		var =readReg(0xC1);
		if ( var == 0xAA)
		{
			var =readReg(0xC2);
			if ( var == 0x10)
			{
				var =readReg16Bit(0x51);
				if ( var == 0x0096) // doc 0x0099 ??
				{
					var =readReg16Bit(0x61);
					if ( var == 0x0000)
					{
						return 0;
					}
				}
			}
		}
	}
	return -1;
}
//===========================================================================
// Initialize sensor using sequence based on VL53L0X_DataInit(),
// VL53L0X_StaticInit(), and VL53L0X_PerformRefCalibration().
// This function does not perform reference SPAD calibration
// (VL53L0X_PerformRefSpadManagement()), since the API user manual says that it
// is performed by ST on the bare modules; it seems like that should work well
// enough unless a cover glass is added.
// If io_2v8 (optional) is true or not given, the sensor is configured for 2V8
// mode.
uint8_t VL53L0X_init( ){
  //****************************************************************************
  // DATA INIT
  // "Set I2C standard mode"
  writeReg(I2C_MODE, 0x00);
  writeReg(POWER_MANAGEMENT_GO1_POWER_FORCE, 0x01);
  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(SYSRANGE_START, 0x00);
  g_stopVariable = readReg(0x91);
  writeReg(SYSRANGE_START, 0x01);
  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(POWER_MANAGEMENT_GO1_POWER_FORCE, 0x00);

  // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
  writeReg(MSRC_CONFIG_CONTROL, readReg(MSRC_CONFIG_CONTROL) | 0x12);

  // set final range signal rate limit to 0.25 MCPS (million counts per second)
  VL53L0X_setSignalRateLimit(0.25);

  writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);
  //****************************************************************************
  // STATIC INIT
  // The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
  // the API, but the same data seems to be more easily readable from
  // GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
  uint8_t spad_count;
  bool spad_type_is_aperture;
  if (!VL53L0X_getSpadInfo(&spad_count, &spad_type_is_aperture)) { return false; }

  // The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
  // the API, but the same data seems to be more easily readable from
  // GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
  uint8_t ref_spad_map[6];
  readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
  // -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)
  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
  writeReg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);
  // -- VL53L0X_set_reference_spads() end
  //****************************************************************************
  // LOAD TUNING SETTINGS
  // DefaultTuningSettings from vl53l0x_tuning.h

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(SYSRANGE_START, 0x00);

  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(SYSTEM_RANGE_CONFIG, 0x00);
  writeReg(0x10, 0x00);
  writeReg(0x11, 0x00);

  writeReg(0x24, 0x01);
  writeReg(0x25, 0xFF);
  writeReg(0x75, 0x00);

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(0x4E, 0x2C);
  writeReg(0x48, 0x00);
  writeReg(0x30, 0x20);

  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(0x30, 0x09);
  writeReg(0x54, 0x00);
  writeReg(0x31, 0x04);
  writeReg(0x32, 0x03);
  writeReg(0x40, 0x83);
  writeReg(0x46, 0x25);
  writeReg(0x60, 0x00);
  writeReg(0x27, 0x00);
  writeReg(0x50, 0x06);
  writeReg(0x51, 0x00);
  writeReg(0x52, 0x96);
  writeReg(0x56, 0x08);
  writeReg(0x57, 0x30);
  writeReg(0x61, 0x00);
  writeReg(0x62, 0x00);
  writeReg(0x64, 0x00);
  writeReg(0x65, 0x00);
  writeReg(0x66, 0xA0);

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(0x22, 0x32);
  writeReg(0x47, 0x14);
  writeReg(0x49, 0xFF);
  writeReg(0x4A, 0x00);

  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(0x7A, 0x0A);
  writeReg(0x7B, 0x00);
  writeReg(0x78, 0x21);

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(0x23, 0x34);
  writeReg(0x42, 0x00);
  writeReg(0x44, 0xFF);
  writeReg(0x45, 0x26);
  writeReg(0x46, 0x05);
  writeReg(0x40, 0x40);
  writeReg(0x0E, 0x06);
  writeReg(0x20, 0x1A);
  writeReg(0x43, 0x40);

  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(0x34, 0x03);
  writeReg(0x35, 0x44);

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(0x31, 0x04);
  writeReg(0x4B, 0x09);
  writeReg(0x4C, 0x05);
  writeReg(0x4D, 0x04);

  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(0x44, 0x00);
  writeReg(0x45, 0x20);
  writeReg(0x47, 0x08);
  writeReg(0x48, 0x28);
  writeReg(0x67, 0x00);
  writeReg(0x70, 0x04);
  writeReg(0x71, 0x01);
  writeReg(0x72, 0xFE);
  writeReg(0x76, 0x00);
  writeReg(0x77, 0x00);

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(0x0D, 0x01);

  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(POWER_MANAGEMENT_GO1_POWER_FORCE, 0x01);
  writeReg(0x01, 0xF8);

  writeReg(INTERNAL_TUNING_2, 0x01);
  writeReg(0x8E, 0x01);
  writeReg(SYSRANGE_START, 0x01);
  writeReg(INTERNAL_TUNING_2, 0x00);
  writeReg(POWER_MANAGEMENT_GO1_POWER_FORCE, 0x00);
  //***************************************************************************
  // "Set interrupt config to new sample ready"
  // -- VL53L0X_SetGpioConfig() begin
  writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
  writeReg(GPIO_HV_MUX_ACTIVE_HIGH, readReg(GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10); // active low
  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
  //***************************************************************************
  // "Disable MSRC and TCC by default"
  // MSRC = Minimum Signal Rate Check
  // TCC = Target CentreCheck
  // -- VL53L0X_SetSequenceStepEnable() begin
  writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
  //***************************************************************************
  // VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())
  // -- VL53L0X_perform_vhv_calibration() begin
  writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
  if (!VL53L0X_performSingleRefCalibration(0x40)) { return false; }
  //***************************************************************************
  // -- VL53L0X_perform_phase_calibration() begin
  writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
  if (!VL53L0X_performSingleRefCalibration(0x00)) { return false; }
  //***************************************************************************
  // "restore the previous Sequence Config"
  writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
  //***************************************************************************
  return true;
}
//=============================================================================
// Set the return signal rate limit check value in units of MCPS (mega counts
// per second). "This represents the amplitude of the signal reflected from the
// target and detected by the device"; setting this limit presumably determines
// the minimum measurement necessary for the sensor to report a valid reading.
// Setting a lower limit increases the potential range of the sensor but also
// seems to increase the likelihood of getting an inaccurate reading because of
// unwanted reflections from objects other than the intended target.
// Defaults to 0.25 MCPS as initialized by the ST API and this library.
uint8_t VL53L0X_setSignalRateLimit(float limit_Mcps)
{
  if (limit_Mcps < 0 || limit_Mcps > 511.99) { return false; }

  // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
  writeReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit_Mcps * (1 << 7));
  return true;
}
//=============================================================================
// Get the return signal rate limit check value in MCPS
float VL53L0X_getSignalRateLimit(void)
{
  return (float)readReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT) / (1 << 7);
}
//=============================================================================
// Performs a single-shot range measurement and returns the reading in
// millimeters
// based on VL53L0X_PerformSingleRangingMeasurement()
// extraStats provides additional info for this measurment. Set to 0 if not needed.
uint16_t VL53L0X_readRangeSingleMillimeters( /*statInfo_t *extraStats */) {
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  writeReg(0x91, g_stopVariable);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);
  writeReg(SYSRANGE_START, 0x01);

  uint16_t temp;

    // assumptions: Linearity Corrective Gain is 1000 (default);
    // fractional ranging is not enabled
  	  temp = readReg16Bit(RESULT_RANGE_STATUS + 10);
  	  temp+=0;

  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
  return temp;
}
//=============================================================================
// based on VL53L0X_perform_single_ref_calibration()
uint8_t VL53L0X_performSingleRefCalibration(uint8_t vhv_init_byte)
{
  writeReg(SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP
  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
  writeReg(SYSRANGE_START, 0x00);
  return true;
}
//=============================================================================
// Get reference SPAD (single photon avalanche diode) count and type
// based on VL53L0X_get_info_from_device(),
// but only gets reference SPAD count and type
uint8_t VL53L0X_getSpadInfo(uint8_t * count, bool * type_is_aperture)
{
  uint8_t tmp;

  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);

  writeReg(0xFF, 0x06);
  writeReg(0x83, readReg(0x83) | 0x04);
  writeReg(0xFF, 0x07);
  writeReg(0x81, 0x01);

  writeReg(0x80, 0x01);

  writeReg(0x94, 0x6b);
  writeReg(0x83, 0x00);
  startTimeout();
  while (readReg(0x83) == 0x00)
  {
    if (checkTimeoutExpired()) { return false; }
  }
  writeReg(0x83, 0x01);
  tmp = readReg(0x92);

  *count = tmp & 0x7f;
  *type_is_aperture = (tmp >> 7) & 0x01;

  writeReg(0x81, 0x00);
  writeReg(0xFF, 0x06);
  writeReg(0x83, readReg(0x83)  & ~0x04);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x01);

  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);

  return true;
}
//=============================================================================
// Start continuous ranging measurements. If period_ms (optional) is 0 or not
// given, continuous back-to-back mode is used (the sensor takes measurements as
// often as possible); otherwise, continuous timed mode is used, with the given
// inter-measurement period in milliseconds determining how often the sensor
// takes a measurement.
// based on VL53L0X_StartMeasurement()
void VL53L0X_startContinuous(uint32_t period_ms)
{
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  writeReg(0x91, g_stopVariable);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);

  if (period_ms != 0)
  {
    // continuous timed mode
    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

    uint16_t osc_calibrate_val = readReg16Bit(OSC_CALIBRATE_VAL);

    if (osc_calibrate_val != 0)
    {
      period_ms *= osc_calibrate_val;
    }
    writeReg32Bit(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);
    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() end
    writeReg(SYSRANGE_START, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
  }
  else
  {
    // continuous back-to-back mode
    writeReg(SYSRANGE_START, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
  }
}
//=============================================================================
// Stop continuous measurements
// based on VL53L0X_StopMeasurement()
void VL53L0X_stopContinuous()
{
  writeReg(SYSRANGE_START, 0x01); // VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT

  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  writeReg(0x91, 0x00);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
}
//=============================================================================
uint16_t VL53L0X_readRangeContinuousMillimeters()
{
 /* startTimeout();
  while ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
  {
    if (checkTimeoutExpired())
    {
      did_timeout = true;
      return 65535;
    }
  }
*/
  // assumptions: Linearity Corrective Gain is 1000 (default);
  // fractional ranging is not enabled
  uint16_t range = readReg16Bit(RESULT_RANGE_STATUS + 10);

  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

  return range;
}
//=============================================================================







