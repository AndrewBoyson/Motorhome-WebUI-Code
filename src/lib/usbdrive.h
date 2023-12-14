extern unsigned int UsbDriveGetSizeBytes();
extern unsigned int UsbDriveGetFreeBytes();
extern void UsbDriveGetLabel(char* label);
extern char UsbDriveGetIsPluggedIn(void);
extern char UsbDriveGetIsMounted(void);
extern int  UsbDriveMount(void);
extern int  UsbDriveUnmount(void);
extern int  UsbDriveChgMount(void);
extern void UsbDrivePoll(void);

#define USB_DRIVE_MOUNT_POINT "/media/usb"