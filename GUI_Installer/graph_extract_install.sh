#!/bin/bash

if [ "$EUID" -ne 0 ]; then
    echo "You must have root privileges to install the libraries."
    echo "Please run the installer as sudo -E ./graph_extract_install.sh."
    exit 1
fi

if [ -z "$http_proxy" ]; then
    echo "Need http proxy to run the installer. Use -E to export the environment variables."
    echo "Please run the installer as: sudo -E ./graph_extract_install.sh."
    exit 1
fi

set -e

echo "--- Installing Dependencies"

add-apt-repository -y ppa:ethereum/ethereum-qt
apt-get update || echo "--- Continuing Installation"

source dependencies.sh

echo "--- Installing GraphExtract"

rm -rf `pwd`/../build/
mkdir `pwd`/../build
cd `pwd`/../build
qmake `pwd`/../GUI/GUI.pro
make
touch Graph_Extract_run.sh
echo "#!/bin/sh" >> Graph_Extract_run.sh
echo "export LD_LIBRARY_PATH=`pwd`/qpdflib"  >> Graph_Extract_run.sh
echo "exec `pwd`/graphextractor/graphextractor" >> Graph_Extract_run.sh
chmod +x Graph_Extract_run.sh

echo "--- Installation Complete"