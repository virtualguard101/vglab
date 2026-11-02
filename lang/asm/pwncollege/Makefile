.PHONY: asm clean

asm:
	@if [ -z "$(src)" ]; then \
		echo "Usage: make asm src=<file.s>" >&2; \
		exit 2; \
	fi
	$(AS) "$(src)" -o "$(basename $(src)).o"
	$(LD) "$(basename $(src)).o" -o "$(basename $(src)).out"

clean:
	rm -f *.o *.out
