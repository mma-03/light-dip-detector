#include "adc.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define DEVICE_PATH "/dev/spidev0.0"
#define SPEED_HZ 500000
#define BITS_PER_WORD 8
#define TRANSFER_LEN_BYTES 3
#define MAX_CHANNEL 7
#define MCP3208_START_BIT 0x04
#define MCP3208_SINGLE_ENDED 0x02
#define RX1_DATA_MASK 0x0F

static int s_fd = -1;

// report init failure and release the device
static bool failInit(const char *msg) {
    perror(msg);
    close(s_fd);
    s_fd = -1;
    return false;
}

bool Adc_init(void) {
    // open device
    s_fd = open(DEVICE_PATH, O_RDWR); // O_RDWR means read and write since SPI goes both ways
    if (s_fd < 0) {
        perror("Adc_init: open");
        return false;
    }

    // configuration
    uint8_t mode = SPI_MODE_0;
    if (ioctl(s_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        return failInit("Adc_init: mode");
    }

    uint8_t bits = BITS_PER_WORD;
    if (ioctl(s_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        return failInit("Adc_init: bits per word");
    }

    uint32_t speed = SPEED_HZ;
    if (ioctl(s_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        return failInit("Adc_init: max speed"); 
    }

    return true;
}


int Adc_read(int channel) {
    // guard: s_fd < 0 (not initialized), or channel is outside 0-7
    if (s_fd < 0 || channel < 0 || channel > MAX_CHANNEL) {
        return -1;
    }

    uint8_t tx[TRANSFER_LEN_BYTES] = {0};
    uint8_t rx[TRANSFER_LEN_BYTES] = {0};

    tx[0] = MCP3208_START_BIT | MCP3208_SINGLE_ENDED | ((channel >> 2) & 0x01);
    tx[1] = (channel & 0x03) << 6;
    tx[2] = 0;

    struct spi_ioc_transfer transfer = {0};

    transfer.tx_buf = (unsigned long)tx;
    transfer.rx_buf = (unsigned long)rx;
    transfer.len = TRANSFER_LEN_BYTES;
    transfer.speed_hz = SPEED_HZ;
    transfer.bits_per_word = BITS_PER_WORD;

    if (ioctl(s_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Adc_read: transfer");
        return -1;
    }

    return ((rx[1] & RX1_DATA_MASK) << 8) | rx[2];
}

void Adc_cleanup(void) {
    if (s_fd >= 0) {
        close(s_fd);
        s_fd = -1;
    }
}