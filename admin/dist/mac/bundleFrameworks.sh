#!/bin/bash
# Author: Jono Cole <jono@last.fm>
#
# A tool for distributing Mac OSX bundles.
#
# Finds and copies dependant frameworks and local dylibs 
# not installed on stardard systems.
# Also ensures that all binaries have the correct paths 
# installed relative to the bundle's @executable_path

function getBundleBin {
    if echo $1|grep -q framework; then
        echo $1/`cat "$1/Contents/Info.plist" | sed -n '/CFBundleExecutable<\/key>/,/<\/string>/ s/.*<string>\(.*\)<.*/\1/p'|sed s/_debug//`
    else
        echo $1/Contents/MacOS/`cat "$1/Contents/Info.plist" | sed -n '/CFBundleExecutable<\/key>/,/<\/string>/ s/.*<string>\(.*\)<.*/\1/p'`
    fi
}

function fixFrameworks {
    echo -n F
    if [ -d "$1" ]; then
        local bin="`getBundleBin "$1"`"
    else
        local bin="$1"
    fi

    # echo Fixing Frameworks for $bin

    libs=`otool -L "$bin"|sed -n '/\/usr\/local.*/ s/^[^\/]*\([^(]*\) [^(]*([^)]*)/\1/p'`
    
    mkdir -p "$bundlePath/Contents/Frameworks"

    local lib
    for lib in $libs; do
        #ignore non-frameworks
        if echo $lib | grep -vq framework; then continue; fi

        #examples for /opt/qt/lib/QtXml.framework/Contents/QtXml

        #framework=/opt/qt/qt.git/lib/QtXml.framework
        framework=`echo $lib |sed -n 's/\(\.framework\).*/\1/p'`
        
        #frameworkLib=/Contents/QtXml
        frameworkLib=`echo $lib |sed -n 's/^.*\.framework//p'`

        #frameworkName=QtXml.framework
        frameworkName=`basename $framework`
        
        destFramework=$bundlePath/Contents/Frameworks/$frameworkName
        installFramework=@executable_path/../Frameworks/$frameworkName
        
        if [ "`basename $lib`" == "`basename $bin`" ]; then continue; fi

        if [ ! -e "$bundlePath/Contents/Frameworks/$frameworkName" ]; then 
            #cp -Rf -P /opt/qt/qt.git/lib/QtXml.framework (app name.app)/Contents/Frameworks
            cp -R -H -f $framework "$bundlePath/Contents/Frameworks"

            chmod -R u+w "$bundlePath/Contents/Frameworks"

            #install_name_tool -id /opt/qt/qt.git/lib/QtXml.framework/Contents/QtXml
            install_name_tool -id $installFramework$frameworkLib "$destFramework$frameworkLib"
        fi
        #install_name_tool -change /opt/qt/qt.git/lib/QtXml.framework/Contents/QtXml @executable_path/../Frameworks/QtXml.framework/Contents/QtXml (bin)
        install_name_tool -change $lib $installFramework$frameworkLib "$bin"
        fixLocalLibs "$destFramework"
        fixFrameworks "$destFramework"
    done
}

function fixLocalLibs {
    echo -n L
    if [ -d "$1" ]; then
        local bin=`getBundleBin "$1"`
    else
        local bin="$1"
    fi
    echo Fixing Local Lib for $bin

    local libs=`otool -L "$bin" | sed -n '/^[^\/]*$/ s/^[[:space:]]*\(.*\) (com.*/\1/p'`
    local extralibs=`otool -L "$bin" | sed -n '/\/usr\/local.*/ s/^[^\/]*\([^(]*\) [^(]*([^)]*)/\1/p'|grep -v framework`
    local moreExtralibs=`otool -L "$bin" | sed -n '/\/usr\/X11.*/ s/^[^\/]*\([^(]*\) [^(]*([^)]*)/\1/p'|grep -v framework`
    local libs="$libs $extralibs $moreExtralibs"
    local lib
    local cpPath
    for lib in $libs; do
        local libPath=$lib
        if [ ! -e $lib ]; then
            cpPath=`locateLib $lib`
        else
            cpPath=$lib
        fi

        resolvedLib=`/usr/local/bin/greadlink -f $cpPath`
        basenameLib=`basename $resolvedLib`

		echo $cpPath "$bundlePath/Contents/MacOS"

        cp -RLf $resolvedLib "$bundlePath/Contents/MacOS/"

        chmod -R u+w "$bundlePath/Contents/MacOS"

        install_name_tool -id @executable_path/../MacOS/$basenameLib "$bundlePath/Contents/MacOS/$basenameLib"
        install_name_tool -change $libPath @executable_path/../MacOS/$basenameLib "$bin"
        
        fixFrameworks "$bundlePath/Contents/MacOS/$basenameLib"
        fixLocalLibs "$bundlePath/Contents/MacOS/$basenameLib"
    done
}

