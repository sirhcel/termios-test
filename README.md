# Tests for hunting and demonstrating a flow control issue with termios on macOS

See https://github.com/Fazecast/jSerialComm/issues/453 for details.


## Testing workaround for jSerialComm

Build jSerialComm with hotifx and put snapshot artifact into local Maven repository:
```
$ git clone https://github.com/sirhcel/jSerialComm.git
jserialComm $ git switch -c test/flowcontrol-on-macos-snapshot-release origin/test/flowcontrol-on-macos-snapshot-release
jSerialComm $ ./gradlew jar
jSerialComm $ mvn install:install-file -Dfile=build/libs/jSerialComm-2.10.0-SNAPSHOT.jar -DgroupId=com.fazecast -DartifactId=jSerialComm -Dversion=2.10.0-SNAPSHOT -Dpackaging=jar
```

Build and run this test application
```
termios-test/java $ ./gradlew run --args=/dev/tty.usbserial-YOUR_DEVICE_HERE
```
