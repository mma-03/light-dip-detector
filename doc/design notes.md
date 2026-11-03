# Design Notes

## Designing the adc module

**Functions:** 
- bool Adc_init(void) // open + configure SPI, false on failure
- int Adc_read(int channel) // 0-4095, or -1 on error
- void Adc_cleanup(void) // close the device

**Raw count vs. Volts:** The module lives in hal/ because it talks to hardware, unit conversion does not belong here. The read function should return the 12-bit reading, unit conversion happens in app/.

**File descriptor:** open() returns a file descriptor, ioctl() in adc_read needs it.
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

## Designing the sampler module

**What other threads need to ask the sampler module:**
- one-per-second thread needs to tell the sample "move current into history", then get history to count dips and print, also needs to get the average, which is also what the dip threshold is measured from.
- The UDP thread answers count, length, and history. Get the total samples taken since the program started (count), and how many samples are in history, the previous second, about 500 (length).
- Sampler_init to start the sampler thread at the beginning, and Sampler_cleanup to stop it at the end.

**Needs:**
- main needs to start the sampler thread
- main needs to stop the sampler thread
- once-per-second thread needs to tell it "move current into history"
- once-per-second and UDP history needs to get the history samples
- once-per-second needs to get the average
- UDP length needs to get how many samples are in history
- UDP count needs to get the total samples taken

**Functions:**
- void Sampler_init(void);
- void Sampler_cleanup(void);
- void Sampler_moveCurrentToHistory(void);
- double *Sampler_getHistory(int *size);
- double Sampler_getAvg(void);
- int Sampler_getHistorySize(void);
- long long Sampler_getTotalSamples(void)

**Units:** Store each sample as volts, as the spec says history prints volts with 3 decimals, and the dip threshold is in volts. This way conversion can be avoided within each function. Could also include a helper function for conversion and store each sample as raw counts.

## Designing the LED modules

**hal/led functions:**
- bool Led_init(void)
- void Led_set(bool state)
- void Led_cleanup(void)

**app/flasher functions:**
- void Flasher_init(void) // starting frequency is 10 Hz
- void Flasher_setFreq(int freq_Hz) // cannot go below 0 or above maximum
- int Flasher_getFreq(void)
- void Flasher_cleanup(void)

**Shared data:** frequency lives in the flasher module. Encoder thread writes it through setFreq(), flasher thread reads it to know how long to sleep for each cycle, main reads it through getFreq() to print.

**At 0 Hz:**
- turn the led off
- sleep for a short fixed time, then loop and read the frequency again

**"Do nothing if unchanged"**
Check in Flasher_SetFrequency:
- lock
- compare the requested frequency with the stored one
- if equal, unlock and return without changing anything
- otherwise, store the new frequency and unlock

**Shutdown:**
- stop the flasher thread and wait for it to exit (Flasher_cleanup())
- turn the LED off (Flasher_cleanup())
- release the pin (Led_cleanup())





