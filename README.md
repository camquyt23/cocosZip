## Build

Only suport linux, import to eclipse cdt to build it, lazy to write Makefile

## Run

### Encrypted zip

`cocosZip --encrypt xxtea --xxtea-key b5730 --xxtea-sign-size 6  --in filename --dir outdir`

### UnEncrypted zip

`cocosZip --in filename --dir outdir`