function locateLib {
    for p in {$rootdir,/usr/local/lib}; do
        if [ -e $p/$1 ]; then
            echo $p/$1
            return 0
        fi
    done
    return 1
}

if [ -d '$1' ]; then
    bundlePath="$1"
else
    bundlePath=$(echo $1 | sed -E "s|^(.*)\.app.*$|\1\.app|g")
fi

rootdir=`dirname "$bundlePath"`
binPath=$bundlePath/Contents/MacOS

echo =========== Fix Local Libs ==============
if [ -d '$1' ]; then
    fixLocalLibs "$bundlePath"
else
    fixLocalLibs "$1"
fi

echo

echo =========== Fix Frameworks ==============
if [ -d '$1' ]; then
    fixFrameworks "$bundlePath"
else
    fixFrameworks "$1"
fi

echo

echo ======= Copying 3rd party frameworks ===========

cp -R -H -f /Library/Frameworks/Growl.framework "$bundlePath/Contents/Frameworks"
cp -R -H -f /Library/Frameworks/Sparkle.framework "$bundlePath/Contents/Frameworks"
#cp -R -L -f /Library/Frameworks/Breakpad.framework "$bundlePath/Contents/Frameworks"

echo

echo ======= Copying Qt plugins ===========
mkdir -p "$bundlePath/Contents/plugins"

plugins="imageformats phonon_backend sqldrivers bearer"

for plugin in $plugins; do
    if [ -d /Developer/Applications/Qt/plugins/ ]; then
        pluginDir=/Developer/Applications/Qt/plugins
    else
        pluginDir=`qmake --version |sed -n 's/^.*in \(\/.*$\)/\1/p'`/../plugins
    fi
    cp -R -H -f $pluginDir/$plugin "$bundlePath/Contents/plugins"
    for i in "$bundlePath"/Contents/plugins/$plugin/*; do
        fixFrameworks "$i"
        fixLocalLibs "$i"
        echo -n P
    done
    echo
done

echo ======= Copying vlc plugins ===========

mkdir -p "$bundlePath/Contents/plugins"

vlcPlugins='libaccess_http_plugin.dylib
            liba52tofloat32_plugin.dylib
            liba52tospdif_plugin.dylib
            libaudio_format_plugin.dylib
            libconverter_fixed_plugin.dylib
            libdolby_surround_decoder_plugin.dylib
            libdtstofloat32_plugin.dylib
            libdtstospdif_plugin.dylib
            libmpgatofixed32_plugin.dylib
            libscaletempo_plugin.dylib
            libsimple_channel_mixer_plugin.dylib
            libspeex_resampler_plugin.dylib
            libtrivial_channel_mixer_plugin.dylib
            libauhal_plugin.dylib
            libugly_resampler_plugin.dylib
            libfloat32_mixer_plugin.dylib
            libmpeg_audio_plugin.dylib
            libes_plugin.dylib
            liblogger_plugin.dylib'


for plugin in $vlcPlugins; do
    pluginDir="/usr/local/lib/vlc/plugins"

    mkdir -p "$bundlePath/Contents/plugins"
    cp -R -H -f $pluginDir/$plugin "$bundlePath/Contents/plugins"
    chmod -R u+w "$bundlePath/Contents/plugins"
    
    fixFrameworks "$bundlePath/Contents/plugins/$plugin"
    fixLocalLibs "$bundlePath/Contents/plugins/$plugin"
    echo
done

echo ======= Copying Qt translations ===========
mkdir -p "$bundlePath/Contents/Resources/qm"

translations="qt_de.qm
                qt_es.qm 
                qt_fr.qm 
                qt_it.qm 
                qt_ja.qm 
                qt_pl.qm 
                qt_pt.qm 
                qt_ru.qm 
                qt_sv.qm 
                qt_tr.qm
                qt_zh_CN.qm"

for translation in $translations; do
    if [ -d /Developer/Applications/Qt/plugins/ ]; then
        translationDir=/Developer/Applications/Qt/translations
    else
        translationDir=`qmake --version |sed -n 's/^.*in \(\/.*$\)/\1/p'`/../translations
    fi

    cp -f $translationDir/$translation "$bundlePath/Contents/Resources/qm"
    echo
done


echo ======= creating qt.conf ===========
qtconf=$bundlePath/Contents/Resources/qt.conf
echo [Paths] > "$qtconf"
echo Plugins = ../plugins >> "$qtconf"


echo ======= signing bundle ===========
codesign -f -s "Developer ID Application: Last.fm" -i fm.last.Scrobbler "$bundlePath"
