# triangl-rssi-polling
OpenWRT Packet for polling clients and logging the RSSI to corresponding clients.


## Note:
### Don't rename folder rssi-polling!

### CI will only start when files are pushed to master

## TODO-Image:
- Add libcurl to image
    - install libcurl into openwrtsdk
    - add updated openwrtsdk to docker image
- Add real logging to the scripts

## TODO-Package:
- Rename helloworld to something more fitting ...
    - rename C-File
    - rename in src/Makefile
    - rename in Package Makefile
    - rename in circleci config
    - rename in package updater script
    - rename in circleci yaml config
    
