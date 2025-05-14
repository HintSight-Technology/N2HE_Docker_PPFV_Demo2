FROM python:3.11-slim 

WORKDIR /backend_database 
COPY . /backend_database 

RUN pip install fastapi uvicorn

EXPOSE 8001 

CMD ["uvicorn", "database:app", "--host", "0.0.0.0", "--port", "8001"]

