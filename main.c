#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>

#define I2C_DEV "/dev/i2c-2"
#define I2C_ADDR 0x27

int lcd_fd;


#define LCD_BACKLIGHT 0x08
#define ENABLE        0x04
#define LCD_CMD       0x00
#define LCD_DATA      0x01

void lcd_toggle_enable(uint8_t val) {
    write(lcd_fd, &val, 1);
    usleep(500);
    val |= ENABLE;
    write(lcd_fd, &val, 1);
    usleep(500);
    val &= ~ENABLE;
    write(lcd_fd, &val, 1);
    usleep(500);
}

void lcd_send(uint8_t value, uint8_t mode) {
    uint8_t high = (value & 0xF0) | mode | LCD_BACKLIGHT;
    uint8_t low  = ((value << 4) & 0xF0) | mode | LCD_BACKLIGHT;

    lcd_toggle_enable(high);
    lcd_toggle_enable(low);
}

void lcd_command(uint8_t cmd) {
    lcd_send(cmd, LCD_CMD);
}

void lcd_char(char ch) {
    lcd_send(ch, LCD_DATA);
}

void lcd_init() {
    usleep(15000);  // Power-on delay
    lcd_command(0x33);  // Init
    lcd_command(0x32);  // 4-bit mode
    lcd_command(0x28);  // 2 lines, 5x7
    lcd_command(0x0C);  // Display ON, cursor OFF
    lcd_command(0x06);  // Entry mode
    lcd_command(0x01);  // Clear
    usleep(2000);
}

void lcd_set_cursor(int col, int row) {
    const int row_offsets[] = { 0x00, 0x40 };
    lcd_command(0x80 | (col + row_offsets[row]));
}

void lcd_clear() {
    lcd_command(0x01);
    usleep(2000);
}

void lcd_print(const char* s) {
    while (*s) lcd_char(*s++);
}

void lcd_animation() {
    lcd_clear();
    lcd_set_cursor(0, 0);
    for (int i = 0; i < 16; ++i) {
        lcd_char('.');
        usleep(100000); // 100ms delay
    }

    lcd_set_cursor(0, 1);
    for (int i = 0; i < 16; ++i) {
        lcd_char('.');
        usleep(100000);
    }

    lcd_clear();
    lcd_set_cursor(2, 0);
    lcd_print("i2c 16x2 Power-On");
    lcd_set_cursor(3, 1);
    lcd_print("LCD Ready!");
}

int main() {
    lcd_fd = open(I2C_DEV, O_RDWR);
    if (lcd_fd < 0) {
        perror("Open I2C failed");
        return 1;
    }

    if (ioctl(lcd_fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("I2C ioctl failed");
        close(lcd_fd);
        return 1;
    }

    lcd_init();
    lcd_animation();

    close(lcd_fd);
    return 0;
}
