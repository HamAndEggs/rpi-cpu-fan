#/bin/bash

echo "Building app, you have to have 'appbuild' installed to do this"
appbuild

echo "Copying app to /usr/bin will need root access"
sudo cp ./bin/release/cpu-fan /usr/bin/

echo "Copying service file"
sudo cp ./cpu-fan.service /lib/systemd/system/

echo "Setting mode"
sudo chmpd 664  /lib/systemd/system/cpu-fan.service

echo "Enabling at boot"
sudo systemctl enable cpu-fan

echo "service is called cpu-fan, will be loaded at next boot. Keep cool! :)"
