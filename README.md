## Build

Only suport linux, import to eclipse cdt to build it, lazy to write Makefile

## Run

### Encrypted zip

`cocosZip --command decrypt-xxtea --xxtea-key abcxyz --xxtea-sign-size 6  --in filename --dir outdir`

### UnEncrypted zip

`cocosZip --in filename --dir outdir`