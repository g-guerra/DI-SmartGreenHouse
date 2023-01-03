FILES = opclient.py
RPI_ADDRESS = 192.168.5.228
RPI_TARGET_DIR = /home/pi/

.phony: upload

upload: opclient.py
	scp $^ pi@${RPI_ADDRESS}:${RPI_TARGET_DIR}
