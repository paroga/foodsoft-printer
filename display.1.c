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
#include <linux/kd.h>

#include "bitmaps.c"
#include "characters.c"

#include <libmm-glib/libmm-glib.h>
#include <glib-unix.h>
#include <libusb-1.0/libusb.h>

#define PRINTER_VENDOR 0x4e8
#define PRINTER_PRODUCT 0x3301

typedef struct {
    unsigned char cloud;
    struct {
        char icon;
        unsigned char id;
        char text[43];
    } slots[7];
} MemoryLayout;

void getModemDetails(GDBusConnection *connection, unsigned char* antenna, bool* connected)
{
    MMModem* modem = NULL;

    MMManager* manager =  mm_manager_new_sync(connection, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, NULL, NULL);
    GList* modems = g_dbus_object_manager_get_objects((GDBusObjectManager*) manager);

    for (GList* l = modems; l != NULL; l = l->next)
    {
        gpointer element_data = l->data;
        modem = mm_object_get_modem(element_data);
        g_object_unref(element_data);
    }

    g_list_free(modems);

    if (!modem) {
        antenna = 0;
        *connected = false;
        return;
    }

    *antenna = mm_modem_get_signal_quality(modem, NULL);
    if (*antenna)
        *antenna = 1 + 6 * 6 * *antenna / 100 / 5;
    *connected = mm_modem_get_state(modem) >= MM_MODEM_STATE_CONNECTED;

    g_object_unref(modem);
    g_object_unref(manager);
}

void getPrinterDetails(libusb_context *context, bool* printer)
{
    bool found = false;

    libusb_device **devices;
    ssize_t count = libusb_get_device_list(context, &devices);

    for (size_t i = 0; i < count; ++i) {
        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(devices[i], &desc))
            continue;

        if (desc.idVendor == PRINTER_VENDOR && desc.idProduct == PRINTER_PRODUCT)
            found = true;
    }

    libusb_free_device_list(devices, count);

    *printer = found;
}


uint16_t* drawBitmap(uint16_t* position, unsigned width, const Bitmap* bmp)
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

uint16_t* drawCharacter(uint16_t* position, unsigned width, const Character* ch, uint16_t color)
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

const Character* getCharacter(char ch)
{
    if ('0' <= ch && ch <= '9')
        return font_SMALL[ch - '0'];
    if ('A' <= ch && ch <= 'Z')
        return font_SMALL[10 + ch - 'A'];
    if ('a' <= ch && ch <= 'z')
        return font_SMALL[10 + ch - 'a'];
    if (ch == ' ')
        return &ch_SPACE;
    if (ch == '.')
        return &ch_PERIOD;
    if (ch == ':')
        return &ch_COLON;
    if (ch == '#')
        return &ch_HASH;
    return &ch_INVALID;
}

uint16_t* drawTextChar(uint16_t* position, unsigned width, const char** text, uint16_t color)
{
    char ch = **text;
    if (!ch)
        return position + 8 * width;
    *text += 1;
    return drawCharacter(position, width, getCharacter(ch), color) + 1;
}

void drawStatus(uint16_t* position, unsigned width, const Character* icon, unsigned char id, const char* text)
{
    const uint16_t color = 0xffff;
    uint16_t buffer[14 * width];
    memset(buffer, 0, sizeof(buffer));

    if (icon) {
        drawCharacter(buffer + width + 1, width, icon, color);

        id = id % 100;
        unsigned id_10 = id / 10;
        unsigned id_1 = id % 10;

        uint16_t* p = buffer + width;
        p = drawCharacter(p + 16, width, font_BIG[id_10], color);
        p = drawCharacter(p + 2, width, font_BIG[id_1], color);

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

void run(uint16_t data[], unsigned width)
{
    uint16_t* p = data;
    memset(p, 0, width * width * sizeof(uint16_t));
    for (unsigned i = 0; i < 7; ++i) {
        p = data + (23 + 15 * i) * width;
        for (unsigned j = 0; j < width; ++j)
            p[j] = 0x2104;
    }

    uint16_t* pConnection = drawBitmap(data, width, &antenna_off);
    uint16_t* pCloud = drawBitmap(pConnection, width, &connection_off);
    uint16_t* pPrinter = drawBitmap(pCloud, width, &cloud_off);
    drawBitmap(pPrinter, width, &printer_off);



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
            drawBitmap(data, width, &antenna_on);
            oldAntenna = true;
        } else if (oldAntenna && !antenna) {
            drawBitmap(data, width, &antenna_off);
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


        drawBitmap(pConnection, width, connected ? &connection_on : &connection_off);
        drawBitmap(pCloud, width, ipc->cloud ? &cloud_on : &cloud_off);
        drawBitmap(pPrinter, width, printer ? &printer_on : &printer_off);

        for (unsigned i = 0; i < 7; ++i) {
            const Character* icon = NULL;
            switch (ipc->slots[i].icon) {
                case 'D':
                    icon = &ch_DOWN;
                    break;
                case 'O':
                    icon = &ch_CLOCK;
                    break;
                case 'P':
                    icon = &ch_PRINTER;
                    break;
                case 'K':
                    icon = &ch_CHECK;
                    break;
                case 'H':
                    icon = &ch_PAUSE;
                    break;
                case 'W':
                    icon = &ch_WARN;
                    break;
                case 'X':
                    icon = &ch_CROSS;
                    break;
            }

            drawStatus(data + (24 + 15 * i) * width, width, icon, ipc->slots[i].id, ipc->slots[i].text);
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
