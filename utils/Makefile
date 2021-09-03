.PHONY: test

dist/roland-capture: lib/*.py interface/*.py app/*.py midi_control.py assets/*
	pyinstaller --onefile --name="roland-capture" --windowed midi_control.py

test: test/*.py
	python3 -m unittest test/test_*.py

dist: dist/roland-capture

clean:
	rm -rf build dist roland-capture.spec
