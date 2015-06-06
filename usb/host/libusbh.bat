@echo off

mkdir ..\..\include
mkdir ..\..\include\usb
mkdir ..\..\include\usb\common
mkdir ..\..\include\usb\host
mkdir ..\..\include\usb\host\classes
mkdir ..\..\include\usb\host\classes\audio
mkdir ..\..\include\usb\host\classes\cdc
mkdir ..\..\include\usb\host\classes\hid
mkdir ..\..\include\usb\host\classes\hub
mkdir ..\..\include\usb\host\classes\msd
mkdir ..\..\include\usb\host\classes\msd\mfs
mkdir ..\..\include\usb\host\classes\phdc
mkdir ..\..\include\usb\host\classes\printer
mkdir ..\..\include\usb\host\khci
mkdir ..\..\include\usb\host\rtos
mkdir ..\..\include\usb\host\rtos\mqx

copy ..\common\include\usb.h ..\..\include\usb\common\usb.h /Y
copy ..\common\include\usb_debug.h ..\..\include\usb\common\usb_debug.h /Y
copy ..\common\include\usb_desc.h ..\..\include\usb\common\usb_desc.h /Y
copy ..\common\include\usb_error.h ..\..\include\usb\common\usb_error.h /Y
copy ..\common\include\usb_misc.h ..\..\include\usb\common\usb_misc.h /Y
copy ..\common\include\usb_misc_prv.h ..\..\include\usb\common\usb_misc_prv.h /Y
copy ..\common\include\usb_prv.h ..\..\include\usb\common\usb_prv.h /Y
copy ..\common\include\usb_types.h ..\..\include\usb\common\usb_types.h /Y
copy source\classes\audio\usb_host_audio.h ..\..\include\usb\host\classes\audio\usb_host_audio.h /Y
copy source\classes\cdc\usb_host_cdc.h ..\..\include\usb\host\classes\cdc\usb_host_cdc.h /Y
copy source\classes\hid\usb_host_hid.h ..\..\include\usb\host\classes\hid\usb_host_hid.h /Y
copy source\classes\hub\usb_host_hub.h ..\..\include\usb\host\classes\hub\usb_host_hub.h /Y
copy source\classes\hub\usb_host_hub_sm.h ..\..\include\usb\host\classes\hub\usb_host_hub_sm.h /Y
copy source\classes\msd\mfs\usbmfs.h ..\..\include\usb\host\classes\msd\mfs\usbmfs.h /Y
copy source\classes\msd\mfs\usbmfspr.h ..\..\include\usb\host\classes\msd\mfs\usbmfspr.h /Y
copy source\classes\msd\usb_host_msd_bo.h ..\..\include\usb\host\classes\msd\usb_host_msd_bo.h /Y
copy source\classes\msd\usb_host_msd_ufi.h ..\..\include\usb\host\classes\msd\usb_host_msd_ufi.h /Y
copy source\classes\phdc\usb_host_phdc.h ..\..\include\usb\host\classes\phdc\usb_host_phdc.h /Y
copy source\classes\printer\usb_host_printer.h ..\..\include\usb\host\classes\printer\usb_host_printer.h /Y
copy source\host\khci\khci.h ..\..\include\usb\khci\khci.h /Y
copy source\include\host_ch9.h ..\..\include\usb\host\host_ch9.h /Y
copy source\include\host_close.h ..\..\include\usb\host\host_close.h /Y
copy source\include\host_cnfg.h ..\..\include\usb\host\host_cnfg.h /Y
copy source\include\host_cnl.h ..\..\include\usb\host\host_cnl.h /Y
copy source\include\host_common.h ..\..\include\usb\host\host_common.h /Y
copy source\include\host_dev_list.h ..\..\include\usb\host\host_dev_list.h /Y
copy source\include\host_main.h ..\..\include\usb\host\host_main.h /Y
copy source\include\host_rcv.h ..\..\include\usb\host\host_rcv.h /Y
copy source\include\host_shut.h ..\..\include\usb\host\host_shut.h /Y
copy source\include\host_snd.h ..\..\include\usb\host\host_snd.h /Y
copy source\include\hostapi.h ..\..\include\usb\host\hostapi.h /Y
copy source\rtos\mqx\mqx_dll.h ..\..\include\usb\host\rtos\mqx\mqx_dll.h /Y
copy source\rtos\mqx\mqx_host.h ..\..\include\usb\host\rtos\mqx\mqx_host.h /Y
copy source\rtos\mqx\usb_mqx.h ..\..\include\usb\host\rtos\mqx\usb_mqx.h /Y

