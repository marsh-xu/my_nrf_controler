nrfjprog --eraseall
nrfjprog --program s110_softdevice.hex
nrfjprog --program nrf51822_xxaa_s110.hex
nrfjprog --memwr 0x10001000 --val 0x18000
nrfjprog -r
