.ONESHELL:
all:
	$(MAKE) -C src/kprx
	-rm -rf src/data/files.zip
	-pushd src/data/
	-zip -r files.zip *
	-popd
	pwd
	$(MAKE) -C src/
	@mkdir -p "PSP/GAME/PSP Tool/" | true
	@cp src/EBOOT.PBP "PSP/GAME/PSP Tool/"
	@printf "\n\nDone. Copy PSP dir to PSP Memory Stick\n\n"

clean:
	$(MAKE) -C src/kprx clean
	$(MAKE) -C src clean
	@rm -rf PSP
	@printf '\n\n** Cleaned **\n\n'

release: clean all
	zip -r psptool.zip PSP/
