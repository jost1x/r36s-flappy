.DEFAULT_GOAL := help

.PHONY: local release r36s deploy run clean hooks pre-commit format format-fix help

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

hooks:
	$(MAKE) -f Makefile-cpp hooks

pre-commit:
	$(MAKE) -f Makefile-cpp pre-commit

format:
	$(MAKE) -f Makefile-cpp format

format-fix:
	$(MAKE) -f Makefile-cpp format-fix

clean:
	$(MAKE) -f Makefile-cpp clean

help:
	@echo "make local    - Compila para la arquitectura local"
	@echo "make release  - Compila la versión local optimizada"
	@echo "make r36s     - Compila para R36S ARM64"
	@echo "make deploy   - Compila y despliega en la R36S"
	@echo "make run      - Compila y ejecuta el binario local"
	@echo "make hooks    - Activa los hooks de Git"
	@echo "make pre-commit - Ejecuta las verificaciones del pre-commit"
	@echo "make format   - Comprueba el formato con clang-format"
	@echo "make format-fix - Corrige el formato con clang-format"
