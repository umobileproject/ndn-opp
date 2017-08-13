NDN Forwarding Daemon on Android
================================

## Environment setup

The code has been tested against version 6.0 of the SDK (API 23) and the underlying compiled
using the Crystax NDK, version 10.3.1 (https://www.crystax.net/en/download). After downloading and
installing the NDK, it is necessary to include it in the PATH variable. Assuming that ${CRYSTAX}
points to the root directory of Crystax and ${ANDROID} points to the root directory of the Android SDK
directory, issue the following commands (or store them in your .bash_profile):

    export ANDROID_HOME=${ANDROID}
    export PATH=${PATH}:${ANDROID}/tools:${ANDROID}/platform-tools:${CRYSTAX}

You can use precompiled version of openssl 1.0.3h (currently, available for CrystaX NDK 10.3.1 only):

    cd crystax-ndk-10.3.1/sources
    curl -L -o openssl.tar.gz https://github.com/named-data-mobile/crystax-prebuilt-openssl/archive/crystax-10.3.1.tar.gz
    tar zx --strip-components 1 -C openssl -f openssl.tar.gz
    rm openssl.tar.gz
    
Furthermore, the project requires the android-support-v7-appcompat library to be available. Android Studio should take care
of that automatically for you if you import the project there. Unfortunately for Eclipse users, this might be more complicated
to achieve. This is basically me nicely suggesting you to use Android Studio :)

Furthermore, the local.properties file must be created and reference the NDK (Crystax) and SDK locations respectively.

When cloning the project, Android Studio does not perform recursive initialization and update of the configured submodules.
Assuming thet ROOT points to the root of the project, the following commands must be issued manually;

$ cd ${ROOT}/app/src/main/jni
$ git submodule update --init --recursive

Currently, the project is configured to only build the native library for the armeabi-v7a architecture.
This can be changed by modifying the APP_ABI variable located in app/src/main/jni/Application.mk

TODO: Change the version string upon compilation to include the git commit short hash.
