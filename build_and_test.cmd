if not exist gpujpeg (
	call build_gpujpeg.cmd
)
if not exist venv (
	python -m venv venv
	pip install -r requirements.txt
)
call venv\scripts\activate
pip uninstall pygpujpeg
python setup.py install
python test.py