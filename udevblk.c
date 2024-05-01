// Spew everything udev knows about block devices as one extremely long line of json, pipe the
// output through "jq ." to make it readable.
// Build with "gcc udevblk.c -ludev -o udevblk".

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libudev.h>

// json stuff
void jstr(const char *s)
{
    if (!s) printf("null");
    else
    {
        putchar('"');
        for(; *s; s++)
            if (*s < 32 || *s > 126 || *s == '\\' || *s == '"')
                printf("\\u00%02X", (unsigned char) *s);
            else
                putchar(*s);
        putchar('"');
    }
}
void jnum(unsigned long long n) { printf("%llu", n); }
bool jcomma = false;
void jkey(const char *k) { if (jcomma) putchar(','); jcomma = true; if (k) { jstr(k); putchar(':'); } }
void jobj(const char *k) { jkey(k); putchar('{'); jcomma = false; }
void jobjend(void) { putchar('}'); jcomma = true; }
void jarray(const char *k) { jkey(k); putchar('['); jcomma = false; }
void jarrayend() { putchar(']'); jcomma = true; }

int main()
{
    struct udev *udev = NULL;
    struct udev_enumerate *enumerate = NULL;
    struct udev_list_entry *entries;
    if ( (udev = udev_new()) &&
         (enumerate = udev_enumerate_new(udev)) &&
         (udev_enumerate_add_match_subsystem(enumerate, "block") >= 0) &&
         (udev_enumerate_scan_devices(enumerate) >= 0) &&
         (entries = udev_enumerate_get_list_entry(enumerate))
       )
    {
        jarray(NULL);
        struct udev_list_entry *entry;
        udev_list_entry_foreach(entry, entries)
        {
            const char *s = udev_list_entry_get_name(entry);
            struct udev_device *device = udev_device_new_from_syspath(udev,s);
            if (device)
            {
                jobj(NULL);
                jkey("devnum"); jnum(udev_device_get_devnum(device));
                jkey("devnode"); jstr(udev_device_get_devnode(device));
                jkey("devpath"); jstr(udev_device_get_devpath(device));
                jkey("devtype"); jstr(udev_device_get_devtype(device));
                jkey("driver"); jstr(udev_device_get_driver(device));
                jkey("subsystem"); jstr(udev_device_get_subsystem(device));
                jkey("sysname"); jstr(udev_device_get_sysname(device));
                jkey("syspath"); jstr(udev_device_get_syspath(device));
                jkey("sysnum"); jstr(udev_device_get_sysnum(device));
                struct udev_list_entry *devlinks = udev_device_get_devlinks_list_entry(device), *devlink;
                if (devlinks)
                {
                    jarray("devlinks");
                    udev_list_entry_foreach(devlink, devlinks) if ((s = udev_list_entry_get_name(devlink))) { jkey(NULL); jstr(s); }
                    jarrayend();
                }
                struct udev_list_entry *properties = udev_device_get_properties_list_entry(device), *property;
                if (properties)
                {
                    jobj("properties");
                    udev_list_entry_foreach(property, properties) if ((s = udev_list_entry_get_name(property))) { jkey(s); jstr(udev_device_get_property_value(device, s)); }
                    jobjend();
                }
                struct udev_list_entry *sysattrs = udev_device_get_sysattr_list_entry(device), *sysattr;
                if (sysattrs)
                {
                    jobj("sysattrs");
                    udev_list_entry_foreach(sysattr, sysattrs) if ((s = udev_list_entry_get_name(sysattr))) { jkey(s); jstr(udev_device_get_sysattr_value(device, s)); }
                    jobjend();
                }
                struct udev_list_entry *tags = udev_device_get_tags_list_entry(device), *tag;
                if (tags)
                {
                    jarray("tags");
                    udev_list_entry_foreach(tag, tags) if ((s = udev_list_entry_get_name(tag))) { jkey(NULL); jstr(s); }
                    jarrayend();
                }
                jobjend();
                udev_device_unref(device);
            }
        }
        jarrayend();
    }
    if (enumerate) udev_enumerate_unref(enumerate);
    if (udev) udev_unref(udev);
    return 0;
}
