.DEFAULT_GOAL := help

.PHONY: local release r36s deploy run clean help

local:
	$(MAKE) -f Makefile-cpp build

release:
	$(MAKE) -f Makefile-cpp release

r36s:
	$(MAKE) -f Makefile-cpp arm64

deploy:
	$(MAKE) -f Makefile-cpp deploy

run:
	$(MAKE) -f Makefile-cpp run

clean:
	$(MAKE) -f Makefile-cpp clean

help:
	@echo "make local    - Compila para la arquitectura local"
	@echo "make release  - Compila la versión local optimizada"
	@echo "make r36s     - Compila para R36S ARM64"
	@echo "make deploy   - Compila y despliega en la R36S"
	@echo "make run      - Compila y ejecuta el binario local"
