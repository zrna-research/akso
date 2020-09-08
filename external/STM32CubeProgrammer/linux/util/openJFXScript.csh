#!/usr/bin/env bash
MYROOTDIR=`dirname ${0}`

JAVA_PATH=`which java`
JAVA_PATH=`readlink -e $JAVA_PATH`
JAVA_PATH=`dirname $JAVA_PATH`

if [ -f $JAVA_PATH/../lib/javafx.properties ]
then
    echo "JavaFX Installed" 
else
    echo "JavaFX NOT installed => warn user" 
    java -jar ${MYROOTDIR}/OpenJFXWarning.jar
    exit 1
fi
