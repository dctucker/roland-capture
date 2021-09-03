.PHONY: test

dist/roland-capture: assets/* app/*.py lib/*.py ui/*.py midi_control.py
	pyinstaller --name="roland-capture" --windowed midi_control.py
	# --onefile

test: test/*.py
	python3 -m unittest test/test_*.py

dist: dist/roland-capture

clean:
	rm -rf build dist roland-capture.spec
