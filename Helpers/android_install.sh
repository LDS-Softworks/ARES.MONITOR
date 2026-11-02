## We can't use shebangs here. because not all Android installations will always be the same.
declare LOGDIR
if [[ -d "./Logs" ]]; then
    LOGDIR="./Logs"
else
    mkdir ./Logs
    LOGDIR="./Logs"
fi
rm $LOGDIR/*android*
if [[ -z $PREFIX ]]; then
    echo "PREFIX is NULL, cannot proceed(risk READONLY/SU clash.)"
    exit 1
fi

INSTDIR="$PREFIX/bin"
DEST="$INSTDIR/ares"
printf "\n \
Destination:\n\
Installation Dir: $INSTDIR \n \
File: $DEST \n\
"
## Perform the installation...
{
    cp bin/ARES.MON.LINUX_x64 $DEST || {
        echo "An error ocurred when copying file to $DEST. [E:$?]"
        exit 1
        } 2>&1 | tee $LOGDIR/android-install.error.log
    } 1> | tee $LOGDIR/android-install.log

## Why does this say the LINUX X64 binary?... well, because that is sadly hardcoded into the binary output. i am an idiot.

if [[ -f "$DEST" ]]; then
    echo "Fixing up permissions"
    {
        chmod +x $DEST || {
            echo "An error ocurred when changing permission mode of $DEST. [E:$?]"
            return 1
        } 2>&1 | $LOGDIR/android-install.error.log
    } 1> | tee $LOGDIR/android-install.log

else
    echo "An error occurred when installing this software."
    exit 1
fi