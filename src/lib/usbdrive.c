
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>    //open
#include <unistd.h>   //close
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "usbdrive.h"
#include "log.h"
#include "file.h"

#define DEV_POINT "/dev/sda1"

#define MOUNT_INFO "/proc/self/mountinfo"

char _label[50];

char* UsbDriveGetLabel()
{
    FILE* fp;
    char* line = NULL;
	char* pLabelNextChar = _label;
    size_t len = 0;
    ssize_t read;

    fp = popen("blkid", "r");
    if (!fp)
	{
		strcpy(_label, "Error opening process blkid");
        return _label;
	}
	strcpy(_label, DEV_POINT " not found");

	#define FIELD_SIZE 50
	char f1[FIELD_SIZE];
	char f2[FIELD_SIZE];
	char f3[FIELD_SIZE];
	char f4[FIELD_SIZE];
	char f5[FIELD_SIZE];
	char f6[FIELD_SIZE];
    while ((read = getline(&line, &len, fp)) != -1)
	{
		sscanf(line, "%s %s %s %s %s %s", f1, f2, f3, f4, f5, f6);
		if (strcmp(f1, DEV_POINT ":") == 0)
		{
			char* p = f2;
			while(1)
			{
				if (!*p) break;
				if (*p == '=')
				{
					p++;
					break;
				}
				p++;
			}
			while(1)
			{
				if (!*p) break;
				if (*p == '=') break;
				if (*p != '\"') *pLabelNextChar++ = *p;
				p++;
			}
			*pLabelNextChar = 0;
		}
    }

    pclose(fp);
    if (line) free(line);
	
	return _label;
}

unsigned int UsbDriveGetSizeBytes()
{
    struct statvfs buf;
    int r = statvfs(USB_DRIVE_MOUNT_POINT, &buf);
	if (r) return 0;
	
	return buf.f_blocks * buf.f_bsize;
}
unsigned int UsbDriveGetFreeBytes()
{
    struct statvfs buf;
    int r = statvfs(USB_DRIVE_MOUNT_POINT, &buf);
	if (r) return 0;
	
	return buf.f_bfree * buf.f_bsize;
}

char UsbDriveGetIsPluggedIn() //Returns 1 if inserted, 0 otherwise
{
	int fd = open(DEV_POINT, O_RDONLY | O_NONBLOCK);
	if (fd < 0) return 0;
	close(fd);
	return 1;
}
char UsbDriveGetIsMounted()
{
	char isMounted = 0;
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(MOUNT_INFO, "r");
    if (!fp)
	{
		LogErrno("ERROR opening mount info");
        return 0;
	}

    while ((read = getline(&line, &len, fp)) != -1)
	{
		#define FIELD_SIZE 50
		char f1[FIELD_SIZE];
		char f2[FIELD_SIZE];
		char f3[FIELD_SIZE];
		char f4[FIELD_SIZE];
		char f5[FIELD_SIZE];
		char f6[FIELD_SIZE];
		sscanf(line, "%s %s %s %s %s %s", f1, f2, f3, f4, f5, f6);
		if (strcmp(f5, USB_DRIVE_MOUNT_POINT) == 0) isMounted = 1;
    }

    fclose(fp);
    if (line) free(line);
	
	return isMounted;

}
int UsbDriveMount()
{
	//Create the mount point - ignore the error if it already exists
	int r = mkdir(USB_DRIVE_MOUNT_POINT, 0777);
	if (r && errno != EEXIST)
	{
		LogErrno("ERROR creating mount point");
		return -1;
	}
	
	//cat /proc/filesystems lists the filesystems available. By default exfat and ntfs are not available. So, if added, try each in turn.
	r = mount(DEV_POINT, USB_DRIVE_MOUNT_POINT, "vfat" , 0, 0);
	if (r)
	{
		LogErrno("ERROR mounting usb drive");
		return -1;
	}
	return 0;
}
int UsbDriveUnmount()
{
	int r = umount(USB_DRIVE_MOUNT_POINT);
	if (r)
	{
		LogErrno("ERROR unmounting usb drive");
		return -1;
	}
	r = rmdir(USB_DRIVE_MOUNT_POINT);
	if (r)
	{
		LogErrno("ERROR removing mount point");
	}
	return 0;
}

int UsbDriveChgMount()
{
	if (UsbDriveGetIsMounted()) return UsbDriveUnmount();
	else                        return UsbDriveMount();
}

void UsbDrivePoll() //Call every few seconds
{
	if ( UsbDriveGetIsPluggedIn() && !UsbDriveGetIsMounted()) UsbDriveMount();
	if (!UsbDriveGetIsPluggedIn() &&  UsbDriveGetIsMounted()) UsbDriveUnmount();
}