@echo off
IF NOT EXIST ..\include\common          mkdir ..\include\common
IF NOT EXIST ..\include\psp             mkdir ..\include\psp
IF NOT EXIST ..\include\psp\cortex_m    mkdir ..\include\psp\cortex_m

copy ..\..\config\common\small_ram_config.h ..\include\common\small_ram_config.h /Y

copy ..\source\include\charq.h ..\include\charq.h /Y
copy ..\source\include\eds.h ..\include\eds.h /Y
copy ..\source\include\edserial.h ..\include\edserial.h /Y
copy ..\source\include\event.h ..\include\event.h /Y
copy ..\source\include\fio.h ..\include\fio.h /Y
copy ..\source\include\gen_rev.h ..\include\gen_rev.h /Y
copy ..\source\include\io.h ..\include\io.h /Y
copy ..\source\include\io_rev.h ..\include\io_rev.h /Y
copy ..\source\include\ioctl.h ..\include\ioctl.h /Y
copy ..\source\include\ipc.h ..\include\ipc.h /Y
copy ..\source\include\ipc_pcb.h ..\include\ipc_pcb.h /Y
copy ..\source\include\ipc_pcbv.h ..\include\ipc_pcbv.h /Y
copy ..\source\include\klog.h ..\include\klog.h /Y
copy ..\source\include\log.h ..\include\log.h /Y
copy ..\source\include\lwevent.h ..\include\lwevent.h /Y
copy ..\source\include\lwlog.h ..\include\lwlog.h /Y
copy ..\source\include\lwmem.h ..\include\lwmem.h /Y
copy ..\source\include\lwmsgq.h ..\include\lwmsgq.h /Y
copy ..\source\include\lwsem.h ..\include\lwsem.h /Y
copy ..\source\include\lwtimer.h ..\include\lwtimer.h /Y
copy ..\source\include\message.h ..\include\message.h /Y
copy ..\source\include\mmu.h ..\include\mmu.h /Y
copy ..\source\include\mqx.h ..\include\mqx.h /Y
copy ..\source\include\mqx_cnfg.h ..\include\mqx_cnfg.h /Y
copy ..\source\include\mqx_cpudef.h ..\include\mqx_cpudef.h /Y
copy ..\source\include\mqx_inc.h ..\include\mqx_inc.h /Y
copy ..\source\include\mqx_ioc.h ..\include\mqx_ioc.h /Y
copy ..\source\include\mqx_macros.h ..\include\mqx_macros.h /Y
copy ..\source\include\mqx_str.h ..\include\mqx_str.h /Y
copy ..\source\include\mutex.h ..\include\mutex.h /Y
copy ..\source\include\name.h ..\include\name.h /Y
copy ..\source\include\partition.h ..\include\partition.h /Y
copy ..\source\include\pcb.h ..\include\pcb.h /Y
copy ..\source\include\posix.h ..\include\posix.h /Y
copy ..\source\include\queue.h ..\include\queue.h /Y
copy ..\source\include\sem.h ..\include\sem.h /Y
copy ..\source\include\timer.h ..\include\timer.h /Y
copy ..\source\include\watchdog.h ..\include\watchdog.h /Y
copy ..\source\psp\cortex_m\cortex.h ..\include\psp\cortex_m\cortex.h /Y
copy ..\source\psp\cortex_m\cpu\kinetis_fcan.h ..\include\psp\cortex_m\kinetis_fcan.h /Y
copy ..\source\psp\cortex_m\cpu\MK60DZ10.h ..\include\psp\cortex_m\MK60DZ10.h /Y
copy ..\source\psp\cortex_m\kinetis.h ..\include\psp\cortex_m\kinetis.h /Y
copy ..\source\psp\cortex_m\kinetis_mpu.h ..\include\psp\cortex_m\kinetis_mpu.h /Y
copy ..\source\psp\cortex_m\nvic.h ..\include\psp\cortex_m\nvic.h /Y
copy ..\source\psp\cortex_m\psp.h ..\include\psp\cortex_m\psp.h /Y
copy ..\source\psp\cortex_m\psp_abi.h ..\include\psp\cortex_m\psp_abi.h /Y
copy ..\source\psp\cortex_m\psp_comp.h ..\include\psp\cortex_m\psp_comp.h /Y
copy ..\source\psp\cortex_m\psp_cpu.h ..\include\psp\cortex_m\psp_cpu.h /Y
copy ..\source\psp\cortex_m\psp_cpudef.h ..\include\psp\cortex_m\psp_cpudef.h /Y
copy ..\source\psp\cortex_m\psp_math.h ..\include\psp\cortex_m\psp_math.h /Y
copy ..\source\psp\cortex_m\psp_rev.h ..\include\psp\cortex_m\psp_rev.h /Y
copy ..\source\psp\cortex_m\psp_supp.h ..\include\psp\cortex_m\psp_supp.h /Y
copy ..\source\psp\cortex_m\psp_time.h ..\include\psp\cortex_m\psp_time.h /Y
copy ..\source\psp\cortex_m\psptypes.h ..\include\psp\cortex_m\psptypes.h /Y
copy ..\source\tad\tad.h ..\include\tad\tad.h /Y

copy ..\source\psp\cortex_m\compiler\iar\asm_mac.h ..\include\psp\cortex_m\asm_mac.h /Y
copy ..\source\psp\cortex_m\compiler\iar\comp.h ..\include\psp\cortex_m\comp.h /Y

