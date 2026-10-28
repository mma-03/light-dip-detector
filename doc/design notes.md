# Design Notes

## Designing the adc module

**Functions:** 
- bool Adc_init(void) // open + configure SPI, false on failure
- int Adc_read(int channel) // 0-4095, or -1 on error
- void Adc_cleanup(void) // close the device

**Raw count vs. Volts:** The module lives in hal/ because it talks to hardware, unit conversion does not belong here. The read function should return the 12-bit reading, unit conversion happens in app/.

**File descriptor:** open() returns a file descriptor, every ioctl() in adc_read needs it.
- open() never returns a negative number on success
- store -1 to mean "not open"
- adc_read checks fd < 0 and fail safely if someone calls it before adc_init

**/dev/spidev0.0:** Opened once instead of on every read

**Errors:**
- Adc_read returns -1
- Adc_init open() can fail (wrong device name, SPI not enabled)
- HAL shouldn't decide to exit the program
- Adc_init reports success or failure
- inside adc.c, when a system call fails, print why with perror("...")

**Header:** 
- #include <stdbool.h>
- bool Adc_init(void)
- int Adc_read(int channel)
- void Adc_cleanup(void)