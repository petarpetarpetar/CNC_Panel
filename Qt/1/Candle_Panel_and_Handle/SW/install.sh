#!/bin/bash

#URL
#https://github.com/Denvi/Candle/issues/147#issuecomment-350221474

DIST=`lsb_release --id | sed 's/^Distributor ID:[\t ]*\(.*\)$/\1/'`
if [[ "$DIST" == "Ubuntu" ]]
then
	sudo apt -y install \
	g++ qt5-default qttools5-dev-tools \
    	libqt5serialport5-dev libqt5opengl5-dev
else 
	sudo dnf install -y \
	g++ qt5-default qttools5-dev-tools \
    	qt5-qtserialport-devel libqt5opengl5-dev 
    	sudo ln -s `which qmake-qt5` /usr/local/bin/qmake
fi

#git clone https://github.com/trasz/grblControl
git clone https://github.com/Denvi/Candle.git
pushd Candle
git checkout grbl_1_1

cd src
qmake

make -j4	
mkdir -p ~/.local/bin/
cp -v Candle ~/.local/bin/

popd
rm -rf Candle

