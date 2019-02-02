#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

#include "bitmaps.h"
#include "characters.h"

static uint16_t* drawBitmap(uint16_t* position, unsigned width, const Bitmap* bmp)
{
    uint16_t* p = position;
    const unsigned char* data = bmp->data;

    for (unsigned i = 0; i < bmp->height; ++i) {
        memcpy(p, data, bmp->width);
        data += bmp->width;
        p += width;
    }

    return position + bmp->width / sizeof(uint16_t);
}

static uint16_t* drawCharacter(uint16_t* position, unsigned width, const Character* ch, uint16_t color)
{
    uint16_t* p = position;
    const char* data = ch->data;
    unsigned len = 0;

    while (*data) {
        if (*data != ' ') {
            *p = color;
        }
        p++;
        data++;
        len++;
        if (len == ch->width) {
            p += width - len;
            len = 0;
        }
    }

    return position + ch->width;
}

static uint16_t* drawTextChar(uint16_t* position, unsigned width, const char** text, uint16_t color)
{
    char ch = **text;
    if (!ch)
        return position + 8 * width;
    *text += 1;
    return drawCharacter(position, width, getCharacter(ch), color) + 1;
}

static void drawStatus(uint16_t* position, unsigned width, char iconId, unsigned char id, const char* text)
{
    const uint16_t color = 0xffff;
    uint16_t buffer[14 * width];
    memset(buffer, 0, sizeof(buffer));

    const Character* icon = getIcon(iconId);

    if (icon) {
        drawCharacter(buffer + width + 1, width, icon, color);

        id = id % 100;
        unsigned char id_10 = id / 10;
        unsigned char id_1 = id % 10;

        uint16_t* p = buffer + width;
        p = drawCharacter(p + 16, width, getDigit(id_10), color);
        p = drawCharacter(p + 2, width, getDigit(id_1), color);

        uint16_t* q = p + 2;
        uint16_t* end = buffer + width * 2 - 3;
        while (q < end)
            q = drawTextChar(q, width, &text, color);

        q = p + 2 + width * 7;
        end = buffer + width * 9 - 3;
        while (q < end)
            q = drawTextChar(q, width, &text, color);
    }

    memcpy(position, buffer, sizeof(buffer));
}

static void drawBitmapAnteanna(uint16_t* data, unsigned width, int value)
{
    static bool oldAntenna = true;
    bool newAntenna = value >= 0;

    if (!oldAntenna && newAntenna) {
        drawBitmap(data, width, &bitmap_antenna_on);
        oldAntenna = true;
    } else if (oldAntenna && !newAntenna) {
        drawBitmap(data, width, &bitmap_antenna_off);
        oldAntenna = false;
    }
    if (!newAntenna)
        return;
    for (unsigned i = 0; i < 21; ++i) {
        uint16_t* p = data + (1 + i) * width + 19;
        for (unsigned j = 1; j < 7; ++j) {
            uint16_t color = j <= value ? 0xffff : 0x2104;
            if (i > 18 || (i > 15 && j > 0) || (i > 12 && j > 1) || (i > 9 && j > 2) || (i > 6 && j > 3) || (i > 3 && j > 4) || j > 5)
                p[0] = p[1] = p[2] = color;
            p += 5;
        }
    }
}

static void drawBitmapConnection(uint16_t* data, unsigned width, bool value)
{
    data += bitmap_antenna_off.width / sizeof(uint16_t);
    drawBitmap(data, width, value ? &bitmap_connection_on : &bitmap_connection_off);
}

static void drawBitmapCloud(uint16_t* data, unsigned width, bool value)
{
    data += bitmap_antenna_off.width / sizeof(uint16_t);
    data += bitmap_connection_off.width / sizeof(uint16_t);
    drawBitmap(data, width, value ? &bitmap_cloud_on : &bitmap_cloud_off);
}

static void drawBitmapPrinter(uint16_t* data, unsigned width, bool value)
{
    data += bitmap_antenna_off.width / sizeof(uint16_t);
    data += bitmap_connection_off.width / sizeof(uint16_t);
    data += bitmap_cloud_off.width / sizeof(uint16_t);
    drawBitmap(data, width, value ? &bitmap_printer_on : &bitmap_printer_off);
}

static void clearDisplay(uint16_t* data, unsigned width)
{
    uint16_t* p = data;
    memset(p, 0, width * width * sizeof(uint16_t));
    for (unsigned i = 0; i < 7; ++i) {
        p = data + (23 + 15 * i) * width;
        for (unsigned j = 0; j < width; ++j)
            p[j] = 0x2104;
    }

    drawBitmapAnteanna(data, width, -1);
    drawBitmapConnection(data, width, false);
    drawBitmapCloud(data, width, false);
    drawBitmapPrinter(data, width, false);
}


typedef struct display_s {
    int fd;
    uint16_t* data;
    size_t screensize;
    uint32_t xres;
    uint32_t yres;
} display_t;

