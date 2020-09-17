pushd "%~dp0"
pip install virtualenv
python -m venv ./venv
call ./venv/Scripts/activate
pip install -r requirements.txt