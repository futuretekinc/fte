@echo off

IF NOT EXIST ..\..\include\usb                  mkdir ..\..\include\usb
IF NOT EXIST ..\..\include\usb\common           mkdir ..\..\include\usb\common
IF NOT EXIST ..\..\include\usb\host             mkdir ..\..\include\usb\host
IF NOT EXIST ..\..\include\usb\host\khci        mkdir ..\..\include\usb\host\khci
IF NOT EXIST ..\..\include\usb\device           mkdir ..\..\include\usb\device
IF NOT EXIST ..\..\include\usb\device\khci      mkdir ..\..\include\usb\device\khci
IF NOT EXIST ..\..\include\usb\device\rtos      mkdir ..\..\include\usb\device\rtos
IF NOT EXIST ..\..\include\usb\device\classes   mkdir ..\..\include\usb\device\classes

copy ..\common\include\usb.h ..\..\include\usb\common\usb.h /Y
copy ..\common\include\usb_debug.h ..\..\include\usb\common\usb_debug.h /Y
copy ..\common\include\usb_desc.h ..\..\include\usb\common\usb_desc.h /Y
copy ..\common\include\usb_error.h ..\..\include\usb\common\usb_error.h /Y
copy ..\common\include\usb_misc.h ..\..\include\usb\common\usb_misc.h /Y
copy ..\common\include\usb_misc_prv.h ..\..\include\usb\common\usb_misc_prv.h /Y
copy ..\common\include\usb_prv.h ..\..\include\usb\common\usb_prv.h /Y
copy ..\common\include\usb_types.h ..\..\include\usb\common\usb_types.h /Y
copy source\classes\include\usb_audio.h ..\..\include\usb\device\classes\usb_audio.h /Y
copy source\classes\include\usb_cdc.h ..\..\include\usb\device\classes\usb_cdc.h /Y
copy source\classes\include\usb_cdc_pstn.h ..\..\include\usb\device\classes\usb_cdc_pstn.h /Y
copy source\classes\include\usb_class.h ..\..\include\usb\device\classes\usb_class.h /Y
copy source\classes\include\usb_framework.h ..\..\include\usb\device\classes\usb_framework.h /Y
copy source\classes\include\usb_hid.h ..\..\include\usb\device\classes\usb_hid.h /Y
copy source\classes\include\usb_msc.h ..\..\include\usb\device\classes\usb_msc.h /Y
copy source\classes\include\usb_msc_scsi.h ..\..\include\usb\device\classes\usb_msc_scsi.h /Y
copy source\classes\include\usb_phdc.h ..\..\include\usb\device\classes\usb_phdc.h /Y
copy source\classes\include\usb_stack_config.h ..\..\include\usb\device\classes\usb_stack_config.h /Y
copy source\include\dev_cnfg.h ..\..\include\usb\device\dev_cnfg.h /Y
copy source\include\dev_main.h ..\..\include\usb\device\dev_main.h /Y
copy source\include\devapi.h ..\..\include\usb\device\devapi.h /Y
copy source\include\khci\khci_dev_main.h ..\..\include\usb\device\khci\khci_dev_main.h /Y
copy source\include\rtos\mqx_dev.h ..\..\include\usb\device\rtos\mqx_dev.h /Y
copy ..\host\source\host\khci\khci_prv.h ..\..\include\usb\host\khci\khci_prv.h /Y