display_t* display_new(const char* path)
{
    display_t* display = malloc(sizeof(display));
    display->fd = open(path, O_RDWR);
    if (display->fd == -1)
    {
        perror("ERROR (Display): cannot open framebuffer device\n");
        exit(1);
    }

    struct fb_fix_screeninfo finfo;
    if (ioctl(display->fd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("ERROR (Display): reading fixed information\n");
        exit(1);
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(display->fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("ERROR (Display): reading variable information\n");
        exit(1);
    }

    display->xres = vinfo.xres;
    display->yres = vinfo.yres;
    unsigned long int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8 ;

    display->data = (uint16_t*) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, display->fd, 0);

    if (display->data == MAP_FAILED)
    {
        printf("ERROR (Display): failed to map framebuffer device to memory\n");
        exit(1);
    }

    clearDisplay(display->data, display->xres);

    return display;
}

void display_free(display_t* display)
{
  munmap(display->data, display->screensize);
  close(display->fd);
}

uint32_t display_slot_count(display_t* display)
{
    return (display->yres - 23) / 15;
}

void display_update_antenna(display_t* display, int level)
{
    drawBitmapAnteanna(display->data, display->xres, level);
}

void display_update_connection(display_t* display, bool active)
{
    drawBitmapConnection(display->data, display->xres, active);
}

void display_update_cloud(display_t* display, bool active)
{
    drawBitmapCloud(display->data, display->xres, active);
}

void display_update_printer(display_t* display, bool active)
{
    drawBitmapPrinter(display->data, display->xres, active);
}

void display_update_slot(display_t* display, uint32_t slot, char icon, unsigned char id, const char* message)
{
    uint32_t width = display->xres;
    drawStatus(display->data + (24 + 15 * slot) * width, width, icon, id, message);
}


#if 0

void run(uint16_t data[], unsigned width)
{

    libusb_context *context = NULL;
    libusb_init(&context);

    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);


    int fd = open("/var/foodsoft", O_RDWR | O_CREAT);
    {
        MemoryLayout buffer;
        memset(&buffer, 0, sizeof(buffer));
        write(fd, &buffer, sizeof(buffer));
    }
    MemoryLayout *ipc = mmap(0, sizeof(*ipc), PROT_READ, MAP_SHARED, fd, 0);


    bool oldAntenna = false;

    while (1) {
        unsigned char antenna = 0;
        bool connected = false;
        bool printer = false;
        getModemDetails(connection, &antenna, &connected);
        getPrinterDetails(context, &printer);

        if (!oldAntenna && antenna) {
            drawBitmap(data, width, &bitmap_antenna_on);
            oldAntenna = true;
        } else if (oldAntenna && !antenna) {
            drawBitmap(data, width, &bitmap_antenna_off);
            oldAntenna = false;
        }
        if (antenna) {
            for (unsigned i = 0; i < 21; ++i) {
                uint16_t* p = data + (1 + i) * width + 19;
                for (unsigned j = 1; j < 7; ++j) {
                    uint16_t color = j < antenna ? 0xffff : 0x2104;
                    if (i > 18 || i > 15 && j > 0 || i > 12 && j > 1 || i > 9 && j > 2 || i > 6 && j > 3 || i > 3 && j > 4 || j > 5)
                        p[0] = p[1] = p[2] = color;
                    p += 5;
                }
            }
        }


        drawBitmap(pConnection, width, connected ? &bitmap_connection_on : &bitmap_connection_off);
        drawBitmap(pCloud, width, ipc->cloud ? &bitmap_cloud_on : &bitmap_cloud_off);
        drawBitmap(pPrinter, width, printer ? &bitmap_printer_on : &bitmap_printer_off);

        for (unsigned i = 0; i < 7; ++i) {
            drawStatus(data + (24 + 15 * i) * width, width, ipc->slots[i].icon, ipc->slots[i].id, ipc->slots[i].text);
        }


        usleep(250);
    }

    munmap(ipc, sizeof(*ipc));
    close(fd);

    libusb_exit(context);
}

int main(int argc, char* argv[])
{
  int fd = open("/dev/fb1", O_RDWR);
  if (fd == -1)
    {
      printf("Error: cannot open framebuffer device");
      exit(1);
    }

  struct fb_fix_screeninfo finfo;
  if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
      printf("Error reading fixed information");
      exit(1);
    }

  struct fb_var_screeninfo vinfo;
  if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
      printf("Error reading variable information");
      exit(1);
    }

  printf("%s\n", finfo.id);
  printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

  unsigned long int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8 ;
  printf("%lu bytes\n", screensize);

  void* fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (fbp == MAP_FAILED)
    {
      printf("Error: failed to map framebuffer device to memory\n");
      exit(1);
    }

  run(fbp, vinfo.xres);

  munmap(fbp, screensize);
  close(fd);

  return 0;
}

#endif
