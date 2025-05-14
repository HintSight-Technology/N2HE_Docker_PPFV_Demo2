#!/bin/bash 

set -e 

/bin/bash /backend_ppfv/run-eval.sh & #runs in the background

#wait
uvicorn server:app --host 0.0.0.0 --port 8000 --reload

